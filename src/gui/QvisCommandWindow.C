/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <QvisCommandWindow.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qfile.h>
#include <qfont.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qtextedit.h>
#include <qtextstream.h>
#include <qtooltip.h>
#include <qvbox.h>

#include <Utility.h>
#include <DataNode.h>

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
// ****************************************************************************

QvisCommandWindow::QvisCommandWindow(const char *captionString,
    const char *shortName, QvisNotepadArea *n) : 
    QvisPostableWindow(captionString, shortName, n)
{
    executeButtonsGroup = 0;
    executeButtons = 0;
    clearButtons = 0;
    clearButtonsGroup = 0;
    lineEdits = 0;

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
//   
// ****************************************************************************

QvisCommandWindow::~QvisCommandWindow()
{
    delete executeButtonsGroup;
    delete [] executeButtons;
    delete clearButtonsGroup;
    delete [] clearButtons;
    delete [] lineEdits;
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
// ****************************************************************************

void
QvisCommandWindow::CreateWindowContents()
{
    QGroupBox *macroBox = new QGroupBox(central, "macroBox");
    macroBox->setTitle("Macro");
    topLayout->addWidget(macroBox);

    QVBoxLayout *innerMacroLayout = new QVBoxLayout(macroBox);
    innerMacroLayout->setMargin(10);
    innerMacroLayout->addSpacing(15);
    QHBoxLayout *macroLayout = new QHBoxLayout(innerMacroLayout);
    macroLayout->setSpacing(5);

    macroRecord = new QPushButton(QIconSet(QPixmap(macrorecord_xpm)),
        "Record", macroBox, "macroRecord");
    connect(macroRecord, SIGNAL(clicked()), this, SLOT(macroRecordClicked()));
    QToolTip::add(macroRecord, "Start recording macro");
    macroLayout->addWidget(macroRecord);

    macroPause = new QPushButton(QIconSet(QPixmap(macropause_xpm)),
        "Pause", macroBox, "macroPause");
    macroPause->setToggleButton(true);
    connect(macroPause, SIGNAL(clicked()), this, SLOT(macroPauseClicked()));
    QToolTip::add(macroPause, "Pause macro recording");
    macroLayout->addWidget(macroPause);

    macroEnd = new QPushButton(QIconSet(QPixmap(macrostop_xpm)),
        "Stop", macroBox, "macroEnd");
    connect(macroEnd, SIGNAL(clicked()), this, SLOT(macroEndClicked()));
    QToolTip::add(macroEnd, "Stop macro recording");
    macroLayout->addWidget(macroEnd);
    macroRecord->setEnabled(true);
    macroPause->setEnabled(false);
    macroEnd->setEnabled(false);

    // Create macro append and storage controls.
    innerMacroLayout->addSpacing(5);
    QGridLayout *mLayout = new QGridLayout(innerMacroLayout, 2, 2);
    mLayout->setSpacing(5);
    mLayout->setColStretch(1, 10);

    macroStorageComboBox = new QComboBox(macroBox, "macroAppendCheckBox");
    macroStorageComboBox->insertItem("Active tab");
    macroStorageComboBox->insertItem("First empty tab");
    connect(macroStorageComboBox, SIGNAL(activated(int)),
            this, SLOT(macroStorageActivated(int)));
    mLayout->addWidget(macroStorageComboBox, 0, 1);
    mLayout->addWidget(new QLabel(macroStorageComboBox, "Store macro in",
        macroBox), 0, 0);

    macroAppendCheckBox = new QCheckBox("Append macro to existing text", macroBox,
        "macroAppendCheckBox");
    connect(macroAppendCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(macroAppendClicked(bool)));
    mLayout->addMultiCellWidget(macroAppendCheckBox, 1, 1, 0, 1);

    tabWidget = new QTabWidget(central, "tabWidget");
    tabWidget->setMinimumHeight(200);
    topLayout->addWidget(tabWidget, 1000);

    executeButtonsGroup = new QButtonGroup(0, "executeButtonsGroup");
    connect(executeButtonsGroup, SIGNAL(clicked(int)),
            this, SLOT(executeClicked(int)));

    clearButtonsGroup = new QButtonGroup(0, "clearButtonsGroup");
    connect(clearButtonsGroup, SIGNAL(clicked(int)),
            this, SLOT(clearClicked(int)));

    lineEdits = new QTextEdit*[MAXTABS];
    executeButtons = new QPushButton*[MAXTABS];
    clearButtons = new QPushButton*[MAXTABS];
    QFont monospaced("Courier");
    for (int i = 0; i < MAXTABS; i++)
    {
        QVBox *vb = new QVBox(central, "page");
        vb->setMargin(10);
        vb->setSpacing(5);
        lineEdits[i]  = new QTextEdit(vb, "lineEdits");
        lineEdits[i]->setWordWrap(QTextEdit::WidgetWidth);
        lineEdits[i]->setReadOnly(false);
        lineEdits[i]->setFont(monospaced);
        lineEdits[i]->setTextFormat(Qt::PlainText);
        lineEdits[i]->setWordWrap(QTextEdit::NoWrap);
        QString slotName(SLOT(textChanged#()));
        QString n; n.sprintf("%d", i);
        slotName.replace(slotName.find("#"), 1, n);
        connect(lineEdits[i], SIGNAL(textChanged()),
                this, slotName.latin1());

        QHBox *hb = new QHBox(vb, "hb");
        hb->setSpacing(5);
        executeButtons[i] = new QPushButton(QIconSet(QPixmap(macroexec_xpm)),
            "Execute", hb, "executeButton");
        executeButtons[i]->setEnabled(false);
        executeButtonsGroup->insert(executeButtons[i], i);

        clearButtons[i] = new QPushButton("Clear", hb,
            "clearButton");
        clearButtons[i]->setEnabled(false);
        clearButtonsGroup->insert(clearButtons[i], i);

        // Add the top vbox as a new tab.
        n.sprintf("%d", i+1);
        tabWidget->addTab(vb, n);
    }

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
// ****************************************************************************

void
QvisCommandWindow::CreateNode(DataNode *node)
{
    QvisPostableWindow::CreateNode(node);

    if(isCreated)
    {
        SaveScripts();

        DataNode *winNode = node->GetNode(std::string(caption().latin1()));
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
// ****************************************************************************

void
QvisCommandWindow::SetFromNode(DataNode *parentNode, const int *borders)
{
    QvisPostableWindow::SetFromNode(parentNode, borders);

    DataNode *winNode = parentNode->GetNode(std::string(caption().latin1()));
    if(winNode == 0)
        return;

    DataNode *n = 0;
    if((n = winNode->GetNode("macroStorageMode")) != 0)
    {
        macroStorageMode = n->AsInt();
        if(macroStorageMode > 1)
            macroStorageMode = 1;
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
//   
// ****************************************************************************

void
QvisCommandWindow::UpdateMacroCheckBoxes()
{
    if(macroStorageComboBox != 0)
    {
        macroStorageComboBox->blockSignals(true);
        macroStorageComboBox->setCurrentItem(macroStorageMode);
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
// Method: QvisCommandWindow::LoadScripts
//
// Purpose: 
//   Loads the scripts into the window.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 22 16:38:30 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisCommandWindow::LoadScripts()
{
    for(int i = 0; i < MAXTABS; ++i)
    {
        QFile file(fileName(i+1));
        if(file.open(IO_ReadOnly))
        {
            QTextStream stream(&file);

            QString lines;
            while(!stream.eof())
            {
                lines += stream.readLine();
                lines += "\n";
            }
            file.close();

            lineEdits[i]->setText(lines);
        }
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
//   
// ****************************************************************************

void
QvisCommandWindow::SaveScripts()
{
    for(int i = 0; i < MAXTABS; ++i)
    {
        QFile file(fileName(i+1));
        QString txt(lineEdits[i]->text());
        if(txt.length() > 0)
        {
            if(file.open(IO_WriteOnly))
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
//   
// ****************************************************************************

void
QvisCommandWindow::executeClicked(int index)
{
    emit runCommand(lineEdits[index]->text());
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
    lineEdits[index]->clear();
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
//   
// ****************************************************************************

#define TEXT_CHANGED(I) void QvisCommandWindow::textChanged##I() { \
    bool e = lineEdits[I]->length() > 0; \
    executeButtons[I]->setEnabled(e); \
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
//   
// ****************************************************************************

void
QvisCommandWindow::acceptRecordedMacro(const QString &s)
{
    int index = 0;
    if(macroStorageMode == 0)
        index = tabWidget->currentPageIndex();
    else
    {
        // Look for the first empty tab.
        bool found = false;
        for(int i = 0; i < MAXTABS; ++i)
        {
            if(lineEdits[i]->text().isEmpty())
            {
                index = i;
                found = true;
                break;
            }
        }

        index = found ? index : 0;
        tabWidget->blockSignals(true);
        tabWidget->setCurrentPage(index);
        tabWidget->blockSignals(false);
    }

    if(macroAppend)
    {
        QString macro(lineEdits[index]->text());
        macro += s;
        lineEdits[index]->setText(macro);
    }
    else
        lineEdits[index]->setText(s);
}
