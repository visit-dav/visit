#ifndef LAUNCHER_APPLICATION_H
#define LAUNCHER_APPLICATION_H
#include <ParentProcess.h>
#include <Xfer.h>
#include <QuitRPC.h>
#include <LaunchRPC.h>
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
//   
// ****************************************************************************

class LauncherApplication
{
public:
    static LauncherApplication *Instance();
    virtual ~LauncherApplication();
    void Execute(int *argc, char **argv[]);
    void LaunchProcess(const stringVector &launchArgs);

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

    ParentProcess           parent;
    Xfer                    xfer;

    QuitRPC                 quitRPC;
    LaunchRPC               launchRPC;

    RPCExecutor<QuitRPC>   *quitExecutor;
    RPCExecutor<LaunchRPC> *launchExecutor;

    bool                    keepGoing;
    int                     timeout;
};

#endif
