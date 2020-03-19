// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef LAUNCHER_STATE_H
#define LAUNCHER_STATE_H

#include <vclrpc_exports.h>

#include <LaunchRPC.h>
#include <ConnectSimRPC.h>

class Xfer;
class VisItRPC;

class LAUNCHER_RPC_API LauncherState
{
public:
    LauncherState();
    ~LauncherState();
    size_t                     GetNumStateObjects();
    VisItRPC*                  GetStateObject(int i);

    void                       SetupComponentRPCs(Xfer* xfer);

    LaunchRPC&                 GetLaunchRPC() { return launchRPC; }
    ConnectSimRPC&             GetConnectSimRPC() { return connectSimRPC; }

private:
    LaunchRPC     launchRPC;
    ConnectSimRPC connectSimRPC;

    std::vector<VisItRPC*> objVector;
    friend class LauncherMethods;
};

#endif
