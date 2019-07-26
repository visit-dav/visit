// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEWER_CHANGE_USERNAME_UI_H
#define VIEWER_CHANGE_USERNAME_UI_H
#include <viewer_exports.h>
#include <ViewerChangeUsername.h>
#include <string>

class VisItChangeUsernameWindow;

// ****************************************************************************
// Class: ViewerChangeUsernameUI
//
// Purpose:
//   Lets the user change the username.
//
// Notes:    
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 11 14:42:36 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class VIEWER_API ViewerChangeUsernameUI : public ViewerChangeUsername
{
public:
    ViewerChangeUsernameUI();
    virtual ~ViewerChangeUsernameUI();

    virtual bool ChangeUsername(const std::string &host, std::string &newUsername);

    // Static function that can be used as a RemoteProcess callback.
    static bool ChangeUsernameCallback(const std::string &host, std::string &newUsername);

private:
    VisItChangeUsernameWindow *win;
};

#endif
