#include <LauncherProxy.h>

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
//   
// ****************************************************************************

LauncherProxy::LauncherProxy() : RemoteProxyBase("-vcl")
{
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
    xfer.Add(&launchRPC);
    xfer.Add(&connectSimRPC);
}

// ****************************************************************************
// Method: LauncherProxy::LaunchProcess
//
// Purpose: 
//   This method tells the VisIt component launcher to launch a program
//   that connects directly back to the component that launched the launcher.
//
// Arguments:
//   programArgs : The program arguments.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 16:41:13 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
LauncherProxy::LaunchProcess(const stringVector &programArgs)
{
    launchRPC(programArgs);
}

// ****************************************************************************
//  Method:  LauncherProxy::ConnectSimulation
//
//  Purpose:
//    Connect to a running simulation
//
//  Arguments:
//    programArgs : The program arguments.
//    simHost     : The host where the simulation is running
//    simPort     : The port where the simulation is listening
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 23, 2004
//
//  Modifications:
//    Jeremy Meredith, Wed May 11 09:04:52 PDT 2005
//    Added security key to simulation connection.
//
// ****************************************************************************

void
LauncherProxy::ConnectSimulation(const stringVector &programArgs,
                                 const std::string &simHost, int simPort,
                                 const std::string &simSecurityKey)
{
    connectSimRPC(programArgs, simHost, simPort, simSecurityKey);
}
