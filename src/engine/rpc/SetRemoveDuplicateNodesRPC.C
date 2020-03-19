// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <SetRemoveDuplicateNodesRPC.h>
#include <DebugStream.h>

using std::string;

// *******************************************************************
// Constructor: SetRemoveDuplicateNodesRPC::SetRemoveDuplicateNodesRPC
//
// Purpose:
//   This is the constructor.
//
// Programmer: Kathleen Biagas
// Creation:   December 22, 2014
//
// Modifications:
//
// *******************************************************************

SetRemoveDuplicateNodesRPC::SetRemoveDuplicateNodesRPC()
    : BlockingRPC("b",NULL)
{
}


// *******************************************************************
// Denstructor: SetRemoveDuplicateNodesRPC::~SetRemoveDuplicateNodesRPC
//
// Purpose:
//   This is the denstructor.
//
// Programmer: Kathleen Biagas
// Creation:   December 22, 2014
//
// Modifications:
//
// *******************************************************************

SetRemoveDuplicateNodesRPC::~SetRemoveDuplicateNodesRPC()
{
}


// *******************************************************************
// Method: SetRemoveDuplicateNodesRPC::operator()
//
// Purpose:
//   This is the RPC's invocation method.
//
// Arguments:
//   pType : This is the new precision type.
//
// Programmer: Kathleen Biagas
// Creation:   December 22, 2014
//
// Modifications:
//
// *******************************************************************

void
SetRemoveDuplicateNodesRPC::operator()(bool flag)
{
    debug3 << "Executing SetRemoveDuplicateNodesRPC\n";

    SetRemoveDuplicateNodes(flag);
    Execute();
}


// *******************************************************************
// Method: SetRemoveDuplicateNodesRPC::SelectAll
//
// Purpose:
//   Select all attributes for sending as parameters.
//
// Programmer: Kathleen Biagas
// Creation:   December 22, 2014
//
// Modifications:
//
// *******************************************************************

void
SetRemoveDuplicateNodesRPC::SelectAll()
{
    Select(0, (void*)&removeDuplicateNodes);
}


// *******************************************************************
// Method: SetRemoveDuplicateNodesRPC::SetRemoveDuplicateNodes
//
// Purpose:
//   Set the removeDuplicateNodes flag.
//
// Programmer: Kathleen Biagas
// Creation:   December 22, 2014
//
// Modifications:
//
// *******************************************************************

void
SetRemoveDuplicateNodesRPC::SetRemoveDuplicateNodes(bool flag)
{
    removeDuplicateNodes = flag;
}

// *******************************************************************
// Method: SetRemoveDuplicateNodesRPC::GetRemoveDuplicateNodes
//
// Purpose:
//   Get the removeDuplicateNodes flag.
//
// Programmer: Kathleen Biagas
// Creation:   December 22, 2014
//
// Modifications:
//
// *******************************************************************

bool
SetRemoveDuplicateNodesRPC::GetRemoveDuplicateNodes() const
{
    return removeDuplicateNodes;
}

// ****************************************************************************
// Method: SetRemoveDuplicateNodesRPC::TypeName
//
// Purpose:
//   Returns the name of the RPC.
//
// Programmer: Kathleen Biagas
// Creation:   December 22, 2014
//
// Modifications:
//
// ****************************************************************************

const std::string
SetRemoveDuplicateNodesRPC::TypeName() const
{
    return "SetRemoveDuplicateNodesRPC";
}
