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

#include <visit-config.h>
#include "ViewerChangeUsernameWindow.h"

#include <vector>

#include <QLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTimer>

#include <MachineProfile.h>
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

ViewerChangeUsernameWindow::ViewerChangeUsernameWindow(QWidget *parent) :
    VisItChangeUsernameWindow(parent), username()
{
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
//   Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//   Split HostProfile int MachineProfile and LaunchProfile.
//   Can just update one machine profile now.
//
//   Kathleen Bonnell, Thu Apr 22 17:54:34 MST 2010
//   Prevent use of mp if NULL.
//
//   Brad Whitlock, Tue Jun 12 14:31:34 PST 2012
//   Use base class's getUsername method.
//
// ****************************************************************************

bool
ViewerChangeUsernameWindow::changeUsername(const std::string &host)
{
    HostProfileList *profiles = ViewerServerManager::GetClientAtts();

    if(!instance)
        instance = new ViewerChangeUsernameWindow();

    // Enter the local event loop for the dialog.
#if !defined(_WIN32)
    viewerSubject->BlockSocketSignals(true);
#endif

    VisItChangeUsernameWindow::ReturnCode status = VisItChangeUsernameWindow::UW_Rejected;
    instance->username = "";
    QString name = instance->getUsername(host.c_str(), status);

#if !defined(_WIN32)
    viewerSubject->BlockSocketSignals(false);
#endif

    if (status == UW_Accepted)
    {
        // Accepted; hit return or Okay.
        std::string new_username(name.toStdString());
        if (new_username.empty())
            return false;

        instance->username = new_username;

        MachineProfile *mp = profiles->GetMachineProfileForHost(host);
        if (mp != NULL)
            mp->SetUserName(new_username);

        profiles->SelectAll();
        profiles->Notify();

        return true;
    }

    // Rejected or cancelled.  
    return false;
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
//   Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//   Split HostProfile int MachineProfile and LaunchProfile.
//   Can just update one machine profile now.
//
//   Kathleen Bonnell, Thu Apr 22 17:54:34 MST 2010
//   Prevent use of mp if NULL.
//
// ****************************************************************************


bool
ViewerChangeUsernameWindow::changeUsername(const std::string &host, std::string &_username)
{
    bool res = changeUsername(host);
    if(res) _username = instance->username;
    return res;
}


