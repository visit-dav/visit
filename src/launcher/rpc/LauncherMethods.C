#include <LauncherMethods.h>

#include <LauncherState.h>
#include <RemoteProcess.h>
#include <Utility.h>

// ****************************************************************************
//  Method: MDServerMethods::Constructor
//
//  Purpose:
//
//  Programmer: Jeremy Meredith
//  Creation:   August 18, 2000
//
//  Modifications:
//
// ****************************************************************************
LauncherMethods::LauncherMethods(LauncherState *_state)
{
    process = 0;
    state = _state;
}

// ****************************************************************************
//  Method: MDServerMethods::Destructor
//
//  Purpose:
//
//  Programmer: Jeremy Meredith
//  Creation:   August 18, 2000
//
//  Modifications:
//
// ****************************************************************************
LauncherMethods::~LauncherMethods()
{}

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
//    Jeremy Meredith, Thu May 24 10:27:55 EDT 2007
//    When sending a launch command, if we're doing ssh port tunneling,
//    map the local host/port to the remote one.
//
//    Thomas R. Treadway, Mon Oct  8 13:27:42 PDT 2007
//    Backing out SSH tunneling on Panther (MacOS X 10.3)
//
// ****************************************************************************

void
LauncherMethods::LaunchProcess(const stringVector &origProgramArgs)
{
    stringVector programArgs(origProgramArgs);

    if ( process && !process->GetPortTunnelMap().empty())
    {
        // If we're doing ssh tunneling, map the local host/port to the
        // remote one.
        bool success = ConvertArgsToTunneledValues(process->GetPortTunnelMap(),
                                                   programArgs);
        if (!success)
        {
            EXCEPTION1(VisItException, "Launcher needed to tunnel to a local "
                       "port that wasn't in the port map.  The number of "
                       "tunneled ports may need to be increased.");
        }
    }

    state->launchRPC(programArgs);
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
LauncherMethods::ConnectSimulation(const stringVector &programArgs,
                                 const std::string &simHost, int simPort,
                                 const std::string &simSecurityKey)
{
    state->connectSimRPC(programArgs, simHost, simPort, simSecurityKey);
}

void
LauncherMethods::SetRemoteProcess(RemoteProcess *_process)
{
    process = _process;
}
