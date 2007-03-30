#ifndef LAUNCHER_PROXY_H
#define LAUNCHER_PROXY_H
#include <vclproxy_exports.h>
#include <RemoteProxyBase.h>
#include <LaunchRPC.h>

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

protected:
    virtual void SetupComponentRPCs();
private:
    LaunchRPC launchRPC;
};

#endif
