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
//
// ****************************************************************************

class STATE_API KeepAliveRPC : public BlockingRPC
{
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

private:
    int value;
};

#endif
