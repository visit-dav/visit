// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QvisDBOptionsHelpWindow.h>
#include <QLayout>
#include <QPushButton>
#include <QTextBrowser>

#include <DBOptionsAttributes.h>

// ****************************************************************************
// Method: QvisDBOptionsHelpWindow::QvisDBOptionsHelpWindow
//
// Purpose: 
//   Constructor
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: 
// Creation:   Tue Apr  8 11:12:48 PDT 2008
//
// Modifications:
//    Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//    Support for internationalization.
//   
//    Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//    Initial Qt4 Port.
//
//    Mark C. Miller, Mon Mar 16 23:10:47 PDT 2009
//    Added logic to skip obsolete options. Fixed issue setting current
//    index for Enums (combo boxes).
//
//    Jeremy Meredith, Mon Jan  4 14:27:45 EST 2010
//    Fixed bug with floats.
//
// ****************************************************************************

QvisDBOptionsHelpWindow::QvisDBOptionsHelpWindow(DBOptionsAttributes *dbatts,
                                         QWidget *parent)
: QDialog(parent), atts(dbatts)
{
    QVBoxLayout *topLayout = new QVBoxLayout(this);

    // Create a multi line edit to display the text.
    QTextBrowser *outputText = new QTextBrowser(this);
    outputText->setReadOnly(true);
    outputText->setOpenExternalLinks(true);
    outputText->setText(tr(dbatts->GetHelp().c_str()));
    topLayout->addWidget(outputText);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    topLayout->addLayout(btnLayout);
    dismissButton = new QPushButton(tr("Dismiss"), this);
    connect(dismissButton, SIGNAL(clicked()), this, SLOT(reject()));
    btnLayout->addWidget(dismissButton);
}

// ****************************************************************************
// Method: QvisDBOptionsHelpWindow::~QvisDBOptionsHelpWindow
//
// Purpose: 
//   Destructor
//
// Creation:   Tue Apr  8 11:12:48 PDT 2008
//
// Modifications:
//
// ****************************************************************************

QvisDBOptionsHelpWindow::~QvisDBOptionsHelpWindow()
{}
