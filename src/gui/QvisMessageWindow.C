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

#include <QvisMessageWindow.h>
#include <Observer.h>
#include <MessageAttributes.h>

#include <TimingsManager.h> // for DELTA_TOA_THIS_LINE
#include <UnicodeHelper.h>

#include <QApplication>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QTextEdit>

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
//   Brad Whitlock, Fri Jan 18 16:11:07 PST 2008
//   Added preserveInformation, doHide slot, and made the window wider&taller by default.
//
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Fri May 30 14:28:01 PDT 2008
//   Qt 4.
//
// *************************************************************************************

QvisMessageWindow::QvisMessageWindow(MessageAttributes *msgAttr,
    const QString &captionString) : QvisWindowBase(captionString),
    Observer(msgAttr)
{
    preserveInformation = false;

    // Create the central widget and the top layout.
    QWidget *central = new QWidget( this );
    setCentralWidget( central );
    QVBoxLayout *topLayout = new QVBoxLayout(central);
    topLayout->setMargin(10);

    // Create a multi line edit to display the message text.
    messageText = new QTextEdit(central);
    messageText->setWordWrapMode(QTextOption::WordWrap);
    messageText->setReadOnly(true);
    messageText->setMinimumWidth(3 * fontMetrics().width("Closed the compute "
        "engine on host sunburn.llnl.gov.  ") / 2);
    messageText->setMinimumHeight(8 * fontMetrics().lineSpacing());
    severityLabel = new QLabel(tr("Message"), central);
    severityLabel->setBuddy(messageText);
    QFont f("helvetica", 18);
    f.setBold(true);
    severityLabel->setFont(f);
    topLayout->addWidget(severityLabel);
    topLayout->addSpacing(10);
    topLayout->addWidget(messageText);
    topLayout->addSpacing(10);

    QHBoxLayout *buttonLayout = new QHBoxLayout(0);
    topLayout->addLayout(buttonLayout);

    // Create a button to hide the window.
    QPushButton *dismissButton = new QPushButton(tr("Dismiss"), central);
    buttonLayout->addStretch(10);
    buttonLayout->addWidget(dismissButton);
    connect(dismissButton, SIGNAL(clicked()), this, SLOT(doHide()));
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
//   Brad Whitlock, Fri Jan 18 14:34:55 PST 2008
//   Added Information, which is similar to Message but shows the window.
//   The Information message is shown until a new information, error, or
//   warning message comes in. Incoming "Message" messages do not overwrite
//   an Information message while the window is showing.
//
//   Brad Whitlock, Tue Apr 29 10:27:01 PDT 2008
//   Support for internationalization.
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
        if (oldSeverityLabel == tr("Error!"))
            oldSeverity = MessageAttributes::Error;
        else if (oldSeverityLabel == tr("Warning"))
            oldSeverity = MessageAttributes::Warning;
        else if (oldSeverityLabel == tr("Message"))
            oldSeverity = MessageAttributes::Message;
        else if (oldSeverityLabel == tr("Information"))
            oldSeverity = MessageAttributes::Information;
        else
            oldSeverity = MessageAttributes::Error;

        // If we're not in information mode, append the incoming messages
        if(!preserveInformation)
        {
            // set severity to whichever is worse
            if (oldSeverity < severity)
                severity = oldSeverity;

            // catenate new message onto old 
            msgText = messageText->toPlainText();
            QString newMsgText = MessageAttributes_GetText(*ma);
            if (msgText.indexOf(newMsgText) == -1)
            {
                msgText += "\n\n";
                msgText += tr("Shortly thereafter, the following occured...");
                msgText += "\n\n";
                msgText += newMsgText;
            }
        }
        else if(severity == MessageAttributes::Information)
        {
            msgText = MessageAttributes_GetText(*ma);
            preserveInformation = false;
        }
        else if((severity == MessageAttributes::Error ||
                 severity == MessageAttributes::Warning) &&
                 oldSeverity == MessageAttributes::Information)
        {
            // Incoming Error, Warnings may overwrite Information.
            msgText = MessageAttributes_GetText(*ma);
            preserveInformation = false;
        }
    }
    else
    {
        msgText = MessageAttributes_GetText(*ma);
        
        // Don't preserve information if a new information message is
        // coming in. Also let error, warning override the existing
        // information message.
        if(preserveInformation && 
           (severity == MessageAttributes::Error ||
            severity == MessageAttributes::Warning ||
            severity == MessageAttributes::Information))
        {
            preserveInformation = false;
        }
    }

    if(!preserveInformation)
    {
        // Set the severity label text.
        if(severity == MessageAttributes::Error)
        {
            show();
            qApp->beep();
            severityLabel->setText(tr("Error!"));
            RestoreCursor();
        }
        else if(severity == MessageAttributes::Warning)
        {
            show();
            severityLabel->setText(tr("Warning"));
            RestoreCursor();
        }
        else if(severity == MessageAttributes::Message)
            severityLabel->setText(tr("Message"));
        else if(severity == MessageAttributes::Information)
        {
            show();
            severityLabel->setText(tr("Information"));
            RestoreCursor();
            preserveInformation = true;
        }

        // Set the message text.
        messageText->setText(msgText);
    }
}

// ****************************************************************************
// Method: QvisMessageWindow::doHide
//
// Purpose: 
//   Hides the window and turns off the preserveInformation mode.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 18 15:30:48 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisMessageWindow::doHide()
{
    preserveInformation = false;
    hide();
}
