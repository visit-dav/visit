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

#include <QvisMessageDialog.h>

#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QTextEdit>

#include <MessageAttributes.h>

// ****************************************************************************
// Method: QvisMessageDialog::QvisMessageDialog
//
// Purpose: 
//   Constructor.
//
// Programmer: Eric Brugger
// Creation:   Thu Feb 12 15:55:45 PST 2009
//
// Modifications:
//
// ****************************************************************************

QvisMessageDialog::QvisMessageDialog()
{
    QVBoxLayout *topLayout = new QVBoxLayout(this);

    QGridLayout *grid = new QGridLayout();
    topLayout->addLayout(grid);

    // Create a label to display the severity.
    severityLabel = new QLabel(tr("Message"), this);
    severityLabel->setBuddy(messageText);
    QFont f("helvetica", 18);
    f.setBold(true);
    severityLabel->setFont(f);
    grid->addWidget(severityLabel);

    // Create a multi line edit to display the message text.
    messageText = new QTextEdit(this);
    messageText->setWordWrapMode(QTextOption::WordWrap);
    messageText->setReadOnly(true);
    grid->addWidget(messageText);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    topLayout->addLayout(buttonLayout);

    // Create a button to hide the window.
    QPushButton *dismissButton = new QPushButton(tr("Dismiss"), this);
    connect(dismissButton, SIGNAL(clicked()), this, SLOT(hide()));
    buttonLayout->addWidget(dismissButton);
    buttonLayout->addStretch(10);
}

// ****************************************************************************
// Method: QvisMessageDialog::~QvisMessageDialog
//
// Purpose: 
//   Destructor.
//
// Programmer: Eric Brugger
// Creation:   Thu Feb 12 15:55:45 PST 2009
//
// Modifications:
//   
// ****************************************************************************

QvisMessageDialog::~QvisMessageDialog()
{
}

// ****************************************************************************
// Method: QvisMessageDialog::DisplayMessage
//
// Purpose: 
//   This method is called to display a message.
//
// Programmer: Eric Brugger
// Creation:   Thu Feb 12 15:55:45 PST 2009
//
// Modifications:
//
// ****************************************************************************

void
QvisMessageDialog::DisplayMessage(MessageAttributes *messageAtts)
{
    // Set the severity label text.
    switch (messageAtts->GetSeverity())
    {
      case MessageAttributes::Error:
        severityLabel->setText(tr("Error!"));
        break;
      case MessageAttributes::Warning:
        severityLabel->setText(tr("Warning"));
        break;
      case MessageAttributes::Message:
        severityLabel->setText(tr("Message"));
        break;
      case MessageAttributes::Information:
        severityLabel->setText(tr("Information"));
        break;
    }

    // Set the message text.
    messageText->setText(QString(messageAtts->GetText().c_str()));

    show();
}
