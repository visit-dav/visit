// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.
#ifndef LAUNCH_SERVICE_H
#define LAUNCH_SERVICE_H
#include <comm_exports.h>
#include <map>
#include <vectortypes.h>

class SocketConnection;

// ****************************************************************************
// Class: LaunchService
//
// Purpose:
//   Launches programs.
//
// Notes:      This class can set up an SSH tunnel for the launched program if
//             that needs to be done. This code used to be in VCL.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 28 17:09:16 PST 2011
//
// Modifications:
//   
// ****************************************************************************

class COMM_API LaunchService
{
public:
    LaunchService();
    ~LaunchService();

    void Launch(const stringVector &launchArgs, bool doBridge=false, 
                SocketConnection **conn = NULL);

    bool SetupGatewaySocketBridgeIfNeeded(stringVector &launchArgs);
private:
    void TerminateConnectionRequest(int, char **);
    
    static void DeadChildHandler(int);
    static std::map<int, bool>  childDied;
};

#endif
