// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEWER_CHANGE_USERNAME_H
#define VIEWER_CHANGE_USERNAME_H
#include <viewercore_exports.h>
#include <ViewerBase.h>
#include <string>

// ****************************************************************************
// Class: ViewerChangeUsername
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

class VIEWERCORE_API ViewerChangeUsername : public ViewerBase
{
public:
    ViewerChangeUsername();
    virtual ~ViewerChangeUsername();

    virtual bool ChangeUsername(const std::string &host, std::string &newUsername);
};

#endif
