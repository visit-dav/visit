// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//   Eric Brugger, Fri May 25 14:27:42 PDT 2012
//   I rearranged some of the code to avoid a crash with new versions of qt.
//
// ****************************************************************************

QvisMessageDialog::QvisMessageDialog()
{
    QVBoxLayout *topLayout = new QVBoxLayout(this);

    QGridLayout *grid = new QGridLayout();
    topLayout->addLayout(grid);

    // Create a multi line edit to display the message text.
    messageText = new QTextEdit(this);
    messageText->setWordWrapMode(QTextOption::WordWrap);
    messageText->setReadOnly(true);

    // Create a label to display the severity.
    severityLabel = new QLabel(tr("Message"), this);
    severityLabel->setBuddy(messageText);
    QFont f("helvetica", 18);
    f.setBold(true);
    severityLabel->setFont(f);

    grid->addWidget(severityLabel);
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
      default:
        severityLabel->setText("");
        break;
    }

    // Set the message text.
    messageText->setText(QString(messageAtts->GetText().c_str()));

    show();
}
