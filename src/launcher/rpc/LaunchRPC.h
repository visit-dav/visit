#ifndef LAUNCH_RPC_H
#define LAUNCH_RPC_H
#include <vclrpc_exports.h>
#include <VisItRPC.h>
#include <vectortypes.h>

// ****************************************************************************
// Class: LaunchRPC
//
// Purpose:
//   This class encodes an RPC that tells the launcher to launch a process.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 16:17:26 PST 2003
//
// Modifications:
//
// ****************************************************************************

class LAUNCHER_RPC_API LaunchRPC : public BlockingRPC
{
public:
    LaunchRPC();
    virtual ~LaunchRPC();

    // Invokation method
    void operator()(const stringVector &args);

    // Property selection methods
    virtual void SelectAll();

    // Methods to access private data.
    const stringVector &GetLaunchArgs() const;
private:
    stringVector launchArgs;
};


#endif
