// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef KEEP_ALIVE_RPC_EXECUTOR_H
#define KEEP_ALIVE_RPC_EXECUTOR_H
#include <Observer.h>

// ****************************************************************************
// Class: KeepAliveRPCExecutor
//
// Purpose:
//   This class handles the keep alive RPC request.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 12 10:21:21 PDT 2004
//
// Modifications:
//
// ****************************************************************************

class KeepAliveRPCExecutor : public Observer
{
public:
    KeepAliveRPCExecutor(Subject *s);
    virtual ~KeepAliveRPCExecutor();

    virtual void Update(Subject *s);
};

#endif
