#include <visit-config.h>
#include <snprintf.h>

#include <ViewerServerManager.h>
#include <HostProfileList.h>
#include <HostProfile.h>
#include <LauncherProxy.h>
#include <RemoteProxyBase.h>
#include <LostConnectionException.h>
#include <CouldNotConnectException.h>
#include <ViewerConnectionProgressDialog.h>
#include <ViewerPasswordWindow.h>
#include <ViewerSubject.h>
#include <ViewerRemoteProcessChooser.h>

#include <DebugStream.h>
#include <avtCallback.h>

//
// Global variables.
//
extern ViewerSubject *viewerSubject;

//
// Static members.
//
HostProfileList *ViewerServerManager::clientAtts = 0;
int ViewerServerManager::debugLevel = 0;
std::string ViewerServerManager::localHost("localhost");
stringVector ViewerServerManager::arguments;
ViewerServerManager::LauncherMap ViewerServerManager::launchers;
void * ViewerServerManager::cbData[2] = {0,0};

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
//   
// ****************************************************************************

ViewerServerManager::ViewerServerManager()
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
// Method: ViewerServerManager::GetClientAtts
//
// Purpose: 
//   Returns a pointer to the host profile list.
//
// Returns:    A pointer to the host profile list.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 3 16:12:56 PST 2002
//
// Modifications:
//   
// ****************************************************************************

HostProfileList *
ViewerServerManager::GetClientAtts()
{
    //
    // If the client attributes haven't been allocated then do so.
    //
    if (clientAtts == 0)
    {
        clientAtts = new HostProfileList;
    }

    return clientAtts;
}

// ****************************************************************************
// Method: ViewerServerManager::SetDebugLevel
//
// Purpose: 
//   Sets the debug level that is passed to the engines.
//
// Arguments:
//   level : The debug level.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 27 17:29:01 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
ViewerServerManager::SetDebugLevel(int level)
{
    debugLevel = level;
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
    return hostName == localHost || hostName == "localhost";
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
// Method: ViewerServerManager::AddProfileArguments
//
// Purpose: 
//   This method finds a host profile that matches the specified hostname
//   and adds the profile arguments to the component proxy.
//
// Arguments:
//   component : The proxy to which we're adding arguments.
//   host      : The host where the component will be run.
//
// Programmer: Brad Whitlock
// Creation:   Tue May 6 14:00:21 PST 2003
//
// Modifications:
//    Eric Brugger, Wed Dec  3 08:27:47 PST 2003
//    I removed the default timeout passed to the mdserver.
//   
// ****************************************************************************

void
ViewerServerManager::AddProfileArguments(RemoteProxyBase *component,
    const std::string &host)
{
    //
    // Check for a host profile for the hostName. If one exists, add
    // any arguments to the command line for the engine proxy.
    //
    const HostProfile *profile =
         clientAtts->FindMatchingProfileForHost(host.c_str());
    if(profile != 0)
    {
        //
        // Set the user's login name.
        //
        component->SetRemoteUserName(profile->GetUserName().c_str());

        //
        // Add the timeout argument
        //
        char temp[10];
        SNPRINTF(temp, 10, "%d", profile->GetTimeout());
        component->AddArgument("-timeout");
        component->AddArgument(temp);

        //
        // Add any additional arguments.
        //
        for(int i = 0; i < profile->GetArguments().size(); ++i)
            component->AddArgument(profile->GetArguments()[i].c_str());
    }
}

// ****************************************************************************
// Method: ViewerServerManager::GetClientMachineNameOptions
//
// Purpose: 
//   This method finds a host profile that matches the specified hostname
//   and returns the client host name options.
//
// Arguments:
//   host          : The host where the component will be run.
//   chd           : The type of client host name determination to use.
//   clientHostName: The manual host name, if manual determination is requested
//
// Programmer: Jeremy Meredith
// Creation:   October  9, 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerServerManager::GetClientMachineNameOptions(const std::string &host,
                                     HostProfile::ClientHostDetermination &chd,
                                     std::string &clientHostName)
{
    //
    // Check for a host profile for the hostName. If one exists, 
    // return the client host name options.
    //
    const HostProfile *profile =
         clientAtts->FindMatchingProfileForHost(host.c_str());
    if(profile != 0)
    {
        chd = profile->GetClientHostDetermination();
        clientHostName = profile->GetManualClientHostName();
    }
    else
    {
        chd = HostProfile::MachineName;
        clientHostName = "";
    }
}

// ****************************************************************************
// Method: ViewerServerManager::GetClientSSHPortOptions
//
// Purpose: 
//   This method finds a host profile that matches the specified hostname
//   and returns the ssh port options.
//
// Arguments:
//   host          : The host where the component will be run.
//   manualSSHPort : True if a manual ssh port was specified
//   sshPort       : The manual ssh port
//
// Programmer: Jeremy Meredith
// Creation:   October  9, 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerServerManager::GetSSHPortOptions(const std::string &host,
                                       bool &manualSSHPort,
                                       int &sshPort)
{
    //
    // Check for a host profile for the hostName. If one exists, 
    // return the ssh port options.
    //
    const HostProfile *profile =
         clientAtts->FindMatchingProfileForHost(host.c_str());
    if(profile != 0)
    {
        manualSSHPort = profile->GetSshPortSpecified();
        sshPort = profile->GetSshPort();
    }
    else
    {
        manualSSHPort = false;
        sshPort = -1;
    }
}

// ****************************************************************************
//  Method: ViewerServerManager::ShouldShareBatchJob
//
//  Purpose: 
//    This method finds a host profile that matches the specified hostname
//    and checks if the MDServer and Engine should share a single batch
//    job.  (If so, then the launcher should start in batch.)
//
//  Arguments:
//    host      : The host where the component will be run.
//
//  Programmer: Jeremy Meredith
//  Creation:   June 17, 2003
//
//  Modifications:
//   
// ****************************************************************************

bool
ViewerServerManager::ShouldShareBatchJob(const std::string &host)
{
    //
    // Check for a host profile for the hostName. If one exists, check it.
    //
    const HostProfile *profile = clientAtts->FindMatchingProfileForHost(host);
    if (profile != 0)
    {
        return profile->GetShareOneBatchJob();
    }
    else
    {
        return false;
    }
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
// ****************************************************************************

void
ViewerServerManager::AddArguments(RemoteProxyBase *component,
    const stringVector &args)
{
    // Add arguments to the mdserver.
    if(debugLevel > 0)
    {
        char temp[10];
        SNPRINTF(temp, 10, "%d", debugLevel);
        component->AddArgument("-debug");
        component->AddArgument(temp);
    }

    //
    // Add arguments stored in ViewerServerManager.
    //
    int i;
    for (i = 0; i < arguments.size(); ++i)
         component->AddArgument(arguments[i].c_str());

    //
    // Add any other arguments given to us by the caller
    //
    for (i = 0; i < args.size(); ++i)
         component->AddArgument(args[i].c_str());
}

// ****************************************************************************
// Method: ViewerServerManager::SetupConnectionProgressWindow
//
// Purpose: 
//   Creates a connection progress dialog that is hooked up to the component
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
//   Brad Whitlock, Mon May 19 17:40:40 PST 2003
//   I made the timeout for showing the window be zero if the component
//   being launched is remote or parallel.
//
//   Brad Whitlock, Wed Oct 8 14:49:45 PST 2003
//   I made the timeout be zero on MacOS X. This can be undone later when
//   VisIt launches faster there.
//
// ****************************************************************************

ViewerConnectionProgressDialog *
ViewerServerManager::SetupConnectionProgressWindow(RemoteProxyBase *component, 
    const std::string &host)
{
    ViewerConnectionProgressDialog *dialog = 0;
#ifdef HAVE_THREADS
    //
    // Set the engine proxy's progress callback.
    //
    if(!avtCallback::GetNowinMode())
    {
#if defined(__APPLE__)
        // Make the timeout on MacOS X be zero since it takes so long to
        // launch an engine. This at least gives the user something to look at.
        int timeout = 0;
#else
        int timeout = (component->Parallel() || !HostIsLocalHost(host)) ? 0 : 4000;
#endif
        // Create a new connection dialog.
        dialog = new ViewerConnectionProgressDialog(
            component->GetComponentName().c_str(),
            host.c_str(), component->Parallel(), timeout);
        cbData[0] = (void *)viewerSubject;
        cbData[1] = (void *)dialog;
        component->SetProgressCallback(ViewerSubject::LaunchProgressCB,
                                       cbData);

        // Register the dialog with the password window so we can set
        // the dialog's timeout to zero if we have to prompt for a
        // password.
        ViewerPasswordWindow::SetConnectionProgressDialog(dialog);
    }
#endif

    return dialog;
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
//   
// ****************************************************************************

void
ViewerServerManager::CloseLaunchers()
{
    LauncherMap::iterator pos;
    for(pos = launchers.begin(); pos != launchers.end(); ++pos)
    {
        pos->second->Close();
        delete pos->second;
        pos->second = 0;
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
//   
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
            pos->second->SendKeepAlive();
            ++pos;
        }
        CATCHALL(...)
        {
            debug2 << "Could not send keep alive signal to launcher on "
                   << pos->first.c_str() << " so that launcher will be closed."
                   << endl;
            delete pos->second;
            pos->second = 0;
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
//    Jeremy Meredith, Thu Oct  9 14:03:11 PDT 2003
//    Added ability to manually specify a client host name or to have it
//    parsed from the SSH_CLIENT (or related) environment variables.  Added
//    ability to specify an SSH port.
//
// ****************************************************************************

void
ViewerServerManager::StartLauncher(const std::string &host,
     const std::string &visitPath, ViewerConnectionProgressDialog *dialog)
{
    if(launchers.find(host) == launchers.end())
    {
        if (ShouldShareBatchJob(host))
        {
            ViewerRemoteProcessChooser *chooser =
                ViewerRemoteProcessChooser::Instance();

            chooser->ClearCache(host);

            if (! chooser->SelectProfile(clientAtts,host,false))
                return;
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

        if (ShouldShareBatchJob(host))
        {
            ViewerRemoteProcessChooser *chooser =
                ViewerRemoteProcessChooser::Instance();

            chooser->AddProfileArguments(newLauncher, true);
        }
        else
        {
            AddProfileArguments(newLauncher, host);
        }

        TRY
        {
            // If we're reusing the dialog, set the launcher's progress
            // callback function so that the window pops up.
            if(dialog)
            {
                cbData[0] = (void *)viewerSubject;
                cbData[1] = (void *)dialog;
                newLauncher->SetProgressCallback(
                    ViewerSubject::LaunchProgressCB, cbData);
                dialog->setIgnoreHide(true);
            }

            // Get the client machine name options
            HostProfile::ClientHostDetermination chd;
            std::string clientHostName;
            GetClientMachineNameOptions(host, chd, clientHostName);

            // Get the ssh port options
            bool manualSSHPort;
            int  sshPort;
            GetSSHPortOptions(host, manualSSHPort, sshPort);

            //
            // Launch the VisIt component launcher on the specified host.
            //
            newLauncher->Create(host, chd, clientHostName,
                                manualSSHPort, sshPort);
            launchers[host] = newLauncher;

            // Set the dialog's information back to the previous values.
            if(dialog)
            {
                dialog->setIgnoreHide(false);
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
// ****************************************************************************

void
ViewerServerManager::OpenWithLauncher(
    const std::string &remoteHost, 
    const stringVector &args, void *data)
{
    bool retry = false;
    int  numAttempts = 0;
    bool launched = false;

    do
    {
        TRY
        {
            // We use the data argument to pass in a pointer to the connection
            // progress window.
            ViewerConnectionProgressDialog *dialog =
                (ViewerConnectionProgressDialog *)data;

            // Search the args list and see if we've supplied the path to
            // the visit executeable.
            std::string visitPath;
            for(int i = 0; i < args.size(); ++i)
            {
                if(args[i] == "-dir" && (i+1) < args.size())
                {
                    visitPath = args[i+1];
                    ++i;
                }
            }

            // Try to start a launcher on remoteHost.
            StartLauncher(remoteHost, visitPath, dialog);

            // Try to make the launcher launch the process.
            launchers[remoteHost]->LaunchProcess(args);

            // Indicate success.
            launched = true;
            retry = false;
        }
        CATCH(LostConnectionException)
        {
            // We lost the connection to the launcher program so we need
            // to delete its proxy and remove it from the launchers map
            // so the next time we go through this loop, we relaunch it.
            delete launchers[remoteHost];
            launchers[remoteHost] = 0;
            LauncherMap::iterator pos = launchers.find(remoteHost);
            launchers.erase(pos);

            retry = true;
            ++numAttempts;
        }
        // All other VisItExceptions are thrown out of this routine so they
        // can be handled by the managers.
        ENDTRY
    } while(retry && numAttempts < 2);

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
// ****************************************************************************

void
ViewerServerManager::SimConnectThroughLauncher(const std::string &remoteHost, 
                                               const stringVector &args,
                                               void *data)
{
    bool retry = false;
    int  numAttempts = 0;
    bool launched = false;

    do
    {
        TRY
        {
            // We use the data argument to pass in a pointer to the connection
            // progress window.
            typedef struct {std::string h; int p;} SimData;
            SimData *simData = (SimData*)data;

            // Search the args list and see if we've supplied the path to
            // the visit executeable.
            std::string visitPath;
            for(int i = 0; i < args.size(); ++i)
            {
                if(args[i] == "-dir" && (i+1) < args.size())
                {
                    visitPath = args[i+1];
                    ++i;
                }
            }

            // Try to start a launcher on remoteHost.
            StartLauncher(remoteHost, visitPath, NULL);

            // Try to make the launcher launch the process.
            launchers[remoteHost]->ConnectSimulation(args, simData->h, simData->p);

            // Indicate success.
            launched = true;
            retry = false;
        }
        CATCH(LostConnectionException)
        {
            // We lost the connection to the launcher program so we need
            // to delete its proxy and remove it from the launchers map
            // so the next time we go through this loop, we relaunch it.
            delete launchers[remoteHost];
            launchers[remoteHost] = 0;
            LauncherMap::iterator pos = launchers.find(remoteHost);
            launchers.erase(pos);

            retry = true;
            ++numAttempts;
        }
        // All other VisItExceptions are thrown out of this routine so they
        // can be handled by the managers.
        ENDTRY
    } while(retry && numAttempts < 2);

    if(!launched)
    {
        EXCEPTION0(CouldNotConnectException);
    }
}

