// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <ViewerChangeUsername.h>

ViewerChangeUsername::ViewerChangeUsername() : ViewerBase()
{
}

ViewerChangeUsername::~ViewerChangeUsername()
{
}

bool
ViewerChangeUsername::ChangeUsername(const std::string &host, std::string &newUsername)
{
    newUsername = std::string();
    return false;
}
