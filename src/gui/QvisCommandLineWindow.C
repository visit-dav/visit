#include <QvisCommandLineWindow.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qhbox.h>
#include <qlayout.h>
#include <qvbox.h>

// ****************************************************************************
// Method: QvisCommandLineWindow::QvisCommandLineWindow
//
// Purpose: 
//   Constructor for the QvisCommandLineWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 25 18:34:47 PST 2000
//
// Modifications:
//   
// ****************************************************************************

QvisCommandLineWindow::QvisCommandLineWindow(const char *captionString, 
    const char *shortName, QvisNotepadArea *notepad) :
    QvisPostableWindow(captionString, shortName, notepad)
{
}

// ****************************************************************************
// Method: QvisCommandLineWindow::~QvisCommandLineWindow
//
// Purpose: 
//   Destructor for the QvisCommandLineWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 25 18:34:47 PST 2000
//
// Modifications:
//   
// ****************************************************************************

QvisCommandLineWindow::~QvisCommandLineWindow()
{
}

// ****************************************************************************
// Method: QvisCommandLineWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets used in the window. This method is provided
//   as a means of delaying the creation of the window widgets until
//   the window needs to be shown.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 25 18:35:29 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisCommandLineWindow::CreateWindowContents()
{
    // Create the line edit and add it to the top layout.
    commandLineEdit = new QLineEdit(central);
    topLayout->addWidget(commandLineEdit);

    // When text is entered, process it.
    connect(commandLineEdit, SIGNAL(returnPressed()), this, SLOT(processText()));
}

// ****************************************************************************
// Method: QvisCommandLineWindow::processText
//
// Purpose: 
//   This is a Qt slot function that reads the text entered into the
//   window's line edit widget and sends it to the interpreter.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 25 18:36:33 PST 2000
//
// Modifications:
//   Brad Whitlock, Wed May 2 11:18:28 PDT 2001
//   Got rid of interpreter until we can decide what to do.
//
// ****************************************************************************

void
QvisCommandLineWindow::processText()
{
    // If the line edit is not empty, send it to the interpreter
    // clear the line edit.
    if(!commandLineEdit->text().isEmpty())
    {
        // Output a message until we can interpret the image.
        QString msg;
        msg.sprintf("Cannot interpret \"%s\".", commandLineEdit->text().latin1());
        Message(msg);
        commandLineEdit->setText(QString(""));
    }
}
