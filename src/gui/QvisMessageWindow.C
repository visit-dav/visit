#include <QvisMessageWindow.h>
#include <Observer.h>
#include <MessageAttributes.h>

#include <TimingsManager.h> // for DELTA_TOA_HERE

#include <qapplication.h>
#include <qlabel.h>
#include <qmultilineedit.h>
#include <qlayout.h>
#include <qpushbutton.h>

// *************************************************************************************
// Method: QvisMessageWindow::QvisMessageWindow
//
// Purpose: 
//   This is the constructor for the QvisMessageWindow class. Note
//   that it creates its widgets right away. This is one of the few
//   windows that needs to do this.
//
// Arguments:
//   msgAttr : A pointer to the MessageAttributes that this window will
//             observe.
//   captionString : The caption that is in the window's window decor.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 30 18:10:10 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Sep 24 17:10:32 PST 2001
//   I stuck in code using a real message to set the minimum size of the widget
//   based on the font.
//
//   Brad Whitlock, Mon May 12 13:00:03 PST 2003
//   I made the text read only.
//
// *************************************************************************************

QvisMessageWindow::QvisMessageWindow(MessageAttributes *msgAttr,
    const char *captionString) : QvisWindowBase(captionString),
    Observer(msgAttr)
{
    // Create the central widget and the top layout.
    QWidget *central = new QWidget( this );
    setCentralWidget( central );
    QVBoxLayout *topLayout = new QVBoxLayout(central, 10);

    // Create a multi line edit to display the message text.
    messageText = new QMultiLineEdit( central, "outputText" );
    messageText->setWordWrap( QMultiLineEdit::WidgetWidth );
    messageText->setReadOnly(true);
    messageText->setMinimumWidth(fontMetrics().width("Closed the compute "
        "engine on host sunburn.llnl.gov.  "));
    severityLabel = new QLabel(messageText, "Message", central, "Severity Label");
    QFont f("helvetica", 18);
    f.setBold(true);
    severityLabel->setFont( f );
    topLayout->addWidget(severityLabel);
    topLayout->addSpacing(10);
    topLayout->addWidget(messageText);
    topLayout->addSpacing(10);

    QHBoxLayout *buttonLayout = new QHBoxLayout(topLayout);

    // Create a button to hide the window.
    QPushButton *dismissButton = new QPushButton("Dismiss", central, "dismiss");
    buttonLayout->addStretch(10);
    buttonLayout->addWidget(dismissButton);
    connect(dismissButton, SIGNAL(clicked()), this, SLOT(hide()));
}

// *************************************************************************************
// Method: QvisMessageWindow::~QvisMessageWindow
//
// Purpose: 
//   Destructor for the QvisMessageWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 30 18:12:09 PST 2000
//
// Modifications:
//   
// *************************************************************************************

QvisMessageWindow::~QvisMessageWindow()
{
    // nothing
}

// *************************************************************************************
// Method: QvisMessageWindow::Update
//
// Purpose: 
//   This method is called when the MessageAttributes object that this
//   window is watching changes. It's this window's responsibility to
//   write the message into the window and display it.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 30 18:12:32 PST 2000
//
// Modifications:
//   Brad Whitlock, Tue May 20 15:07:59 PST 2003
//   I made it work with the regenerated MessageAttributes.
//
//   Brad Whitlock, Wed Sep 10 09:44:44 PDT 2003
//   I made the cursor get reset for error and warning messages.
//
//   Mark C. Miller Wed Apr 21 12:42:13 PDT 2004
//   I made it smarter about dealing with messages that occur close together in time.
//   Now, it will catenate them.
//
// *************************************************************************************

void
QvisMessageWindow::Update(Subject *)
{
    MessageAttributes *ma = (MessageAttributes *)subject;

    double secondsSinceLastMessage = DELTA_TOA_HERE;

    QString msgText;
    MessageAttributes::Severity severity = ma->GetSeverity();
    if (secondsSinceLastMessage < 5.0)
    {
        MessageAttributes::Severity oldSeverity;
        QString oldSeverityLabel = severityLabel->text();
        if (oldSeverityLabel == "Error!")
            oldSeverity = MessageAttributes::Error;
        else if (oldSeverityLabel == "Warning")
            oldSeverity = MessageAttributes::Warning;
        else if (oldSeverityLabel == "Message")
            oldSeverity = MessageAttributes::Message;
        else
            oldSeverity = MessageAttributes::Error;

        // set severity to whichever is worse
        if (oldSeverity < severity)
            severity = oldSeverity;

        // catenate new message onto old 
        msgText = messageText->text();
        msgText += "\n\nShortly thereafter, the following occured...\n\n";
        QString newMsgText = QString(ma->GetText().c_str());
        msgText += newMsgText;
    }
    else
    {
        msgText = QString(ma->GetText().c_str());
    }

    // Set the severity label text.
    if(severity == MessageAttributes::Error)
    {
        show();
        qApp->beep();
        severityLabel->setText(QString("Error!"));
        RestoreCursor();
    }
    else if(severity == MessageAttributes::Warning)
    {
        show();
        severityLabel->setText(QString("Warning"));
        RestoreCursor();
    }
    else if(severity == MessageAttributes::Message)
        severityLabel->setText(QString("Message"));

    // Set the message text.
    messageText->setText(msgText);
}
