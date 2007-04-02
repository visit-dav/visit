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

#include <QvisMessageWindow.h>
#include <Observer.h>
#include <MessageAttributes.h>

#include <TimingsManager.h> // for DELTA_TOA_THIS_LINE

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
//   Mark C. Miller, Wed Jun 29 17:04:13 PDT 2005
//   I made it catenate new message *only* if existing message didn't already
//   contain text of new message.
//
//   Brad Whitlock, Thu May 11 15:01:21 PST 2006
//   Return if the message is ErrorClear.
//
// *************************************************************************************

void
QvisMessageWindow::Update(Subject *)
{
    MessageAttributes *ma = (MessageAttributes *)subject;

    MessageAttributes::Severity severity = ma->GetSeverity();
    if(severity == MessageAttributes::ErrorClear)
        return;

    double secondsSinceLastMessage = DELTA_TOA_THIS_LINE;
    QString msgText;
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
        QString newMsgText = QString(ma->GetText().c_str());
        if (msgText.find(newMsgText) == -1)
        {
            msgText += "\n\nShortly thereafter, the following occured...\n\n";
            msgText += newMsgText;
        }
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
