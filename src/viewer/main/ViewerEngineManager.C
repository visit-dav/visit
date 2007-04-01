// ************************************************************************* //
//                            ViewerEngineManager.C                          //
// ************************************************************************* //

#include <ViewerEngineManager.h>

#include <BadHostException.h>
#include <EngineList.h>
#include <EngineProxy.h>
#include <HostProfileList.h>
#include <HostProfile.h>
#include <LostConnectionException.h>
#include <NoEngineException.h>
#include <IncompatibleVersionException.h>
#include <IncompatibleSecurityTokenException.h>
#include <CancelledConnectException.h>
#include <CouldNotConnectException.h>
#include <AnnotationAttributes.h>
#include <PickAttributes.h>
#include <QueryAttributes.h>
#include <StatusAttributes.h>
#include <ViewerFileServer.h>
#include <ViewerMessaging.h>
#include <ViewerOperator.h>
#include <ViewerPlot.h>
#include <ViewerSubject.h>
#include <ViewerWindow.h>
#include <ViewerWindowManager.h>
#include <WindowAttributes.h>
#include <DebugStream.h>
#include <ViewerConnectionProgressDialog.h>
#include <ViewerRemoteProcessChooser.h>
#include <MaterialAttributes.h>

#include <avtCallback.h>
#include <avtDatabaseMetaData.h>

#include <algorithm>
#include <utility>
#include <stdio.h>
#include <snprintf.h>

using std::vector;
using std::string;
using std::map;
using std::pair;

//
// Define some boiler plate macros that wrap blocking RPCs.
//
// EngineProxy Macro Modifications:
//    Jeremy Meredith, Wed Mar 17 15:09:07 PST 2004
//    Inserted "false" before numRestarts so that it wouldn't take numRestarts
//    as the value for skipChooser. 
//
//    Jeremy Meredith, Mon Mar 22 17:15:44 PST 2004
//    Added a setting of retry=false when creating the engine in the first
//    section of code, because if the user cancels the launch of the engine,
//    it could otherwise get into an infinite loop.
//
//    Jeremy Meredith, Fri Apr  2 14:29:25 PST 2004
//    Made restartArguments be saved on a per-host (per-enginekey) basis.
//
#define ENGINE_PROXY_RPC_BEGIN(rpcname)  \
    bool retval = false; \
    bool retry = false; \
    int  numAttempts = 0; \
    do \
    { \
        if (!EngineExists(ek)) \
        { \
            if (ek.SimName() == "") \
            { \
                debug1 << "****\n**** Trying to execute the " << rpcname \
                       << " RPC before an engine was started" << endl \
                       << "**** on " << ek.HostName().c_str() \
                       << ". Starting an engine on " \
                       << ek.HostName().c_str() << ".\n****" << endl; \
                CreateEngine(ek, restartArguments[ek], false, numRestarts); \
                retry = false; \
            } \
            else \
            { \
                LaunchMessage(ek); \
                retry = false; \
            } \
        } \
        if (EngineExists(ek)) \
        { \
            TRY \
            { \
                EngineProxy *engine = engines[ek]; \
                debug3 << "Calling " << rpcname << " RPC on " \
                       << ek.HostName().c_str() << "'s engine." << endl;

#define ENGINE_PROXY_RPC_END  \
                retval = true; \
                retry = false; \
            } \
            CATCH(LostConnectionException) \
            { \
                if (ek.SimName() != "") \
                { \
                    LaunchMessage(ek); \
                    RemoveFailedEngine(ek); \
                    retry = false; \
                } \
                else if (numAttempts < numRestarts) \
                { \
                   retry = true; \
                   RemoveFailedEngine(ek); \
                   LaunchMessage(ek); \
                   CreateEngine(ek, restartArguments[ek],false,numRestarts); \
                   ++numAttempts; \
                } \
                else \
                { \
                   retry = false; \
                   retval = false; \
                } \
            } \
            CATCH(VisItException) \
            { \
                retry = false; \
                retval = false; \
            } \
            ENDTRY \
        } \
    } while(retry && numAttempts < numRestarts); \
    if(!retval || (retry && retval)) \
        UpdateEngineList(); \
    return retval;


#define ENGINE_PROXY_RPC_END_NORESTART_RETHROW  \
                retval = true; \
                retry = false; \
            } \
            CATCH(LostConnectionException) \
            { \
                retry = false; \
                retval = false; \
                RemoveFailedEngine(ek); \
            } \
            CATCH(VisItException) \
            { \
                retry = false; \
                retval = false; \
                RETHROW; \
            } \
            ENDTRY \
        } \
    } while(retry && numAttempts < numRestarts); \
    if(!retval || (retry && retval)) \
        UpdateEngineList(); \
    return retval;


#define ENGINE_PROXY_RPC_END_NORESTART_RETHROW2  \
                retval = true; \
                retry = false; \
            } \
            CATCH(LostConnectionException) \
            { \
                retry = false; \
                retval = false; \
                RemoveFailedEngine(ek); \
                RETHROW; \
            } \
            CATCH(VisItException) \
            { \
                retry = false; \
                retval = false; \
                RETHROW; \
            } \
            ENDTRY \
        } \
        else \
        { \
            retry = false; \
            retval = false; \
            EXCEPTION0(NoEngineException); \
        } \
    } while(retry && numAttempts < numRestarts); \
    if(!retval || (retry && retval)) \
        UpdateEngineList(); \
    return retval;


#define ENGINE_PROXY_RPC_END_NORESTART  \
                retval = true; \
                retry = false; \
            } \
            CATCH(LostConnectionException) \
            { \
                retry = false; \
                retval = false; \
                RemoveFailedEngine(ek); \
            } \
            CATCH(VisItException) \
            { \
                retry = false; \
                retval = false; \
            } \
            ENDTRY \
        } \
    } while(retry && numAttempts < numRestarts); \
    if(!retval || (retry && retval)) \
        UpdateEngineList(); \
    return retval;

//
// to remember numRestarts across VEM destructors
//
static int numRestarts = -1;

//
// Global variables.  These should be removed.
//
extern ViewerSubject  *viewerSubject;

//
// Storage for static data elements.
//
ViewerEngineManager *ViewerEngineManager::instance=0;
EngineList *ViewerEngineManager::clientEngineAtts=0;

MaterialAttributes *ViewerEngineManager::materialClientAtts=0;
MaterialAttributes *ViewerEngineManager::materialDefaultAtts=0;

//
// Function prototypes.
//
static void GetImageCallback(void *, int, avtDataObject_p &);
static void UpdatePlotAttsCallback(void*,const string&,int,AttributeSubject*);


// ****************************************************************************
//  Method: ViewerEngineManager constructor
//
//  Programmer: Eric Brugger
//  Creation:   September 22, 2000
//
//  Modifications:
//    Brad Whitlock, Tue May 1 12:20:20 PDT 2001
//    Made it inherit from SimpleObserver.
//
//    Brad Whitlock, Mon Sep 24 13:58:17 PST 2001
//    Added initialization of localHost.
//
//    Hank Childs, Tue Nov 20 15:08:43 PST 2001
//    Register the GetImage and UpdatePlotAtts callback.
//
//    Brad Whitlock, Fri May 3 16:11:32 PST 2002
//    Made it call the ViewerServerManager constructor.
//
//    Brad Whitlock, Thu May 9 12:32:55 PDT 2002
//    Added executing member.
//
//    Brad Whitlock, Fri Dec 27 12:00:59 PDT 2002
//    I added initialization for restartArguments.
//
//    Mark C. Miller, Sat Jan 17 12:40:16 PST 2004
//    Changed numRestarts to file scope static and changed it numRestarts it
//    gets initialized here
//
//    Jeremy Meredith, Fri Mar 26 16:59:59 PST 2004
//    Use a map of engines based on a key, and be aware of simulations.
//
// ****************************************************************************

ViewerEngineManager::ViewerEngineManager() : ViewerServerManager(),
    SimpleObserver()
{
    executing = false;
    if (numRestarts == -1)
        numRestarts = 2;
    avtCallback::RegisterImageCallback(GetImageCallback, this);
    avtCallback::RegisterUpdatePlotAttributesCallback(UpdatePlotAttsCallback,
                                                      this);
}

// ****************************************************************************
//  Method: ViewerEngineManager destructor
//
//  Programmer: Eric Brugger
//  Creation:   September 22, 2000
//
//  Modifications:
//
// ****************************************************************************

ViewerEngineManager::~ViewerEngineManager()
{
    //
    // This should never be executed.
    //
}

// ****************************************************************************
//  Method: ViewerEngineManager::Instance
//
//  Purpose:
//      Return a pointer to the sole instance of the ViewerEngineManager
//      class.
//
//  Returns:    A pointer to the sole instance of the ViewerEngineManager
//              class.
//
//  Programmer: Eric Brugger
//  Creation:   September 22, 2000
//
//  Modifications:
//
// ****************************************************************************

ViewerEngineManager *ViewerEngineManager::Instance()
{
    //
    // If the sole instance hasn't been instantiated, then instantiate it.
    //
    if (instance == 0)
    {
        instance = new ViewerEngineManager;
    }

    return instance;
}

// ****************************************************************************
// Method: ViewerEngineManager::EngineExists
//
// Purpose: 
//   Returns true if the engine exists.
//
// Arguments:
//   ek:       the key to find the engine
//
// Returns:    true if the engine exists
//
// Note:       
//
// Programmer: Jeremy Meredith
// Creation:   March 26, 2004
//
// Modifications:
//
// ****************************************************************************

bool
ViewerEngineManager::EngineExists(const EngineKey &ek) const
{
    return engines.count(ek) > 0;
}

// ****************************************************************************
//  Method: ViewerEngineManager::CreateEngine
//
//  Purpose:
//      Create an engine for the specified host.
//
//  Arguments:
//      engineKey      contains the host name for the engine
//      args           the arguments to pass to the engine
//      skipChooser    do we not want to ask the user which profile to use
//      numRestarts    the number of restart attempts to use when engines fail
//
//  Programmer: Eric Brugger
//  Creation:   September 23, 2000
//
//  Modifications:
//    Brad Whitlock, Fri Sep 29 19:14:52 PST 2000
//    I added code to check the host profile and add any special arguments
//    to the engine proxy before launching it on the remote machine.
//
//    Brad Whitlock, Fri Oct 20 12:58:14 PDT 2000
//    I added code to set the user's login name on the remote machine if a
//    host profile is found.
//
//    Eric Brugger, Wed Oct 25 14:49:58 PDT 2000
//    I modified the routine to match a change to the EngineProxy Create
//    method.
//
//    Brad Whitlock, Mon Nov 27 17:30:42 PST 2000
//    I added code to pass the debug level to the engine that gets launched.
//
//    Jeremy Meredith, Fri Apr 20 10:36:43 PDT 2001
//    Added code to pass other arguments to engine.
//
//    Brad Whitlock, Mon Apr 23 14:21:55 PST 2001
//    Added exception handling code for launching an engine.
//
//    Brad Whitlock, Wed Apr 25 17:25:01 PST 2001
//    Added exception handling code for IncompatibleVersionException.
//
//    Jeremy Meredith, Fri Apr 27 15:43:55 PDT 2001
//    Added catching of CouldNotConnectException.
//
//    Brad Whitlock, Mon Apr 30 14:22:48 PST 2001
//    Added code to update the engine list and send a status message.
//
//    Jeremy Meredith, Tue Jul 17 18:06:16 PDT 2001
//    Added code to handle nodes, launch, and partition for parallel jobs.
//
//    Jeremy Meredith, Fri Sep 21 14:29:45 PDT 2001
//    Added force static, force dynamic.
//
//    Brad Whitlock, Mon Sep 24 15:04:31 PST 2001
//    I changed the code to make sure that if the hostName that is used
//    is equal to the localHost then we launch the engine using "localhost"
//    to make sure that it is launched locally.
//
//    Brad Whitlock, Mon Oct 22 18:33:37 PST 2001
//    Changed the exception keywords to macros.
//
//    Jeremy Meredith, Thu Feb 14 15:05:09 PST 2002
//    Made it use a more sophisticated check for a matching profile.
//
//    Brad Whitlock, Mon Feb 25 09:53:20 PDT 2002
//    Changed code to account for the fact that GetEngineIndex will now
//    return -1 if an engine is not in the list.
//
//    Sean Ahern, Thu Feb 21 12:50:28 PST 2002
//    Added timeout support.
//
//    Jeremy Meredith, Wed Mar  6 14:59:04 PST 2002
//    Changed default timeout to "240" instead of "60*4".
//
//    Jeremy Meredith, Mon Jul 22 12:22:12 PDT 2002
//    Added support for banks and time limits.
//    Temporarily force static load balancing.
//    Moved most code to the new class ViewerEngineChooser.
//    Added code to reset cached options if launching engines failed.
//
//    Brad Whitlock, Fri Sep 27 15:23:52 PST 2002
//    I added support for a progress window that pops up when a process
//    is being launched.
//
//    Jeremy Meredith, Wed Dec 18 17:19:20 PST 2002
//    Added support for skipping the chooser window when launching engines.
//
//    Brad Whitlock, Fri Dec 27 12:02:22 PDT 2002
//    I added code to save the arguments into the restartArguments variable
//    which is used later to restart the engine with arguments that we
//    specified.
//
//    Brad Whitlock, Wed May 7 10:01:34 PDT 2003
//    I moved some code into the base class and I made possible to launch
//    the engine using a launcher program.
//
//    Jeremy Meredith, Thu Jun 26 10:46:04 PDT 2003
//    Allowed the engine to share a batch job with the mdserver.  In this
//    case, VCL gets launched in batch and the engine needs only add 
//    the 'par' arguments and can skip the profile chooser (since the
//    user chose one when launching the VCL).  Renamed ViewerEngineChooser
//    to ViewerRemoteProcessChooser.
//
//    Jeremy Meredith, Thu Oct  9 13:55:27 PDT 2003
//    Added ability to manually specify a client host name or to have it
//    parsed from the SSH_CLIENT (or related) environment variables.  Added
//    ability to specify an SSH port.
//
//    Mark C. Miller, Sat Jan 17 12:40:16 PST 2004
//    Changed how numRestarts is set 
//
//    Jeremy Meredith, Mon Mar 22 17:55:36 PST 2004
//    Added a boolean "success" return value to this method.  Cancelling
//    or failing to start an engine results in a return value of false.
//
//    Jeremy Meredith, Tue Mar 30 10:24:45 PST 2004
//    It now uses an EngineKey to specify the host, and it uses
//    a map of Engines instead of an array.
//
//    Jeremy Meredith, Fri Apr  2 14:28:23 PST 2004
//    Don't add the given arguments to "restartArguments"; instead, we cache
//    them with the cached host profile so they get picked up under a more
//    appropriate set of circumstances.
//
// ****************************************************************************

bool
ViewerEngineManager::CreateEngine(const EngineKey &ek,
                                  const stringVector &args,
                                  bool skipChooser,
                                  int numRestarts_)
{
    if (numRestarts_ == -1)
    {
        if (numRestarts == -1)
            numRestarts = 2;
    }
    else
       numRestarts = numRestarts_;

    //
    // Check if an engine already exists for the host.
    //
    if (EngineExists(ek))
        return true;

    //
    // If an engine for the host doesn't already exist, create one.
    //
    bool success = false;
    ViewerRemoteProcessChooser *chooser =
                                        ViewerRemoteProcessChooser::Instance();

    if (! chooser->SelectProfile(clientAtts,ek.HostName(),skipChooser))
    {
        return false;
    }

    EngineProxy *newEngine = new EngineProxy;

    chooser->AddProfileArguments(newEngine,
                                          !ShouldShareBatchJob(ek.HostName()));

    //
    // Add some arguments to the engine proxy before we try to
    // launch the engine.  Cache them if needed for an automatic launch.
    //
    AddArguments(newEngine, args);
    chooser->AddRestartArgsToCachedProfile(ek.HostName(),  args);

    //
    // Set up the connection progress window.
    //
    ViewerConnectionProgressDialog *dialog =
                       SetupConnectionProgressWindow(newEngine, ek.HostName());


    //
    // Send a status message.
    //
    char msg[250];
    SNPRINTF(msg, 250, "Launching engine on %s", ek.HostName().c_str());
    Status(msg);

    //
    // Add the new engine proxy to the engine list.
    //
    TRY
    {
        // Get the client machine name options
        HostProfile::ClientHostDetermination chd;
        string clientHostName;
        GetClientMachineNameOptions(ek.HostName(), chd, clientHostName);

        // Get the ssh port options
        bool manualSSHPort;
        int  sshPort;
        GetSSHPortOptions(ek.HostName(), manualSSHPort, sshPort);

        //
        // Launch the engine.
        //
        if (!ShouldShareBatchJob(ek.HostName()) && HostIsLocalHost(ek.HostName()))
            newEngine->Create("localhost", chd, clientHostName,
                              manualSSHPort, sshPort);
        else
        {
            // Use VisIt's launcher to start the remote engine.
            newEngine->Create(ek.HostName(),  chd, clientHostName,
                              manualSSHPort, sshPort,
                              OpenWithLauncher, (void *)dialog,
                              true);
        }

        // Add the new engine to the engine list.
        engines[ek] = newEngine;

        // Make the engine manager observe the proxy's status atts.
        newEngine->GetStatusAttributes()->Attach(this);

        // Now that the new engine is in the list, tell the GUI.
        UpdateEngineList();

        // Success!
        success = true;
    }
    CATCH2(BadHostException, e)
    {
        // Delete the new engine since it could not launch anyway.
        delete newEngine;
        ViewerRemoteProcessChooser::Instance()->ClearCache(ek.HostName());

        // Tell the user that the engine could not be launched.
        SNPRINTF(msg, 250, "VisIt could not launch a compute engine on host "
                "\"%s\" because that host does not exist.",
                e.GetHostName().c_str());
        Error(msg);
    }
    CATCH(IncompatibleVersionException)
    {
        // Delete the new engine since talking to it could be bad since
        // it is a different version.
        delete newEngine;
        ViewerRemoteProcessChooser::Instance()->ClearCache(ek.HostName());

        // Tell the user that the engine is a different version.
        SNPRINTF(msg, 250, "VisIt cannot use the compute engine on "
                 "host \"%s\" because the engine has an incompatible "
                 " version number.", ek.HostName().c_str());
        Error(msg);
    }
    CATCH(IncompatibleSecurityTokenException)
    {
        // Delete the new engine since talking to it could be bad since
        // it did not provide the right credentials.
        delete newEngine;
        ViewerRemoteProcessChooser::Instance()->ClearCache(ek.HostName());

        // Tell the user that the engine is a different version.
        SNPRINTF(msg, 250, "VisIt cannot use the compute engine on host \"%s\""
                 "because the compute engine did not provide the proper "
                 "credentials.", ek.HostName().c_str());
        Error(msg);
    }
    CATCH(CouldNotConnectException)
    {
        // Delete the new engine since it was not launched
        delete newEngine;
        ViewerRemoteProcessChooser::Instance()->ClearCache(ek.HostName());

        // Tell the user that the engine was not launched
        SNPRINTF(msg, 250, "VisIt could not launch the compute engine on "
                "host \"%s\".", ek.HostName().c_str());
        Error(msg);
    }
    CATCH(CancelledConnectException)
    {
        // Delete the new engine since it was not launched
        delete newEngine;
        ViewerRemoteProcessChooser::Instance()->ClearCache(ek.HostName());

        // Tell the user that the engine was not launched
        SNPRINTF(msg, 250, "The launch of the compute engine on "
                "host \"%s\" has been cancelled.", ek.HostName().c_str());
        Error(msg);
    }
    ENDTRY

    // Clear the status message.
    ClearStatus();

    // Delete the connection dialog
    delete dialog;

    return success;
}

// ****************************************************************************
//  Method:  ViewerEngineManager::ConnectSim
//
//  Purpose:
//    Connect to a running simulation code.
//
//  Arguments:
//      engineKey      contains the host and sim-file name for the simulation
//      args           the arguments to pass to the engine
//      simHost        the host name where the simulation is listening
//      numRestarts    the port number where the simulation is listening
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 26, 2004
//
//  Modifications:
//    Jeremy Meredith, Fri Apr  2 14:38:37 PST 2004
//    Made restartArguments on a per-hostname (per-engine) basis.
//    Note that we keep using restartArguments here (but not in CreateEngine)
//    because this method never uses the chooser, so any extra arguments must
//    go through the restartArguments instead of the chooser's profile cache.
//
// ****************************************************************************
bool
ViewerEngineManager::ConnectSim(const EngineKey &ek,
                                const stringVector &args,
                                const string &simHost,
                                int simPort)
{
    //
    // Check if an engine already exists for the host.
    //
    if (EngineExists(ek))
        return true;

    bool success = false;

    //
    // If an engine for the host doesn't already exist, create one.
    //
    EngineProxy *newEngine = new EngineProxy;

    //
    // Add some arguments to the engine proxy before we try to
    // launch the engine.
    //
    AddArguments(newEngine, args);

    //
    // Copy the arguments into the restart arguments that are
    // used to restart failed engines.
    //
    restartArguments[ek] = args;

    //
    // Send a status message.
    //
    char msg[250];
    SNPRINTF(msg, 250, "Connecting to simulation at %s:%d", simHost.c_str(), simPort);
    Status(msg);

    //
    // Add the new engine proxy to the engine list.
    //
    TRY
    {
        // Get the client machine name options
        HostProfile::ClientHostDetermination chd;
        string clientHostName;
        GetClientMachineNameOptions(ek.HostName(), chd, clientHostName);

        // Get the ssh port options
        bool manualSSHPort;
        int  sshPort;
        GetSSHPortOptions(ek.HostName(), manualSSHPort, sshPort);

        //
        // Launch the engine.
        //
        typedef struct {string h; int p;} SimData;
        SimData simData;
        // The windows compiler can't accept non aggregate types in an
        // initializer list so initialize them like this:
        simData.h = simHost;
        simData.p = simPort;

        newEngine->Create(ek.HostName(),  chd, clientHostName,
                          manualSSHPort, sshPort,
                          SimConnectThroughLauncher, (void *)&simData,
                          true);

        engines[ek] = newEngine;

        // Make the engine manager observe the proxy's status atts.
        newEngine->GetStatusAttributes()->Attach(this);

        // Now that the new engine is in the list, tell the GUI.
        UpdateEngineList();

        // Success!
        success = true;
    }
    CATCH2(BadHostException, e)
    {
        // Delete the new engine since it could not launch anyway.
        delete newEngine;
        ViewerRemoteProcessChooser::Instance()->ClearCache(ek.HostName());

        // Tell the user that the engine could not be launched.
        SNPRINTF(msg, 250, "VisIt could not launch a compute engine on host "
                 "\"%s\" because that host does not exist.",
                 e.GetHostName().c_str());
        Error(msg);
    }
    CATCH(IncompatibleVersionException)
    {
        // Delete the new engine since talking to it could be bad since
        // it is a different version.
        delete newEngine;
        ViewerRemoteProcessChooser::Instance()->ClearCache(ek.HostName());

        // Tell the user that the engine is a different version.
        SNPRINTF(msg, 250, "VisIt cannot use the compute engine on "
                 "host \"%s\" because the engine has an incompatible "
                 " version number.", ek.HostName().c_str());
        Error(msg);
    }
    CATCH(IncompatibleSecurityTokenException)
    {
        // Delete the new engine since talking to it could be bad since
        // it did not provide the right credentials.
        delete newEngine;
        ViewerRemoteProcessChooser::Instance()->ClearCache(ek.HostName());

        // Tell the user that the engine is a different version.
        SNPRINTF(msg, 250, "VisIt cannot use the compute engine on host \"%s\""
                 "because the compute engine did not provide the proper "
                 "credentials.", ek.HostName().c_str());
        Error(msg);
    }
    CATCH(CouldNotConnectException)
    {
        // Delete the new engine since it was not launched
        delete newEngine;
        ViewerRemoteProcessChooser::Instance()->ClearCache(ek.HostName());

        // Tell the user that the engine was not launched
        SNPRINTF(msg, 250, "VisIt could not launch the compute engine on "
                 "host \"%s\".", ek.HostName().c_str());
        Error(msg);
    }
    CATCH(CancelledConnectException)
    {
        // Delete the new engine since it was not launched
        delete newEngine;
        ViewerRemoteProcessChooser::Instance()->ClearCache(ek.HostName());

        // Tell the user that the engine was not launched
        SNPRINTF(msg, 250, "The launch of the compute engine on "
                 "host \"%s\" has been cancelled.", ek.HostName().c_str());
        Error(msg);
    }
    ENDTRY

    // Clear the status message.
    ClearStatus();

    return success;
}

// ****************************************************************************
//  Method: ViewerEngineManager::CloseEngines
//
//  Purpose:
//      Close all the currently open engines.
//
//  Programmer: Eric Brugger
//  Creation:   September 23, 2000
//
//  Modifications:
//    Hank Childs, Mon Oct 16 11:11:26 PDT 2000
//    Fixed up memory leak.
//
//    Brad Whitlock, Fri Dec 27 14:42:36 PST 2002
//    I added debugging information.
//
//    Jeremy Meredith, Fri Mar 26 16:59:59 PST 2004
//    Changed to use a map and be aware of simulations.
//
// ****************************************************************************

void
ViewerEngineManager::CloseEngines()
{
    //
    // If we have any engines close them all and free any storage
    // associatied with them.
    //
    for (EngineMap::iterator i = engines.begin() ; i != engines.end() ; i++)
    {
        const EngineKey &key = i->first;
        EngineProxy *engine  = i->second;

        if (key.IsSimulation())
        {
            debug1 << "Disconnecting from simulation \""<<key.SimName().c_str()
                   <<"\" on host " << key.HostName().c_str() << "." << endl;
        }
        else
        {
            debug1 << "Closing compute engine on host " << key.HostName().c_str()
                   << "." << endl;
        }

        engine->GetStatusAttributes()->Detach(this);
        TRY
        {
            engine->Close();
        }
        CATCHALL(...)
        {
            debug1 << "Caught an exception while closing the engine." << endl;
        }
        ENDTRY

        delete i->second;
    }
}

// ****************************************************************************
// Method: ViewerEngineManager::CloseEngine
//
// Purpose: 
//   This is a public method that closes down the engine on the specified host.
//
// Arguments:
//   hostName : The hostname of the engine to close.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 30 12:38:54 PDT 2001
//
// Modifications:
//   Brad Whitlock, Mon Sep 24 14:21:51 PST 2001
//   Made it use the real hostname when printing out the message if the
//   hostname is "localhost".
//
//   Brad Whitlock, Mon Feb 25 09:53:20 PDT 2002
//   Changed code to account for the fact that GetEngineIndex will now
//   return -1 if an engine is not in the list.
//
//   Jeremy Meredith, Wed Aug 14 17:27:00 PDT 2002
//   Added code to clear the cache for the engine launcher.
//
//   Jeremy Meredith, Thu Jun 26 10:48:20 PDT 2003
//   Renamed ViewerEngineChooser to ViewerRemoteProcessChooser.
//
//   Jeremy Meredith, Fri Mar 26 16:59:59 PST 2004
//   Use a map of engines based on a key, and be aware of simulations.
//
// ****************************************************************************

void
ViewerEngineManager::CloseEngine(const EngineKey &ek)
{
    // Since we're closing the engine intentionally, let us change
    // the options the next time we launch an engine
    ViewerRemoteProcessChooser::Instance()->ClearCache(ek.HostName());

    char message[200];

    // We found an engine.
    if (EngineExists(ek))
    {
        RemoveEngine(ek, true);
        UpdateEngineList();

        SNPRINTF(message, 200, "Closed the compute engine on host %s.",
                 ek.HostName().c_str());
        Message(message);
    }
    else
    {
        SNPRINTF(message, 200, "Cannot close the compute engine on "
                 "host %s because there is no compute engine running on "
                 "that host.", ek.HostName().c_str());
        Error(message);
    }
}

// ****************************************************************************
// Method: ViewerEngineManager::InterruptEngine
//
// Purpose: 
//   Interrupts the engine on the specified host.
//
// Arguments:
//   hostName : The hostname on which the engine is running.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 30 12:45:15 PDT 2001
//
// Modifications:
//   Jeremy Meredith, Tue Jul  3 15:05:50 PDT 2001
//   Enabled engine execution interruption.
//
//   Brad Whitlock, Mon Sep 24 14:21:51 PST 2001
//   Made it use the real hostname when printing out the message if the
//   hostname is "localhost".
//
//   Brad Whitlock, Mon Feb 25 09:53:20 PDT 2002
//   Changed code to account for the fact that GetEngineIndex will now
//   return -1 if an engine is not in the list.
//
//   Jeremy Meredith, Fri Mar 26 16:59:59 PST 2004
//   Use a map of engines based on a key, and be aware of simulations.
//
// ****************************************************************************

void
ViewerEngineManager::InterruptEngine(const EngineKey &ek)
{
    char message[200];

    // We found an engine.
    if (EngineExists(ek))
    {
        EngineProxy *engine = engines[ek];
        engine->Interrupt();

        SNPRINTF(message, 200, "Interrupting the compute engine on host %s.",
                 ek.HostName().c_str());
        Message(message);
    }
    else
    {
        SNPRINTF(message, 200, "Cannot interrupt the compute engine on host %s "
                 "because there is no compute engine running on that host.",
                 ek.HostName().c_str());
        Error(message);
    }
}

// ****************************************************************************
// Method: ViewerEngineManager::InExecute
//
// Purpose: 
//   Returns whether or not any engine is in the execute stage.
//
// Programmer: Brad Whitlock
// Creation:   Thu May 9 12:31:00 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerEngineManager::InExecute() const
{
    return executing;
}

// ****************************************************************************
// Method: ViewerEngineManager::SendKeepAlives
//
// Purpose: 
//   Sends a keep alive signal to all of the engines.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 12 11:46:01 PDT 2004
//
// Modifications:
//    Jeremy Meredith, Fri Mar 26 16:59:59 PST 2004
//    Use a map of engines based on a key, and be aware of simulations.
//
// ****************************************************************************

void
ViewerEngineManager::SendKeepAlives()
{
    if(!executing)
    {
        bool updateList = false;
        vector<EngineKey> failedEngines;

        for (EngineMap::iterator i = engines.begin() ; i != engines.end(); i++)
        {
            debug1 << "Sending keep alive signal to compute engine on host "
                   << i->first.HostName().c_str() << "." << endl;

            TRY
            {
                i->second->SendKeepAlive();
            }
            CATCHALL(...)
            {
                debug1 << "Caught an exception while sending a keep alive "
                          "signal to the engine."
                       << endl;
                failedEngines.push_back(i->first);
                updateList = true;
            }
            ENDTRY
        }

        while (failedEngines.size() > 0)
        {
            RemoveFailedEngine(failedEngines.front());
            failedEngines.pop_back();
        }

        // If we had to remove an engine, update the list on the client.
        if(updateList)
            UpdateEngineList();
    }
}

// ****************************************************************************
// Method:  ViewerEngineManager::GetEngine
//
// Purpose:
//   find the engine from a hostname
//
// Arguments:
//   hostName : The hostname of the engine to find
//
// Programmer:  Jeremy Meredith
// Creation:    November  8, 2001
//
// Modifications:
//   Brad Whitlock, Mon Feb 25 09:53:20 PDT 2002
//   Moved some error message code into the new LaunchMessage method.
//   Changed code to account for the fact that GetEngineIndex will now
//   return -1 if an engine is not in the list.
//
//   Brad Whitlock, Mon Dec 30 15:42:12 PST 2002
//   I made it use the restart arguments.
//
//   Jeremy Meredith, Wed Mar 17 15:09:07 PST 2004
//   Inserted "false" before numRestarts so that it wouldn't take numRestarts
//   as the value for skipChooser.
//
//   Jeremy Meredith, Fri Mar 26 16:59:59 PST 2004
//   Use a map of engines based on a key, and be aware of simulations.
//
//   Jeremy Meredith, Fri Apr  2 14:29:25 PST 2004
//   Made restartArguments be saved on a per-host (per-enginekey) basis.
//
// ****************************************************************************

EngineProxy *
ViewerEngineManager::GetEngine(const EngineKey &ek)
{
    //
    // If the engine doesn't exist, try and launch one. If it cannot be done,
    // it is an error so return without doing the rest of the plot.
    //
    if (!EngineExists(ek))
    {
        // Send a message to the client indicating that we're launching a
        // new engine.
        LaunchMessage(ek);

        if (ek.SimName() != "")
        {
            // Can't relaunch a simulation
            return NULL;
        }

        // Try to launch an engine.
        CreateEngine(ek, restartArguments[ek], false, numRestarts);

        // If no engine was launched, return.
        if (!EngineExists(ek))
            return NULL;
    }

    //
    // Return the engine
    //
    return engines[ek];
}

// ****************************************************************************
// Method: ViewerEngineManager::LaunchMessage
//
// Purpose: 
//   Sends a message to the client that we're about to launch a new compute
//   engine.
//
// Arguments:
//   hostName : The host where we're going to launch the new compute engine.
//
// Programmer: Brad Whitlock
// Creation:   Mon Feb 25 09:31:14 PDT 2002
//
// Modifications:
//   Jeremy Meredith, Fri Mar 26 16:59:59 PST 2004
//   Made it be aware of simulations and issue an warning that we were not
//   connected instead of informing the user that we were about to restart
//   an engine for that host.
//
// ****************************************************************************

void
ViewerEngineManager::LaunchMessage(const EngineKey &ek)  const
{
    char message[200];
    if (ek.SimName() != "")
    {
        SNPRINTF(message, 200, "VisIt is not connected to the simulation '%s' "
                 "on host %s", ek.SimName().c_str(), ek.HostName().c_str());
    }
    else
    {
        SNPRINTF(message, 200, "VisIt could not find a compute engine to use "
                 "for the plot on host %s. VisIt will try to launch a compute "
                 "engine on that host.", ek.HostName().c_str());
    }
        
    Warning(message);
}

// ****************************************************************************
// Method: ViewerEngineManager::ExternalRender
//
// Purpose: 
//   Sends various RPC's to engine(s) necessary to perform an external render
//   request.
//
// Arguments:
//   For each plot to be externally rendered, we have the following...
//
//   pluginIDsList: the pluginID for the plot
//   hostLists:     the host of the engine where the plot will be computed
//   plotIdsList:   the network (or plot) id of the plot on the assoc. engine
//   attsList:      the attributes of the plot
//
//   winAtts:       window attributes for the window servicing the external
//                  render request.
//   annotAtts:     annotation attributes for the window servicing the external
//                  render request.
//
//   shouldTurnOffScalableRendering: set to true if the engine has sent results
//                  back to the viewer indicating that scalable rendering is
//                  no longer required for this window.
//   imgList:       the list of images, one for each engine's render, returned
//                  to the caller.
//
// Programmer: Mark C. Miller 
// Creation:   November 11, 2003 
//
// Modifications:
//   Brad Whitlock, Thu Dec 11 08:27:14 PDT 2003
//   Fixed a small coding error related to the declaration of engineIndex.
//
//   Mark C. Miller, Wed Feb  4 19:47:30 PST 2004
//   Added setting of engineIndex to the loop over plots. Added use of
//   RealHostName in computing the engineIndex
//
//   Jeremy Meredith, Mon Mar 22 17:57:32 PST 2004
//   Added a check to make sure the engine actually existed before clearing
//   its status inside the exception handler.  It is possible the engine died
//   or was closed when it gets to that piece of code.
//
//   Mark C. Miller, Mon Mar 29 14:52:08 PST 2004
//   Added bool to control annotations on engine
//
//   Jeremy Meredith, Fri Mar 26 16:59:59 PST 2004
//   Use a map of engines based on a key, and be aware of simulations.
//
//   Mark C. Miller, Wed Apr 14 16:41:32 PDT 2004
//   Added argument for extents type string.
//   Passed extents type string in call to SetWinAnnotAtts
//
// ****************************************************************************

bool
ViewerEngineManager::ExternalRender(vector<const char*> pluginIDsList,
                                    vector<EngineKey> engineKeysList,
                                    intVector plotIdsList,
                                    vector<const AttributeSubject *> attsList,
                                    WindowAttributes winAtts,
                                    AnnotationAttributes annotAtts,
                                    string extStr,
                                    bool& shouldTurnOffScalableRendering,
                                    bool doAllAnnotations,
                                    vector<avtImage_p>& imgList)
{
    bool retval = true;
    EngineKey ek;

    // container for per-engine vector of plot ids 
    map<EngineKey,vector<int> > perEnginePlotIds;

    TRY
    {
        // send per-plot RPCs
        for (int i = 0; i < plotIdsList.size(); i++)
        {
            ek = engineKeysList[i];
            if (!UpdatePlotAttributes(ek, pluginIDsList[i],
                                      plotIdsList[i],attsList[i]))
            {
                retval = false;
                char msg[200];
                SNPRINTF(msg,200,"Unsuccessful attempt to update plot "
                         "attributes for plot ID %d, (%d of %d)",
                         plotIdsList[i], i, plotIdsList.size());
                EXCEPTION1(VisItException, msg); 
            }
            perEnginePlotIds[ek].push_back(plotIdsList[i]);
        }

        int numEnginesToRender = perEnginePlotIds.size();
        bool sendZBuffer = numEnginesToRender > 1 ? true : false;

        // send per-engine RPCs 
        std::map<EngineKey, intVector>::iterator pos;
        for (pos = perEnginePlotIds.begin(); pos != perEnginePlotIds.end();
                                                                         pos++)
        {
            EngineKey ek = pos->first;

            if (!SetWinAnnotAtts(ek, &winAtts, &annotAtts, extStr))
            {
                retval = false;
                char msg[200];
                SNPRINTF(msg,200,"Unsuccessful attempt to update window "
                         "attributes for engine %s", ek.HostName().c_str());
                EXCEPTION1(VisItException, msg); 
            }

            avtDataObjectReader_p rdr = GetDataObjectReader(sendZBuffer,
                                                            ek,
                                                            pos->second,
                                                            doAllAnnotations);

            if (*rdr == NULL)
            {
                retval = false;
                char msg[200];
                SNPRINTF(msg,200,"obtained null data reader for rendered image "
                         "for engine %s", ek.HostName().c_str());
                EXCEPTION1(VisItException, msg); 
            }

            // check to see if engine decided that SR mode is no longer necessary
            if (rdr->InputIs(AVT_NULL_IMAGE_MSG))
            {
               shouldTurnOffScalableRendering = true;
               break;
            }

            // do some magic to update the network so we don't need the reader anymore
            avtDataObject_p tmpDob = rdr->GetOutput();
            avtPipelineSpecification_p spec = 
                tmpDob->GetTerminatingSource()->GetGeneralPipelineSpecification();
            tmpDob->Update(spec);

            // put the resultant image in the returned list
            avtImage_p img;
            CopyTo(img,tmpDob);
            imgList.push_back(img);
        }

    }
    CATCH(LostConnectionException)
    {
#ifndef VIEWER_MT
        EndEngineExecute();
#endif
        // Remove the specified engine from the list of engines.
        RemoveFailedEngine(ek);
        UpdateEngineList();
    }
    CATCH(VisItException)
    {
#ifndef VIEWER_MT
        EndEngineExecute();
#endif
        // Send a message to the client to clear the status for the
        // engine that had troubles.
        if (EngineExists(ek))
            ClearStatus(ek.ID().c_str());

        //
        //  Let calling method handle this exception. 
        //
        RETHROW;
    }
    ENDTRY

    return retval;
}

// ****************************************************************************
//  Method: ViewerEngineManager::GetDataObjectReader
//
//  Purpose:
//      Return a pointer to a avt data object reader for the specified plot at
//      the specified time.
//
//  Arguments:
//      plot    The plot for which to create the data set.
//
//  Returns:    A pointer to the avt data object reader.
//
//  Programmer: Eric Brugger
//  Creation:   September 23, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Sep 28 22:16:55 PDT 2000
//    Made the return type be an avtDataset.
//
//    Hank Childs, Tue Oct 17 08:26:01 PDT 2000
//    Made the return type be an avtDataSetReader and changed the name of the
//    method to GetDataSetReader.
//
//    Kathleen Bonnell, Fri Dec  1 16:22:08 PST 2000
//    Added call to plot->Setup.
//
//    Hank Childs, Fri Dec 29 08:39:27 PST 2000
//    Changed DataSetReader to DataObjectReader.
//
//    Brad Whitlock, Tue Apr 24 12:34:08 PDT 2001
//    Added code to handle connections being lost.
//
//    Brad Whitlock, Mon Apr 30 14:21:29 PST 2001
//    Added code to update the engine list that is displayed in the GUI.
//
//    Kathleen Bonnell, Wed May  2 14:18:47 PDT 2001 
//    Added catch block to catch and rethrow general VisItExceptions. 
//
//    Hank Childs, Wed Jun 13 11:03:30 PDT 2001
//    Added sil restrictions.
//
//    Hank Childs, Mon Jul 23 11:07:03 PDT 2001
//    Removed call to plot->Setup.
//
//    Brad Whitlock, Wed Aug 22 17:10:23 PST 2001
//    Added code to clear the status for the engine in the client.
//
//    Brad Whitlock, Mon Oct 22 18:33:37 PST 2001
//    Changed the exception keywords to macros.
//
//    Jeremy Meredith, Fri Nov  9 10:12:22 PST 2001
//    Made this use the new GetEngine function.
//
//    Brad Whitlock, Fri Feb 22 14:34:16 PST 2002
//    Changed how some exceptional cases are handled so RPC calls are less
//    prone to error.
//
//    Brad Whitlock, Tue May 7 16:13:42 PST 2002
//    Passed a callback function to the engine's Execute method. The callback
//    processes some events for the viewer while we're waiting for the
//    engine to return a result.
//
//    Sean Ahern, Fri Apr 19 16:12:29 PDT 2002
//    Updated to use Jeremy's parser.  Removed debugging.
//
//    Sean Ahern, Fri Jun 28 11:19:24 PDT 2002
//    Made the ViewerExpressionList be a singleton, accessible from anywhere.
//
//    Brad Whitlock, Thu Jul 18 14:31:37 PST 2002
//    I moved some of the expression code into ViewerExpressionList.
//
//    Sean Ahern, Thu Oct 17 17:05:45 PDT 2002
//    Moved the expression parsing into the engine.
//
//    Jeremy Meredith, Thu Oct 24 16:17:19 PDT 2002
//    Added setting of the plot material options from the global options.
//
//    Sean Ahern, Wed Nov 20 14:44:16 PST 2002
//    Moved the expression stuff that was here into the engine.
//
//    Eric Brugger, Thu Dec 19 11:01:49 PST 2002
//    I added keyframing support.
//
//    Brad Whitlock, Wed Mar 26 09:44:56 PDT 2003
//    I added support for virtual databases.
//
//    Jeremy Meredith, Mon Sep 15 17:23:17 PDT 2003
//    Removed the call to SetFinalVariableName.  It wasn't doing anything
//    and was likely the wrong long-term infrastructure, so it was removed.
//
//    Hank Childs, Fri Mar  5 15:59:20 PST 2004
//    Add file format type when defining a virtual database.
//
//    Brad Whitlock, Sat Jan 31 22:28:40 PST 2004
//    I removed the frame argument because plots now know where they
//    are in time so we don't need to pass it.
//
//    Jeremy Meredith, Fri Mar 26 16:59:59 PST 2004
//    Use a map of engines based on a key, and be aware of simulations.
//
//    Kathleen Bonnell, Wed Mar 31 16:40:15 PST 2004 
//    Only defineVirtualDatabase and ReadDataObject if plot isn't using a 
//    cloned network.
//
//    Mark C. Miller, Wed Apr 14 16:41:32 PDT 2004
//    Added code to pass extents type string to SetWinAnnotAtts
//
// ****************************************************************************

avtDataObjectReader_p
ViewerEngineManager::GetDataObjectReader(ViewerPlot *const plot)
{
    // The return value.
    avtDataObjectReader_p retval(NULL);

    //
    // Read the variable
    //
    EngineKey ek(plot->GetEngineKey());
    if (!EngineExists(ek))
        return retval;

    EngineProxy *engine = engines[ek];

    TRY
    {
        // 
        // Only do this if plot isn't using a cloned network.
        // 
        if (!plot->CloneNetwork())
        {
            int state = plot->GetState();

            // Is the plot's database a virtual database? If it is, then we
            // need define the virtual database on the engine.
            ViewerFileServer *fileServer = ViewerFileServer::Instance();
            const avtDatabaseMetaData *md =
                fileServer->GetMetaData(plot->GetHostName(),
                                        plot->GetDatabaseName());
            if(md && md->GetIsVirtualDatabase())
            {
                engine->DefineVirtualDatabase(md->GetFileFormat().c_str(),
                                              plot->GetDatabaseName(),
                                              md->GetTimeStepPath(),
                                              md->GetTimeStepNames(), state);
            }

            // Tell the engine to generate the plot
                engine->ReadDataObject(md->GetFileFormat(), 
                                       plot->GetDatabaseName(),
                                       plot->GetVariableName(),
                                       state, plot->GetSILRestriction(),
                                       *GetMaterialClientAtts());
        }

        //
        // Apply any operators.
        //
        bool success = true;
        for (int o=0; o < plot->GetNOperators() && success; o++)
        {
            success &= plot->GetOperator(o)->ExecuteEngineRPC();
        }

        //
        // Do the plot.
        //
        if (success)
            success = plot->ExecuteEngineRPC();

        // MCM_FIX_ME
        ViewerWindowManager *vwm = ViewerWindowManager::Instance();
        ViewerWindow *w = vwm->GetActiveWindow();
        bool replyWithNullData = w->GetScalableRendering();
        if (w->IsChangingScalableRenderingMode(false))
        {
           WindowAttributes winAtts = w->GetWindowAttributes();
           AnnotationAttributes annotAtts = *(w->GetAnnotationAttributes());
           string extStr = avtExtentType_ToString(w->GetViewExtentsType());
           engine->SetWinAnnotAtts(&winAtts,&annotAtts,extStr);
        }

        //
        // Return the result.
        //
        if (success)
        {
#ifdef VIEWER_MT
            retval = engine->Execute(replyWithNullData, 0, 0);

            // deal with possibility that engine may decide to scalable render
            //  this output
            if (!replyWithNullData && retval->InputIs(AVT_NULL_DATASET_MSG))
            {
               // ask for the engine's output as null data 'cause the 
               // avtDataObject in the current reader is just the message that 
               // it exceeded threshold
               retval = engine->Execute(true, 0, 0);

               // now, tell viewer to go into SR mode
               w->SendScalableRenderingModeChangeMessage(true);
            }
#else
            BeginEngineExecute();

            retval = engine->Execute(replyWithNullData, 
                                     ViewerSubject::ProcessEventsCB,
                                     (void *)viewerSubject);

            // deal with possibility that engine may decide to scalable render 
            // this output
            if (!replyWithNullData && retval->InputIs(AVT_NULL_DATASET_MSG))
            {
               // ask for the engine's output as null data 'cause the 
               // avtDataObject in the current reader is just the message that 
               //it exceeded threshold
               retval = engine->Execute(true, 0, 0);

               // now, tell viewer to go into SR mode
               w->SendScalableRenderingModeChangeMessage(true);
            }

            EndEngineExecute();
#endif
        }
    }
    CATCH(LostConnectionException)
    {
#ifndef VIEWER_MT
        EndEngineExecute();
#endif
        // Remove the specified engine from the list of engines.
        RemoveFailedEngine(ek);
        UpdateEngineList();
    }
    CATCH(VisItException)
    {
#ifndef VIEWER_MT
        EndEngineExecute();
#endif
        // Send a message to the client to clear the status for the
        // engine that had troubles.
        ClearStatus(ek.ID().c_str());

        //
        //  Let calling method handle this exception. 
        //
        RETHROW;
    }
    ENDTRY

    return retval;
}

// ****************************************************************************
//  Method: ViewerEngineManager::UseDataObjectReader
//
//  Purpose:
//      Return a pointer to an avt data object reader for a currently existing
//      plot. This is only ever used in the transition into and out of 
//      Scalable Rendering mode.
//
//  Returns:    A pointer to the avt data object reader.
//
//  Programmer: Mark C. Miller 
//  Creation:   Wed Oct 29 16:56:14 PST 2003 
//
//  Modifications:
//    Jeremy Meredith, Fri Mar 26 16:59:59 PST 2004
//    Use a map of engines based on a key, and be aware of simulations.
//
// ****************************************************************************

avtDataObjectReader_p
ViewerEngineManager::UseDataObjectReader(ViewerPlot *const plot,
                                         bool turningOffScalableRendering)
{
    // The return value.
    avtDataObjectReader_p retval(NULL);

    EngineKey ek(plot->GetEngineKey());
    if (!EngineExists(ek))
        return retval;

    EngineProxy *engine = engines[ek];

    TRY
    {
        // tell engine which network to re-use
        UseNetwork(ek, plot->GetNetworkID());

        bool replyWithNullData = !turningOffScalableRendering; 

        //
        // Return the result.
        //
        {
#ifdef VIEWER_MT
            retval = engine->Execute(replyWithNullData, 0, 0);
#else
            BeginEngineExecute();

            retval = engine->Execute(replyWithNullData, ViewerSubject::ProcessEventsCB,
                                     (void *)viewerSubject);

            EndEngineExecute();
#endif
        }
    }
    CATCH(LostConnectionException)
    {
#ifndef VIEWER_MT
        EndEngineExecute();
#endif
        // Remove the specified engine from the list of engines.
        RemoveFailedEngine(ek);
        UpdateEngineList();
    }
    CATCH(VisItException)
    {
#ifndef VIEWER_MT
        EndEngineExecute();
#endif
        // Send a message to the client to clear the status for the
        // engine that had troubles.
        ClearStatus(ek.ID().c_str());

        //
        //  Let calling method handle this exception. 
        //
        RETHROW;
    }
    ENDTRY

    return retval;
}


// ****************************************************************************
//  Method: ViewerEngineManager::GetDataObjectReader
//
//  Purpose:
//      Return a pointer to an avt data object reader for the rendered image
//      of the plots specified in the list of plotIds
//
//  Arguments:
//
//  Returns:    A pointer to the avt data object reader.
//
//  Programmer: Mark C. Miller 
//  Creation:   08Apr03 
//
//  Modifications:
//    Mark C. Miller, Mon Mar 29 14:52:08 PST 2004
//    Added bool to control annotations on engine
//
//    Jeremy Meredith, Fri Mar 26 16:59:59 PST 2004
//    Use a map of engines based on a key, and be aware of simulations.
//
//    Mark C. Miller, Tue Apr 20 07:44:34 PDT 2004
//    Added args to the proxy's render method to support warning call backs
//    comming from the engine.
//
// ****************************************************************************

avtDataObjectReader_p
ViewerEngineManager::GetDataObjectReader(bool sendZBuffer,
                                         const EngineKey &ek,
                                         intVector ids,
                                         bool doAllAnnotations)
{
    // The return value.
    avtDataObjectReader_p retval(NULL);

    if (!EngineExists(ek))
        return retval;

    TRY
    {
        retval = engines[ek]->Render(sendZBuffer, ids, !doAllAnnotations,
                                     ViewerSubject::ProcessEventsCB,
                                     (void *)viewerSubject);
    }
    CATCH(LostConnectionException)
    {
        // Remove the specified engine from the list of engines.
        RemoveFailedEngine(ek);
        UpdateEngineList();
    }
    CATCH(VisItException)
    {
        // Send a message to the client to clear the status for the
        // engine that had troubles.
        ClearStatus(ek.ID().c_str());

        //
        //  Let calling method handle this exception. 
        //
        RETHROW;
    }
    ENDTRY

    return retval;
}

// ****************************************************************************
// Method: ViewerEngineManager::BeginEngineExecute
//
// Purpose: 
//   Sends a message to the client that prevents it from doing things that
//   will modify engine execution.
//
// Programmer: Brad Whitlock
// Creation:   Tue May 7 17:18:57 PST 2002
//
// Modifications:
//   Brad Whitlock, Wed Oct 30 15:01:01 PST 2002
//   Moved code to ViewerWindowManager.
//
// ****************************************************************************

void
ViewerEngineManager::BeginEngineExecute()
{
    executing = true;
    ViewerWindowManager::Instance()->BeginEngineExecute();
}

// ****************************************************************************
// Method: ViewerEngineManager::EndEngineExecute
//
// Purpose: 
//   Sends a message to the client that allows it to do things that will
//   modify engine execution.
//
// Programmer: Brad Whitlock
// Creation:   Tue May 7 17:22:03 PST 2002
//
// Modifications:
//   Brad Whitlock, Wed Oct 30 15:01:01 PST 2002
//   Moved code to ViewerWindowManager.
//   
// ****************************************************************************

void
ViewerEngineManager::EndEngineExecute()
{
    executing = false;
    ViewerWindowManager::Instance()->EndEngineExecute();
}

// ****************************************************************************
// Method: ViewerEngineManager::OpenDatabase
//
// Purpose: 
//   Engine OpenDatabase RPC wrapped for safety.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 10 15:27:40 PST 2002
//
// Modifications:
//    Hank Childs, Fri Mar  5 11:13:32 PST 2004
//    Added a format.
//
//    Jeremy Meredith, Fri Mar 26 16:59:59 PST 2004
//    Use a map of engines based on a key, and be aware of simulations.
//
// ****************************************************************************

bool
ViewerEngineManager::OpenDatabase(const EngineKey &ek, const char *format,
                                  const char *filename, int time)
{
    ENGINE_PROXY_RPC_BEGIN("OpenDatabase");
    engine->OpenDatabase(format, filename, time);
    ENGINE_PROXY_RPC_END;
}

// ****************************************************************************
// Method: ViewerEngineManager::DefineVirtualDatabase
//
// Purpose: 
//   Engine DefineVirtualDatabase RPC wrapped for safety.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 25 14:15:21 PST 2003
//
// Modifications:
//    Hank Childs, Fri Mar  5 16:02:03 PST 2004
//    Pass along the format as well.
//
//    Jeremy Meredith, Fri Mar 26 16:59:59 PST 2004
//    Use a map of engines based on a key, and be aware of simulations.
//
// ****************************************************************************

bool
ViewerEngineManager::DefineVirtualDatabase(const EngineKey &ek,
                      const char *format, const char *dbName, const char *path,
                      const stringVector &files, int time)
{
    ENGINE_PROXY_RPC_BEGIN("DefineVirtualDatabase");
    engine->DefineVirtualDatabase(format, dbName, path, files, time);
    ENGINE_PROXY_RPC_END;
}

// ****************************************************************************
// Method: ViewerEngineManager::ApplyOperator
//
// Purpose: 
//   Engine ApplyOperator RPC wrapped for safety.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 22 14:50:06 PST 2002
//
// Modifications:
//    Jeremy Meredith, Thu Nov 21 11:11:11 PST 2002
//    Changed catch block to rethrow.
//
//    Jeremy Meredith, Fri Mar 26 16:59:59 PST 2004
//    Use a map of engines based on a key, and be aware of simulations.
//
// ****************************************************************************

bool
ViewerEngineManager::ApplyOperator(const EngineKey &ek,
                                   const char *name,
    const AttributeSubject *atts)
{
    ENGINE_PROXY_RPC_BEGIN("ApplyOperator");
    engine->ApplyOperator(name, atts);
    ENGINE_PROXY_RPC_END_NORESTART_RETHROW;
}

// ****************************************************************************
// Method: ViewerEngineManager::MakePlot
//
// Purpose: 
//   Engine MakePlot RPC wrapped for safety.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 22 14:50:06 PST 2002
//
// Modifications:
//    Jeremy Meredith, Thu Nov 21 11:11:31 PST 2002
//    Changed catch block to rethrow.
//
//    Jeremy Meredith, Fri Mar 26 16:59:59 PST 2004
//    Use a map of engines based on a key, and be aware of simulations.
//
//    Eric Brugger, Tue Mar 30 14:54:04 PST 2004
//    Added the plot data extents.
//
// ****************************************************************************

bool
ViewerEngineManager::MakePlot(const EngineKey &ek, const char *name, 
    const AttributeSubject *atts, const vector<double> &extents,
    int *networkId)
{
    ENGINE_PROXY_RPC_BEGIN("MakePlot");
    *networkId = engine->MakePlot(name, atts, extents);
    ENGINE_PROXY_RPC_END_NORESTART_RETHROW;
}

// ****************************************************************************
// Method: ViewerEngineManager::UseNetwork
//
// Purpose: 
//   Engine UseNetwork RPC wrapped for safety.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 22 14:50:06 PST 2002
//
// Modifications:
//    Jeremy Meredith, Fri Mar 26 16:59:59 PST 2004
//    Use a map of engines based on a key, and be aware of simulations.
//
// ****************************************************************************

bool
ViewerEngineManager::UseNetwork(const EngineKey &ek, int id)
{
    ENGINE_PROXY_RPC_BEGIN("UseNetwork");
    engine->UseNetwork(id);
    ENGINE_PROXY_RPC_END;
}

// ****************************************************************************
// Method: ViewerEngineManager::UpdatePlotAttributes
//
// Purpose: 
//   Engine UpdatePlotAttributes RPC wrapped for safety.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 22 14:50:06 PST 2002
//
// Modifications:
//    Jeremy Meredith, Fri Mar 26 16:59:59 PST 2004
//    Use a map of engines based on a key, and be aware of simulations.
//
// ****************************************************************************

bool
ViewerEngineManager::UpdatePlotAttributes(const EngineKey &ek,
                                          const char *name, int id,
                                          const AttributeSubject *atts)
{
    ENGINE_PROXY_RPC_BEGIN("UpdatePlotAttributes");
    engine->UpdatePlotAttributes(name, id, atts);
    ENGINE_PROXY_RPC_END;
}

// ****************************************************************************
// Method: ViewerEngineManager::Pick
//
// Purpose: 
//   Engine Pick RPC wrapped for safety.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 22 14:50:06 PST 2002
//
// Modifications:
//    Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002 
//    Use different RPC_END macro, so that exceptions are rethrown, and failed
//    engines aren't restarted.
//   
//    Kathleen Bonnell, Wed Feb 26 10:56:19 PST 2003 
//    Change catch block to second version of rethrow, which will rethrow
//    the LostConnectionException, and possibly throw a NoEngineException.  
//
//    Jeremy Meredith, Fri Mar 26 16:59:59 PST 2004
//    Use a map of engines based on a key, and be aware of simulations.
//
// ****************************************************************************

bool
ViewerEngineManager::Pick(const EngineKey &ek,
                          const int nid, const PickAttributes *atts,
                          PickAttributes &retAtts)
{
    ENGINE_PROXY_RPC_BEGIN("Pick");
    engine->Pick(nid, atts, retAtts);
    ENGINE_PROXY_RPC_END_NORESTART_RETHROW2;
}

// ****************************************************************************
// Method: ViewerEngineManager::StartPick
//
// Purpose: 
//   Engine StartPick RPC wrapped for safety.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 22 14:50:06 PST 2002
//
// Modifications:
//    Jeremy Meredith, Fri Mar 26 16:59:59 PST 2004
//    Use a map of engines based on a key, and be aware of simulations.
//
// ****************************************************************************

bool
ViewerEngineManager::StartPick(const EngineKey &ek,
                               const bool flag, const int nid)
{
    ENGINE_PROXY_RPC_BEGIN("StartPick");
    engine->StartPick(flag, nid);
    ENGINE_PROXY_RPC_END;
}

// ****************************************************************************
// Method: ViewerEngineManager::SetWinAnnotAtts
//
// Purpose: 
//   Engine SetWinAnnotAtts RPC wrapped for safety.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 22 14:50:06 PST 2002
//
// Modifications:
//    Jeremy Meredith, Fri Mar 26 16:59:59 PST 2004
//    Use a map of engines based on a key, and be aware of simulations.
//
//    Mark C. Miller, Wed Apr 14 16:41:32 PDT 2004
//    Added extents type string argument
//
// ****************************************************************************

bool
ViewerEngineManager::SetWinAnnotAtts(const EngineKey &ek,
                                     const WindowAttributes *wa,
                                     const AnnotationAttributes *aa,
                                     const string extstr)
{
    ENGINE_PROXY_RPC_BEGIN("SetWinAnnotAtts");
    engine->SetWinAnnotAtts(wa,aa,extstr);
    ENGINE_PROXY_RPC_END;
}

// ****************************************************************************
// Method: ViewerEngineManager::ClearCache
//
// Purpose: 
//   Engine ClearCache RPC wrapped for safety.
//
// Notes:      Note that we're using a different ending macro that does not
//             attempt to restart the engine in the event of a failure.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 30 13:39:41 PST 2002
//
// Modifications:
//    Jeremy Meredith, Fri Mar 26 16:59:59 PST 2004
//    Use a map of engines based on a key, and be aware of simulations.
//
// ****************************************************************************

bool
ViewerEngineManager::ClearCache(const EngineKey &ek,
                                const char *dbName)
{
    ENGINE_PROXY_RPC_BEGIN("ClearCache");
    if (dbName == 0)
        engine->ClearCache();
    else
        engine->ClearCache(dbName);
    ENGINE_PROXY_RPC_END_NORESTART;
}

// ****************************************************************************
// Method: ViewerEngineManager::ClearCacheForAllEngines
//
// Purpose: 
//   Tells all engines to clear their caches.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 26 13:03:46 PST 2004
//
// Modifications:
//    Jeremy Meredith, Fri Mar 26 16:59:59 PST 2004
//    Use a map of engines based on a key, and be aware of simulations.
//
// ****************************************************************************

void
ViewerEngineManager::ClearCacheForAllEngines()
{
    for (EngineMap::iterator i = engines.begin() ; i != engines.end() ; i++)
    {
        const EngineKey &key = i->first;
        ClearCache(key);
    }
}

// ****************************************************************************
// Method: ViewerEngineManager::GetEngineList
//
// Purpose: 
//   Returns a pointer to the engine manager's list of engines.
//
// Returns:    A pointer to the engine manager's list of engines.
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 30 14:14:49 PST 2001
//
// Modifications:
//   
// ****************************************************************************
EngineList *
ViewerEngineManager::GetEngineList()
{
    //
    // If the engine list has not been allocated then do so.
    //
    if (clientEngineAtts == 0)
    {
        clientEngineAtts = new EngineList;
    }

    return clientEngineAtts;
}

// ****************************************************************************
// Method: ViewerEngineManager::RemoveEngine
//
// Purpose: 
//   Removes the engine at the specified list index.
//
// Arguments:
//   engineIndex : The index of the engine to remove.
//   close       : Whether or not to close down the engine before removing it.
//                 This is provided so we do not have to try and close engines
//                 that have crashed.
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 24 12:54:25 PDT 2001
//
// Modifications:
//   Brad Whitlock, Mon Feb 25 12:43:37 PDT 2002
//   Modified it to account for a slight change in engine indices.
//
//   Jeremy Meredith, Fri Mar 26 16:59:59 PST 2004
//   Use a map of engines based on a key, and be aware of simulations.
//
// ****************************************************************************
void
ViewerEngineManager::RemoveEngine(const EngineKey &ek, bool close)
{
    if (EngineExists(ek))
    {
        // Delete the entry in the engine list for the specified index.    
        engines[ek]->GetStatusAttributes()->Detach(this);
        if (close)
            engines[ek]->Close();
        delete engines[ek];
        engines.erase(ek);
    }
}

// ****************************************************************************
// Method: ViewerEngineManager::RemoveFailedEngine
//
// Purpose: 
//   Removes the specified dead engine from the engine list and tells the
//   viewer's client about it.
//
// Arguments:
//   engineIndex : The index of the engine to be removed.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 22 14:52:35 PST 2002
//
// Modifications:
//    Jeremy Meredith, Fri Mar 26 16:59:59 PST 2004
//    Use a map of engines based on a key, and be aware of simulations.
//
// ****************************************************************************

void
ViewerEngineManager::RemoveFailedEngine(const EngineKey &ek)
{
    // Tell the GUI about the error.
    char message[200];
    if (ek.SimName() != "")
    {
        SNPRINTF(message, 200, "VisIt has been disconnected from the "
                 "simulation '%s' on host %s",
                 ek.SimName().c_str(), ek.HostName().c_str());
    }
    else
    {
        SNPRINTF(message, 200, "The compute engine running on %s has exited "
                 "abnormally.", ek.HostName().c_str());
    }
    Error(message);

    // Send a message to the client to clear the status for the
    // engine that had troubles.
    ClearStatus(ek.ID().c_str());

    // Remove the specified engine from the list of engines.
    RemoveEngine(ek, false);
}

// ****************************************************************************
// Method: ViewerEngineManager::UpdateEngineList
//
// Purpose: 
//   Updates the list of engines that is displayed in the viewer's client.
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 30 14:18:44 PST 2001
//
// Modifications:
//   Jeremy Meredith, Thu May  3 17:56:05 PDT 2001
//   Removed the third argument from sort.  It should sort alphabetically
//   by default, and std::less was causing small portability problems.
//
//   Brad Whitlock, Wed Nov 27 13:27:48 PST 2002
//   I added code to send back other engine information to the client.
//
//   Jeremy Meredith, Fri Mar 26 16:59:59 PST 2004
//   Use a map of engines based on a key, and be aware of simulations.
//
// ****************************************************************************

void
ViewerEngineManager::UpdateEngineList()
{
    vector<EngineKey> ids;
    stringVector hostNames, simulationNames;
    intVector    numProcs, numNodes, loadBalancing;
    
    // Go through the list of engines and add each engine to the engine list
    // that gets returned to the viewer's client.
    int i;
    for (EngineMap::iterator it = engines.begin() ; it != engines.end(); it++)
    {
        ids.push_back(it->first);
    }

    // Sort the strings.
    if (ids.size() > 1)
        std::sort(ids.begin(), ids.end());

    // Add the other information about the engine.
    for(i = 0; i < ids.size(); ++i) 
    {
        EngineKey ek = ids[i];
        if (EngineExists(ek))
        {
            EngineProxy *engine = engines[ek];
            numProcs.push_back(engine->NumProcessors());
            numNodes.push_back(engine->NumNodes());
            loadBalancing.push_back(engine->LoadBalancing());
            hostNames.push_back(ek.HostName());
            simulationNames.push_back(ek.SimName());
        }
    }

    // Send the engine list to the viewer's client.
    clientEngineAtts->SetEngines(hostNames);
    clientEngineAtts->SetSimulationName(simulationNames);
    clientEngineAtts->SetNumProcessors(numProcs);
    clientEngineAtts->SetNumNodes(numNodes);
    clientEngineAtts->SetLoadBalancing(loadBalancing);
    clientEngineAtts->Notify();
}

// ****************************************************************************
// Method: ViewerEngineManager::Update
//
// Purpose: 
//   This method is called whenever the i'th engine proxy wants to send a
//   bit of status information along.
//
// Arguments:
//   TheChangedSubject : A pointer to the status attributes of the proxy that
//                       is sending status.
//
// Programmer: Brad Whitlock
// Creation:   Tue May 1 12:13:41 PDT 2001
//
// Modifications:
//    Jeremy Meredith, Fri Jun 29 15:11:08 PDT 2001
//    Made it check MessageType to determine the type of message.
//   
//    Jeremy Meredith, Tue Jul  3 15:07:23 PDT 2001
//    Added interruption of the engine.
//
//    Brad Whitlock, Wed Oct 17 16:08:54 PST 2001
//    Added support for warning messages from the engine.
//
//    Brad Whitlock, Tue Mar 26 09:52:29 PDT 2002
//    Modified the communication code.
//
//    Jeremy Meredith, Fri Mar 26 16:59:59 PST 2004
//    Use a map of engines based on a key, and be aware of simulations.
//
// ****************************************************************************

void
ViewerEngineManager::Update(Subject *TheChangedSubject)
{
    StatusAttributes *statusAtts = (StatusAttributes *)TheChangedSubject;

    // Find the key to the engine
    EngineKey ek;
    for (EngineMap::iterator i = engines.begin() ; i != engines.end() ; i++)
    {
        if (i->second->GetStatusAttributes() == statusAtts)
        {
            ek = i->first;
            break;
        }
    }

    // Relay the message to the GUI.
    if (statusAtts->GetClearStatus())
    {
        // Send a message to clear the status bar.
        if (EngineExists(ek))
            ClearStatus(ek.ID().c_str());
        else
            ClearStatus();
    }
    else if (statusAtts->GetMessageType() == 1)
    {
        // The message field was selected.
        if (EngineExists(ek))
            Status(ek.ID().c_str(), statusAtts->GetMessage().c_str());
        else
            Status(statusAtts->GetMessage().c_str());
    }
    else if (statusAtts->GetMessageType() == 3)
    {
        // The message field was selected.
        if (EngineExists(ek))
        {
            string tmp1("The compute engine running on host ");
            string tmp2(ek.HostName());
            string tmp3(" issued the following warning: ");
            string tmp;
            tmp = tmp1 + tmp2 + tmp3 + statusAtts->GetMessage();
            Warning(tmp.c_str());
        }
        else
            Warning(statusAtts->GetMessage().c_str());
    }
    else if (EngineExists(ek))
    {
        Status(ek.ID().c_str(), statusAtts->GetPercent(),
               statusAtts->GetCurrentStage(),
               statusAtts->GetCurrentStageName().c_str(),
               statusAtts->GetMaxStage());
    }

    if (viewerSubject->ReadFromParentAndCheckForInterruption())
    {
        InterruptEngine(ek);
    }
}

// ****************************************************************************
//  Method: ViewerEngineManager::GetImage
//
//  Purpose:
//      Gets an image for the specified plot using window properties.
//
//  Arguments:
//      index   The index of the plot.
//      dob     The image is placed in this argument.
//
//  Programmer: Hank Childs
//  Creation:   November 20, 2001
//
//  Modifications:
//    Brad Whitlock, Wed Dec 5 11:48:34 PDT 2001
//    Moved code to get the window attributes into ViewerWindow.
//
//    Brad Whitlock, Tue May 7 16:38:52 PST 2002
//    Passed a callback to the engine's Execute method.
//
//    Mark C. Miller, 16Jul03
//    Modified SetWindowAtts call to SetWinAnnotAtts
//
//    Jeremy Meredith, Fri Mar 26 16:59:59 PST 2004
//    Use a map of engines based on a key, and be aware of simulations.
//
//    Mark C. Miller, Wed Apr 14 16:41:32 PDT 2004
//    Added code to pass extents type string to SetWinAnnotAtts
//
// ****************************************************************************

void
ViewerEngineManager::GetImage(int index, avtDataObject_p &dob)
{
    // WHOA!  ASSUMING ENGINE 0
    EngineProxy *engine = (engines.begin())->second;

    // WHOA!  ASSUMING ACTIVE WINDOW
    ViewerWindowManager *vwm = ViewerWindowManager::Instance();
    ViewerWindow *w = vwm->GetActiveWindow();
    WindowAttributes winAtts = w->GetWindowAttributes();
    AnnotationAttributes annotAtts = *(w->GetAnnotationAttributes());
    string extStr = avtExtentType_ToString(w->GetViewExtentsType());

    // send to the engine
    engine->SetWinAnnotAtts(&winAtts,&annotAtts,extStr);
    
    engine->UseNetwork(index);
#ifdef VIEWER_MT
    avtDataObjectReader_p rdr = engine->Execute(false, 0,0);
#else
    avtDataObjectReader_p rdr = engine->Execute(false,
        ViewerSubject::ProcessEventsCB, (void *)viewerSubject);
#endif

    //
    // Set up the return value and do some magic to update the network so we
    // don't need the reader anymore.
    //
    dob = rdr->GetOutput();
    avtPipelineSpecification_p spec;
    spec = dob->GetTerminatingSource()->GetGeneralPipelineSpecification();
    dob->Update(spec);
    dob->SetSource(NULL);
}


// ****************************************************************************
//  Method: ViewerEngineManager::UpdatePlotAttributes
//
//  Purpose:
//      Updates the plot attributes for the specified plot.
//
//  Arguments:
//      str     The ID (string) of the plot.
//      index   The index of the plot.
//      atts    The new attributs.
//
//  Programmer: Hank Childs
//  Creation:   November 28, 2001
//
//  Modifications:
//    Jeremy Meredith, Fri Mar 26 16:59:59 PST 2004
//    Use a map of engines based on a key, and be aware of simulations.
//
// ****************************************************************************

void
ViewerEngineManager::UpdatePlotAttributes(const string &str, int index,
                                          AttributeSubject *atts)
{
    // WHOA!  ASSUMING ENGINE 0
    EngineProxy *engine = (engines.begin())->second;

    engine->UpdatePlotAttributes(str, index, atts);
}

// ****************************************************************************
//  Function: GetImageCallback
//
//  Purpose:
//      Gets an image from the viewer engine manager.
//
//  Arguments:
//      vem     A void * ptr to the viewer engine manager.
//      index   The index of the plot.
//      dob     A place to put the image.
//
//  Programmer: Hank Childs
//  Creation:   November 20, 2001
//
// ****************************************************************************

void
GetImageCallback(void *vem, int index, avtDataObject_p &dob)
{
    ViewerEngineManager *viewerEngineManager = (ViewerEngineManager *) vem;
    viewerEngineManager->GetImage(index, dob);
}

// ****************************************************************************
//  Function: UpdatePlotAttsCallback
//
//  Purpose:
//      Updates the plots attributes on the engine.
//
//  Arguments:
//      vem     A void * ptr to the viewer engine manager.
//      str     The ID (string) of the plot.
//      index   The index of the plot.
//      atts    The new attributes for that plot.
//
//  Programmer: Hank Childs
//  Creation:   November 28, 2001
//
// ****************************************************************************

void
UpdatePlotAttsCallback(void *vem, const string &str, int index,
                       AttributeSubject *atts)
{
    ViewerEngineManager *viewerEngineManager = (ViewerEngineManager *) vem;
    viewerEngineManager->UpdatePlotAttributes(str, index, atts);
}


// ****************************************************************************
// Method: ViewerEngineManager::Query
//
// Purpose:
//   Engine Query RPC wrapped for safety.
//
// Programmer: Kathleen Bonnell 
// Creation:   September 17, 2002 
//
// Modifications:
//   Kathleen Bonnell, Wed Feb 26 10:56:19 PST 2003 
//   Change catch block to second version of rethrow, which will rethrow
//   the LostConnectionException, and possibly throw a NoEngineException.  
//
//   Hank Childs, Thu Oct  2 16:18:11 PDT 2003
//   Allow for multiple network ids.
//
//   Jeremy Meredith, Fri Mar 26 16:59:59 PST 2004
//   Use a map of engines based on a key, and be aware of simulations.
//
// ****************************************************************************
 
bool
ViewerEngineManager::Query(const EngineKey &ek,
                           const vector<int> &nid,
                           const QueryAttributes *atts,
                           QueryAttributes &retAtts)
{
    ENGINE_PROXY_RPC_BEGIN("Query");
    engine->Query(nid, atts, retAtts);
    ENGINE_PROXY_RPC_END_NORESTART_RETHROW2;
}


// ****************************************************************************
// Method: ViewerEngineManager::ReleaseData
//
// Purpose: 
//   Engine ReleaseData RPC wrapped for safety.
//
// Programmer: Kathleen Bonnell 
// Creation:   September 18, 2002 
//
// Modifications:
//    Jeremy Meredith, Mon Mar 22 17:59:09 PST 2004
//    First, I made it not bother attempting this if there was no engine
//    for this host.  Second, I made it not attempt to restart an engine
//    if it had died.
//
//    Jeremy Meredith, Fri Mar 26 16:59:59 PST 2004
//    Use a map of engines based on a key, and be aware of simulations.
//
// ****************************************************************************

bool
ViewerEngineManager::ReleaseData(const EngineKey &ek, int id)
{
    // If the engine has gone away, we have no need to call this method!
    if (!EngineExists(ek))
        return true;

    ENGINE_PROXY_RPC_BEGIN("ReleaseData");
    engine->ReleaseData(id);
    ENGINE_PROXY_RPC_END_NORESTART;
}

// ****************************************************************************
//  Method: ViewerWindowManager::GetMaterialClientAtts
//
//  Purpose: 
//    Returns a pointer to the material attributes.
//
//  Returns:    A pointer to the material attributes.
//
//  Programmer: Jeremy Meredith
//  Creation:   October 24, 2002
//
// ****************************************************************************

MaterialAttributes *
ViewerEngineManager::GetMaterialClientAtts()
{
    //
    // If the client attributes haven't been allocated then do so.
    //
    if (materialClientAtts == 0)
    {
        materialClientAtts = new MaterialAttributes;
    }

    return materialClientAtts;
}

// ****************************************************************************
//  Method: ViewerWindowManager::GetMaterialDefaultAtts
//
//  Purpose: 
//    Returns a pointer to the default material attributes.
//
//  Returns:    A pointer to the default material attributes.
//
//  Programmer: Jeremy Meredith
//  Creation:   October 24, 2002
//
// ****************************************************************************

MaterialAttributes *
ViewerEngineManager::GetMaterialDefaultAtts()
{
    //
    // If the client attributes haven't been allocated then do so.
    //
    if (materialDefaultAtts == 0)
    {
        materialDefaultAtts = new MaterialAttributes;
    }

    return materialDefaultAtts;
}

// ****************************************************************************
//  Method:  ViewerEngineManager::SetClientMaterialAttsFromDefault
//
//  Purpose:
//    Copy the current client atts to be the default.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 24, 2002
//
// ****************************************************************************

void
ViewerEngineManager::SetClientMaterialAttsFromDefault()
{
    if (materialDefaultAtts != 0 && materialClientAtts != 0)
    {
        *materialClientAtts = *materialDefaultAtts;
        materialClientAtts->Notify();
    }
}

// ****************************************************************************
//  Method:  ViewerEngineManager::SetDefaultMaterialAttsFromClient
//
//  Purpose:
//    Copy the default atts back to the client.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 24, 2002
//
// ****************************************************************************

void
ViewerEngineManager::SetDefaultMaterialAttsFromClient()
{
    if (materialDefaultAtts != 0 && materialClientAtts != 0)
    {
        *materialDefaultAtts = *materialClientAtts;
    }
}


// ****************************************************************************
// Method: ViewerEngineManager::CloneNetwork
//
// Purpose:
//   Engine CloneNetwork RPC wrapped for safety.
//
// Programmer: Kathleen Bonnell 
// Creation:   March 31, 2004 
//
// Modifications:
//
// ****************************************************************************
 
bool
ViewerEngineManager::CloneNetwork(const EngineKey &ek, int nid,
                           const QueryOverTimeAttributes *qatts)
{
    ENGINE_PROXY_RPC_BEGIN("CloneNetwork");
    engine->CloneNetwork(nid, qatts);
    ENGINE_PROXY_RPC_END_NORESTART_RETHROW2;
}


