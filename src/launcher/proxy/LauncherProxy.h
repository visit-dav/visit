#ifndef LAUNCHER_PROXY_H
#define LAUNCHER_PROXY_H
#include <vclproxy_exports.h>
#include <RemoteProxyBase.h>
#include <LaunchRPC.h>
#include <ConnectSimRPC.h>

// ****************************************************************************
// Class: LauncherProxy
//
// Purpose:
//   This is a proxy class for the launcher program.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 16:11:43 PST 2003
//
// Modifications:
//    Jeremy Meredith, Tue Mar 30 10:08:18 PST 2004
//    I added support for simulations.
//
// ****************************************************************************

class LAUNCHER_PROXY_API LauncherProxy : public RemoteProxyBase
{
public:
    LauncherProxy();
    virtual ~LauncherProxy();

    virtual std::string GetComponentName() const;

    // RPCs to access functionality on the visit component launcher.
    void LaunchProcess(const stringVector &programArgs);
    void ConnectSimulation(const stringVector &programArgs,
                           const std::string &simHost, int simPort);

protected:
    virtual void SetupComponentRPCs();
private:
    LaunchRPC     launchRPC;
    ConnectSimRPC connectSimRPC;
};

#endif
