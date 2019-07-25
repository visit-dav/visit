// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <KeepAliveRPCExecutor.h>
#include <KeepAliveRPC.h>
#include <DebugStream.h>

// ****************************************************************************
// Method: KeepAliveRPCExecutor::KeepAliveRPCExecutor
//
// Purpose: 
//   Constructor for the KeepAliveRPCExecutor class.
//
// Arguments:
//   s : A pointer to the KeepAliveRPC object that this executor observes.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 12 10:22:57 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

KeepAliveRPCExecutor::KeepAliveRPCExecutor(Subject *s) : Observer(s)
{
}

// ****************************************************************************
// Method: KeepAliveRPCExecutor::~KeepAliveRPCExecutor
//
// Purpose: 
//   Destructor for the KeepAliveRPCExecutor class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 12 10:22:57 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

KeepAliveRPCExecutor::~KeepAliveRPCExecutor()
{
}

// ****************************************************************************
// Method: KeepAliveRPCExecutor::Update
//
// Purpose: 
//   Does the work for the KeepAliveRPC.
//
// Arguments:
//   s : A pointer to the KeepAliveRPC that caused this method to be called.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 12 10:22:57 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
KeepAliveRPCExecutor::Update(Subject *s)
{
    debug3 << "KeepAliveRPCExecutor::Update" << endl;
    KeepAliveRPC *keepAlive = (KeepAliveRPC *)s;
    keepAlive->SendReply();
}
