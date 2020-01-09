// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef KEEP_ALIVE_RPC_H
#define KEEP_ALIVE_RPC_H
#include <state_exports.h>
#include <VisItRPC.h>

// ****************************************************************************
// Class: KeepAliveRPC
//
// Purpose:
//   This is an RPC that is sent periodically to keep the sockets to remote
//   computers alive.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 11 12:32:22 PDT 2004
//
// Modifications:
//   Brad Whitlock, Fri Dec  7 11:55:07 PST 2007
//   Added TypeName override.
//
// ****************************************************************************

class STATE_API KeepAliveRPC : public BlockingRPC
{
    using BlockingRPC::SetValue;
    using BlockingRPC::GetValue;
public:
    KeepAliveRPC();
    virtual ~KeepAliveRPC();

    // Invokation method
    void operator()();

    // Property selection methods
    virtual void SelectAll();

    // Property setting methods
    void SetValue(int v);

    // Property getting methods
    int GetValue() const;

    virtual const std::string TypeName() const;
private:
    int value;
};

#endif
