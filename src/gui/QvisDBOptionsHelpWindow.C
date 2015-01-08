/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
