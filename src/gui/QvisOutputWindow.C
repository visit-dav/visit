#include <qmultilineedit.h>
#include <qtabwidget.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qstring.h>

#include <QvisOutputWindow.h>
#include <QvisMainWindow.h>
#include <MessageAttributes.h>

// ****************************************************************************
// Method: QvisOutputWindow::QvisOutputWindow
//
// Purpose: 
//   This is the constructor for the QvisOutputWindow class. It
//   creates the widgets that make up the window.
//
// Arguments:
//   msgAtts : The MessageAttributes object that the window observes.
//   caption : The name of the window that appears in the window
//             decoration.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 27 17:27:13 PST 2000
//
// Modifications:
//   Brad Whitlock, Tue Sep 25 08:50:30 PDT 2001
//   Prevented stretch from being added to the window.
//
// ****************************************************************************

QvisOutputWindow::QvisOutputWindow(MessageAttributes *subj,
    const char *caption, const char *shortName, QvisNotepadArea *notepad) : 
    QvisPostableWindowObserver(subj, caption, shortName, notepad,
                               QvisPostableWindowObserver::NoExtraButtons,
                               false)
{
    // nothing here
}

// ****************************************************************************
// Method: QvisOutputWindow::~QvisOutputWindow
//
// Purpose: 
//   Destructor for the QvisOutputWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 27 17:29:22 PST 2000
//
// Modifications:
//   
// ****************************************************************************

QvisOutputWindow::~QvisOutputWindow()
{
    // nothing here
}

// ****************************************************************************
// Method: QvisOutputWindow::CreateWindowContents
//
// Purpose: 
//   This method creates the window's widgets.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 6 12:17:28 PDT 2001
//
// Modifications:
//   Brad Whitlock, Tue Sep 25 08:51:08 PDT 2001
//   Set the minimum width based on part of a sample message from the viewer
//   and the current font size.
//
// ****************************************************************************

void
QvisOutputWindow::CreateWindowContents()
{
    // Create a multi line edit to display the text.
    outputText = new QMultiLineEdit(central, "outputText");
    outputText->setMinimumWidth(fontMetrics().width("MESSAGE: Closed the "
        "compute engine on host"));
    outputText->setWordWrap(QMultiLineEdit::WidgetWidth);
    outputText->setReadOnly(true);
    topLayout->addWidget(outputText);
}

// ****************************************************************************
// Method: QvisOutputWindow::UpdateWindow
//
// Purpose: 
//   Updates the output window when the MessageAttributes object that
//   the window watches is updated.
//
// Programmer: Brad Whitlock
// Creation:   Sat Sep 16 11:52:07 PDT 2000
//
// Modifications:
//   Brad Whitlock, Tue Sep 25 09:14:15 PDT 2001
//   Added a newline in between messages.
//
//   Brad Whitlock, Thu Oct 25 18:45:27 PST 2001
//   Modified how the main window is told to update its icon.
//
//   Brad Whitlock, Tue May 20 15:09:11 PST 2003
//   Made it work with the regenerated MessageAttributes.
//
//   Brad Whitlock, Thu May 11 14:59:11 PST 2006
//   Added support for ErrorClear.
//
// ****************************************************************************

void
QvisOutputWindow::UpdateWindow(bool)
{
    MessageAttributes *msgAtts = (MessageAttributes *)subject;

    // Return without doing anything if it's ErrorClear.
    if(msgAtts->GetSeverity() == MessageAttributes::ErrorClear)
        return;

    // Create a string to add to the output text.
    QString temp;
    if(msgAtts->GetSeverity() == MessageAttributes::Error)
        temp = "ERROR: ";
    if(msgAtts->GetSeverity() == MessageAttributes::Message)
        temp = "MESSAGE: ";
    if(msgAtts->GetSeverity() == MessageAttributes::Warning)
        temp = "WARNING: ";
    temp += QString(msgAtts->GetText().c_str());
    temp += QString("\n");

    // Add the line of text.
    outputText->insertLine(temp);
    outputText->setCursorPosition(outputText->numLines() - 1, 0);

    // If the window is visible then tell the main window to turn its
    // unread icon back to blue.
    if(isVisible() || posted())
        emit unreadOutput(false);
}
