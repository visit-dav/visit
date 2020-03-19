// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <RestoreSessionRPCExecutor.h>
#include <MDServerConnection.h>

// ****************************************************************************
// Method: RestoreSessionRPCExecutor::RestoreSessionRPCExecutor
//
// Purpose: 
//   Constructor for the RestoreSessionRPCExecutor class.
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

RestoreSessionRPCExecutor::RestoreSessionRPCExecutor(
    MDServerConnection *parent_, Subject *s) : Observer(s)
{
    parent = parent_;
}

// ****************************************************************************
// Method: RestoreSessionRPCExecutor::~RestoreSessionRPCExecutor
//
// Purpose: 
//   Destructor for the RestoreSessionRPCExecutor class.
//
// Programmer: David Camp
// Creation:   Tue Jul  7 07:56:12 PDT 2015
//
// Modifications:
//   
// ****************************************************************************

RestoreSessionRPCExecutor::~RestoreSessionRPCExecutor()
{
}

// ****************************************************************************
// Method: RestoreSessionRPCExecutor::Update
//
// Purpose: 
//   Restore session file on remote server.
//
// Arguments:
//   s : A pointer to the RestoreSessionRPC that caused this method to be called.
//
// Programmer: David Camp
// Creation:   Tue Jul  7 07:56:12 PDT 2015
//
// Modifications:
//
// ****************************************************************************

void
RestoreSessionRPCExecutor::Update(Subject *s)
{
    RestoreSessionRPC *rpc = (RestoreSessionRPC *)s;

    // If RestoreSessionFile fails it will send an error message, so don't reply here.
    rpc->RestoreSessionFile();
}

