/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                                ViewerProxy.C                              //
// ************************************************************************* //

#include <ViewerProxy.h>

#include <PlotPluginInfo.h>
#include <PlotPluginManager.h>
#include <OperatorPluginInfo.h>
#include <OperatorPluginManager.h>

#include <DebugStream.h>
#include <ParentProcess.h>
#include <RemoteProcess.h>
#include <SocketConnection.h>
#include <ViewerMethods.h>
#include <ViewerRPC.h>
#include <ViewerState.h>
#include <VisItException.h>
#include <Xfer.h>


#include <ClientInformation.h>
#include <ClientMethod.h>
#include <SILRestrictionAttributes.h>
#include <PluginManagerAttributes.h>

#include <snprintf.h>

// ****************************************************************************
//  Method: ViewerProxy constructor
//
//  Programmer: Eric Brugger
//  Creation:   August 4, 2000
//
//  Modifications:
//   Brad Whitlock, Fri Feb 9 18:15:39 PST 2007
//   I rewrote the method so it uses ViewerState and ViewerMethods and I
//   removed old modification comments.
//
// ****************************************************************************

ViewerProxy::ViewerProxy() : SimpleObserver(), argv()
{
    viewer = 0;
    viewerP = 0;
    xfer = new Xfer;

    state = new ViewerState;
    methods = new ViewerMethods(state);

    // Make the proxy observe the SIL restriction attributes.
    state->GetSILRestrictionAttributes()->Attach(this);

    animationStopOpcode = 0;
    iconifyOpcode = 0;
}

// ****************************************************************************
//  Method: ViewerProxy destructor
//
//  Programmer: Eric Brugger
//  Creation:   August 4, 2000
//
//  Modifications:
//    Brad Whitlock, Fri Feb 9 18:18:36 PST 2007
//    I rewrote the method so it uses ViewerState and I removed old modification
//    comments.
//
// ****************************************************************************

ViewerProxy::~ViewerProxy()
{
    int i;

    delete viewer;
    delete viewerP;
    delete xfer;
    delete state;
    delete methods;
}

// ****************************************************************************
// Method: ViewerProxy::GetViewerState
//
// Purpose: 
//   Returns the ViewerState state objects.
//
// Returns:    Returns the ViewerState state objects.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 13 11:39:16 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

ViewerState *
ViewerProxy::GetViewerState() const
{
    return state;
}

// ****************************************************************************
// Method: ViewerProxy::GetViewerMethods
//
// Purpose: 
//   Returns the methods object.
//
// Returns:    Returns the methods object.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 13 11:39:55 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

ViewerMethods *
ViewerProxy::GetViewerMethods() const
{
    return methods;
}

// ****************************************************************************
// Method: ViewerProxy::Update
//
// Purpose: 
//   This method is called when objects that the ViewerProxy observes are
//   updated. This is an internal method and is primarily a means to update
//   the internal SIL restriction object.
//
// Arguments:
//   subj : A pointer to the subject that updated.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 21 13:23:50 PST 2001
//
// Modifications:
//   Hank Childs, Mon Dec  2 14:13:55 PST 2002
//   Create a new SIL restriction as a reference counted object from the heap.
//
//   Brad Whitlock, Fri Feb 9 18:19:41 PST 2007
//   Made it use ViewerState.
//
// ****************************************************************************

void
ViewerProxy::Update(Subject *subj)
{
    // If the SIL restriction attributes have changed, update the internal
    // SIL restriction object internalSILRestriction. This is done so the
    // internalSILRestriction is available to the client and we have to
    // make the conversion far less often.
    if(subj == state->GetSILRestrictionAttributes())
    {
        internalSILRestriction = new avtSILRestriction(
            *state->GetSILRestrictionAttributes());
    }
}

// ****************************************************************************
//  Method: ViewerProxy::GetReadConnection
//
//  Purpose:
//    Get the connection that the viewer reads from.
//
//  Returns:    The connection the viewer reads from.
//
//  Programmer: Eric Brugger
//  Creation:   August 4, 2000
//
//  Modifications:
//    Brad Whitlock, Tue May 3 15:46:49 PST 2005
//    Added viewerP.
//
// ****************************************************************************

Connection *
ViewerProxy::GetReadConnection() const
{
    if (viewer == 0 && viewerP == 0)
        return 0;

    if (viewer != 0)      
        return viewer->GetReadConnection();

    return viewerP->GetReadConnection();
}

// ****************************************************************************
//  Method: ViewerProxy::GetWriteConnection
//
//  Purpose:
//    Get the connection that the viewer writes to.
//
//  Returns:    The connection the viewer writes to.
//
//  Programmer: Eric Brugger
//  Creation:   August 4, 2000
//
//  Modifications:
//    Brad Whitlock, Tue May 3 15:46:49 PST 2005
//    Added viewerP.
//
// ****************************************************************************

Connection *
ViewerProxy::GetWriteConnection() const
{
    if (viewer == 0 && viewerP == 0)
        return 0;

    if(viewer != 0)
        return viewer->GetWriteConnection();

    return viewerP->GetWriteConnection();
}

// ****************************************************************************
// Method: ViewerProxy::GetLocalHostName
//
// Purpose: 
//   Returns the name of the local machine.
//
// Returns:    The name of the local machine.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 24 11:32:45 PDT 2001
//
// Modifications:
//   Brad Whitlock, Tue May 3 15:49:40 PST 2005
//   Added viewerP.
//
// ****************************************************************************

const std::string &
ViewerProxy::GetLocalHostName() const
{
    if(viewer == 0 && viewerP == 0)
    {
        EXCEPTION1(VisItException, "Viewer not created.");
    }

    if(viewer != 0)
        return viewer->GetLocalHostName();

#if 0
    return viewerP->GetLocalHostName();
#else
    cerr << "Fix ViewerProxy::GetLocalHostName: " << __LINE__ << endl;
    return *(new std::string("localhost"));
#endif
}

// ****************************************************************************
// Method: ViewerProxy::GetLocalUserName
//
// Purpose: 
//   Returns the local user name.
//
// Returns:    The local user name.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 21 10:06:52 PDT 2002
//
// Modifications:
//   Brad Whitlock, Tue May 3 15:49:57 PST 2005
//   Added viewerP.
//
//   Brad Whitlock, Wed Jan 11 17:03:46 PST 2006
//   I fixed the code for viewerP->GetLocalUseName.
//
// ****************************************************************************

const std::string &
ViewerProxy::GetLocalUserName() const
{
    if(viewer == 0 && viewerP == 0)
    {
        EXCEPTION1(VisItException, "Viewer not created.");
    }

    if(viewer != 0)
        return viewer->GetLocalUserName();

    return viewerP->GetLocalUserName();
}

// ****************************************************************************
//  Method: ViewerProxy::ProcessInput
//
//  Purpose:
//    Read data from the viewer and process it.
//
//  Programmer: Eric Brugger
//  Creation:   August 22, 2000
//
//  Modifications:
//    Brad Whitlock, Tue Apr 24 10:35:56 PDT 2001
//    Added code to detect that the viewer died.
//
//    Brad Whitlock, Wed Mar 20 17:45:28 PST 2002
//    I abstracted the read code.
//
//    Brad Whitlock, Tue May 3 15:52:06 PST 2005
//    I made it get the connection from xfer instead of viewer.
//
// ****************************************************************************

void
ViewerProxy::ProcessInput()
{
    //
    // Try and read from the viewer.
    //
    int amountRead = xfer->GetInputConnection()->Fill();

    //
    // Try and process the input.
    //
    if (amountRead > 0)
        xfer->Process();
}

// ****************************************************************************
// Method: ViewerProxy::AddArgument
//
// Purpose: 
//   Adds an argument to the viewer's argument list.
//
// Arguments:
//   arg : The argument to be added.
//
// Note:
//   This only has an effect if called before the Create call.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 7 13:48:12 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Dec 20 11:53:23 PDT 2002
//   I made argv be a stringVector.
//
// ****************************************************************************

void
ViewerProxy::AddArgument(const std::string &arg)
{
    if(arg.size() == 0)
        return;

    argv.push_back(arg);
}

// ****************************************************************************
//  Method: ViewerProxy::Create
//
//  Purpose:
//    Create a viewer.
//
//  Programmer: Eric Brugger
//  Creation:   August 4, 2000
//
//  Modifications:
//    Brad Whitlock, Fri Feb 9 18:20:24 PST 2007
//    I made it use ViewerState and I removed old modification comments.
//
// ****************************************************************************

void
ViewerProxy::Create(int *inputArgc, char ***inputArgv)
{
    Create("visit", inputArgc, inputArgv);
}

void
ViewerProxy::Create(const char *visitProgram, int *inputArgc, char ***inputArgv)
{
    //
    // Look for flags required for reverse launching.
    //
    bool haveRL = false;
    bool haveKey = false;
    bool havePort = false;
    if(inputArgc != 0 && inputArgv != 0)
    {
        int count = *inputArgc;
        char **arg = *inputArgv;
        for(int i = 0; i < count; ++i)
        {
            if(strcmp(arg[i], "-reverse_launch") == 0)
                haveRL = true;
            else if(strcmp(arg[i], "-key") == 0)
                haveKey = true;
            else if(strcmp(arg[i], "-port") == 0)
                havePort = true;
        }
    }
    bool reverseLaunch = haveRL && haveKey && havePort;

    if(!reverseLaunch)
    {
        //
        // Create the viewer process.  The viewer is executed using
        // "visit -viewer".
        //
        viewer = new RemoteProcess(std::string(visitProgram));
        viewer->AddArgument(std::string("-viewer"));

        //
        // Add any extra arguments to the viewer before opening it.
        //
        for (int i = 0; i < argv.size(); ++i)
            viewer->AddArgument(argv[i]);

        //
        // Open the viewer.
        //
        viewer->Open("localhost",
                     HostProfile::MachineName, "", 
                     false, 0,
                     1, 1);

        // Use viewer's connections for xfer.
        xfer->SetInputConnection(viewer->GetWriteConnection());
        xfer->SetOutputConnection(viewer->GetReadConnection());
    }
    else
    {
        //
        // Connect to the viewer process. Our command line arguments
        // should contain  The viewer is executed using
        // "visit -viewer".
        //
        viewerP = new ParentProcess;
        viewerP->Connect(1, 1, inputArgc, inputArgv, true);

        // Use viewerP's connections for xfer.
        xfer->SetInputConnection(viewerP->GetWriteConnection());
        xfer->SetOutputConnection(viewerP->GetReadConnection());
    }

    //
    // Attach the AttributeSubjects to the xfer object. Note that the
    // plugin objects have not yet been added.
    //
    for(int i = 0; i < state->GetNumStateObjects(); ++i)
        xfer->Add(state->GetStateObject(i));

    xfer->ListObjects();

    // Create the special opcodes.
    animationStopOpcode = xfer->CreateNewSpecialOpcode();
    iconifyOpcode = xfer->CreateNewSpecialOpcode();
}

// ****************************************************************************
//  Method:  ViewerProxy::LoadPlugins
//
//  Purpose:
//    Unload and reload all the plugins
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 26, 2001
//
//  Modifications:
//    Brad Whitlock, Mon Mar 25 13:00:10 PST 2002
//    I added code to print the list of connected objects to the log file.
//
//    Jeremy Meredith, Wed Aug 21 12:51:28 PDT 2002
//    I renamed some plot/operator plugin manager methods for refactoring.
//
//    Jeremy Meredith, Fri Feb 28 12:21:01 PST 2003
//    Renamed LoadPlugins to LoadPluginsNow (since there is a corresponding
//    LoadPluginsOnDemand).
//
//    Brad Whitlock, Fri Feb 9 18:22:54 PST 2007
//    I made it use ViewerState and I improved the exception error message.
//
// ****************************************************************************

void
ViewerProxy::LoadPlugins()
{
    int i;
    PlotPluginManager     *pMgr = PlotPluginManager::Instance();
    OperatorPluginManager *oMgr = OperatorPluginManager::Instance();

    int nPlots = state->GetNumPlotStateObjects();
    int nOperators = state->GetNumOperatorStateObjects();
    if (nPlots > 0 || nOperators > 0)
    {
        debug1 << "Reloading plugins at runtime not supported\n";
        return;
    }

    //
    // Go through the plugin atts and en/disable the ones specified
    // by the plugin attributes
    //
    PluginManagerAttributes *pluginManagerAttributes = state->GetPluginManagerAttributes();
    for (i=0; i<pluginManagerAttributes->GetId().size(); i++)
    {
        if (! pluginManagerAttributes->GetEnabled()[i]) // not enabled
        {
            std::string id = pluginManagerAttributes->GetId()[i];
            if (pluginManagerAttributes->GetType()[i] == "plot")
            {
                if (pMgr->PluginExists(id))
                    pMgr->DisablePlugin(id);
            }
            else if (pluginManagerAttributes->GetType()[i] == "operator")
            {
                if (oMgr->PluginExists(id))
                    oMgr->DisablePlugin(id);
            }
        }
        else // is enabled -- it had better be available
        {
            std::string id = pluginManagerAttributes->GetId()[i];
            if (pluginManagerAttributes->GetType()[i] == "plot")
            {
                if (pMgr->PluginExists(id))
                    pMgr->EnablePlugin(id);
                else
                {
                    std::string msg(std::string("The ") + id + 
                        std::string(" plot plugin enabled by the viewer "
                            "was not available in the client."));
                    EXCEPTION1(VisItException, msg);
                }
            }
            else if (pluginManagerAttributes->GetType()[i] == "operator")
            {
                if (oMgr->PluginExists(id))
                    oMgr->EnablePlugin(id);
                else
                {
                    std::string msg(std::string("The ") + id + 
                        std::string(" operator plugin enabled by the viewer "
                            "was not available in the client."));
                    EXCEPTION1(VisItException, msg);
                }
            }
        }
    }

    //
    // Now load dynamic libraries
    //
    pMgr->LoadPluginsNow();
    oMgr->LoadPluginsNow();

    //
    // Initialize the plot attribute state objects.
    //
    nPlots = pMgr->GetNEnabledPlugins();
    for (i = 0; i < nPlots; ++i)
    {
        CommonPlotPluginInfo *info =
            pMgr->GetCommonPluginInfo(pMgr->GetEnabledID(i));
        AttributeSubject *obj = info->AllocAttributes();
        state->RegisterPlotAttributes(obj);
        xfer->Add(obj);
    }

    //
    // Initialize the operator attribute state objects.
    //
    nOperators = oMgr->GetNEnabledPlugins();
    for (i = 0; i < nOperators; ++i)
    {
        CommonOperatorPluginInfo *info = 
            oMgr->GetCommonPluginInfo(oMgr->GetEnabledID(i));
        AttributeSubject *obj = info->AllocAttributes();
        state->RegisterOperatorAttributes(obj);
        xfer->Add(obj);
    }

    //
    // Print the list of connected objects to the log file.
    //
    xfer->ListObjects();
}

// ****************************************************************************
//  Method: ViewerProxy::Close
//
//  Purpose:
//    Terminate the viewer.
//
//  Programmer: Eric Brugger
//  Creation:   August 11, 2000
//
//  Modifications:
//    Brad Whitlock, Wed Apr 3 12:50:01 PDT 2002
//    Called a new method of RemoteProcess to wait for the viewer to quit.
//
//    Brad Whitlock, Tue May 3 16:04:17 PST 2005
//    Only wait for termination if we launched the viewer.
//
// ****************************************************************************

void
ViewerProxy::Close()
{
    // Tell the viewer to close.
    methods->Close();

    //
    // Wait for the viewer to exit.
    //
    if(viewer != 0)
        viewer->WaitForTermination();
}

// ****************************************************************************
// Method: ViewerProxy::Detach
//
// Purpose: 
//   Tells the viewer to detach this client from the list of clients.
//
// Programmer: Brad Whitlock
// Creation:   Thu May 5 17:37:55 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::Detach()
{
    methods->Detach();
}

// ****************************************************************************
// Method: ViewerProxy::InterruptComputeEngine
//
// Purpose: 
//   Interrupts the engine on the specified host.
//
// Arguments:
//   hostName : The host of the engine that we want to interrupt.
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 30 12:24:28 PDT 2001
//
// Modifications:
//    Jeremy Meredith, Tue Jul  3 15:10:28 PDT 2001
//    Changed the interruption mechanism.
//   
//    Jeremy Meredith, Tue Mar 30 11:07:45 PST 2004
//    Added a simulation name to the interfaces, as some engines now
//    can be simulations, meaning there might be more than one engine
//    per host.
//
// ****************************************************************************
void
ViewerProxy::InterruptComputeEngine(const std::string &hostName,
                                    const std::string &simName)
{
    xfer->SendInterruption();
}

// ****************************************************************************
//  Method: ViewerProxy::AnimationStop
//
//  Purpose:
//    Stop the active animation.
//
//  Programmer: Eric Brugger
//  Creation:   August 15, 2000
//
//  Modifications:
//    Brad Whitlock, Thu Feb 27 11:33:16 PDT 2003
//    I made this function also send a special opcode to stop the animation.
//
// ****************************************************************************

void
ViewerProxy::AnimationStop()
{
    //
    // Send a special opcode to stop the animation.
    //
    xfer->SendSpecialOpcode(animationStopOpcode);

    methods->AnimationStop();
}

// ****************************************************************************
// Method: ViewerProxy::SetPlotSILRestriction
//
// Purpose: 
//   Tells the viewer to apply the SIL restriction to the selected plots in
//   the plot list.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 21 13:33:41 PST 2001
//
// Modifications:
//   
//   Hank Childs, Mon Dec  2 14:13:55 PST 2002
//   Account for the SIL restriction now being a pointer.
//
// ****************************************************************************

void
ViewerProxy::SetPlotSILRestriction()
{
    SILRestrictionAttributes *newSRA =internalSILRestriction->MakeAttributes();
    if(newSRA)
    {
        // Copy the attribute representation of the SIL restriction into the
        // silRestrictionAtts object and send notify observers. Note that
        // SetUpdate is set to false so the proxy's Update method is NOT called
        SILRestrictionAttributes *silRestrictionAtts = 
            state->GetSILRestrictionAttributes();
        *silRestrictionAtts = *newSRA;
        SetUpdate(false);
        silRestrictionAtts->Notify();

        // Now that the new SIL restriction attributes have been sent to the
        // viewer, send the RPC that tells the viewer to apply them.
        methods->SetPlotSILRestriction();

        // Delete the new SRA since we're done with it.
        delete newSRA;
    }
}

// ****************************************************************************
// Method: ViewerProxy::SetPlotSILRestriction
//
// Purpose: 
//   Tells the viewer to apply the specified new SIL restriction to the
//   selected plots in the plot list.
//
// Arguments:
//   newRestriction the new SIL restriction.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 21 13:33:41 PST 2001
//
// Modifications:
//   
//   Hank Childs, Mon Dec  2 14:13:55 PST 2002
//   Account for the SIL restriction now being a pointer.
//
// ****************************************************************************

void
ViewerProxy::SetPlotSILRestriction(avtSILRestriction_p newRestriction)
{
    // Copy the new SIL restriction into the internal SIL restriction object.
    internalSILRestriction = newRestriction;

    SILRestrictionAttributes *newSRA =internalSILRestriction->MakeAttributes();
    if(newSRA)
    {
        // Copy the attribute representation of the SIL restriction into the
        // silRestrictionAtts object and send notify observers. Note that
        // SetUpdate is set to false so the proxy's Update method is NOT called
        SILRestrictionAttributes *silRestrictionAtts = 
            state->GetSILRestrictionAttributes();
        *silRestrictionAtts = *newSRA;
        SetUpdate(false);
        silRestrictionAtts->Notify();

        // Now that the new SIL restriction attributes have been sent to the
        // viewer, send the RPC that tells the viewer to apply them.
        methods->SetPlotSILRestriction();

        // Delete the new SRA since we're done with it.
        delete newSRA;
    }
}

// ****************************************************************************
// Method: ViewerProxy::MethodRequestHasRequiredInformation
//
// Purpose: 
//   Looks at the current method request and determines if the method is
//   supported by the client and whether the method request has enough
//   information to support the method.
//
// Returns:    0 for method not supported
//             1 for method supported, not enough info
//             2 for method supported, enough info
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu May 5 20:04:16 PST 2005
//
// Modifications:
//   
// ****************************************************************************

int
ViewerProxy::MethodRequestHasRequiredInformation() const
{
    int retval;
    int index = state->GetClientInformation()->GetMethodIndex(
        state->GetClientMethod()->GetMethodName());

    if(index == -1)
    {
        // Method not supported.
        retval = 0;
    }
    else
    {
        std::string proto(state->GetClientInformation()->
            GetMethodPrototype(index));
        if(proto.size() < 1)
        {
            // Supported. Does not need args.
            retval = 2; 
        }
        else
        {
            int sCount = 0; 
            int dCount = 0;
            int iCount = 0;
            for(int i = 0; i < proto.size(); ++i)
            {
                if(proto[i] == 's')
                    ++sCount;
                else if(proto[i] == 'd')
                    ++dCount;
                else if(proto[i] == 'i')
                    ++iCount;
            }

            if(state->GetClientMethod()->GetIntArgs().size() >= iCount &&
               state->GetClientMethod()->GetDoubleArgs().size() >= dCount &&
               state->GetClientMethod()->GetStringArgs().size() >= sCount)
            {
                // Supported, has enough args.
                retval = 2;
            }
            else
            {
                // Supported, not enough args.
                retval = 1;
            }
        }
    }
 
    return retval;
}

// ****************************************************************************
// Method: ViewerProxy::SetXferUpdate
//
// Purpose: 
//   Sets Xfer's update flag.
//
// Arguments:
//   val : Whether updates should be allowed.
//
// Note:       This method allows a state object observer to send back
//             information to the viewer by following the following pattern:
//
//             SetXferUpdate(true);
//             obj->Notify();
//             SetXferUpdate(false);
//
//             Note that setting the xfer update flag to false after the
//             object's Notify method is important or the object that we're
//             observing will get back to the client. Be careful if the object
//             that you're Notifying has any other observers.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 6 09:50:22 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::SetXferUpdate(bool val)
{
    xfer->SetUpdate(val);
}
