// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef LAUNCHER_APPLICATION_H
#define LAUNCHER_APPLICATION_H
#include <visit-config.h>
#include <ParentProcess.h>
#include <Xfer.h>
#include <QuitRPC.h>
#include <KeepAliveRPC.h>
#include <LaunchRPC.h>
#include <ConnectSimRPC.h>
#include <RPCExecutor.h>
#include <LaunchService.h>
#include <LauncherState.h>
#include <map>

// ****************************************************************************
// Class: LauncherApplication
//
// Purpose:
//   This class contains the launcher application that is responsible for
//   launching VisIt components.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 16:55:28 PST 2003
//
// Modifications:
//   Brad Whitlock, Fri Mar 12 10:35:29 PDT 2004
//   I added KeepAliveRPC.
//
//   Jeremy Meredith, Tue Mar 30 10:06:33 PST 2004
//   I added support for connecting to running simulations.
//
//   Jeremy Meredith, Mon May  9 16:09:06 PDT 2005
//   I added security protocols for simulations.
//
//   Jeremy Meredith, Thu May 24 12:31:29 EDT 2007
//   Added support for SSH tunneling.  Specifically, added a bridge
//   so that the login node could act as a gateway between the SSH 
//   tunnel origination (on the login node) and the compute node
//   where a parallel engine might run.
//
//   Thomas R. Treadway, Mon Oct  8 13:27:42 PDT 2007
//   Backout SSH tunneling changes for Panther (MacOS X 10.3)
//
//   Brad Whitlock, Wed Nov 21 10:34:14 PST 2007
//   Added support for forwarding child process output to the client.
//
//   Brad Whitlock, Mon Apr 27 16:25:34 PST 2009
//   I changed the code so we can tunnel simulation data connections.
//
//   Brad Whitlock, Tue Nov 29 11:23:39 PST 2011
//   I moved some functionality into LaunchService.
//
// ****************************************************************************

class LauncherApplication
{
public:
    static LauncherApplication *Instance();
    virtual ~LauncherApplication();
    void Execute(int *argc, char **argv[]);
    void LaunchProcess(const stringVector &launchArgs);
    void ConnectSimulation(const stringVector &launchArgs,
                           const std::string &simHost, int simPort,
                           const std::string &simSecurityKey);

protected:
    LauncherApplication();
    void ProcessArguments(int *argc, char **argv[]);
    void Connect(int *argc, char **argv[]);
    void MainLoop();
    bool ProcessInput();
    void TurnOnAlarm();
    void TurnOffAlarm();

    static void AlarmHandler(int);
private:
    static LauncherApplication *instance;

    ParentProcess               parent;
    Xfer                        xfer;

    QuitRPC                     quitRPC;
    KeepAliveRPC                keepAliveRPC;
    LauncherState               launcherstate;
    RPCExecutor<QuitRPC>       *quitExecutor;
    RPCExecutor<KeepAliveRPC>  *keepAliveExecutor;
    RPCExecutor<LaunchRPC>     *launchExecutor;
    RPCExecutor<ConnectSimRPC> *connectSimExecutor;

    bool                        useSSHTunneling;
    bool                        keepGoing;
    bool                        fixedBufferMode;
    int                         timeout;
    std::vector<Connection*>    childOutput;
    LaunchService               launch;
};

#endif
