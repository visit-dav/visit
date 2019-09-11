// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <SetMFileOpenOptionsRPCExecutor.h>
#include <SetMFileOpenOptionsRPC.h>
#include <DebugStream.h>
#include <MDServerConnection.h>

// ****************************************************************************
// Method:
//  SetMFileOpenOptionsRPCExecutor::SetMFileOpenOptionsRPCExecutor
//
// Purpose: 
//   Constructor for the SetMFileOpenOptionsRPCExecutor class.
//
// Arguments:
//   p       : A pointer to the object that created this RPC executor.
//   s       : A pointer to the rpc that invokes this executor.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2000
//
// Modifications:
//   
// ****************************************************************************

SetMFileOpenOptionsRPCExecutor::SetMFileOpenOptionsRPCExecutor(
    MDServerConnection *p, Subject *s) : Observer(s)
{
    parent = p;
}

// ****************************************************************************
// Method:
//  SetMFileOpenOptionsRPCExecutor::~SetMFileOpenOptionsRPCExecutor
//
// Purpose: 
//   Destructor for the SetMFileOpenOptionsRPCExecutor class.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2000
//
// Modifications:
//   
// ****************************************************************************

SetMFileOpenOptionsRPCExecutor::~SetMFileOpenOptionsRPCExecutor()
{
}

// ****************************************************************************
// Method: SetMFileOpenOptionsRPCExecutor::Update
//
// Purpose: 
//   Performs the work of the RPC executor which, in this case, is to change
//   the current directory.
//
// Arguments:
//   s : A pointer to the SetMFileOpenOptionsRPC that caused this method to
//       be called.
//
// Programmer: Jeremy Meredith
// Creation:   January 18, 2008
//
// Modifications:
//
// ****************************************************************************

void
SetMFileOpenOptionsRPCExecutor::Update(Subject *s)
{
    SetMFileOpenOptionsRPC *rpc = (SetMFileOpenOptionsRPC *)s;
#ifdef DEBUG
    debug2 << "SetMFileOpenOptionsRPCExecutor::Update" << endl;
#endif
    // Either send a successful reply or send an error.
    parent->SetDefaultFileOpenOptions(rpc->GetFileOpenOptions());
    rpc->SendReply();
}
