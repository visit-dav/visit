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
//   
// ****************************************************************************

void
LauncherProxy::SetupComponentRPCs()
{
    xfer.Add(&launchRPC);
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

