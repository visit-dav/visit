// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <ViewerServerManager.h>

#include <visit-config.h>

#include <CancelledConnectException.h>
#include <Connection.h>
#include <CouldNotConnectException.h>
#include <HostProfileList.h>
#include <LauncherProxy.h>
#include <LostConnectionException.h>
#include <MachineProfile.h>
#include <RemoteProxyBase.h>

#include <ViewerConnectionPrinter.h>
#include <ViewerConnectionProgress.h>
#include <ViewerFactory.h>
#include <ViewerMessaging.h>
#include <ViewerProperties.h>
#include <ViewerRemoteProcessChooser.h>
#include <ViewerState.h>
#include <ViewerText.h>

#include <DebugStream.h>
#include <avtCallback.h>
#include <Utility.h>

#include <map>
#include <string>

//
// Static members.
//
std::string ViewerServerManager::localHost("localhost");
stringVector ViewerServerManager::arguments;
ViewerServerManager::LauncherMap ViewerServerManager::launchers;
void * ViewerServerManager::cbData[2] = {0,0};
void (*ViewerServerManager::OpenWithEngineCB)(const std::string &, 
                                              const stringVector &args,
                                              void *) = NULL;
void *ViewerServerManager::OpenWithEngineCBData = NULL;
bool (*ViewerServerManager::LaunchProgressCB)(void *, int) = NULL;
void *ViewerServerManager::LaunchProgressCBData = NULL;

// ****************************************************************************
// Method: ViewerServerManager::ViewerServerManager
//
// Purpose: 
//   This is the constructor for the ViewerServerManager class.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 3 16:23:16 PST 2002
//
// Modifications:
//   Brad Whitlock, Mon Feb 12 17:47:27 PST 2007
//   Added ViewerBase base class.
//
// ****************************************************************************

ViewerServerManager::ViewerServerManager() : ViewerBase()
{
}

// ****************************************************************************
// Method: ViewerServerManager::ViewerServerManager
//
// Purpose: 
//   This is the destructor for the ViewerServerManager class.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 3 16:23:16 PST 2002
//
// Modifications:
//   
// ****************************************************************************

ViewerServerManager::~ViewerServerManager()
{
}

// ****************************************************************************
// Method: ViewerFileServer::SetArguments
//
// Purpose: 
//   Sets the arguements passed to the mdservers that get launched.
//
// Arguments:
//   args : the arguments
//
// Programmer: Jeremy Meredith
// Creation:   April 19, 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerServerManager::SetArguments(const stringVector &arg)
{
    arguments = arg;
}

// ****************************************************************************
// Method: ViewerServerManager::SetLocalHost
//
// Purpose: 
//   Sets the localHost name used internally to determine which engines are
//   active.
//
// Arguments:
//   hostName : The name of the localhost machine.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 24 14:00:59 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerServerManager::SetLocalHost(const std::string &hostName)
{
    localHost = hostName;
}

// ****************************************************************************
// Method: ViewerServerManager::HostIsLocalHost
//
// Purpose: 
//   Determines if the hostname is localhost.
//
// Arguments:
//   hostName : The hostname to consider.
//
// Programmer: Brad Whitlock
// Creation:   Wed May 7 10:45:42 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerServerManager::HostIsLocalHost(const std::string &hostName)
{
    return hostName == localHost || hostName == "localhost" || hostName == "127.0.0.1";
}

// ****************************************************************************
// Method: ViewerServerManager::RealHostName
//
// Purpose: 
//   Compares the hostName to "localhost" and if they are equal then the
//   real localhost name is returned.
//
// Arguments:
//   hostName : The hostname to check.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 24 14:42:04 PST 2001
//
// Modifications:
//   
// ****************************************************************************

const char *
ViewerServerManager::RealHostName(const char *hostName) const
{
    const char *retval = hostName;
    if(strcmp(hostName, "localhost") == 0)
        retval = localHost.c_str();
    return retval;
}

// ****************************************************************************
// Method: ViewerServerManager::GetMachineProfile
//
// Purpose: 
//   This method finds a host profile that matches the specified hostname
//   and returns it.
//
// Arguments:
//   host : The name of the host.
//
// Returns:    A MachineProfile object for the host.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun  5 16:57:10 PDT 2012
//
// Modifications:
//   
// ****************************************************************************

MachineProfile
ViewerServerManager::GetMachineProfile(const std::string &host) const
{
    const MachineProfile *profile =
         GetViewerState()->GetHostProfileList()->GetMachineProfileForHost(host);
    MachineProfile p2;
    if(profile != NULL)
        p2 = *profile;
    else
        p2 = MachineProfile::Default(host);

    if (GetViewerProperties()->GetForceSSHTunneling())
    {
        p2.SetTunnelSSH(true);
    }

    return p2;
}

// ****************************************************************************
// Method: ViewerServerManager::AddArguments
//
// Purpose: 
//   Adds standard arguments to the remote component.
//
// Arguments:
//   component : The proxy to which we're adding arguments.
//   args      : The arguments to add to the component.
//
// Programmer: Brad Whitlock
// Creation:   Tue May 6 14:01:33 PST 2003
//
// Modifications:
//   
//    Mark C. Miller, Tue Apr 21 14:24:18 PDT 2009
//    Added bufferDebug to control buffering of debug logs.
// ****************************************************************************

void
ViewerServerManager::AddArguments(RemoteProxyBase *component,
    const stringVector &args)
{
    // Add arguments to the mdserver.
    if(GetViewerProperties()->GetDebugLevel() > 0)
    {
        char const *bufferDebug = "b";
        char const *decorateDebug = "d";
        char temp[10];
        snprintf(temp, 10, "%d%s%s",
            GetViewerProperties()->GetDebugLevel(),
            GetViewerProperties()->GetBufferDebug()?bufferDebug:"",
            GetViewerProperties()->GetDecorateDebug()?decorateDebug:"");
        component->AddArgument("-debug");
        component->AddArgument(temp);
    }

    //
    // Add arguments stored in ViewerServerManager.
    //
    for (size_t i = 0; i < arguments.size(); ++i)
         component->AddArgument(arguments[i].c_str());

    //
    // Add any other arguments given to us by the caller
    //
    for (size_t i = 0; i < args.size(); ++i)
         component->AddArgument(args[i].c_str());
}

// ****************************************************************************
// Method: ViewerServerManager::CreateConnectionProgress
//
// Purpose: 
//   Creates a connection progress object that is hooked up to the component
//   that we're launching. This lets us see how things are launched and lets
//   us cancel the launch if we want.
//
// Arguments:
//   component : The component to launch.
//   host      : The host where the component will be run.
//
// Programmer: Brad Whitlock
// Creation:   Tue May 6 14:02:41 PST 2003
//
// Modifications:
//   Brad Whitlock, Sat Sep  6 00:15:33 PDT 2014
//   Use factory.
//
//   Kathleen Biagas, Tue Oct 17, 2023
//   Threads are always required, so removed #ifdef HAVE_THREADS.
//
// ****************************************************************************

ViewerConnectionProgress *
ViewerServerManager::CreateConnectionProgress(const std::string &host)
{
    ViewerConnectionProgress *progress = NULL;
    //
    // Set the engine proxy's progress callback.
    //
    progress = GetViewerFactory()->CreateConnectionProgress();
    if(progress != NULL)
        progress->SetHostName(host);

    return progress;
}

// ****************************************************************************
// Method: ViewerServerManager::SetupConnectionProgress
//
// Purpose: 
//   Hook up the progress object to the component that we're launching. This
//   lets us see how things are launched and lets us cancel the launch if 
//   we want.
//
// Arguments:
//   component : The component we're hooking up.
//   progress  : The progress object that we're initializing.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 29 19:51:10 PST 2011
//
// Modifications:
//    Brad Whitlock, Tue Sep  2 13:01:25 PDT 2014
//    Changed how the callbacks are set up.
//   
//   Kathleen Biagas, Tue Oct 17, 2023
//   Threads are always required, so removed #ifdef HAVE_THREADS.
//
// ****************************************************************************

void
ViewerServerManager::SetupConnectionProgress(RemoteProxyBase *component,
    ViewerConnectionProgress *progress)
{
    if(progress != NULL)
    {
        // Set some properties on the object.
        progress->SetParallel(component->Parallel());
        progress->SetComponentName(component->GetComponentName());
        int timeout = (component->Parallel() ||
                       !HostIsLocalHost(progress->GetHostName())) ? 0 : 4000;
        progress->SetTimeout(timeout);    

        // Install a callback with the component
        cbData[0] = (void *)LaunchProgressCBData;
        cbData[1] = (void *)progress;
        component->SetProgressCallback(LaunchProgressCB, cbData);
    }
}

// ****************************************************************************
// Method: ViewerServerManager::CloseLaunchers
//
// Purpose: 
//   Close all of the launcher programs.
//
// Programmer: Brad Whitlock
// Creation:   Wed May 7 14:00:16 PST 2003
//
// Modifications:
//   Brad Whitlock, Wed Nov 21 15:00:45 PST 2007
//   Changed map storage type.
//
// ****************************************************************************

void
ViewerServerManager::CloseLaunchers()
{
    LauncherMap::iterator pos;
    for(pos = launchers.begin(); pos != launchers.end(); ++pos)
    {
        pos->second.launcher->Close();
        delete pos->second.launcher;
        pos->second.launcher = 0;
        delete pos->second.notifier;
        pos->second.notifier = 0;
    }
}

// ****************************************************************************
// Method: ViewerServerManager::SendKeepAlivesToLaunchers
//
// Purpose: 
//   Sends keep alive signals to the component launchers.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 12 12:02:25 PDT 2004
//
// Modifications:
//   Brad Whitlock, Wed Nov 21 15:01:35 PST 2007
//   Changed map storage type.
//
//   Mark C. Miller, Wed Jun 17 14:27:08 PDT 2009
//   Replaced CATCHALL(...) with CATCHALL.
// ****************************************************************************

void
ViewerServerManager::SendKeepAlivesToLaunchers()
{
    LauncherMap::iterator pos;
    for(pos = launchers.begin(); pos != launchers.end();)
    {
        TRY
        {
            debug2 << "Sending keep alive signal to launcher on "
                   << pos->first.c_str() << endl;
            pos->second.launcher->SendKeepAlive();
            ++pos;
        }
        CATCHALL
        {
            debug2 << "Could not send keep alive signal to launcher on "
                   << pos->first.c_str() << " so that launcher will be closed."
                   << endl;
            delete pos->second.launcher;
            pos->second.launcher = 0;
            delete pos->second.notifier;
            pos->second.notifier = 0;
            launchers.erase(pos++);
        }
        ENDTRY
    }
}

// ****************************************************************************
// Method: ViewerServerManager::StartLauncher
//
// Purpose: 
//   This method starts a launcher process on the specified host.
//
// Arguments:
//   host : The host where we want to run the launcher.
//
// Programmer: Brad Whitlock
// Creation:   Tue May 6 13:59:41 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Jun 10 14:21:38 PST 2003
//   I made it use the visitPath if it is a valid value.
//
//   Jeremy Meredith, Thu Jun 26 10:51:14 PDT 2003
//   Added ability for launcher to start a parallel batch job
//   for itself, the mdserver, and the engine if they are told
//   to share one batch job in the host profile.
//
//   Jeremy Meredith, Thu Oct  9 14:03:11 PDT 2003
//   Added ability to manually specify a client host name or to have it
//   parsed from the SSH_CLIENT (or related) environment variables.  Added
//   ability to specify an SSH port.
//
//   Brad Whitlock, Thu Aug 5 10:52:40 PDT 2004
//   I made it get its profile from the chooser if possible.
//
//   Jeremy Meredith, Thu May 24 10:17:57 EDT 2007
//   Since SSH tunneling is only useful when launching the VCL, we
//   check its actual value and use pass it along.
//
//   Brad Whitlock, Wed Nov 21 14:39:37 PST 2007
//   Added support for reading console output from VCL's children.
//
//   Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//   Split HostProfile int MachineProfile and LaunchProfile.
//
//   Eric Brugger, Mon May  2 17:14:06 PDT 2011
//   I added the ability to use a gateway machine when connecting to a
//   remote host.
//
//   Brad Whitlock, Tue Sep  2 13:02:12 PDT 2014
//   I changed how the launch callback gets set up.
//
// ****************************************************************************

void
ViewerServerManager::StartLauncher(const std::string &host,
     const std::string &visitPath, ViewerConnectionProgress *progress)
{
    if(launchers.find(host) == launchers.end())
    {
        MachineProfile profile;
        const MachineProfile *mp = GetViewerState()->GetHostProfileList()->
            GetMachineProfileForHost(host);
        bool shouldShareBatchJob = false;
        if (mp != 0)
        {
            shouldShareBatchJob = mp->GetShareOneBatchJob();
            profile = *mp;
            profile.SetHost(host);
        }

        if (shouldShareBatchJob)
        {
            ViewerRemoteProcessChooser *chooser =
                ViewerRemoteProcessChooser::Instance();

            chooser->ClearCache(host);

            if (! chooser->SelectProfile(GetViewerState()->GetHostProfileList(),
                                         host, false, profile))
            {
                return;
            }
        }

        if(HostIsLocalHost(host))
        {
            // We don't set up tunnels on localhost.
            profile.SetTunnelSSH(false);

            // We don't use a gateway on localhost.
            profile.SetUseGateway(false);
            profile.SetGatewayHost("");
        }

        // Create a new launcher proxy and add the right arguments to it.
        LauncherProxy *newLauncher = new LauncherProxy;
        stringVector args;
        if(visitPath.size() > 0)
        {
            args.push_back("-dir");
            args.push_back(visitPath);
        }
        AddArguments(newLauncher, args);
        newLauncher->AddProfileArguments(profile, shouldShareBatchJob);

        TRY
        {
            // If we're reusing the progress, set the launcher's progress
            // callback function so that the window pops up.
            if(progress != NULL)
            {
                cbData[0] = (void *)LaunchProgressCBData;
                cbData[1] = (void *)progress;
                newLauncher->SetProgressCallback(LaunchProgressCB, cbData);
                progress->SetIgnoreHide(true);
            }

            //
            // Launch the VisIt component launcher on the specified host.
            //
            newLauncher->Create(profile);
            launchers[host].launcher = newLauncher;

            // Create a socket notifier for the launcher's data socket so
            // we can read remote console output as it is forwarded.
            launchers[host].notifier = GetViewerFactory()->CreateConnectionPrinter();
            launchers[host].notifier->SetConnection(newLauncher->GetWriteConnection(1));

            // Set the progress' information back to the previous values.
            if(progress != NULL)
            {
                progress->SetIgnoreHide(false);
            }
        }
        CATCH(VisItException)
        {
            delete newLauncher;
            RETHROW;
        }
        ENDTRY
    }
}

// ****************************************************************************
// Method: ViewerServerManager::OpenWithLauncher
//
// Purpose: 
//   This method is a callback function that lets a component proxy be launched
//   via a launcher program that we run on a remote machine. This lets us
//   spawn multiple processes remotely without needing multiple passwords.
//
// Arguments:
//   remoteHost : The name of the computer where we want the component to run.
//   args       : The command line to run on the remote machine.
//   data       : Optional callback data.
//
// Programmer: Brad Whitlock
// Creation:   Tue May 6 14:04:41 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Jun 10 14:22:01 PST 2003
//   I made it extract the path to VisIt from the arguments.
//
//   Brad Whitlock, Thu Mar 9 10:35:40 PDT 2006
//   I made it throw a CancelledConnect exception in the event that we're
//   launching on a machine that shares mdserver and engine batch jobs. That
//   way, if the launch was cancelled, we don't crash!
//
//   Brad Whitlock, Wed Nov 21 14:41:54 PST 2007
//   Changed map storage type.
//
// ****************************************************************************

void
ViewerServerManager::OpenWithLauncher(
    const std::string &remoteHost, 
    const stringVector &args, void *data)
{
    bool retry = false;
    int  numAttempts = 0;
    bool launched = false;
    bool cancelled = false;

    do
    {
        TRY
        {
            // We use the data argument to pass in a pointer to the connection
            // progress object.
            ViewerConnectionProgress *progress = (ViewerConnectionProgress *)data;

            // Search the args list and see if we've supplied the path to
            // the visit executable.
            std::string visitPath;
            for(size_t i = 0; i < args.size(); ++i)
            {
                if(args[i] == "-dir" && (i+1) < args.size())
                {
                    visitPath = args[i+1];
                    ++i;
                }
            }

            // Try to start a launcher on remoteHost.
            StartLauncher(remoteHost, visitPath, progress);

            // Try to make the launcher launch the process.
            if(launchers.find(remoteHost) == launchers.end())
                cancelled = true;
            else
            {
                launchers[remoteHost].launcher->GetLauncherMethods()->LaunchProcess(args);
                // Indicate success.
                launched = true;
            }

            retry = false;
        }
        CATCH(LostConnectionException)
        {
            // We lost the connection to the launcher program so we need
            // to delete its proxy and remove it from the launchers map
            // so the next time we go through this loop, we relaunch it.
            delete launchers[remoteHost].launcher;
            launchers[remoteHost].launcher = 0;
            delete launchers[remoteHost].notifier;
            launchers[remoteHost].notifier = 0;
            LauncherMap::iterator pos = launchers.find(remoteHost);
            launchers.erase(pos);

            retry = true;
            ++numAttempts;
        }
        // All other VisItExceptions are thrown out of this routine so they
        // can be handled by the managers.
        ENDTRY
    } while(retry && numAttempts < 2);

    if(cancelled)
    {
        EXCEPTION0(CancelledConnectException);
    }

    if(!launched)
    {
        EXCEPTION0(CouldNotConnectException);
    }
}

// ****************************************************************************
//  Method: ViewerServerManager::SimConnectThroughLauncher 
//
//  Purpose:
//    Connect to a simulation using the launcher.  This is similar to 
//    OpenWithLauncher, but it connects to a running simulation instead
//    of starting a new engine process.
//
//  Arguments:
//   remoteHost : The name of the computer where we want the component to run.
//   args       : The command line to run on the remote machine.
//   data       : Optional callback data.
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 30, 2004
//
//  Modifications:
//    Brad Whitlock, Wed Mar 31 10:20:33 PDT 2004
//    Fixed code so it builds on the SGI.
//
//    Jeremy Meredith, Wed May 11 09:04:52 PDT 2005
//    Added security key to simulation connection.
//
//    Brad Whitlock, Thu Mar 9 10:35:40 PDT 2006
//    I made it throw a CancelledConnect exception in the event that we're
//    launching on a machine that shares mdserver and engine batch jobs. That
//    way, if the launch was cancelled, we don't crash! I also added support
//    for a connection progress dialog when launching VCL. A connection
//    progress dialog for aborting the connection to the sim will take quite
//    a bit more work.
//
//    Brad Whitlock, Wed Nov 21 14:43:04 PST 2007
//    Changed the map storage type.
//
// ****************************************************************************

void
ViewerServerManager::SimConnectThroughLauncher(const std::string &remoteHost, 
                                               const stringVector &args,
                                               void *data)
{
    bool retry = false;
    int  numAttempts = 0;
    bool launched = false;
    bool cancelled = false;

    do
    {
        TRY
        {
            // We use the data argument to pass in a pointer to the connection
            // progress window.
            typedef struct {
                std::string h; int p; std::string k;
                ViewerConnectionProgress *progress;
                bool tunnel;} SimData;
            SimData *simData = (SimData*)data;

            // Search the args list and see if we've supplied the path to
            // the visit executeable.
            std::string visitPath;
            for(size_t i = 0; i < args.size(); ++i)
            {
                if(args[i] == "-dir" && (i+1) < args.size())
                {
                    visitPath = args[i+1];
                    ++i;
                }
            }

            // Try to start a launcher on remoteHost.
            StartLauncher(remoteHost, visitPath, simData->progress);

            // Try to make the launcher launch the process.
            if(launchers.find(remoteHost) == launchers.end())
                cancelled = true;
            else
            {
                // If we're doing SSH tunneling, change the arguments here.
                stringVector args2(args);
                if(simData->tunnel)
                    ConvertArgsToTunneledValues(GetPortTunnelMap(remoteHost), args2);

                launchers[remoteHost].launcher->GetLauncherMethods()->ConnectSimulation(args2,
                    simData->h, simData->p, simData->k);

                // Indicate success.
                launched = true;
            }

            retry = false;
        }
        CATCH(LostConnectionException)
        {
            // We lost the connection to the launcher program so we need
            // to delete its proxy and remove it from the launchers map
            // so the next time we go through this loop, we relaunch it.
            delete launchers[remoteHost].launcher;
            launchers[remoteHost].launcher = 0;
            delete launchers[remoteHost].notifier;
            launchers[remoteHost].notifier = 0;
            LauncherMap::iterator pos = launchers.find(remoteHost);
            launchers.erase(pos);

            retry = true;
            ++numAttempts;
        }
        // All other VisItExceptions are thrown out of this routine so they
        // can be handled by the managers.
        ENDTRY
    } while(retry && numAttempts < 2);

    if(cancelled)
    {
        EXCEPTION0(CancelledConnectException);
    }

    if(!launched)
    {
        EXCEPTION0(CouldNotConnectException);
    }
}



// ****************************************************************************
//  Method:  ViewerServerManager::GetPortTunnelMap
//
//  Purpose:
//    Retrieve the SSH tunneling local-to-remote port map for the
//    appropriate host.
//
//  Arguments:
//    host       the host for which we need the ssh port tunnel map
//
//  Programmer:  Jeremy Meredith
//  Creation:    May 24, 2007
//
//  Modifications:
//    Thomas R. Treadway, Mon Oct  8 13:27:42 PDT 2007
//    Backing out SSH tunneling on Panther (MacOS X 10.3)
//
//    Brad Whitlock, Wed Nov 21 14:43:24 PST 2007
//    Changed the map storage type.
//
// ****************************************************************************

std::map<int,int>
ViewerServerManager::GetPortTunnelMap(const std::string &host)
{
    std::map<int,int> ret;
    if (launchers.count(host))
        ret = launchers[host].launcher->GetPortTunnelMap();
    return ret;
}

// ****************************************************************************
// Method: ViewerServerManager::SetLaunchProgressCallback
//
// Purpose:
//   Set a launch progress callback function.
//
// Arguments:
//   cb     : The callback function.
//   cbdata : Data for the callback function.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep  2 12:58:28 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
ViewerServerManager::SetLaunchProgressCallback(bool (*cb)(void *, int), void *cbdata)
{
    LaunchProgressCB = cb;
    LaunchProgressCBData = cbdata;
}

// ****************************************************************************
// Method: ViewerServerManager::SetOpenWithEngineCallback
//
// Purpose:
//   Set a callback function to use when components should be launched by the
//   compute engine.
//
// Arguments:
//   cb     : The callback function.
//   cbdata : Data for the callback function.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep  2 12:58:28 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
ViewerServerManager::SetOpenWithEngineCallback(
    void (*cb)(const std::string &, const stringVector &, void *),
    void *cbdata)
{
    OpenWithEngineCB = cb;
    OpenWithEngineCBData = cbdata;
}
