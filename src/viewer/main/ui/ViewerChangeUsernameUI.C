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
#include <ViewerChangeUsernameUI.h>

#include <VisItChangeUsernameWindow.h>

#include <MachineProfile.h>
#include <HostProfileList.h>
#include <ViewerMessaging.h>
#include <ViewerState.h>

// ****************************************************************************
// Method: ViewerChangeUsernameUI::ViewerChangeUsernameUI
//
// Purpose:
//   Constructor.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 11 15:01:51 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerChangeUsernameUI::ViewerChangeUsernameUI() : ViewerChangeUsername()
{
    win = new VisItChangeUsernameWindow;
}

// ****************************************************************************
// Method: ViewerChangeUsernameUI::~ViewerChangeUsernameUI
//
// Purpose:
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 11 15:02:05 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerChangeUsernameUI::~ViewerChangeUsernameUI()
{
    delete win;
}

// ****************************************************************************
// Method: ViewerChangeUsernameUI::ChangeUsername
//
// Purpose:
//   Changes the username by poking the dialog results into the host profiles.
//
// Arguments:
//   host        : The host for which we're changing the username.
//   newUsername : The new user name.
//
// Returns:    True on success; False on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 11 15:02:20 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
ViewerChangeUsernameUI::ChangeUsername(const std::string &host, std::string &newUsername)
{
    HostProfileList *profiles = GetViewerState()->GetHostProfileList();

    // Enter the local event loop for the dialog.
#if !defined(_WIN32)
    GetViewerMessaging()->BlockClientInput(true);
#endif

    VisItChangeUsernameWindow::ReturnCode status = VisItChangeUsernameWindow::UW_Rejected;
    QString name = win->getUsername(host.c_str(), status);

#if !defined(_WIN32)
    GetViewerMessaging()->BlockClientInput(false);
#endif

    if (status == VisItChangeUsernameWindow::UW_Accepted)
    {
        // Accepted; hit return or Okay.
        newUsername = std::string(name.toStdString());
        if (newUsername.empty())
            return false;

        MachineProfile *mp = profiles->GetMachineProfileForHost(host);
        if (mp != NULL)
            mp->SetUserName(newUsername);

        profiles->SelectAll();
        profiles->Notify();

        return true;
    }

    // Rejected or cancelled.  
    return false;
}

// ****************************************************************************
// Method: ViewerChangeUsernameUI::ChangeUsernameCallback
//
// Purpose:
//   Changes the username by poking the dialog results into the host profiles.
//
// Arguments:
//   host        : The host for which we're changing the username.
//   newUsername : The new user name.
//
// Returns:    True on success; False on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 11 15:02:20 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
ViewerChangeUsernameUI::ChangeUsernameCallback(const std::string &host,
     std::string &newUsername)
{
    ViewerChangeUsernameUI *inst = new ViewerChangeUsernameUI;
    bool retval = inst->ChangeUsername(host, newUsername);
    delete inst;
    return retval;
}
