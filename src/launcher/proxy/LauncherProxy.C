// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <visit-config.h>
#include <LauncherProxy.h>
#include <RemoteProcess.h>
#include <Utility.h>

// ****************************************************************************
// Method: LauncherProxy::LauncherProxy
//
// Purpose: 
//   Constructor for the LauncherProxy class.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 16:38:42 PST 2003
//
// Modifications:
//   Brad Whitlock, Wed Nov 21 13:25:27 PST 2007
//   Added a second write socket.
//
// ****************************************************************************

LauncherProxy::LauncherProxy() : RemoteProxyBase("-vcl")
{
    // Indicate that we want 2 write sockets from VCL.
    nWrite = 2;
    state = new LauncherState();
    methods = new LauncherMethods(state);
}

// ****************************************************************************
// Method: LauncherProxy::~LauncherProxy
//
// Purpose: 
//   Destructor for the LauncherProxy class.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 16:39:00 PST 2003
//
// Modifications:
//   
// ****************************************************************************

LauncherProxy::~LauncherProxy()
{
    delete state;
    delete methods;
}

// ****************************************************************************
// Method: LauncherProxy::GetComponentName
//
// Purpose: 
//   Returns the name of the component.
//
// Returns:    The component name.
//
// Programmer: Brad Whitlock
// Creation:   Tue May 6 10:58:04 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

std::string
LauncherProxy::GetComponentName() const
{
    return "component launcher";
}

// ****************************************************************************
// Method: LauncherProxy::Create
//
// Purpose: 
//   Create the engine.
//
// Arguments:
//   profile             : The profile to use when launching the engine.
//   connectCallback     : A callback function.
//   connectCallbackData : Data for the callback function.
//   createAsThoughLocal : Whether to create local arguments.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 15 11:21:21 PDT 2012
//
// Modifications:
//   
// ****************************************************************************

void
LauncherProxy::Create(const MachineProfile &profile, 
    ConnectCallback *connectCallback, void *connectCallbackData,
    bool createAsThoughLocal)
{
    RemoteProxyBase::Create(profile, 
        connectCallback, connectCallbackData, createAsThoughLocal);
    methods->SetRemoteProcess(component);
}

// ****************************************************************************
// Method: LauncherProxy::SetupComponentRPCs
//
// Purpose: 
//   Sets up the RPC's used by this proxy.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 16:40:45 PST 2003
//
// Modifications:
//    Jeremy Meredith, Tue Mar 30 10:07:06 PST 2004
//    I added connectSimRPC.
//
// ****************************************************************************

void
LauncherProxy::SetupComponentRPCs()
{
    state->SetupComponentRPCs(&xfer);
}

// ****************************************************************************
//  Method:  LauncherProxy::GetPortTunnelMap
//
//  Purpose:
//    Retrieve the SSH tunneling local-to-remote port map from the
//    remote process object.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    May 24, 2007
//
//  Modifications:
//    Thomas R. Treadway, Mon Oct  8 13:27:42 PDT 2007
//    Backing out SSH tunneling on Panther (MacOS X 10.3) 
//   
// ****************************************************************************

std::map<int,int>
LauncherProxy::GetPortTunnelMap()
{
    return component->GetPortTunnelMap();
}
