/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <QvisCommandWindow.h>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QFile>
#include <QFont>
#include <QInputDialog>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QTabWidget>
#include <QTextEdit>
#include <QTextStream>
#include <QToolTip>
#include <QWidget>

#include <InstallationFunctions.h>
#include <DataNode.h>
#include <DebugStream.h>
#include <QvisPythonSyntaxHighlighter.h>

#include <icons/macrorecord.xpm>
#include <icons/macropause.xpm>
#include <icons/macrostop.xpm>
#include <icons/macroexec.xpm>

#define MAXTABS 8

// ****************************************************************************
// Method: QvisCommandWindow::QvisCommandWindow
//
// Purpose: 
//   QvisCommandWindow constructor.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 9 10:50:53 PDT 2005
//
// Modifications:
//   Brad Whitlock, Fri Jan 6 13:36:05 PST 2006
//   Inititalized new members.
//
//   Brad Whitlock, Fri Jun 15 13:36:56 PST 2007
//   Added new buttons.
//
//   Brad Whitlock, Wed Apr  9 11:35:58 PDT 2008
//   QString for captionString, shortName.
//
//   Cyrus Harrison, Mon Feb  8 15:40:54 PST 2010
//   Added python syntax highlighter.
//
// ****************************************************************************

QvisCommandWindow::QvisCommandWindow(const QString &captionString,
    const QString &shortName, QvisNotepadArea *n) : 
    QvisPostableWindow(captionString, shortName, n)
{
    executeButtonsGroup = 0;
    executeButtons = 0;
    clearButtons = 0;
    clearButtonsGroup = 0;
    addMacroButtons = 0;
    addMacroButtonsGroup = 0;
    editors = 0;
    highlighters = 0;

    macroRecord = 0;
    macroPause = 0;
    macroEnd = 0;
    macroAppendCheckBox = 0;
    macroStorageComboBox = 0;

    macroStorageMode = 0;
    macroAppend = false;
}

// ****************************************************************************
// Method: QvisCommandWindow::~QvisCommandWindow
//
// Purpose: 
//   QvisCommandWindow destructor.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 9 10:51:22 PDT 2005
//
// Modifications:
//   Brad Whitlock, Fri Jun 15 13:37:31 PST 2007
//   Delete new widgets.
//
//   Cyrus Harrison, Wed Aug 27 08:31:25 PDT 2008
//   Ensured all of button groups have parents, so we don't need to 
//   explicitly delete them.
//
//   Cyrus Harrison, Mon Feb  8 15:40:54 PST 2010
//   Added python syntax highlighter.
//
// ****************************************************************************

QvisCommandWindow::~QvisCommandWindow()
{
    delete [] executeButtons;
    delete [] clearButtons;
    delete [] addMacroButtons;
    delete [] editors;
    delete [] highlighters;
}

// ****************************************************************************
// Method: QvisCommandWindow::CreateWindowContents
//
// Purpose: 
//   This method creates the window contents.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 9 10:51:42 PDT 2005
//
// Modifications:
//   Brad Whitlock, Fri Jan 6 13:36:19 PST 2006
//   Added new buttons to record macros.
//
//   Brad Whitlock, Wed Jan 11 09:53:23 PDT 2006
//   I fixed an errant tooltip.
//
//   Brad Whitlock, Fri Mar 17 09:46:58 PDT 2006
//   Added UpdateMacroCheckBoxes.
//
//   Brad Whitlock, Fri Jun 15 13:37:48 PST 2007
//   Added Macros tab and buttons to convert "code" to "macros".
//
//   Cyrus Harrison, Tue Jun 10 15:00:05 PDT 2008
//   Initial Qt4 Port.
//
//   Hank Childs, Tue Jul 22 12:22:38 PDT 2008
//   Always have the Execute button enabled.
//
//   Cyrus Harrison, Mon Feb  8 15:40:54 PST 2010
//   Added python syntax highlighter.
//
// ****************************************************************************

void
QvisCommandWindow::CreateWindowContents()
{
    QGroupBox *macroBox = new QGroupBox(central);
    macroBox->setTitle(tr("Commands"));
    topLayout->addWidget(macroBox);

    QVBoxLayout *innerMacroLayout = new QVBoxLayout(macroBox);
    innerMacroLayout->setMargin(10);
    innerMacroLayout->addSpacing(15);
    QHBoxLayout *macroLayout = new QHBoxLayout();
    innerMacroLayout->addLayout(macroLayout);
    macroLayout->setSpacing(5);

    macroRecord = new QPushButton(QIcon(QPixmap(macrorecord_xpm)),
                                  tr("Record"), macroBox);
    connect(macroRecord, SIGNAL(clicked()), this, SLOT(macroRecordClicked()));

    macroRecord->setToolTip(tr("Start recording commands"));
    macroLayout->addWidget(macroRecord);

    macroPause = new QPushButton(QIcon(QPixmap(macropause_xpm)),
                                 tr("Pause"), macroBox);
    macroPause->setCheckable(true);
    connect(macroPause, SIGNAL(clicked()), this, SLOT(macroPauseClicked()));
    macroPause->setToolTip(tr("Pause recording commands"));
    macroLayout->addWidget(macroPause);

    macroEnd = new QPushButton(QIcon(QPixmap(macrostop_xpm)),
                               tr("Stop"), macroBox);
    connect(macroEnd, SIGNAL(clicked()), this, SLOT(macroEndClicked()));
    macroEnd->setToolTip(tr("Stop recording commands"));
    macroLayout->addWidget(macroEnd);
    macroRecord->setEnabled(true);
    macroPause->setEnabled(false);
    macroEnd->setEnabled(false);

    // Create macro append and storage controls.
    innerMacroLayout->addSpacing(5);
    QGridLayout *mLayout = new QGridLayout();
    innerMacroLayout->addLayout(mLayout);
    mLayout->setSpacing(5);
    mLayout->setColumnMinimumWidth(1, 10);

    macroStorageComboBox = new QComboBox(macroBox);
    macroStorageComboBox->addItem(tr("Active tab"));
    macroStorageComboBox->addItem(tr("First empty tab"));
    macroStorageComboBox->addItem(tr("Macros"));
    connect(macroStorageComboBox, SIGNAL(activated(int)),
            this, SLOT(macroStorageActivated(int)));
    mLayout->addWidget(macroStorageComboBox, 0, 1);
    mLayout->addWidget(new QLabel(tr("Store commands in"),macroBox), 0, 0);

    macroAppendCheckBox = new QCheckBox(tr("Append commands to existing text"), 
                                        macroBox);
    connect(macroAppendCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(macroAppendClicked(bool)));
    mLayout->addWidget(macroAppendCheckBox, 1, 0, 1, 2);

    tabWidget = new QTabWidget(central);
    tabWidget->setMinimumHeight(200);
    topLayout->addWidget(tabWidget, 1000);

    executeButtonsGroup = new QButtonGroup(central);
    connect(executeButtonsGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(executeClicked(int)));

    clearButtonsGroup = new QButtonGroup(central);
    connect(clearButtonsGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(clearClicked(int)));

    addMacroButtonsGroup = new QButtonGroup(central);
    connect(addMacroButtonsGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(macroCreate(int)));

    // Create the tabs that let us edit command scripts.
    editors       = new QTextEdit*[MAXTABS];
    highlighters  = new QSyntaxHighlighter*[MAXTABS];
    executeButtons  = new QPushButton*[MAXTABS];
    clearButtons    = new QPushButton*[MAXTABS];
    addMacroButtons = new QPushButton*[MAXTABS];

    QFont monospaced("Courier");
    for (int i = 0; i < MAXTABS; i++)
    {
        QWidget *widget = new QWidget(central);
        QVBoxLayout *vlayout = new QVBoxLayout(widget);
        vlayout->setMargin(10);
        vlayout->setSpacing(5);
        editors[i]  = new QTextEdit(widget);
        editors[i]->setReadOnly(false);
        editors[i]->setFont(monospaced);
        editors[i]->setWordWrapMode(QTextOption::NoWrap);

        // hook up a python syntax highlighter
        highlighters[i] = new QvisPythonSyntaxHighlighter(editors[i]->document());


        QString slotName(SLOT(textChanged#()));
        QString n = QString("%1").arg(i);
        slotName.replace(slotName.indexOf("#"), 1, n);
        vlayout->addWidget(editors[i]);
        connect(editors[i], SIGNAL(textChanged()),
                this, slotName.toStdString().c_str());

        QHBoxLayout *hlayout = new QHBoxLayout();
        vlayout->addLayout(hlayout);
        hlayout->setSpacing(5);
        executeButtons[i] = new QPushButton(QIcon(QPixmap(macroexec_xpm)),
                                            tr("Execute"), widget);
        hlayout->addWidget(executeButtons[i]);
        executeButtonsGroup->addButton(executeButtons[i], i);

        clearButtons[i] = new QPushButton(tr("Clear"), widget);
        clearButtons[i]->setEnabled(false);
        hlayout->addWidget(clearButtons[i]);
        clearButtonsGroup->addButton(clearButtons[i], i);

        addMacroButtons[i] = new QPushButton(tr("Make macro"), widget);
        addMacroButtons[i]->setEnabled(false);
        hlayout->addWidget(addMacroButtons[i]);
        addMacroButtonsGroup->addButton(addMacroButtons[i], i);

        // Add the top vbox as a new tab.
        n.sprintf("%d", i+1);
        tabWidget->addTab(widget, n);
    }

    // Create the Macros tab.
    macroTab = new QWidget(central);
    QVBoxLayout *macro_tab_vlayout = new QVBoxLayout(macroTab);
    macro_tab_vlayout->setMargin(10);
    macro_tab_vlayout->setSpacing(5);
    macroEdit = new QTextEdit(macroTab);
    macroEdit->setWordWrapMode(QTextOption::NoWrap);
    macroEdit->setReadOnly(false);
    macroEdit->setFont(monospaced);
    macroHighlighter = new QvisPythonSyntaxHighlighter(macroEdit->document());

    macro_tab_vlayout->addWidget(macroEdit);
    QHBoxLayout *macro_tab_hlayout = new QHBoxLayout();
    macro_tab_vlayout->addLayout(macro_tab_hlayout);
    macro_tab_hlayout->setSpacing(5);
    macroUpdateButton = new QPushButton(tr("Update macros"),macroTab);
    macro_tab_hlayout->addWidget(macroUpdateButton);
    connect(macroUpdateButton, SIGNAL(clicked()),
            this, SLOT(macroUpdateClicked()));
    macroClearButton = new QPushButton(tr("Clear"), macroTab);
    macro_tab_hlayout->addWidget(macroClearButton );
    connect(macroClearButton, SIGNAL(clicked()),
            this, SLOT(macroClearClicked()));
    tabWidget->addTab(macroTab, tr("Macros"));

    // Load the saved Python scripts.
    LoadScripts();

    // Update the window's check boxes.
    UpdateMacroCheckBoxes();
}

// ****************************************************************************
// Method: QvisCommandWindow::CreateNode
//
// Purpose: 
//   This method is called when we're saving settings. We save the scripts
//   if the window has been created.
//
// Arguments:
//   node : The data node.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 22 16:37:28 PST 2005
//
// Modifications:
//   Brad Whitlock, Fri Jan 6 16:58:03 PST 2006
//   Added new macro-related things.
//
//   Cyrus Harrison, Tue Jun 10 15:00:05 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisCommandWindow::CreateNode(DataNode *node)
{
    QvisPostableWindow::CreateNode(node);

    if(isCreated)
    {
        SaveScripts();

        DataNode *winNode = node->GetNode(windowTitle().toStdString());
        if(winNode != 0)
        {
            winNode->AddNode(new DataNode("macroStorageMode", macroStorageMode));
            winNode->AddNode(new DataNode("macroAppend", macroAppend));
        }
    }
}

// ****************************************************************************
// Method: QvisCommandWindow::SetFromNode
//
// Purpose: 
//   Reads the macro storage mode and append mode from the log.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 6 17:01:57 PST 2006
//
// Modifications:
//   Brad Whitlock, Fri Mar 17 09:43:25 PDT 2006
//   Moved code to update widgets into UpdateMacroCheckBoxes.
//
//   Brad Whitlock, Fri Jun 15 13:02:38 PST 2007
//   Added 1 to macroStorageMode.
//
//   Cyrus Harrison, Tue Jun 10 15:00:05 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisCommandWindow::SetFromNode(DataNode *parentNode, const int *borders)
{
    QvisPostableWindow::SetFromNode(parentNode, borders);

    DataNode *winNode = parentNode->GetNode(windowTitle().toStdString());
    if(winNode == 0)
        return;

    DataNode *n = 0;
    if((n = winNode->GetNode("macroStorageMode")) != 0)
    {
        macroStorageMode = n->AsInt();
        if(macroStorageMode > 2)
            macroStorageMode = 2;
    }
    if((n = winNode->GetNode("macroAppend")) != 0)
        macroAppend = n->AsBool();

    UpdateMacroCheckBoxes();
}

// ****************************************************************************
// Method: QvisCommandWindow::UpdateMacroCheckBoxes
//
// Purpose: 
//   Updates the macro check boxes.
//
// Note:       If anymore code is added to this method, you must check to
//             make sure the widget you're setting is non-NULL since this
//             method can be called before the window's widgets are created.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 17 09:43:39 PDT 2006
//
// Modifications:
//   Cyrus Harrison, Tue Jun 10 15:00:05 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisCommandWindow::UpdateMacroCheckBoxes()
{
    if(macroStorageComboBox != 0)
    {
        macroStorageComboBox->blockSignals(true);
        macroStorageComboBox->setCurrentIndex(macroStorageMode);
        macroStorageComboBox->blockSignals(false);
    }

    if(macroAppendCheckBox != 0)
    {
        macroAppendCheckBox->blockSignals(true);
        macroAppendCheckBox->setChecked(macroAppend);
        macroAppendCheckBox->blockSignals(false);
    }
}

// ****************************************************************************
// Method: QvisCommandWindow::fileName
//
// Purpose: 
//   Creates a filename for a script.
//
// Arguments:
//   index : A number to include in the script name.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 22 16:38:05 PST 2005
//
// Modifications:
//   
// ****************************************************************************

QString
QvisCommandWindow::fileName(int index) const
{
    QString n;
    n.sprintf("%sscript%d.py", GetUserVisItDirectory().c_str(), index);
    return n;
}

// ****************************************************************************
// Method: QvisCommandWindow::RCFileName
//
// Purpose: 
//   Returns the name of the "visitrc" file.
//
// Returns:    The name of the visitrc file.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 15 14:08:47 PST 2007
//
// Modifications:
//   
// ****************************************************************************

QString
QvisCommandWindow::RCFileName() const
{
    return QString(GetUserVisItRCFile().c_str());
}

// ****************************************************************************
// Method: QvisCommandWindow::LoadScripts
//
// Purpose: 
//   Loads the scripts into the window.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 22 16:38:30 PST 2005
//
// Modifications:
//   Brad Whitlock, Fri Jun 15 14:13:17 PST 2007
//   Added code to load the visitrc file.
//
//   Cyrus Harrison, Tue Jun 10 15:00:05 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisCommandWindow::LoadScripts()
{
    // Load scripts into the tabs.
    for(int i = 0; i < MAXTABS; ++i)
    {
        QFile file(fileName(i+1));
        if(file.open(QIODevice::ReadOnly))
        {
            QTextStream stream(&file);

            QString lines;
            while(!stream.atEnd())
            {
                lines += stream.readLine();
                lines += "\n";
            }
            file.close();

            editors[i]->setText(lines);
        }
    }

    // Try and load the visitrc file.
    QFile file(RCFileName());
    if(file.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&file);
        QString lines;
        while(!stream.atEnd())
        {
            lines += stream.readLine();
            lines += "\n";
        }
        file.close();

        macroEdit->setText(lines);
    }
}

// ****************************************************************************
// Method: QvisCommandWindow::SaveScripts
//
// Purpose: 
//   Saves the window's scripts.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 22 16:38:48 PST 2005
//
// Modifications:
//   Cyrus Harrison, Tue Jun 10 15:00:05 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisCommandWindow::SaveScripts()
{
    for(int i = 0; i < MAXTABS; ++i)
    {
        QFile file(fileName(i+1));
        QString txt(editors[i]->toPlainText());
        if(txt.length() > 0)
        {
            if(file.open(QIODevice::WriteOnly))
            {
                QTextStream stream(&file);
                stream << txt;
                file.close();
            }
        }
        else
            file.remove();
    }
}

// ****************************************************************************
// Method: CreateMacroFromText
//
// Purpose: 
//   Converts the supplied text to a macro, prompting the user for names, and
//   adds the new macro text to the macro tab.
//
// Arguments:
//   s : The macro string that we're turning into a function.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 15 16:08:59 PST 2007
//
// Modifications:
//   Hank Childs, Wed May  7 11:47:25 PDT 2008
//   Add checking for bad function names.
//
//   Cyrus Harrison, Tue Jun 10 15:00:05 PDT 2008
//   Initial Qt4 Port.
//
//   Hank Childs, Wed May  7 11:47:25 PDT 2008
//   Add checking for bad function names.
//
// ****************************************************************************

void
QvisCommandWindow::CreateMacroFromText(const QString &s)
{
    QString func, funcName, macroName;
    bool ok = true;

    // Get the name of the function to create from the user.
    funcName = QInputDialog::getText(this,"VisIt",
        tr("Please enter the name of the Python function to be defined for the macro."),
        QLineEdit::Normal, QString::null, &ok);
    if(!ok || funcName.isEmpty())
        return;
    std::string str = funcName.toStdString();
    if(str.size() < 1 || !isalpha(str[0]))
    {
        Error(tr("Function names must start with a letter. Please try to create "
              "the macro again with a function name that starts with a letter."));
        return;
    }
    for (int i = 0 ; i < str.size() ; i++)
    {
        if (!isalpha(str[i]) && !isdigit(str[i]) && str[i] != '_')
        {
            Error(tr("Function names may only contain letters, numbers, and underscores."
                     "  Please try again with only those characters."));
            return;
        }
    }

    // Get the name of the macro from the user.
    macroName = QInputDialog::getText(this,"VisIt",
        tr("Please enter the name of the macro to be defined (as you want it to appear in a button)."),
        QLineEdit::Normal, QString::null, &ok);
    if(!ok || macroName.isEmpty())
        return;

#if 1
    // This function is assuming Python code.

    // Now, iterate over the lines of text and indent appropriately to
    // make a Python function.
    func = QString("def ") + funcName + QString("():\n");
    QStringList lines = s.split("\n");
    for(int i = 0; i < lines.count(); ++i)
        func += QString("    ") + lines[i] + QString("\n");
    func += QString("RegisterMacro(\"") + macroName + QString("\", ") +
            funcName + QString(")\n");

    // Add the function definition to the Macros tab.
    macroEdit->setPlainText(macroEdit->toPlainText() + func);

    // Make the Macros tab active.
    tabWidget->setCurrentWidget((QWidget *)macroTab);
#endif
}

//
// Qt slots
//

// ****************************************************************************
// Method: QvisCommandWindow::executeClicked
//
// Purpose: 
//   This is a Qt slot function that gets called when we click on one of the
//   execute buttons.
//
// Arguments:
//   index : The index of the execute button that was clicked.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 9 10:52:02 PDT 2005
//
// Modifications:
//   Cyrus Harrison, Tue Jun 10 15:00:05 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisCommandWindow::executeClicked(int index)
{
    emit runCommand(editors[index]->toPlainText());
}

// ****************************************************************************
// Method: QvisCommandWindow::clearClicked
//
// Purpose: 
//   This is a Qt slot function that clears the text for a line edit when
//   its clear button gets clicked.
//
// Arguments:
//   index : The index of the clear button that was clicked.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 22 16:44:58 PST 2005
//
// Modifications:
//
// ****************************************************************************

void
QvisCommandWindow::clearClicked(int index)
{
    editors[index]->clear();
}

// ****************************************************************************
// Method: QvisCommandWindow::textChanged
//
// Purpose: 
//   This is a Qt slot function that makes sure that the execute buttons are
//   only enabled when the text edit is not empty.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 9 10:52:56 PDT 2005
//
// Modifications:
//   Brad Whitlock, Fri Jun 15 14:21:22 PST 2007
//   Set addMacroButton enabled.
//
//   Cyrus Harrison, Tue Jun 10 15:00:05 PDT 2008
//   Initial Qt4 Port.
//
//   Hank Childs, Tue Jul 22 12:22:38 PDT 2008
//   Always have the Execute button enabled.
//
// ****************************************************************************

#define TEXT_CHANGED(I) void QvisCommandWindow::textChanged##I() { \
    bool e = !editors[I]->toPlainText().isEmpty(); \
    addMacroButtons[I]->setEnabled(e); \
    clearButtons[I]->setEnabled(e); \
}

TEXT_CHANGED(0)
TEXT_CHANGED(1)
TEXT_CHANGED(2)
TEXT_CHANGED(3)
TEXT_CHANGED(4)
TEXT_CHANGED(5)
TEXT_CHANGED(6)
TEXT_CHANGED(7)

// ****************************************************************************
// Method: void QvisCommandWindow::macroRecordClicked
//
// Purpose: 
//   This is a Qt slot function that is called when we click on the button
//   to start recording a macro.
//
// Note:       This method tells the client to start recording a macro using
//             an interpreted ClientMethod() string so it ensures that the
//             interpreter (who is responsible for recording macros) gets
//             launched if it is not already launched.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 6 15:14:45 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisCommandWindow::macroRecordClicked()
{
    emit runCommand("ClientMethod(\"MacroStart\")");

    macroRecord->setEnabled(false);
    macroPause->setEnabled(true);
    macroEnd->setEnabled(true);
}

// ****************************************************************************
// Method: QvisCommandWindow::macroPauseClicked
//
// Purpose: 
//   This is a Qt slot function that pauses macro recording.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 6 15:15:42 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisCommandWindow::macroPauseClicked()
{
    emit runCommand("ClientMethod(\"MacroPause\")");
}

// ****************************************************************************
// Method: QvisCommandWindow::macroEndClicked
//
// Purpose: 
//   This is a Qt slot function that ends macro recording.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 6 15:17:49 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisCommandWindow::macroEndClicked()
{
    emit runCommand("ClientMethod(\"MacroEnd\")");
    macroRecord->setEnabled(true);
    macroPause->setEnabled(false);
    macroEnd->setEnabled(false);
}

void
QvisCommandWindow::macroAppendClicked(bool val)
{
    macroAppend = val;
}

void
QvisCommandWindow::macroStorageActivated(int val)
{
    macroStorageMode = val;
}

// ****************************************************************************
// Method: QvisCommandWindow::acceptRecordedMacro
//
// Purpose: 
//   This is a Qt slot function that pastes a recorded macro into the active
//   tab's line edit.
//
// Arguments:
//   s : The macro string.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 6 15:18:21 PST 2006
//
// Modifications:
//   Brad Whitlock, Fri Jun 15 16:10:33 PST 2007
//   Added ability to record to the Macros tab.
//
//   Cyrus Harrison, Tue Jun 10 15:00:05 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisCommandWindow::acceptRecordedMacro(const QString &s)
{
    if(macroStorageMode == 2)
    {
        // Add the recorded macro to the macros tab.
        CreateMacroFromText(s);
    }
    else
    {
        int index = 0;
        if(macroStorageMode == 0)
        {
            index = tabWidget->currentIndex();

            // If the active tab is the macros tab, record there.
            if(index == MAXTABS)
            {
                CreateMacroFromText(s);
                return;
            }
        }
        else
        {
            // Look for the first empty tab.
            bool found = false;
            for(int i = 0; i < MAXTABS; ++i)
            {
                if(editors[i]->toPlainText().isEmpty())
                {
                    index = i;
                    found = true;
                    break;
                }
            }

            index = found ? index : 0;
            tabWidget->blockSignals(true);
            tabWidget->setCurrentIndex(index);
            tabWidget->blockSignals(false);
        }

        if(macroAppend)
        {
            QString macro(editors[index]->toPlainText());
            macro += s;
            editors[index]->setText(macro);
        }
        else
            editors[index]->setText(s);
    }
}

// ****************************************************************************
// Method: QvisCommandWindow::macroClearClicked
//
// Purpose: 
//   Clear the macros tab.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 15 14:14:53 PST 2007
//
// Modifications:
//   
//    Hank Childs, Wed May  7 11:44:01 PDT 2008
//    Add a warning so users know that simply clicking clear will not suffice
//    in clearing out their macros.
//
// ****************************************************************************

void
QvisCommandWindow::macroClearClicked()
{
    Warning("The Clear button will only clear this text area.  If you now want"
            " to permanently remove the macros (and prevent the CLI from "
            "launching at startup), you need to also click \"Update Macros\"");
    macroEdit->clear();
}

// ****************************************************************************
// Method: QvisCommandWindow::macroUpdateClicked
//
// Purpose: 
//   Update the macros by executing the visitrc again.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 15 14:15:32 PST 2007
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
// 
//   Cyrus Harrison, Tue Jun 10 15:00:05 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisCommandWindow::macroUpdateClicked()
{
    // Save the updated visitrc file based on the contents in the Macros tab.
    QFile file(RCFileName());
    QString txt(macroEdit->toPlainText());
    if(txt.length() > 0)
    {
        if(file.open(QIODevice::WriteOnly))
        {
            QTextStream stream(&file);
            stream << txt;
            file.close();
            debug1 << "Saved updated " << RCFileName().toStdString()
                   << " file." << endl;

           // Tell the CLI to source the file so we get our macros back with
           // the changes that have been put into place.
           QString command("ClearMacros()\nSource(\"%1\")\n");
           command = command.arg(RCFileName());
           emit runCommand(command);
        }
        else
        {
            QString msg;
            msg = tr("VisIt could not update the file: ") + RCFileName();
            Message(msg);
        }
    }
    else
    {
        debug1 << "Removing empty " << RCFileName().toStdString()
               << " file. " << endl;
        file.remove();
        emit runCommand("ClearMacros()");
    }
}

// ****************************************************************************
// Method: QvisCommandWindow::macroCreate
//
// Purpose: 
//   Creates a macro from the code defined in the specified tab.
//
// Arguments:
//   tab : The tab whose code will become a macro.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 15 14:19:28 PST 2007
//
// Modifications:
//   Cyrus Harrison, Tue Jun 10 15:00:05 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisCommandWindow::macroCreate(int tab)
{
    // Add to the macro tab.
    CreateMacroFromText(editors[tab]->toPlainText());
}
