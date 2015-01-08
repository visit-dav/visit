/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                            ViewerEngineManagerImplementation.C                          //
// ************************************************************************* //

#include <ViewerEngineManagerImplementation.h>

#include <EngineProxy.h>
#include <InstallationFunctions.h>
#include <DebugStream.h>

#include <AnnotationAttributes.h>
#include <BadHostException.h>
#include <CancelledConnectException.h>
#include <ConstructDataBinningAttributes.h>
#include <CouldNotConnectException.h>
#include <DataNode.h>
#include <EngineList.h>
#include <ExportDBAttributes.h>
#include <FileOpenOptions.h>
#include <GlobalAttributes.h>
#include <HostProfileList.h>
#include <IncompatibleSecurityTokenException.h>
#include <IncompatibleVersionException.h>
#include <LostConnectionException.h>
#include <MachineProfile.h>
#include <MaterialAttributes.h>
#include <MeshManagementAttributes.h>
#include <NoEngineException.h>
#include <PickAttributes.h>
#include <ProcessAttributes.h>
#include <QueryAttributes.h>
#include <SelectionList.h>
#include <SelectionSummary.h>
#include <SimulationCommand.h>
#include <StatusAttributes.h>
#include <WindowAttributes.h>

#include <ViewerMessaging.h>
#include <ViewerOperator.h>
#include <ViewerPlot.h>
#include <ViewerProperties.h>
#include <ViewerState.h>
#include <ViewerText.h>
#include <ViewerWindow.h>
#include <ViewerWindowManager.h>

// UI??
#include <ViewerConnectionProgress.h>
#include <ViewerRemoteProcessChooser.h>

#include <TimingsManager.h>

#include <avtCallback.h>
#include <avtDatabaseMetaData.h>

#include <algorithm>
#include <utility>
#include <stdio.h>
#include <snprintf.h>
#include <sstream>

using std::vector;
using std::string;
using std::map;
using std::pair;
using std::ostringstream;

#include <ViewerEngineManagerImplementation_macros.h>

//
// to remember numRestarts across VEM destructors
//
static int numRestarts = -1;

//
// Storage for static data elements.
//
ViewerEngineManagerImplementation *ViewerEngineManagerImplementation::instance=0;

FileOpenOptions *ViewerEngineManagerImplementation::defaultFileOpenOptions=0;

//
// Function prototypes.
//
#ifdef VISIT_SUPPORTS_WINDOWS_HPC
void ViewerSubmitParallelEngineToWindowsHPC(const std::string &remoteHost, 
                                            const stringVector &args, void *data);
#endif

// ****************************************************************************
//  Method: ViewerEngineManagerImplementation constructor
//
//  Programmer: Eric Brugger
//  Creation:   September 22, 2000
//
//  Modifications:
//
// ****************************************************************************

ViewerEngineManagerImplementation::ViewerEngineManagerImplementation() : ViewerServerManager(),
    SimpleObserver()
{
    if (numRestarts == -1)
        numRestarts = 2;
}

// ****************************************************************************
//  Method: ViewerEngineManagerImplementation destructor
//
//  Programmer: Eric Brugger
//  Creation:   September 22, 2000
//
//  Modifications:
//
// ****************************************************************************

ViewerEngineManagerImplementation::~ViewerEngineManagerImplementation()
{
    //
    // This should never be executed.
    //
}

// ****************************************************************************
//  Method: ViewerEngineManagerImplementation::Instance
//
//  Purpose:
//      Return a pointer to the sole instance of the ViewerEngineManagerImplementation
//      class.
//
//  Returns:    A pointer to the sole instance of the ViewerEngineManagerImplementation
//              class.
//
//  Programmer: Eric Brugger
//  Creation:   September 22, 2000
//
//  Modifications:
//
// ****************************************************************************

ViewerEngineManagerImplementation *ViewerEngineManagerImplementation::Instance()
{
    //
    // If the sole instance hasn't been instantiated, then instantiate it.
    //
    if (instance == 0)
    {
        instance = new ViewerEngineManagerImplementation;
    }

    return instance;
}

// ****************************************************************************
// Method: ViewerEngineManagerImplementation::EngineExists
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
ViewerEngineManagerImplementation::EngineExists(const EngineKey &ek) const
{
    return engines.count(ek) > 0;
}

// ****************************************************************************
// Method: ViewerEngineManagerImplementation::CreateEngine
//
// Purpose: 
//   Create an engine for the specified host.
//
// Arguments:
//      engineKey      contains the host name for the engine
//      args           the arguments to pass to the engine
//      skipChooser    do we not want to ask the user which profile to use
//      numRestarts    the number of restart attempts to use when engines fail
//      reverseLaunch  Whether the engine is being reverse launched (engine launched viewer)
//
// Returns:    True on success; false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 29 16:44:47 PST 2011
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerEngineManagerImplementation::CreateEngine(const EngineKey &ek,
    const stringVector &args, bool skipChooser, int numRestarts_, bool reverseLaunch)
{
    ViewerConnectionProgress *progress = CreateConnectionProgress(ek.HostName());

    bool retval = false;

    TRY
    {
        retval = CreateEngineEx(ek, args, skipChooser, numRestarts_, 
                                reverseLaunch, progress);
        // Delete the connection progress
        delete progress;
    }
    CATCHALL
    {
        // Delete the connection progress
        delete progress;
        RETHROW;
    }
    ENDTRY

    return retval;
}

// ****************************************************************************
//  Method: ViewerEngineManagerImplementation::CreateEngineEx
//
//  Purpose:
//      Create an engine for the specified host.
//
//  Arguments:
//      engineKey      contains the host name for the engine
//      args           the arguments to pass to the engine
//      skipChooser    do we not want to ask the user which profile to use
//      numRestarts    the number of restart attempts to use when engines fail
//      reverseLaunch  Whether the engine is being reverse launched (engine launched viewer)
//      progress       The connection progress object to use.
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
//    Hank Childs, Fri May 19 17:31:50 PDT 2006
//    Don't let an engine be launched if we are already in the process of
//    launching it.
//
//    Hank Childs, Thu May 25 09:06:23 PDT 2006
//    Change CATCH to CATCHALL.
//
//    Jeremy Meredith, Thu May 24 10:33:27 EDT 2007
//    Added SSH tunneling option to EngineProxy::Create, and set it to false.
//    If we need to tunnel, the VCL will do the host/port translation for us.
//
//    Jeremy Meredith, Wed Jan 23 15:41:02 EST 2008
//    For new engines, make sure they get our current default file opening
//    options.
//
//    Brad Whitlock, Tue Apr 29 13:20:06 PDT 2008
//    Support for internationalization.
//
//    Brad Whitlock, Thu Jul 31 08:59:27 PDT 2008
//    Added code to send a keep alive after creation as a workaround to
//    some firewalls that try to close sockets that don't send data within
//    some short period of time.
//
//    Brad Whitlock, Wed Jun 10 16:46:39 PST 2009
//    I added code to catch LostConnectionException. I was getting it while
//    debugging an engine that crashed right after connecting. We were
//    were allowing the exception to propagate to a level where the viewer
//    self-quit because it incorrectly thought the GUI had died.
//
//    Mark C. Miller, Wed Jun 17 14:27:08 PDT 2009
//    Replaced CATCHALL(...) with CATCHALL.
//
//    Brad Whitlock, Mon Nov  9 11:40:27 PST 2009
//    I expanded when inLaunch is true so we can use it to prevent certain
//    types of events from executing elsewhere via the engine chooser's
//    event loop.
//
//    Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//    Split HostProfile int MachineProfile and LaunchProfile.
//
//    Eric Brugger, Mon May  2 17:06:31 PDT 2011
//    I added the ability to use a gateway machine when connecting to a
//    remote host.
//
//    Brad Whitlock, Mon Oct 10 12:30:54 PDT 2011
//    Replace a keep-alive with a call to get engine properties.
//
//    Brad Whitlock, Tue Nov 29 16:43:01 PST 2011
//    I added a dialog argument and renamed the method. I changed the code so 
//    we launch the engine directly when the mdserver and the engine have to
//    share the same batch job.
//
//    Brad Whitlock, Tue Jun  5 17:11:18 PDT 2012
//    Pass MachineProfile down into Create.
//
//    Kathleen Biagas, Wed Aug  7 13:02:34 PDT 2013
//    Send precision type to newly created engine.
//
//    Cameron Christensen, Tuesday, June 10, 2014
//    Send backend type to newly created engine.
//
//    Brad Whitlock, Tue Sep  9 18:11:01 PDT 2014
//    Use ViewerConnectionProgress.
//
//    Kathleen Biagas, Mon Dec 22 10:13:14 PST 2014
//    Send removeDuplicateNodes to newly created engine.
//
// ****************************************************************************

bool
ViewerEngineManagerImplementation::CreateEngineEx(const EngineKey &ek,
    const stringVector &args, bool skipChooser, int numRestarts_, bool reverseLaunch,
    ViewerConnectionProgress *progress)
{
    const char *mName = "ViewerEngineManagerImplementation::CreateEngineEx: ";
    debug1 << mName << "ek=" << ek.HostName() << ", args={";
    for(size_t i = 0; i < args.size(); ++i) 
        debug1 << args[i] << ", ";
    debug1 << "}, skipChooser=" << (skipChooser?"true":"false")
           << ", numRestarts_=" << numRestarts_
           << ", reverseLaunch=" << (reverseLaunch?"true":"false")
           << ", progress=" << ((void*)progress) << endl;

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
    {
        debug1 << mName << "Engine exists. Return early." << endl;
        return true;
    }

    if (GetViewerProperties()->GetInLaunch())
    {
        debug1 << mName << "In launch. Return early." << endl;
        return false;
    }

    // Consider the state to be inLaunch from now on so we can check for
    // recursion into this function as a result of getting into the
    // engine chooser's event loop.
    GetViewerProperties()->SetInLaunch(true);

    //
    // If an engine for the host doesn't already exist, create one.
    //
    bool success = false;
    ViewerRemoteProcessChooser *chooser =
        ViewerRemoteProcessChooser::Instance();
    EngineInformation newEngine;
    if (!chooser->SelectProfile(GetViewerState()->GetHostProfileList(),
                                ek.HostName(),skipChooser,
                                newEngine.profile))
    {
        debug1 << mName << "Did not select profile. Return early." << endl;
        GetViewerProperties()->SetInLaunch(false);
        return false;
    }

    //
    // Create a new engine proxy and add arguments from the profile to it.
    //
    debug1 << mName << "Creating new engine proxy." << endl;
    newEngine.proxy = new EngineProxy;
    if(!reverseLaunch)
        newEngine.proxy->AddProfileArguments(newEngine.profile, true);

    //
    // Add some arguments to the engine proxy before we try to
    // launch the engine.  Cache them if needed for an automatic launch.
    //
    debug1 << mName << "Adding arguments." << endl;
    AddArguments(newEngine.proxy, args);
    chooser->AddRestartArgsToCachedProfile(ek.HostName(),  args);
    stringVector eArgs;
    if (newEngine.profile.GetActiveLaunchProfile()) 
        eArgs = newEngine.profile.GetActiveLaunchProfile()->GetArguments();
    for(size_t s = 0; s < args.size(); ++s)
        eArgs.push_back(args[s]);
    if (newEngine.profile.GetActiveLaunchProfile()) 
        newEngine.profile.GetActiveLaunchProfile()->SetArguments(eArgs);

    //
    // Set up the connection progress.
    //
    SetupConnectionProgress(newEngine.proxy, progress);

    //
    // Send a status message.
    //
    GetViewerMessaging()->Status(
        TR("Launching engine on %1").
        arg(ek.HostName()));

    //
    // Add the new engine proxy to the engine list.
    //
    bool rethrowCancel = false;
    TRY
    {
        // We don't set up tunnels when launching an engine, just the VCL
        newEngine.profile.SetTunnelSSH(false);

        // We don't use a gateway when launching an engine, just the VCL
        newEngine.profile.SetUseGateway(false);
        newEngine.profile.SetGatewayHost(std::string());

        //
        // Launch the engine.
        //
        TRY
        {
            // Be sure that the hostname is right in the profile.
            newEngine.profile.SetHost(ek.HostName());

            if (HostIsLocalHost(ek.HostName()))
            {
                if(reverseLaunch)
                {
                    debug1 << mName << "Connecting to an existing engine" << endl;
                    newEngine.proxy->Connect(args);
                }
                else
                {
                    debug1 << mName << "Launching a local engine" << endl;
                    newEngine.profile.SetHost("localhost");
                    newEngine.proxy->Create(newEngine.profile);
                }
            }
            else if(newEngine.profile.GetShareOneBatchJob())
            {
#ifdef VISIT_SUPPORTS_WINDOWS_HPC
                bool launchWindowsHPC = false;
                const LaunchProfile *lp = newEngine.profile.GetActiveLaunchProfile();
                if(lp != NULL)
                {
                    debug1 << mName << "The active launch profile is: " << lp->GetProfileName() << endl;
                    debug1 << mName << "\tlaunchMethodSet=" << (lp->GetLaunchMethodSet()?"true":"false") << endl;
                    debug1 << mName << "\tlaunchMethod=" << lp->GetLaunchMethod() << endl;
                    // If we're launching via WindowsHPC and we're on windows, that
                    // will end up running the visit.exe launcher locally and we'll
                    // use that to run Windows HPC job command.
                    launchWindowsHPC = (lp->GetLaunchMethodSet() && 
                        lp->GetLaunchMethod() == "WindowsHPC");
                }

                debug1 << mName << "launchWindowsHPC=" << (launchWindowsHPC?"true":"false") << endl;
                if(launchWindowsHPC)
                {
                     // Launch the engine directly through the job launcher API.
                     debug1 << mName << "Launching an engine through Windows HPC launcher" << endl;
                     rethrowCancel = true;
                     newEngine.proxy->Create(newEngine.profile,
                                             ViewerSubmitParallelEngineToWindowsHPC, (void*)&newEngine.profile,
                                             true);
                }
                else
#endif
                {
                     debug1 << mName << "Launching an engine directly" << endl;

                     newEngine.proxy->Create(newEngine.profile, NULL, NULL, false);
                }
            }
            else
            { 
                debug1 << mName << "Launching an engine with the launcher" << endl;

                // Use VisIt's launcher to start the remote engine.
                newEngine.proxy->Create(newEngine.profile, OpenWithLauncher, (void *)progress, true);
            }

            debug1 << mName << "Send keep alive and engine properties rpc" << endl;

            // Do a keep alive immediately to ensure that data is sent
            // over the engine socket. The other 2 sockets share data when
            // exchanging type representations on connect. This ensures that
            // data has come through all sockets, thwarting some evil firewalls
            // that try to close sockets that don't send data within some
            // small amount of time.
            newEngine.proxy->SendKeepAlive();

            // Request the engine properties so we have a better idea of
            // what we're talking to.
            newEngine.properties = newEngine.proxy->GetEngineMethods()->GetEngineProperties();
        }
        CATCHALL
        {
            debug1 << mName << "Caught exception, rethrowing" << endl;
            GetViewerProperties()->SetInLaunch(false);
            RETHROW;
        }
        ENDTRY

        // Add the new engine to the engine list.
        engines[ek] = newEngine;

        // Make the engine manager observe the proxy's status atts.
        newEngine.proxy->GetEngineMethods()->GetStatusAttributes()->Attach(this);

        // Tell the new engine what the default file open options are.
        newEngine.proxy->GetEngineMethods()->SetDefaultFileOpenOptions(*defaultFileOpenOptions);

        // Tell the new engine the requested floating point precision
        newEngine.proxy->GetEngineMethods()->SetPrecisionType(
            GetViewerState()->GetGlobalAttributes()->
                    GetPrecisionType());

        // Tell the new engine the requested backend type
        newEngine.proxy->GetEngineMethods()->SetBackendType(
            GetViewerState()->GetGlobalAttributes()->
                    GetBackendType());

        // Tell the new engine the removeduplicatenodes
        newEngine.proxy->GetEngineMethods()->SetRemoveDuplicateNodes(
            GetViewerState()->GetGlobalAttributes()->
                    GetRemoveDuplicateNodes());

        // Now that the new engine is in the list, tell the GUI.
        UpdateEngineList();

        // Success!
        success = true;
    }
    CATCH2(BadHostException, e)
    {
        // Delete the new engine since it could not launch anyway.
        delete newEngine.proxy;
        ViewerRemoteProcessChooser::Instance()->ClearCache(ek.HostName());

        // Tell the user that the engine could not be launched.
        GetViewerMessaging()->Error(
            TR("VisIt could not launch a compute engine on host "
               "\"%1\" because that host does not exist.").
            arg(e.GetHostName()));
    }
    CATCH(IncompatibleVersionException)
    {
        // Delete the new engine since talking to it could be bad since
        // it is a different version.
        delete newEngine.proxy;
        ViewerRemoteProcessChooser::Instance()->ClearCache(ek.HostName());

        // Tell the user that the engine is a different version.
        GetViewerMessaging()->Error(
            TR("VisIt cannot use the compute engine on "
               "host \"%1\" because the engine has an incompatible "
               " version number.").
            arg(ek.HostName()));
    }
    CATCH(IncompatibleSecurityTokenException)
    {
        // Delete the new engine since talking to it could be bad since
        // it did not provide the right credentials.
        delete newEngine.proxy;
        ViewerRemoteProcessChooser::Instance()->ClearCache(ek.HostName());

        // Tell the user that the engine is a different version.
        GetViewerMessaging()->Error(
            TR("VisIt cannot use the compute engine on host \"%1\""
               "because the compute engine did not provide the proper "
               "credentials.").
            arg(ek.HostName()));
    }
    CATCH(CouldNotConnectException)
    {
        // Delete the new engine since it was not launched
        delete newEngine.proxy;
        ViewerRemoteProcessChooser::Instance()->ClearCache(ek.HostName());

        // Tell the user that the engine was not launched
        GetViewerMessaging()->Error(
            TR("VisIt could not launch the compute engine on "
               "host \"%1\".").
            arg(ek.HostName()));
    }
    CATCH(CancelledConnectException)
    {
        // Delete the new engine since it was not launched
        delete newEngine.proxy;
        ViewerRemoteProcessChooser::Instance()->ClearCache(ek.HostName());

        // Tell the user that the engine was not launched
        GetViewerMessaging()->Error(
            TR("The launch of the compute engine on "
               "host \"%1\" has been cancelled.\n\nYou "
               "might want to check the job control system "
               "on \"%2\" to be sure the job is no longer present").
            arg(ek.HostName()).
            arg(ek.HostName()));

        if(rethrowCancel)
        {
            RETHROW;
        }
    }
    CATCH(LostConnectionException)
    {
        // Delete the new engine since we lost the connection to it.
        delete newEngine.proxy;
        ViewerRemoteProcessChooser::Instance()->ClearCache(ek.HostName());

        // Tell the user that the engine was not launched
        GetViewerMessaging()->Error(
            TR("Communication with the compute engine on "
               "host \"%1\" has been lost.").
            arg(ek.HostName()));
    }
    ENDTRY

    // Nothing bad happened and the engine is launched so turn off this flag.
    GetViewerProperties()->SetInLaunch(false);

    // Clear the status message.
    GetViewerMessaging()->ClearStatus();

    debug1 << mName << "end" << endl;

    return success;
}

// ****************************************************************************
//  Method:  ViewerEngineManagerImplementation::ConnectSim
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
//    Brad Whitlock, Thu Aug 5 10:40:00 PDT 2004
//    I changed EngineMap. I also changed the exception messages so they say
//    "simulation" instead of "compute engine".
//
//    Jeremy Meredith, Mon May  9 14:39:44 PDT 2005
//    Added security key.
//
//    Jeremy Meredith, Thu May 24 10:33:27 EDT 2007
//    Added SSH tunneling option to EngineProxy::Create, and set it to false.
//
//    Brad Whitlock, Tue Apr 29 13:23:21 PDT 2008
//    Support for internationalization.
//
//    Brad Whitlock, Tue Apr 28 09:32:04 PDT 2009
//    Pass the SSH tunneling option to the launcher callback.
//
//    Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//    Split HostProfile int MachineProfile and LaunchProfile.
//
//    Brad Whitlock, Wed Dec 1 23:35:34 PST 2010
//    Tell EngineProxy that it is a simulation.
//
//    Eric Brugger, Mon May  2 17:06:31 PDT 2011
//    I added the ability to use a gateway machine when connecting to a
//    remote host.
//
//    Brad Whitlock, Mon Oct 10 12:32:22 PDT 2011
//    Get the engine properties.
//
//    Brad Whitlock, Tue Jun  5 17:11:02 PDT 2012
//    Use profile to launch.
//
//    Brad Whitlock, Tue Aug 20 11:12:34 PDT 2013
//    Use original value for ssh tunneling to set into simData to ensure that
//    host/port arguments are translated in SimConnectThroughLauncher when we
//    do ssh tunneling.
//
//    Brad Whitlock, Tue Nov 11 23:38:28 PST 2014
//    Add extra arguments from the launch profile it it exists. This will
//    ensure that argmuments like -fixed-buffer-sockets get passed along.
//
// ****************************************************************************

bool
ViewerEngineManagerImplementation::ConnectSim(const EngineKey &ek,
                                const stringVector &args,
                                const string &simHost,
                                int simPort,
                                const string &simSecurityKey)
{
    debug1 << "ConnectSim: ek.hostname=" << ek.HostName()
           << " ek.originalhostname=" << ek.OriginalHostName()
           << " ek.SimName=" << ek.SimName();
    debug1 << " args={";
    for(size_t i = 0; i < args.size(); ++i)
        debug1 << args[i] << ", ";
    debug1 << "} simHost=" << simHost << " simPort=" << simPort
           << " simSecurityKey=" << simSecurityKey << endl;

    //
    // Check if an engine already exists for the host.
    //
    if (EngineExists(ek))
        return true;

    bool success = false;

    //
    // If an engine for the host doesn't already exist, create one.
    //
    EngineInformation newEngine;
    newEngine.proxy = new EngineProxy(true);

    //
    // Add some arguments to the engine proxy before we try to
    // launch the engine.
    //
    AddArguments(newEngine.proxy, args);

    //
    // Copy the arguments into the restart arguments that are
    // used to restart failed engines.
    //
    restartArguments[ek] = args;

    //
    // Send a status message.
    //
    GetViewerMessaging()->Status(
        TR("Connecting to simulation at %1:%2").
        arg(simHost).
        arg(simPort));

    //
    // Add the new engine proxy to the engine list.
    //
    TRY
    {
        MachineProfile profile = GetMachineProfile(ek.HostName());

        // Add extra arguments from the launch profile
        const LaunchProfile *launch = profile.GetActiveLaunchProfile();
        if(launch != NULL)
        {
            if(!launch->GetArguments().empty())
            {
                for(size_t i = 0; i < launch->GetArguments().size(); ++i)
                   newEngine.proxy->AddArgument(launch->GetArguments()[i]);
            }
        }

        //
        // Launch the engine.
        //
        typedef struct {
            string h; int p; string k;
            ViewerConnectionProgress *d;
            bool tunnel;} SimData;
        SimData simData;
        // The windows compiler can't accept non aggregate types in an
        // initializer list so initialize them like this:
        simData.h = simHost;
        simData.p = simPort;
        simData.k = simSecurityKey;
        simData.d = CreateConnectionProgress(ek.HostName());
        simData.tunnel = profile.GetTunnelSSH();
        SetupConnectionProgress(newEngine.proxy, simData.d);

        // We don't set up tunnels when connecting to a simulation,
        // just when launching the VCL
        profile.SetTunnelSSH(false);
        
        // We don't use a gateway when connecting to a simulation,
        // just when launching the VCL
        profile.SetUseGateway(false);
        profile.SetGatewayHost("");

        newEngine.proxy->Create(profile,
                                SimConnectThroughLauncher, (void *)&simData,
                                true);

        newEngine.properties = newEngine.proxy->GetEngineMethods()->GetEngineProperties();

        engines[ek] = newEngine;

        // Make the engine manager observe the proxy's status atts.
        newEngine.proxy->GetEngineMethods()->GetStatusAttributes()->Attach(this);

        // Now that the new engine is in the list, tell the GUI.
        UpdateEngineList();

        // Success!
        success = true;
    }
    CATCH2(BadHostException, e)
    {
        // Delete the new engine since it could not launch anyway.
        delete newEngine.proxy;
        ViewerRemoteProcessChooser::Instance()->ClearCache(ek.HostName());

        // Tell the user that the engine could not be launched.
        GetViewerMessaging()->Error(
            TR("VisIt could not connect to the simulation on host "
               "\"%1\" because that host does not exist.").
            arg(e.GetHostName()));
    }
    CATCH(IncompatibleVersionException)
    {
        // Delete the new engine since talking to it could be bad since
        // it is a different version.
        delete newEngine.proxy;
        ViewerRemoteProcessChooser::Instance()->ClearCache(ek.HostName());

        // Tell the user that the engine is a different version.
        GetViewerMessaging()->Error(
            TR("VisIt cannot use the simulation on "
               "host \"%1\" because the simulation has an incompatible "
               " version number.").
            arg(ek.HostName()));
    }
    CATCH(IncompatibleSecurityTokenException)
    {
        // Delete the new engine since talking to it could be bad since
        // it did not provide the right credentials.
        delete newEngine.proxy;
        ViewerRemoteProcessChooser::Instance()->ClearCache(ek.HostName());

        // Tell the user that the engine is a different version.
        GetViewerMessaging()->Error(
            TR("VisIt cannot use the simulation on host \"%1\""
               "because the simulation did not provide the proper "
               "credentials.").
            arg(ek.HostName()));
    }
    CATCH(CouldNotConnectException)
    {
        // Delete the new engine since it was not launched
        delete newEngine.proxy;
        ViewerRemoteProcessChooser::Instance()->ClearCache(ek.HostName());

        // Tell the user that the engine was not launched
        GetViewerMessaging()->Error(
           TR("VisIt could not connect to the simulation on "
              "host \"%1\".").
           arg(ek.HostName()));
    }
    CATCH(CancelledConnectException)
    {
        // Delete the new engine since it was not launched
        delete newEngine.proxy;
        ViewerRemoteProcessChooser::Instance()->ClearCache(ek.HostName());

        // Tell the user that the engine was not launched
        GetViewerMessaging()->Error(
            TR("The connection to the simulation on "
               "host \"%1\" has been cancelled.").
            arg(ek.HostName()));
    }
    ENDTRY

    // Clear the status message.
    GetViewerMessaging()->ClearStatus();

    return success;
}

// ****************************************************************************
//  Method: ViewerEngineManagerImplementation::CloseEngines
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
//    Brad Whitlock, Wed Aug 4 17:21:25 PST 2004
//    I changed EngineMap.
//
//    Mark C. Miller, Wed Jun 17 14:27:08 PDT 2009
//    Replaced CATCHALL(...) with CATCHALL.
// ****************************************************************************

void
ViewerEngineManagerImplementation::CloseEngines()
{
    //
    // If we have any engines close them all and free any storage
    // associatied with them.
    //
    for (EngineMap::iterator i = engines.begin() ; i != engines.end() ; i++)
    {
        const EngineKey &key = i->first;
        EngineProxy *engine  = i->second.proxy;

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

        engine->GetEngineMethods()->GetStatusAttributes()->Detach(this);
        TRY
        {
            engine->Close();
        }
        CATCHALL
        {
            debug1 << "Caught an exception while closing the engine." << endl;
        }
        ENDTRY

        delete i->second.proxy;
    }
}

// ****************************************************************************
// Method: ViewerEngineManagerImplementation::CloseEngine
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
//   Brad Whitlock, Tue Apr 29 14:39:47 PDT 2008
//   Support for internationalization.
//
// ****************************************************************************

void
ViewerEngineManagerImplementation::CloseEngine(const EngineKey &ek)
{
    // Since we're closing the engine intentionally, let us change
    // the options the next time we launch an engine
    ViewerRemoteProcessChooser::Instance()->ClearCache(ek.HostName());

    // We found an engine.
    if (EngineExists(ek))
    {
        RemoveEngine(ek, true);
        UpdateEngineList();

        GetViewerMessaging()->Message(
            TR("Closed the compute engine on host %1.").
            arg(ek.HostName()));
    }
    else
    {
        GetViewerMessaging()->Error(
            TR("Cannot close the compute engine on host %1 because there is no "
               "compute engine running on that host.").
              arg(ek.HostName()));
    }
}

// ****************************************************************************
// Method: ViewerEngineManagerImplementation::InterruptEngine
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
//   Brad Whitlock, Wed Aug 4 17:21:59 PST 2004
//   Changed EngineMap.
//
//   Brad Whitlock, Tue Apr 29 14:41:03 PDT 2008
//   Support for internationalization.
//
// ****************************************************************************

void
ViewerEngineManagerImplementation::InterruptEngine(const EngineKey &ek)
{
    // We found an engine.
    if (EngineExists(ek))
    {
        engines[ek].proxy->Interrupt();
        GetViewerMessaging()->Message(
            TR("Interrupting the compute engine on host %1.").
            arg(ek.HostName()));
    }
    else
    {
        GetViewerMessaging()->Error(
            TR("Cannot interrupt the compute engine on host %1 "
               "because there is no compute engine running on that host.").
            arg(ek.HostName()));
    }
}

// ****************************************************************************
// Method: ViewerEngineManagerImplementation::SendKeepAlives
//
// Purpose: 
//   Sends a keep alive signal to all of the engines.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 12 11:46:01 PDT 2004
//
// Modifications:
//   Jeremy Meredith, Fri Mar 26 16:59:59 PST 2004
//   Use a map of engines based on a key, and be aware of simulations.
//
//   Brad Whitlock, Wed Aug 4 17:23:00 PST 2004
//   Changed EngineMap.
//
//   Mark C. Miller, Wed Jun 17 14:27:08 PDT 2009
//   Replaced CATCHALL(...) with CATCHALL.
// ****************************************************************************

void
ViewerEngineManagerImplementation::SendKeepAlives()
{
    if(!GetViewerProperties()->GetInExecute())
    {
        bool updateList = false;
        vector<EngineKey> failedEngines;

        for (EngineMap::iterator i = engines.begin() ; i != engines.end(); i++)
        {
            debug1 << "Sending keep alive signal to compute engine on host "
                   << i->first.HostName().c_str() << "." << endl;

            TRY
            {
                i->second.proxy->SendKeepAlive();
            }
            CATCHALL
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
// Method:  ViewerEngineManagerImplementation::GetEngine
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
//   Brad Whitlock, Wed Aug 4 17:23:53 PST 2004
//   Changed EngineMap.
//
// ****************************************************************************

EngineProxy *
ViewerEngineManagerImplementation::GetEngine(const EngineKey &ek)
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
    return engines[ek].proxy;
}

// ****************************************************************************
// Method: ViewerEngineManagerImplementation::LaunchMessage
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
//   Brad Whitlock, Tue Apr 29 14:42:28 PDT 2008
//   Support for internationalization.
//
//   Jeremy Meredith, Thu May  1 13:42:55 EDT 2008
//   Account for null error strings.
//
// ****************************************************************************

void
ViewerEngineManagerImplementation::LaunchMessage(const EngineKey &ek)  const
{
    if (ek.IsSimulation())
    {
        GetViewerMessaging()->Warning(
            TR("VisIt is not connected to the simulation '%1' "
               "on host %2").
            arg(ek.SimName()).
            arg(ek.HostName()));
    }
    else if (ek.HostName() != "<unset>")
    {
        GetViewerMessaging()->Warning(
            TR("VisIt could not find a compute engine to use "
               "for the plot on host %1. VisIt will try to launch a compute "
               "engine on that host.").
            arg(ek.HostName()));
    }
}

// ****************************************************************************
// Method: ViewerEngineManagerImplementation::ClearCache
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
//    Brad Whitlock, Wed Feb 23 16:43:42 PST 2005
//    I made it use a new macro that does not start an engine if none exists
//    because a fresh engine would not need its cache cleared anyway.
//
// ****************************************************************************

bool
ViewerEngineManagerImplementation::ClearCache(const EngineKey &ek,
                                const std::string &dbName)
{
    ENGINE_PROXY_RPC_BEGIN_NOSTART("ClearCache");
    if (dbName == "")
        engine->GetEngineMethods()->ClearCache();
    else
        engine->GetEngineMethods()->ClearCache(dbName);
    ENGINE_PROXY_RPC_END_NORESTART;
}

// ****************************************************************************
// Method: ViewerEngineManagerImplementation::ClearCacheForAllEngines
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
ViewerEngineManagerImplementation::ClearCacheForAllEngines()
{
    for (EngineMap::iterator i = engines.begin() ; i != engines.end() ; i++)
    {
        const EngineKey &key = i->first;
        ClearCache(key);
    }
}

// ****************************************************************************
// Method: ViewerEngineManagerImplementation::RemoveEngine
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
//   Brad Whitlock, Wed Aug 4 17:27:16 PST 2004
//   Changed EngineMap.
//
// ****************************************************************************

void
ViewerEngineManagerImplementation::RemoveEngine(const EngineKey &ek, bool close)
{
    if (EngineExists(ek))
    {
        // Delete the entry in the engine list for the specified index.    
        engines[ek].proxy->GetEngineMethods()->GetStatusAttributes()->Detach(this);
        if (close)
            engines[ek].proxy->Close();
        delete engines[ek].proxy;
        engines.erase(ek);
    }
}

// ****************************************************************************
// Method: ViewerEngineManagerImplementation::RemoveFailedEngine
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
//   Jeremy Meredith, Fri Mar 26 16:59:59 PST 2004
//   Use a map of engines based on a key, and be aware of simulations.
//
//   Brad Whitlock, Mon May 3 14:39:35 PST 2004
//   I made it tell all plots that use the failed engine key to reset
//   their network ids.
//
//   Brad Whitlock, Tue Apr 29 14:44:53 PDT 2008
//   Support for internationalization.
//
// ****************************************************************************

void
ViewerEngineManagerImplementation::RemoveFailedEngine(const EngineKey &ek)
{
    // Tell the GUI about the error.
    if (ek.SimName() != "")
    {
        GetViewerMessaging()->Error(
            TR("VisIt has been disconnected from the simulation '%1' on host %2").
            arg(ek.SimName()).
            arg(ek.HostName()));
    }
    else
    {
        GetViewerMessaging()->Error(
            TR("The compute engine running on %1 has exited abnormally.").
            arg(ek.HostName()));
    }

    // Send a message to the client to clear the status for the
    // engine that had troubles.
    GetViewerMessaging()->ClearStatus(ek.ID());

    // Tell all plots that use the failed engine for their engine to reset
    // their network ids so pick works properly.
    ViewerWindowManager::Instance()->ResetNetworkIds(ek);

    // Remove the specified engine from the list of engines.
    RemoveEngine(ek, false);
}

// ****************************************************************************
// Method: ViewerEngineManagerImplementation::UpdateEngineList
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
//   Brad Whitlock, Wed Aug 4 17:28:35 PST 2004
//   Changed EngineMap.
//
//   Brad Whitlock, Mon Oct 10 12:36:58 PDT 2011
//   Rewrite.
//
// ****************************************************************************

void
ViewerEngineManagerImplementation::UpdateEngineList()
{
    vector<EngineKey> ids;
    
    // Go through the list of engines and add each engine to the engine list
    // that gets returned to the viewer's client.
    for (EngineMap::iterator it = engines.begin() ; it != engines.end(); it++)
    {
        ids.push_back(it->first);
    }

    // Sort the strings.
    if (ids.size() > 1)
        std::sort(ids.begin(), ids.end());

    EngineList newEL;

    // Add the other information about the engine.
    for(size_t i = 0; i < ids.size(); ++i) 
    {
        EngineKey ek = ids[i];
        if (EngineExists(ek))
        {
            newEL.GetEngineName().push_back(ek.HostName());
            newEL.GetSimulationName().push_back(ek.SimName());
            newEL.AddProperties(engines[ek].properties);
        }
    }

    // Send the engine list to the viewer's client.
    *(GetViewerState()->GetEngineList()) = newEL;
    GetViewerState()->GetEngineList()->Notify();
}

// ****************************************************************************
// Method: ViewerEngineManagerImplementation::Update
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
//    Brad Whitlock, Wed Aug 4 17:29:21 PST 2004
//    Changed EngineMap.
//
//    Brad Whitlock, Thu May 5 17:10:40 PST 2005
//    I removed interruption of the engine since it's handled as a special
//    opcode coming from the clients and we don't need to check for it here
//    anymore since we always listen to the clients now.
//
//    Brad Whitlock, Tue Apr 29 14:47:05 PDT 2008
//    Support for internationalization.
//
// ****************************************************************************

void
ViewerEngineManagerImplementation::Update(Subject *TheChangedSubject)
{
    StatusAttributes *statusAtts = (StatusAttributes *)TheChangedSubject;

    // Find the key to the engine
    EngineKey ek;
    for (EngineMap::iterator i = engines.begin() ; i != engines.end() ; i++)
    {
        if (i->second.proxy->GetEngineMethods()->GetStatusAttributes() == statusAtts)
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
            GetViewerMessaging()->ClearStatus(ek.ID());
        else
            GetViewerMessaging()->ClearStatus();
    }
    else if (statusAtts->GetMessageType() == 1)
    {
        // The message field was selected.
        if (EngineExists(ek))
            GetViewerMessaging()->Status(ek.ID(),
                                         statusAtts->GetStatusMessage());
        else
            GetViewerMessaging()->Status(statusAtts->GetStatusMessage());
    }
    else if (statusAtts->GetMessageType() == 3)
    {
        // The message field was selected.
        if (EngineExists(ek))
        {
            GetViewerMessaging()->Warning(
                TR("The compute engine running on host %1 issued "
                   "the following warning: %2").
                arg(ek.HostName()).
                arg(statusAtts->GetStatusMessage()));
        }
        else
            GetViewerMessaging()->Warning(statusAtts->GetStatusMessage());
    }
    else if (EngineExists(ek))
    {
        GetViewerMessaging()->Status(ek.ID(),
               statusAtts->GetPercent(),
               statusAtts->GetCurrentStage(),
               statusAtts->GetCurrentStageName(),
               statusAtts->GetMaxStage());
    }
}

// ****************************************************************************
//  Method:  ViewerEngineManagerImplementation::GetWriteSocket
//
//  Purpose:
//    Gets the socket the engine uses to send data to the viewer.
//
//  Arguments:
//    ek         the key that identifies the engine
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 25, 2004
//
// ****************************************************************************

int
ViewerEngineManagerImplementation::GetWriteSocket(const EngineKey &ek)
{
    if (EngineExists(ek))
        return engines[ek].proxy->GetWriteSocket();
    else
        return -1;
}

// ****************************************************************************
//  Method:  ViewerEngineManagerImplementation::ReadDataAndProcess
//
//  Purpose:
//    Read from an engine (if it exists) and process the data.
//
//  Arguments:
//    ek         the key that identifies the engine
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 25, 2004
//
// ****************************************************************************

void
ViewerEngineManagerImplementation::ReadDataAndProcess(const EngineKey &ek)
{
    if (EngineExists(ek))
        engines[ek].proxy->ReadDataAndProcess();
}

// ****************************************************************************
//  Method:  ViewerEngineManagerImplementation::GetSimulationMetaData
//
//  Purpose:
//    Get the metadata object for the simulation.
//
//  Arguments:
//    ek         the key that identifies the engine
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 25, 2004
//
// ****************************************************************************

avtDatabaseMetaData *
ViewerEngineManagerImplementation::GetSimulationMetaData(const EngineKey &ek)
{
    if (EngineExists(ek))
        return engines[ek].proxy->GetSimulationMetaData();
    else
        EXCEPTION0(NoEngineException);
}

// ****************************************************************************
//  Method:  ViewerEngineManagerImplementation::GetSimulationSILAtts
//
//  Purpose:
//    Get the SILAttributes for the simulation.
//
//  Arguments:
//    ek         the key that identifies the engine
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 25, 2004
//
// ****************************************************************************

SILAttributes *
ViewerEngineManagerImplementation::GetSimulationSILAtts(const EngineKey &ek)
{
    if (EngineExists(ek))
        return engines[ek].proxy->GetSimulationSILAtts();
    else
        EXCEPTION0(NoEngineException);
}

// ****************************************************************************
//  Method:  ViewerEngineManagerImplementation::GetCommandFromSimulation
//
//  Purpose:
//    Get the SimulationCommand for the simulation so we can watch to see if
//    it sent any commands to VisIt.
//
//  Arguments:
//    ek         the key that identifies the engine
//
//  Programmer:  Brad Whitlock
//  Creation:    Thu Jan 25 14:13:03 PST 2007
//
// ****************************************************************************

SimulationCommand *
ViewerEngineManagerImplementation::GetCommandFromSimulation(const EngineKey &ek)
{
    if (EngineExists(ek))
        return engines[ek].proxy->GetCommandFromSimulation();
    else
        EXCEPTION0(NoEngineException);
}

// ****************************************************************************
// Method: ViewerEngineManagerImplementation::CreateNode
//
// Purpose: 
//   Writes the host profiles used by the active compute engines to the
//   DataNode for later use in session files.
//
// Arguments:
//   vemNode : The parent node that will contain the data for this object.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 3 15:16:25 PST 2004
//
// Modifications:
//   Brad Whitlock, Wed Aug 4 17:31:48 PST 2004
//   Made it use the profile stored in the EngineMap so we don't get the 
//   wrong profile just because there are multiple profiles for the same
//   host.
//
//   Brad Whitlock, Mon Oct 31 11:56:45 PDT 2005
//   I set the host name into the profile in case the host profile's host
//   was not set in the stored profile.
//
//   Cyrus Harrison, Thu Mar 15 11:26:58 PDT 2007
//   Added save of material attributes and mesh management attributes
//
//   Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//   Split HostProfile int MachineProfile and LaunchProfile.
//
//   Jeremy Meredith, Tue Jul 13 12:52:14 EDT 2010
//   Make sure the "RunningEngines" machine profile as written only has a
//   single launch profile (the active one) -- this simplifies later parsing.
//
//   Brad Whitlock, Fri Jul 25 1:12:23 EDT 2014
//   Add export atts.
//
// ****************************************************************************

void
ViewerEngineManagerImplementation::CreateNode(DataNode *vemNode, bool detailed) const
{
    bool haveNonSimEngines = false;
    EngineMap::const_iterator it;
    for (it = engines.begin() ; it != engines.end(); ++it)
        haveNonSimEngines |= (!it->first.IsSimulation());

    if(detailed && haveNonSimEngines)
    {
        DataNode *runningEnginesNode = new DataNode("RunningEngines");
        vemNode->AddNode(runningEnginesNode);

        for (it = engines.begin() ; it != engines.end(); ++it)
        {
            if(!it->first.IsSimulation())
            {
                MachineProfile temp(it->second.profile);
                temp.SetHost(it->first.HostName());
                if(temp.GetActiveLaunchProfile() != 0)
                {
                    LaunchProfile launch(*temp.GetActiveLaunchProfile());
                    launch.SetNumProcessors(it->second.proxy->NumProcessors());
                    launch.SetNumNodes(it->second.proxy->NumNodes());
                    temp.ClearLaunchProfiles();
                    temp.AddLaunchProfiles(launch);
                }
                temp.CreateNode(runningEnginesNode, true, true);
            }
        }
    }
}


// ****************************************************************************
// Method: ViewerEngineManagerImplementation::SetFromNode
//
// Purpose: 
//   Restores material and mesh management attributes 
//
// Arguments:
//   parentNode    : The parent node that will contain the data for this object.
//   configVersion : The version of the config file.
//
// Programmer: Cyrus Harrison
// Creation:   Thu Mar 15 11:30:11 PDT 2007
//
// Modifications:
//   Brad Whitlock, Wed Feb 13 14:45:27 PST 2008
//   Added configVersion.
//
//   Brad Whitlock, Fri Jul 25 1:12:23 EDT 2014
//   Add export atts.
//
// ****************************************************************************

void
ViewerEngineManagerImplementation::SetFromNode(DataNode *parentNode, 
    const std::string &configVersion)
{
}

// ****************************************************************************
// ENGINE RPC's
// ****************************************************************************

// ****************************************************************************
// Method: ViewerEngineManagerImplementation::OpenDatabase
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
//    Kathleen Bonnell, Tue Oct  9 14:40:10 PDT 2007
//    Added meshquality/timederivative creation flags.
//
//    Mark C. Miller, Tue Jun 10 22:36:25 PDT 2008
//    Added support for ignoring bad extents from dbs.
// ****************************************************************************

bool
ViewerEngineManagerImplementation::OpenDatabase(const EngineKey &ek, 
    const std::string &format, const std::string &filename, int time)
{
    ENGINE_PROXY_RPC_BEGIN("OpenDatabase");
    bool cmq = GetViewerState()->GetGlobalAttributes()->
                    GetCreateMeshQualityExpressions();
    bool ctd = GetViewerState()->GetGlobalAttributes()->
                    GetCreateTimeDerivativeExpressions();
    bool ie = GetViewerState()->GetGlobalAttributes()->
                    GetIgnoreExtentsFromDbs();
    engine->GetEngineMethods()->OpenDatabase(format, filename, time, cmq, ctd, ie);
    ENGINE_PROXY_RPC_END;
}

// ****************************************************************************
// Method: ViewerEngineManagerImplementation::DefineVirtualDatabase
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
//    Kathleen Bonnell, Tue Oct  9 14:40:10 PDT 2007
//    Added meshquality/timederivative creation flags.
//
// ****************************************************************************

bool
ViewerEngineManagerImplementation::DefineVirtualDatabase(const EngineKey &ek,
    const std::string &format, const std::string &dbName,
    const std::string &path, const stringVector &files, int time)
{
    ENGINE_PROXY_RPC_BEGIN("DefineVirtualDatabase");
    bool cmq = GetViewerState()->GetGlobalAttributes()->
                    GetCreateMeshQualityExpressions();
    bool ctd = GetViewerState()->GetGlobalAttributes()->
                    GetCreateTimeDerivativeExpressions();
    engine->GetEngineMethods()->DefineVirtualDatabase(format, dbName, path, files, time, cmq, ctd);
    ENGINE_PROXY_RPC_END;
}

// ****************************************************************************
// Method: ViewerEngineManagerImplementation::ReadDataObject
//
// Purpose:
//   Reads the data object from the engine.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 10 14:15:53 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
ViewerEngineManagerImplementation::ReadDataObject(const EngineKey &ek,
    const std::string &format,
    const std::string &filename,
    const std::string &var, const int ts,
    avtSILRestriction_p silr,
    const MaterialAttributes &ma,
    const ExpressionList &el,
    const MeshManagementAttributes &mma,
    bool treatAllDbsAsTimeVarying,
    bool ignoreExtents,
    const std::string &selName,
    int windowID)
{
    ENGINE_PROXY_RPC_BEGIN("ReadDataObject");
    engine->GetEngineMethods()->ReadDataObject(format, filename, var, ts, silr,
         ma, el, mma, treatAllDbsAsTimeVarying, ignoreExtents, selName, windowID);
    ENGINE_PROXY_RPC_END_NORESTART_RETHROW;
}

// ****************************************************************************
// Method: ViewerEngineManagerImplementation::ApplyOperator
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
ViewerEngineManagerImplementation::ApplyOperator(const EngineKey &ek, const std::string &name,
    const AttributeSubject *atts)
{
    ENGINE_PROXY_RPC_BEGIN("ApplyOperator");
    engine->GetEngineMethods()->ApplyOperator(name, atts);
    ENGINE_PROXY_RPC_END_NORESTART_RETHROW;
}

// ****************************************************************************
// Method: ViewerEngineManagerImplementation::MakePlot
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
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Added winID
//
//    Brad Whitlock, Wed Mar 21 22:41:09 PST 2007
//    Added plotName.
//
// ****************************************************************************

bool
ViewerEngineManagerImplementation::MakePlot(const EngineKey &ek, const std::string &plotName,
    const std::string &pluginID, const AttributeSubject *atts, 
    const vector<double> &extents, int winID, int *networkId)
{
    ENGINE_PROXY_RPC_BEGIN("MakePlot");
    *networkId = engine->GetEngineMethods()->MakePlot(plotName, pluginID, atts, extents, winID);
    ENGINE_PROXY_RPC_END_NORESTART_RETHROW;
}

// ****************************************************************************
// Method: ViewerEngineManagerImplementation::UpdatePlotAttributes
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
ViewerEngineManagerImplementation::UpdatePlotAttributes(const EngineKey &ek,
                                          const std::string &name, int id,
                                          const AttributeSubject *atts)
{
    ENGINE_PROXY_RPC_BEGIN("UpdatePlotAttributes");
    engine->GetEngineMethods()->UpdatePlotAttributes(name, id, atts);
    ENGINE_PROXY_RPC_END;
}

// ****************************************************************************
// Method: ViewerEngineManagerImplementation::Pick
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
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Added wid
//
// ****************************************************************************

bool
ViewerEngineManagerImplementation::Pick(const EngineKey &ek,
                          const int nid, int wid, const PickAttributes *atts,
                          PickAttributes &retAtts)
{
    ENGINE_PROXY_RPC_BEGIN("Pick");
    engine->GetEngineMethods()->Pick(nid, atts, retAtts, wid);
    ENGINE_PROXY_RPC_END_NORESTART_RETHROW2;
}

// ****************************************************************************
// Method: ViewerEngineManagerImplementation::StartQuery
//
// Purpose: 
//   Engine StartQuery RPC wrapped for safety.
//
// Programmer: Hank Childs
// Creation:   March 1, 2005
//
// ****************************************************************************

bool
ViewerEngineManagerImplementation::StartQuery(const EngineKey &ek, const bool flag,
                               const int nid)
{
    ENGINE_PROXY_RPC_BEGIN("StartQuery");
    engine->GetEngineMethods()->StartQuery(flag, nid);
    ENGINE_PROXY_RPC_END;
}

// ****************************************************************************
// Method: ViewerEngineManagerImplementation::StartPick
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
//    Kathleen Bonnell, Tue Jun  1 17:57:52 PDT 2004 
//    Added forZones arg. 
//
//    Kathleen Bonnell, Thu Jul 14 09:12:43 PDT 2005 
//    Made it use the macro that does not start an engine if none exists.
//
// ****************************************************************************

bool
ViewerEngineManagerImplementation::StartPick(const EngineKey &ek, const bool forZones,
                               const bool flag, const int nid)
{
    ENGINE_PROXY_RPC_BEGIN_NOSTART("StartPick");
    engine->GetEngineMethods()->StartPick(forZones, flag, nid);
    ENGINE_PROXY_RPC_END_NORESTART;
}

// ****************************************************************************
// Method: ViewerEngineManagerImplementation::SetWinAnnotAtts
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
//    Mark C. Miller, Tue May 25 20:44:10 PDT 2004
//    Added AnnotationObjectList arg
//
//    Mark C. Miller, Wed Jun  9 17:44:38 PDT 2004
//    Added VisualCueList arg
//
//    Mark C. Miller, Tue Jul 27 15:11:11 PDT 2004
//    Added code to deal with frame and state in window/annotation atts
//
//    Mark C. Miller, Wed Oct  6 18:12:29 PDT 2004
//    Added code for view extents
//
//    Mark C. Miller, Tue Oct 19 20:18:22 PDT 2004
//    Added code to pass along name of last color table to change
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Added winID
//
//    Hank Childs, Mon Feb 20 16:40:37 PST 2006
//    Do not launch an engine just to set the window annotation attributes.
//
//    Hank Childs, Fri Mar  3 09:19:13 PST 2006
//    Remove behavior where engine is not launched.  This is important
//    for picking on crashed engines (regression test pick.py will fail).
//
// ****************************************************************************

bool
ViewerEngineManagerImplementation::SetWinAnnotAtts(const EngineKey &ek,
                                     const WindowAttributes *wa,
                                     const AnnotationAttributes *aa,
                                     const AnnotationObjectList *ao,
                                     const string extstr,
                                     const VisualCueList *visCues,
                                     const int *frameAndState,
                                     const double *viewExtents,
                                     const string ctName,
                                     const int winID)
{
    ENGINE_PROXY_RPC_BEGIN("SetWinAnnotAtts");
    engine->GetEngineMethods()->SetWinAnnotAtts(wa,aa,ao,extstr,visCues,frameAndState,viewExtents,
        ctName, winID);
    ENGINE_PROXY_RPC_END;
}

// ****************************************************************************
// Method: ViewerEngineManagerImplementation::UseNetwork
//
// Purpose:
//   Uses an existing network.
//
// Arguments:
//   ek        : The engine key for the engine to use.
//   networkId : The network id.
//
// Returns:    True on success; false on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 10 13:57:45 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
ViewerEngineManagerImplementation::UseNetwork(const EngineKey &ek, int networkId)
{
    ENGINE_PROXY_RPC_BEGIN("UseNetwork");
    engine->GetEngineMethods()->UseNetwork(networkId);
    ENGINE_PROXY_RPC_END_NORESTART_RETHROW2;
}

// ****************************************************************************
// Method: ViewerEngineManagerImplementation::Execute
//
// Purpose:
//   Executes a network.
//
// Arguments:
//   ek                : The engine key for the engine to use.
//   rdr               : The data object reader produced as a result of executing.
//   replyWithNullData : Whether to send back null data.
//   waitCB            : A callback function to handle events while we wait for data.
//   waitCBData        : callback data.
//
// Returns:    True on success; false on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 10 13:57:45 PDT 2014
//
// Modifications:
//
// ****************************************************************************
bool
ViewerEngineManagerImplementation::Execute(const EngineKey &ek,
   avtDataObjectReader_p &rdr,
   bool replyWithNullData, void (*waitCB)(void*), void *waitCBData)
{
    ENGINE_PROXY_RPC_BEGIN("Execute");
    rdr = engine->GetEngineMethods()->Execute(replyWithNullData, waitCB, waitCBData);
    ENGINE_PROXY_RPC_END_NORESTART_RETHROW;
}

// ****************************************************************************
// Method: ViewerEngineManagerImplementation::Render
//
// Purpose:
//   Render existing networks.
//
// Arguments:
//   ek         : The engine key for the engine to use.
//   rdr        : The data object reader produced as a result of executing.
//   waitCB     : A callback function to handle events while we wait for data.
//   waitCBData : callback data.
//
// Returns:    True on success; false on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 10 13:57:45 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
ViewerEngineManagerImplementation::Render(const EngineKey &ek,
   avtDataObjectReader_p &rdr,
   bool sendZBuffer, const intVector &networkIds, 
   int annotMode, int windowID, bool leftEye,
   void (*waitCB)(void *), void *waitCBData)
{
    ENGINE_PROXY_RPC_BEGIN("Render");
    rdr = engine->GetEngineMethods()->Render(sendZBuffer, networkIds, 
                                             annotMode, windowID, leftEye,
                                             waitCB, waitCBData);
    ENGINE_PROXY_RPC_END_NORESTART_RETHROW;
}


// ****************************************************************************
// Method: ViewerEngineManagerImplementation::Query
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
ViewerEngineManagerImplementation::Query(const EngineKey &ek,
                           const vector<int> &nid,
                           const QueryAttributes *atts,
                           QueryAttributes &retAtts)
{
    ENGINE_PROXY_RPC_BEGIN("Query");
    engine->GetEngineMethods()->Query(nid, atts, retAtts);
    ENGINE_PROXY_RPC_END_NORESTART_RETHROW2;
}

// ****************************************************************************
// Method: ViewerEngineManagerImplementation::GetQueryParameters
//
// Purpose:
//   Engine GetQueryParametersRPC wrapped for safety.
//
// Programmer: Kathleen Biagas 
// Creation:   July 15, 2011 
//
// ****************************************************************************
 
bool
ViewerEngineManagerImplementation::GetQueryParameters(const EngineKey &ek, 
    const std::string &qName,  string *params)
{
    ENGINE_PROXY_RPC_BEGIN("GetQueryParameters");
    *params = engine->GetEngineMethods()->GetQueryParameters(qName);
    ENGINE_PROXY_RPC_END_NORESTART_RETHROW2;
}

// ****************************************************************************
// Method: ViewerEngineManagerImplementation::GetProcInfo
//
// Purpose:
//   Engine GetProcInfoRPC wrapped for safety.
//
// Programmer: Mark C. Miller 
// Creation:   November 15, 2004
//
// ****************************************************************************
 
bool
ViewerEngineManagerImplementation::GetProcInfo(const EngineKey &ek, ProcessAttributes &retAtts)
{
    ENGINE_PROXY_RPC_BEGIN("GetProcInfo");
    engine->GetEngineMethods()->GetProcInfo(retAtts);
    ENGINE_PROXY_RPC_END_NORESTART_RETHROW2;
}

// ****************************************************************************
// Method: ViewerEngineManagerImplementation::ReleaseData
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
ViewerEngineManagerImplementation::ReleaseData(const EngineKey &ek, int id)
{
    // If the engine has gone away, we have no need to call this method!
    if (!EngineExists(ek))
        return true;

    ENGINE_PROXY_RPC_BEGIN("ReleaseData");
    engine->GetEngineMethods()->ReleaseData(id);
    ENGINE_PROXY_RPC_END_NORESTART;
}

// ****************************************************************************
//  Method: ViewerEngineManagerImplementation::UpdateDefaultFileOpenOptions
//
//  Purpose:
//      Sets the default file open options.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 22, 2008
//
// ****************************************************************************

void
ViewerEngineManagerImplementation::UpdateDefaultFileOpenOptions(FileOpenOptions *opts)
{
    if (defaultFileOpenOptions == 0)
    {
        defaultFileOpenOptions = new FileOpenOptions;
    }

    // Update the local copy we use when starting a new engine.
    *defaultFileOpenOptions = *opts;

    // And send the updated one to the existing engines.
    for (EngineMap::iterator it = engines.begin() ; it != engines.end(); it++)
    {
        it->second.proxy->GetEngineMethods()->SetDefaultFileOpenOptions(*defaultFileOpenOptions);
    }    
}


// ****************************************************************************
//  Method: ViewerEngineManagerImplementation::UpdatePrecisionType
//
//  Purpose:
//      Sets the precision type.
//
//  Programmer: Kathleen Biagas
//  Creation:   August 1, 2013
//
// ****************************************************************************

void
ViewerEngineManagerImplementation::UpdatePrecisionType(const int pType)
{
    for (EngineMap::iterator it = engines.begin() ; it != engines.end(); it++)
    {
        it->second.proxy->GetEngineMethods()->SetPrecisionType(pType);
    }
}

// ****************************************************************************
//  Method: ViewerEngineManagerImplementation::UpdateBackendType
//
//  Purpose:
//      Sets the backend type.
//
//  Programmer: Cameron Christensen
//  Creation:   June 10, 2014
//
// ****************************************************************************

void
ViewerEngineManagerImplementation::UpdateBackendType(const int bType)
{
    for (EngineMap::iterator it = engines.begin() ; it != engines.end(); it++)
    {
        it->second.proxy->GetEngineMethods()->SetBackendType(bType);
    }
}


// ****************************************************************************
//  Method: ViewerEngineManagerImplementation::UpdateRemoveDuplicateNodes
//
//  Purpose:
//      Sets the flag for removing duplicate nodes.
//
//  Programmer: Kathleen Biagas
//  Creation:   December 18, 2014
//
// ****************************************************************************

void
ViewerEngineManagerImplementation::UpdateRemoveDuplicateNodes(const bool flag)
{
    for (EngineMap::iterator it = engines.begin() ; it != engines.end(); it++)
    {
        it->second.proxy->GetEngineMethods()->SetRemoveDuplicateNodes(flag);
    }
}

// ****************************************************************************
//  Method: ViewerEngineManagerImplementation::ExportDatabases
//
//  Purpose:
//      Exports a database.
//
//  Programmer: Hank Childs
//  Creation:   May 25, 2005
//
//  Modifications:
//    Brad Whitlock, Fri Nov 3 09:49:33 PDT 2006
//    Prevented non-sim data from being exported to a simulation.
//
//    Brad Whitlock, Tue Apr 29 14:48:11 PDT 2008
//    Added tr()
//
//    Brad Whitlock, Fri Jan 24 16:34:24 PST 2014
//    Allow exporting of multiple plots.
//    Work partially supported by DOE Grant SC0007548.
//
//    Brad Whitlock, Thu Jul 24 21:52:34 EDT 2014
//    Pass in the export attributes.
//
//    Brad Whitlock, Thu Jul 24 22:18:34 EDT 2014
//    Pass timeSuffix.
//
// ****************************************************************************

bool
ViewerEngineManagerImplementation::ExportDatabases(const EngineKey &ek, 
    const intVector &ids, const ExportDBAttributes &expAtts, 
    const std::string &timeSuffix)
{
    ENGINE_PROXY_RPC_BEGIN("ExportDatabase");
    engine->GetEngineMethods()->ExportDatabases(ids, expAtts, timeSuffix);
    ENGINE_PROXY_RPC_END_NORESTART_RETHROW2;
}

// ****************************************************************************
//  Method: ViewerEngineManagerImplementation::CreateNamedSelection
//
//  Purpose:
//      Creates a named selection.
//
//  Programmer: Hank Childs
//  Creation:   January 28, 2009
//
//  Modifications:
//    Brad Whitlock, Tue Dec 14 11:48:13 PST 2010
//    Pass the selection properties to the engine.
//
// ****************************************************************************

bool
ViewerEngineManagerImplementation::CreateNamedSelection(const EngineKey &ek, 
    int id, const SelectionProperties &props, SelectionSummary &summary)
{
    ENGINE_PROXY_RPC_BEGIN("CreateNamedSelection");
        summary = engine->GetEngineMethods()->CreateNamedSelection(id, props);
    ENGINE_PROXY_RPC_END;
}


// ****************************************************************************
//  Method: ViewerEngineManagerImplementation::UpdateNamedSelection
//
//  Purpose:
//      Update a named selection with new properties.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Sep  7 14:44:15 PDT 2011
//
//  Modifications:
//
// ****************************************************************************

bool
ViewerEngineManagerImplementation::UpdateNamedSelection(const EngineKey &ek, 
    int id, const SelectionProperties &props, bool allowCache,
    SelectionSummary &summary)
{
    ENGINE_PROXY_RPC_BEGIN("UpdateNamedSelection");
        summary = engine->GetEngineMethods()->UpdateNamedSelection(id, props, allowCache);
    ENGINE_PROXY_RPC_END;
}


// ****************************************************************************
//  Method: ViewerEngineManagerImplementation::DeleteNamedSelection
//
//  Purpose:
//      Deletes a named selection.
//
//  Programmer: Hank Childs
//  Creation:   January 28, 2009
//
// ****************************************************************************

bool
ViewerEngineManagerImplementation::DeleteNamedSelection(const EngineKey &ek, 
                                          const std::string &selName)
{
    ENGINE_PROXY_RPC_BEGIN("DeleteNamedSelection");
        engine->GetEngineMethods()->DeleteNamedSelection(selName);
    ENGINE_PROXY_RPC_END_NORESTART_RETHROW2;
}


// ****************************************************************************
//  Method: ViewerEngineManagerImplementation::LoadNamedSelection
//
//  Purpose:
//      Loads a named selection.
//
//  Programmer: Hank Childs
//  Creation:   January 28, 2009
//
// ****************************************************************************

bool
ViewerEngineManagerImplementation::LoadNamedSelection(const EngineKey &ek, 
                                         const std::string &selName)
{
    ENGINE_PROXY_RPC_BEGIN("LoadNamedSelection");
    engine->GetEngineMethods()->LoadNamedSelection(selName);
    ENGINE_PROXY_RPC_END_NORESTART_RETHROW2;
}


// ****************************************************************************
//  Method: ViewerEngineManagerImplementation::SaveNamedSelection
//
//  Purpose:
//      Saves a named selection.
//
//  Programmer: Hank Childs
//  Creation:   January 28, 2009
//
// ****************************************************************************

bool
ViewerEngineManagerImplementation::SaveNamedSelection(const EngineKey &ek, 
                                         const std::string &selName)
{
    ENGINE_PROXY_RPC_BEGIN("SaveNamedSelection");
    engine->GetEngineMethods()->SaveNamedSelection(selName);
    ENGINE_PROXY_RPC_END_NORESTART_RETHROW2;
}


// ****************************************************************************
//  Method: ViewerEngineManagerImplementation::ConstructDataBinning
//
//  Purpose:
//      Constructs a data binning.
//
//  Programmer: Hank Childs
//  Creation:   February 13, 2006
//
//  Modifications:
//
//    Hank Childs, Sat Aug 21 14:05:14 PDT 2010
//    Rename method from DDF to data binning.
//
// ****************************************************************************

bool
ViewerEngineManagerImplementation::ConstructDataBinning(const EngineKey &ek, int id)
{
    ENGINE_PROXY_RPC_BEGIN("ConstructDataBinning");
    engine->GetEngineMethods()->ConstructDataBinning(id, 
        GetViewerState()->GetConstructDataBinningAttributes());
    ENGINE_PROXY_RPC_END_NORESTART_RETHROW2;
}


// ****************************************************************************
// Method: ViewerEngineManagerImplementation::CloneNetwork
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
ViewerEngineManagerImplementation::CloneNetwork(const EngineKey &ek, int nid,
                           const QueryOverTimeAttributes *qatts)
{
    ENGINE_PROXY_RPC_BEGIN("CloneNetwork");
    engine->GetEngineMethods()->CloneNetwork(nid, qatts);
    ENGINE_PROXY_RPC_END_NORESTART_RETHROW2;
}

// ****************************************************************************
//  Method:  ViewerEngineManagerImplementation::UpdateExpressions
//
//  Purpose:
//    Tells the engine to update its epxressions. 
//
//  Arguments:
//    ek : The key that identifies the engine.
//    eL : The new expression list we're sending to the engine. 
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 1, 2005 
//
//  Modifications:
//    Brad Whitlock, Fri Aug 19 09:57:15 PDT 2011
//    I changed it from being plot-specific to accepting any expression list.
//    I also made it start an engine if one is needed.
//
// ****************************************************************************

bool
ViewerEngineManagerImplementation::UpdateExpressions(const EngineKey &ek, 
    const ExpressionList &eL)
{
    ENGINE_PROXY_RPC_BEGIN("UpdateExpressions");  
        engine->GetEngineMethods()->UpdateExpressions(eL);
    ENGINE_PROXY_RPC_END
}

// ****************************************************************************
//  Method:  ViewerEngineManagerImplementation::SendSimulationCommand
//
//  Purpose:
//    Allow sending a command to the engine.
//
//  Arguments:
//    ek         the engine key
//    command    the command string
//    argument   the argument to the command
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  4, 2005
//
// ****************************************************************************
void
ViewerEngineManagerImplementation::SendSimulationCommand(const EngineKey &ek,
                                           const std::string &command,
                                           const std::string &argument)
{
    if (EngineExists(ek))
    {
        engines[ek].proxy->GetEngineMethods()->ExecuteSimulationControlCommand(command, argument);
    }
    else
    {
        ostringstream oss;
        oss << "In ViewerEngineManagerImplementation::SendSimulationCommand"
               " failed to find the engine for key:" << endl
            << "  ";
        ek.Print(oss);
        oss << endl
            << "Known keys:" << endl;
        EngineMap::iterator it = engines.begin();
        EngineMap::iterator end = engines.end();
        for (; it != end; ++it)
          {
          oss << "  ";
          (*it).first.Print(oss);
          oss << endl;
          }
        debug1 << oss.str() << endl;
        EXCEPTION0(NoEngineException);
    }
}

// ****************************************************************************
// Method: ViewerEngineManagerImplementation::LaunchProcess
//
// Purpose: 
//   Launch a process via the compute engine.
//
// Arguments:
//  ek   : The engine key.
//  args : The program to launch and its arguments.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 29 20:40:21 PST 2011
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerEngineManagerImplementation::LaunchProcess(const EngineKey &ek, const stringVector &args)
{
    ENGINE_PROXY_RPC_BEGIN("LaunchProcess");  
        engine->GetEngineMethods()->LaunchProcess(args);
    ENGINE_PROXY_RPC_END
}
