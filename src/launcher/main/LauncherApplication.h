#ifndef LAUNCHER_APPLICATION_H
#define LAUNCHER_APPLICATION_H
#include <ParentProcess.h>
#include <Xfer.h>
#include <QuitRPC.h>
#include <KeepAliveRPC.h>
#include <LaunchRPC.h>
#include <ConnectSimRPC.h>
#include <RPCExecutor.h>
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
    void TerminateConnectionRequest(int, char **);

    static void AlarmHandler(int);
    static void DeadChildHandler(int);
private:
    static LauncherApplication *instance;
    static std::map<int, bool>  childDied;

    ParentProcess               parent;
    Xfer                        xfer;

    QuitRPC                     quitRPC;
    KeepAliveRPC                keepAliveRPC;
    LaunchRPC                   launchRPC;
    ConnectSimRPC               connectSimRPC;

    RPCExecutor<QuitRPC>       *quitExecutor;
    RPCExecutor<KeepAliveRPC>  *keepAliveExecutor;
    RPCExecutor<LaunchRPC>     *launchExecutor;
    RPCExecutor<ConnectSimRPC> *connectSimExecutor;

    bool                        keepGoing;
    int                         timeout;
};

#endif
