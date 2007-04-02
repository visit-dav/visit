#include <QvisCommandWindow.h>
#include <qbuttongroup.h>
#include <qfile.h>
#include <qfont.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qtextedit.h>
#include <qtextstream.h>
#include <qvbox.h>

#include <Utility.h>

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
//   
// ****************************************************************************

void
QvisCommandWindow::CreateWindowContents()
{
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
        executeButtons[i] = new QPushButton("Execute", hb,
            "executeButton");
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
//   
// ****************************************************************************

void
QvisCommandWindow::CreateNode(DataNode *node)
{
    QvisPostableWindow::CreateNode(node);

    if(isCreated)
        SaveScripts();
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

