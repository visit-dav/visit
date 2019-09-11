// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
