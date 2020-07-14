// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef LAUNCH_RPC_H
#define LAUNCH_RPC_H
#include <state_exports.h>
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
//   Brad Whitlock, Fri Dec  7 13:28:21 PST 2007
//   Added TypeName override.
//
// ****************************************************************************

class STATE_API LaunchRPC : public BlockingRPC
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

    virtual const std::string TypeName() const;
private:
    stringVector launchArgs;
};


#endif
