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

#include <visit-config.h>
#include "ViewerChangeUsernameWindow.h"

#include <vector>

#include <QLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTimer>

#include <HostProfile.h>
#include <HostProfileList.h>

#include <ViewerServerManager.h>

#if !defined(_WIN32)
#include <unistd.h>
#include <ViewerSubject.h>
extern ViewerSubject *viewerSubject;
#endif

// Static members
ViewerChangeUsernameWindow *ViewerChangeUsernameWindow::instance = NULL;

// ****************************************************************************
//  Constructor:  ViewerChangeUsernameWindow::ViewerChangeUsernameWindow
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

ViewerChangeUsernameWindow::ViewerChangeUsernameWindow(QWidget *parent)
    : QDialog(parent)
{
    setModal(true);

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
//  Destructor:  ViewerChangeUsernameWindow::~ViewerChangeUsernameWindow
//
//  Programmer:  Hank Childs
//  Creation:    November 11, 2007
//
// ****************************************************************************

ViewerChangeUsernameWindow::~ViewerChangeUsernameWindow()
{
}

// ****************************************************************************
// Method: ViewerChangeUsernameWindow::getUsername
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

std::string
ViewerChangeUsernameWindow::getUsername()
{
    // if never instantiated, no user name has been set here
    if (!instance)
        return NULL;

    return instance->usernameedit->text().toStdString();
}

// ****************************************************************************
// Method: ViewerChangeUsernameWindow::changeUsername
//
// Purpose: 
//   Pops up the change username window and sets the new username in the
//   host profiles.
//
// Arguments:
//   host     : The name of the host for which we're setting the username.
//
// Returns:    True if the user entered a new username, false otherwise
//
// Programmer: Hank Childs
// Creation:   November 11, 2007
//
// Modifications:
//   Brad Whitlock, Tue Apr 29 11:58:35 PDT 2008
//   Added tr()'s
//
//   Brad Whitlock, Fri May 23 11:03:20 PDT 2008
//   Use std::string, Qt 4.
//
// ****************************************************************************

bool
ViewerChangeUsernameWindow::changeUsername(const std::string &host)
{
    HostProfileList *profiles = ViewerServerManager::GetClientAtts();

    if(!instance)
        instance = new ViewerChangeUsernameWindow();

    instance->setWindowTitle(tr("Choose new username"));

    // Set the username prompt.
    QString labelText(tr("New username for %1: ").arg(host.c_str()));
    instance->label->setText(labelText);

    // Make the username window be the active window.
    instance->activateWindow();
    instance->raise();

    // Clear the username.
    instance->usernameedit->clear();
    instance->usernameedit->setText("");

    // Give focus to the username window.
    QTimer::singleShot(300, instance->usernameedit, SLOT(setFocus()));

    // Enter the local event loop for the dialog.
#if !defined(_WIN32)
    viewerSubject->BlockSocketSignals(true);
#endif
    int status = instance->exec();
#if !defined(_WIN32)
    viewerSubject->BlockSocketSignals(false);
#endif

    if (status == Accepted)
    {
        // Accepted; hit return or Okay.
        
        std::string new_username(instance->usernameedit->text().toStdString());
        if (new_username.size() == 0)
            return false;
       
        // Let this convenience function do things like looking up the fully
        // qualified host name.  It returns a const, which is a pain, but just
        // work around that later.
        std::vector<const HostProfile *> hp = 
                                 profiles->FindAllMatchingProfileForHost(host);
        
        for (size_t i = 0 ; i < hp.size() ; i++)
        {
            for (int j = 0 ; j < profiles->GetNumProfiles() ; j++)
            {
                // hack to get around const ... we have the const ptr, find non-const
                if (hp[i] == &(profiles->GetProfiles(j))) 
                {
                    profiles->GetProfiles(j).SetUserName(new_username);
                }
            }
        }

        profiles->SelectAll();
        profiles->Notify();

        return true;
    }

    // Rejected or cancelled.  
    return false;
}


