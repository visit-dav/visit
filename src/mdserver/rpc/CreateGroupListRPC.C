// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <CreateGroupListRPC.h>

#include <string>
#include <vector>

// ****************************************************************************
// Method: CreateGroupListRPC::CreateGroupListRPC
//
// Purpose: 
//   Constructor for the CreateGroupListRPC class.
//
// Programmer: Sean Ahern
// Creation:   Tue Feb 13 14:46:58 PST 2001
//
// Modifications:
//   
// ****************************************************************************

CreateGroupListRPC::CreateGroupListRPC() : NonBlockingRPC("ss*")
{
}

// ****************************************************************************
// Method: CreateGroupListRPC::operator()
//
// Purpose: 
//   This is the invokation method for this function object. It copies the
//   arguments into local attributes and calls RPC's Execute method.
//
// Arguments:
//   f  : The filename of the group list to create
//   gl : The group list itself
//
// Returns:    
//
// Note:       
//
// Programmer: Sean Ahern
// Creation:   Tue Feb 13 14:47:54 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
CreateGroupListRPC::operator()(const std::string f, std::vector<std::string> gl)
{
    filename = f;
    groupList = gl;

    Execute();
}

// ****************************************************************************
// Method: CreateGroupListRPC::SelectAll
//
// Purpose: 
//   Gets the addresses of the local attributes so the attributes can be
//   transmitted over the network as AttributeGroups.
//
// Programmer: Sean Ahern
// Creation:   Tue Feb 13 15:22:24 PST 2001
//
// Modifications:
//   
// ****************************************************************************
void
CreateGroupListRPC::SelectAll()
{
    Select(0, (void *)&filename);
    Select(1, (void *)&groupList);
}

// ****************************************************************************
// Method: CreateGroupListRPC::TypeName
//
// Purpose: 
//   Returns the RPC name.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec  7 11:09:23 PST 2007
//
// Modifications:
//   
// ****************************************************************************

const std::string
CreateGroupListRPC::TypeName() const
{
    return "CreateGroupListRPC";
}
