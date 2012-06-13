/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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

#include "VisItChangeUsernameWindow.h"

#include <QLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTimer>

// ****************************************************************************
//  Constructor:  VisItChangeUsernameWindow::VisItChangeUsernameWindow
//
//  Programmer:  Hank Childs
//  Creation:    November 11, 2007
//
//  Modifications:
//    Brad Whitlock, Tue Apr 29 11:58:00 PDT 2008
//    Added tr()'s
//
//    Brad Whitlock, Fri May 23 10:47:52 PDT 2008
//    Qt 4.
//
// ****************************************************************************

VisItChangeUsernameWindow::VisItChangeUsernameWindow(QWidget *parent) : 
    QDialog(parent)
{
    setModal(true);

    setWindowTitle(tr("Choose new username"));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(10);

    QHBoxLayout *l2 = new QHBoxLayout;
    layout->addLayout(l2);
    l2->setSpacing(5);
    label = new QLabel(tr("Username for localhost: "), this);
    l2->addWidget(label);

    usernameedit = new QLineEdit(this);
    l2->addWidget(usernameedit);
    connect(usernameedit, SIGNAL(returnPressed()), this, SLOT(accept()));
    layout->addSpacing(20);

    QHBoxLayout *l3 = new QHBoxLayout;
    layout->addLayout(l3);
    QPushButton *okay = new QPushButton(tr("Confirm username"), this);
    connect(okay, SIGNAL(clicked()), this, SLOT(accept()));
    l3->addWidget(okay);
    l3->addStretch(10);

    QPushButton *cancel = new QPushButton(tr("Cancel"), this);
    connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
    l3->addWidget(cancel);
}

// ****************************************************************************
//  Destructor:  VisItChangeUsernameWindow::~VisItChangeUsernameWindow
//
//  Programmer:  Hank Childs
//  Creation:    November 11, 2007
//
// ****************************************************************************

VisItChangeUsernameWindow::~VisItChangeUsernameWindow()
{
}

// ****************************************************************************
// Method: VisItChangeUsernameWindow::getUsername
//
// Purpose: 
//   Retrieve the username from the widget.
//
// Returns:    Returns the username name if this has been instantiated, 
//             NULL otherwise. 
//
// Programmer: Kathleen Bonnell 
// Creation:   February 13, 2008 
//
// ****************************************************************************

QString
VisItChangeUsernameWindow::getUsername(const QString &host, 
    VisItChangeUsernameWindow::ReturnCode &ret)
{
    // Set the username prompt.
    QString labelText(tr("New username for %1: ").arg(host));
    this->label->setText(labelText);

    // Make the username window be the active window.
    this->activateWindow();
    this->raise();

    // Clear the username.
    this->usernameedit->clear();
    this->usernameedit->setText("");

    // Give focus to the username window.
    QTimer::singleShot(300, this->usernameedit, SLOT(setFocus()));

    // Enter the local event loop for the dialog.
    int status = this->exec();

    QString username;
    if (status == Accepted)
    {
        ret = UW_Accepted;
        username = this->usernameedit->text();
    }
    else
        ret = UW_Rejected;

    return username;
}
