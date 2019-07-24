// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <SaveSessionRPCExecutor.h>
#include <MDServerConnection.h>

// ****************************************************************************
// Method: SaveSessionRPCExecutor::SaveSessionRPCExecutor
//
// Purpose: 
//   Constructor for the SaveSessionRPCExecutor class.
//
// Arguments:
//   parent_ : A pointer to the object that created this RPC executor.
//   s       : A pointer to the rpc that invokes this executor.
//
// Programmer: David Camp
// Creation:   Tue Jul  7 07:56:12 PDT 2015
//
// Modifications:
//   
// ****************************************************************************

SaveSessionRPCExecutor::SaveSessionRPCExecutor(
    MDServerConnection *parent_, Subject *s) : Observer(s)
{
    parent = parent_;
}

// ****************************************************************************
// Method: SaveSessionRPCExecutor::~SaveSessionRPCExecutor
//
// Purpose: 
//   Destructor for the SaveSessionRPCExecutor class.
//
// Programmer: David Camp
// Creation:   Tue Jul  7 07:56:12 PDT 2015
//
// Modifications:
//   
// ****************************************************************************

SaveSessionRPCExecutor::~SaveSessionRPCExecutor()
{
}

// ****************************************************************************
// Method: SaveSessionRPCExecutor::Update
//
// Purpose: 
//   Save session file on remote server.
//
// Arguments:
//   s : A pointer to the SaveSessionRPC that caused this method to be called.
//
// Programmer: David Camp
// Creation:   Tue Jul  7 07:56:12 PDT 2015
//
// Modifications:
//
// ****************************************************************************

void
SaveSessionRPCExecutor::Update(Subject *s)
{
    SaveSessionRPC *rpc = (SaveSessionRPC *)s;

    // If SaveSessionFile fails it will send an error message, so don't reply.
    if(rpc->SaveSessionFile())
    {
        rpc->SendReply();
    }
}

