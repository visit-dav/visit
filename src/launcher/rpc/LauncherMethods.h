// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              LauncherMethods.h                            //
// ************************************************************************* //

#ifndef LAUNCHER_METHODS_H
#define LAUNCHER_METHODS_H


#include <vclrpc_exports.h>
#include <string>
#include <maptypes.h>
#include <vectortypes.h>

class LauncherState;
class RemoteProcess;

class LAUNCHER_RPC_API LauncherMethods
{
public:
    LauncherMethods(LauncherState*);
    ~LauncherMethods();


    // RPCs to access functionality on the visit component launcher.
    void LaunchProcess(const stringVector &programArgs);
    void ConnectSimulation(const stringVector &programArgs,
                           const std::string &simHost, int simPort,
                           const std::string &simSecurityKey);
    void SetRemoteProcess(RemoteProcess*);
private:
    LauncherState* state;
    RemoteProcess* process;
};
#endif


