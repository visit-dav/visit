// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef LAUNCHER_PROXY_H
#define LAUNCHER_PROXY_H
#include <visit-config.h>
#include <vclproxy_exports.h>
#include <RemoteProxyBase.h>
#include <LauncherState.h>
#include <LauncherMethods.h>
#include <map>

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
//    Jeremy Meredith, Wed May 11 09:04:52 PDT 2005
//    Added security key to simulation connection.
//
//    Jeremy Meredith, Thu May 24 10:21:48 EDT 2007
//    Added method to retrieve the SSH tunneling local-to-remote port map.
//
//    Thomas R. Treadway, Mon Oct  8 13:27:42 PDT 2007
//    Backing out SSH tunneling on Panther (MacOS X 10.3)
//
// ****************************************************************************

class LAUNCHER_PROXY_API LauncherProxy : public RemoteProxyBase
{
public:
    LauncherProxy();
    virtual ~LauncherProxy();

    virtual std::string GetComponentName() const;

    virtual void Create(const MachineProfile &profile, 
                        ConnectCallback *connectCallback = 0, 
                        void *connectCallbackData = 0,
                        bool createAsThoughLocal = false);

    std::map<int,int> GetPortTunnelMap();

    LauncherMethods *GetLauncherMethods() { return methods; }
    LauncherState   *GetLauncherState() { return state; }

protected:
    virtual void SetupComponentRPCs();
private:
    LauncherMethods *methods;
    LauncherState   *state;
};

#endif
