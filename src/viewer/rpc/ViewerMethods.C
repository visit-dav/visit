/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
#include <ViewerMethods.h>
#include <ViewerState.h>
#include <ViewerRPC.h>
#include <ColorTableAttributes.h>

#include <snprintf.h>

// ****************************************************************************
// Method: ViewerMethods::ViewerMethods
//
// Purpose: 
//   Constructor for the ViewerMethods class.
//
// Arguments:
//   s : The ViewerState object that will be used to create messages to the
//       viewer.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 13 10:47:11 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

ViewerMethods::ViewerMethods(ViewerState *s)
{
    // Keep a pointer to the viewer state.
    state = s;
}

// ****************************************************************************
// Method: ViewerMethods::~ViewerMethods
//
// Purpose: 
//   Destructor for the ViewerMethods class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 13 10:47:49 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

ViewerMethods::~ViewerMethods()
{
}


// ****************************************************************************
//  Method: ViewerMethods::Close
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
ViewerMethods::Close()
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::CloseRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::Detach
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
ViewerMethods::Detach()
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::DetachRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::AddWindow
//
//  Purpose:
//    Add a window.
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2000
//
// ****************************************************************************

void
ViewerMethods::AddWindow()
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::AddWindowRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::CloneWindow
//
//  Purpose:
//    Clones the current window.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Oct 15 16:25:24 PST 2002
//
// ****************************************************************************

void
ViewerMethods::CloneWindow()
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::CloneWindowRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::DeleteWindow
//
//  Purpose:
//    Delete the active window.
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2000
//
// ****************************************************************************

void
ViewerMethods::DeleteWindow()
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::DeleteWindowRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::SetWindowLayout
//
//  Purpose:
//    Set the window layout.
//
//  Arguments:
//    layout    The layout to use.
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2000
//
// ****************************************************************************

void
ViewerMethods::SetWindowLayout(int layout)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetWindowLayoutRPC);
    state->GetViewerRPC()->SetWindowLayout(layout);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::SetActiveWindow
//
//  Purpose:
//    Set the active window.
//
//  Arguments:
//    windowId  The identifier of the window to make active.
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2000
//
// ****************************************************************************

void
ViewerMethods::SetActiveWindow(int windowId)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetActiveWindowRPC);
    state->GetViewerRPC()->SetWindowId(windowId);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::IconifyAllWindows
//
// Purpose: 
//   Tells the viewer to iconify all of its windows.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 19 10:58:05 PDT 2001
//
// Modifications:
//   Brad Whitlock, Wed Mar 12 10:47:20 PDT 2003
//   I made the method also send a special opcode to immediately iconify
//   the windows.
//
// ****************************************************************************

void
ViewerMethods::IconifyAllWindows()
{
#ifdef FROM_VIEWER_PROXY
    //
    // Send a special opcode to stop the animation.
    //
    xfer->SendSpecialOpcode(iconifyOpcode);
#endif

    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::IconifyAllWindowsRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::DeIconifyAllWindows
//
// Purpose: 
//   Tells the viewer to de-iconify all of its windows.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 19 10:58:37 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::DeIconifyAllWindows()
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::DeIconifyAllWindowsRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::ShowAllWindows
//
// Purpose: 
//   Tells the viewer to show all of its windows.
//
// Programmer: Sean Ahern
// Creation:   Tue Apr 16 12:32:12 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::ShowAllWindows()
{
    // Set the rpc type and arguments.
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ShowAllWindowsRPC);

    // Issue the RPC.
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::HideAllWindows
//
// Purpose: 
//   Tells the viewer to hide all of its windows.
//
// Programmer: Sean Ahern
// Creation:   Tue Apr 16 12:32:33 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::HideAllWindows()
{
    // Set the rpc type and arguments.
    state->GetViewerRPC()->SetRPCType(ViewerRPC::HideAllWindowsRPC);

    // Issue the RPC.
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::ClearWindow
//
//  Purpose:
//    Clear the active window.
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2000
//
// ****************************************************************************

void
ViewerMethods::ClearWindow(bool clearAllPlots)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ClearWindowRPC);
    state->GetViewerRPC()->SetBoolFlag(clearAllPlots);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::ClearAllWindows
//
//  Purpose:
//    Clear all the windows.
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2000
//
// ****************************************************************************

void
ViewerMethods::ClearAllWindows()
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ClearAllWindowsRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::ConnectToMetaDataServer
//
// Purpose: 
//   Tells the viewer's metadata server running on hostName to connect to the
//   program running on localHost that is listening on the specified port.
//
// Arguments:
//   hostName  : The host on which the mdserver is running.
//   args      : The arguments that tell the mdserver how to connect back.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 21 11:26:00 PDT 2000
//
// Modifications:
//   Brad Whitlock, Thu Dec 19 11:41:13 PDT 2002
//   I added the key argument.
//
//   Brad Whitlock, Mon May 5 14:10:09 PST 2003
//   I replaced several arguments with textual args.
//
// ****************************************************************************

void
ViewerMethods::ConnectToMetaDataServer(const std::string &hostName,
    const stringVector &args)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ConnectToMetaDataServerRPC);
    state->GetViewerRPC()->SetProgramHost(hostName);
    state->GetViewerRPC()->SetProgramOptions(args);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::OpenDatabase
//
//  Purpose:
//    Open a database.
//
//  Arguments:
//    database        : The name of the database to open.
//    timeState       : The timestate that we want to open.
//    addDefaultPlots : Whether we want to allow the viewer to add default
//                      plots.
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2000
//
//  Modifications:
//    Brad Whitlock, Thu May 15 13:03:57 PST 2003
//    I made it possible to open a database at a later time state.
//
//    Brad Whitlock, Wed Oct 22 12:22:44 PDT 2003
//    I made it possible to tell the viewer that we don't want to add default
//    plots even if the database has them.
//   
//    Jeremy Meredith, Mon Aug 28 16:55:01 EDT 2006
//    Added ability to force using a specific plugin when opening a file.
//
// ****************************************************************************

void
ViewerMethods::OpenDatabase(const std::string &database, int timeState,
    bool addDefaultPlots, const std::string &forcedFileType)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::OpenDatabaseRPC);
    state->GetViewerRPC()->SetDatabase(database);
    state->GetViewerRPC()->SetIntArg1(timeState);
    state->GetViewerRPC()->SetBoolFlag(addDefaultPlots);
    state->GetViewerRPC()->SetStringArg1(forcedFileType);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::CloseDatabase
//
// Purpose: 
//   Closes the specified database.
//
// Arguments:
//   database : The database that we're closing.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 27 11:59:32 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::CloseDatabase(const std::string &database)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::CloseDatabaseRPC);
    state->GetViewerRPC()->SetDatabase(database);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::ActivateDatabase
//
// Purpose: 
//   Activates the specified database, which makes it the active source
//   but does not mess with time or anything like that.
//
// Arguments:
//   database : The new active source.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 29 22:13:52 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::ActivateDatabase(const std::string &database)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ActivateDatabaseRPC);
    state->GetViewerRPC()->SetDatabase(database);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::CheckForNewStates
//
// Purpose: 
//   Checks the specified database for new states.
//
// Arguments:
//   database : The database we're checking for new states.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 29 22:13:52 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::CheckForNewStates(const std::string &database)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::CheckForNewStatesRPC);
    state->GetViewerRPC()->SetDatabase(database);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::ReOpenDatabase
//
//  Purpose:
//    Reopens a database.
//
//  Arguments:
//    database  The name of the database to reopen.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Jul 29 15:16:37 PST 2002
//
//  Modifications:
//    Jeremy Meredith, Wed Aug 25 10:33:09 PDT 2004
//    Made it use the generic integer argument so as to not be misleading.
//
// ****************************************************************************

void
ViewerMethods::ReOpenDatabase(const std::string &database, bool forceClose)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ReOpenDatabaseRPC);
    state->GetViewerRPC()->SetDatabase(database);
    // Store the flag in the generic integer argument
    state->GetViewerRPC()->SetIntArg1(forceClose ? 1 : 0);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::ReplaceDatabase
//
//  Purpose:
//    Replaces the open database with this database.
//
//  Arguments:
//    database  The name of the database to use.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Mar 6 16:07:47 PST 2002
//
//  Modifications:
//    Brad Whitlock, Wed Oct 15 15:37:44 PST 2003
//    I added an optional timeState argument so we can replace databases
//    at later time states.
//
// ****************************************************************************

void
ViewerMethods::ReplaceDatabase(const std::string &database, int timeState)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ReplaceDatabaseRPC);
    state->GetViewerRPC()->SetDatabase(database);
    state->GetViewerRPC()->SetIntArg1(timeState);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::OverlayDatabase
//
//  Purpose:
//    Overlayes a database.
//
//  Arguments:
//    database  The name of the database to overlay.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Mar 6 16:08:26 PST 2002
//
// ****************************************************************************

void
ViewerMethods::OverlayDatabase(const std::string &database)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::OverlayDatabaseRPC);
    state->GetViewerRPC()->SetDatabase(database);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::RequestMetaData
//
//  Purpose:
//    Requests metadata for the specified database. The resulting metadata
//    comes back to the client via the metadata object.
//
//  Arguments:
//    database  The name of the database for which we want metadata.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Mar 9 16:23:46 PST 2007
//
// ****************************************************************************

void
ViewerMethods::RequestMetaData(const std::string &database, int ts)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::RequestMetaDataRPC);
    state->GetViewerRPC()->SetDatabase(database);
    state->GetViewerRPC()->SetStateNumber(ts);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::ExportDatabase
//
// Purpose: 
//     Exports a database.
//
// Programmer: Hank Childs
// Creation:   May 25, 2005
//
// ****************************************************************************

void
ViewerMethods::ExportDatabase()
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ExportDBRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::ConstructDDF
//
// Purpose: 
//     Construct a DDF.
//
// Programmer: Hank Childs
// Creation:   February 13, 2006
//
// ****************************************************************************

void
ViewerMethods::ConstructDDF()
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ConstructDDFRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::ClearCache
//
// Purpose: 
//   Tells the viewer to clear the cache for the compute engine on the
//   specified host.
//
// Arguments:
//   hostName : The host where the compute engine is running.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 30 14:16:15 PST 2002
//
// Modifications:
//    Jeremy Meredith, Tue Mar 30 11:07:45 PST 2004
//    Added a simulation name to the interfaces, as some engines now
//    can be simulations, meaning there might be more than one engine
//    per host.
//
// ****************************************************************************

void
ViewerMethods::ClearCache(const std::string &hostName,const std::string &simName)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ClearCacheRPC);
    state->GetViewerRPC()->SetProgramHost(hostName);
    state->GetViewerRPC()->SetProgramSim(simName);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::ClearCacheForAllEngines
//
// Purpose: 
//   Tells the viewer to clear the cache on all compute engines.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 26 13:37:38 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::ClearCacheForAllEngines()
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ClearCacheForAllEnginesRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::UpdateDBPluginInfo
//
// Purpose: 
//     Tells the viewer to update the DB plugin info.
//
// Arguments:
//   hostName : The host where the compute engine is running.
//
// Programmer: Hank Childs
// Creation:   May 25, 2005
//
// ****************************************************************************

void
ViewerMethods::UpdateDBPluginInfo(const std::string &hostName)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::UpdateDBPluginInfoRPC);
    state->GetViewerRPC()->SetProgramHost(hostName);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::CreateDatabaseCorrelation
//
// Purpose: 
//   Creates a database correlation for the databases using the given
//   correlation method.
//
// Arguments:
//   name    : The name of the correlation.
//   dbs     : The databases in the correlation.
//   method  : The correlation method.
//   nStates : The number of states in the correlation (or -1 if you don't care).
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 30 21:19:28 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void 
ViewerMethods::CreateDatabaseCorrelation(const std::string &name,
    const stringVector &dbs, int method, int nStates)
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::CreateDatabaseCorrelationRPC);
    state->GetViewerRPC()->SetDatabase(name);
    state->GetViewerRPC()->SetProgramOptions(dbs);
    state->GetViewerRPC()->SetIntArg1(method);
    state->GetViewerRPC()->SetIntArg2(nStates);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::AlterDatabaseCorrelation
//
// Purpose: 
//   Alters an existing database correlation.
//
// Arguments:
//   name    : The name of the correlation.
//   dbs     : The databases in the correlation.
//   method  : The correlation method.
//   nStates : The number of states in the correlation (or -1 if you don't care).
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 30 21:19:28 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::AlterDatabaseCorrelation(const std::string &name,
    const stringVector &dbs, int method, int nStates)
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::AlterDatabaseCorrelationRPC);
    state->GetViewerRPC()->SetDatabase(name);
    state->GetViewerRPC()->SetProgramOptions(dbs);
    state->GetViewerRPC()->SetIntArg1(method);
    state->GetViewerRPC()->SetIntArg2(nStates);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::DeleteDatabaseCorrelation
//
// Purpose: 
//   Deletes the named database correlation.
//
// Arguments:
//   name : The name of the correlation to delete.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 30 21:24:19 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::DeleteDatabaseCorrelation(const std::string &name)
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::DeleteDatabaseCorrelationRPC);
    state->GetViewerRPC()->SetDatabase(name);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::OpenComputeEngine
//
//  Purpose:
//    Open a compute engine.
//
//  Arguments:
//    hostName    The name of the host to open the engine on.
//    engineName  The name of the engine to execute.
//    options     The options to start the engine with.
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2000
//
//  Modifications:
//    Brad Whitlock, Mon Apr 30 12:21:56 PDT 2001
//    I modified the interface.
//
// ****************************************************************************
void
ViewerMethods::OpenComputeEngine(const std::string &hostName, const stringVector &args)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::OpenComputeEngineRPC);
    state->GetViewerRPC()->SetProgramHost(hostName);
    state->GetViewerRPC()->SetProgramOptions(args);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::CloseComputeEngine
//
// Purpose: 
//   Tells the viewer to close a compute engine on the specified host.
//
// Arguments:
//   hostName : The host's engine that will be terminated.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 30 12:23:41 PDT 2001
//
// Modifications:
//    Jeremy Meredith, Tue Mar 30 11:07:45 PST 2004
//    Added a simulation name to the interfaces, as some engines now
//    can be simulations, meaning there might be more than one engine
//    per host.
//   
// ****************************************************************************
void
ViewerMethods::CloseComputeEngine(const std::string &hostName,
                                const std::string &simName)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::CloseComputeEngineRPC);
    state->GetViewerRPC()->SetProgramHost(hostName);
    state->GetViewerRPC()->SetProgramSim(simName);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::InterruptComputeEngine
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
ViewerMethods::InterruptComputeEngine(const std::string &hostName,
                                    const std::string &simName)
{
#ifdef FROM_VIEWER_PROXY
    xfer->SendInterruption();
#endif
}

// ****************************************************************************
//  Method: ViewerMethods::OpenMDServer
//
//  Purpose:
//    Opens an mdserver on the specified host using the specified arguments.
//
//  Arguments:
//    hostName : The host on which to launch the mdserver.
//    argv     : The arguments to use when launching the mdserver.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Jan 13 08:44:24 PDT 2003
//
// ****************************************************************************

void
ViewerMethods::OpenMDServer(const std::string &hostName, const stringVector &args)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::OpenMDServerRPC);
    state->GetViewerRPC()->SetProgramHost(hostName);
    state->GetViewerRPC()->SetProgramOptions(args);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::AnimationSetNFrames
//
//  Purpose:
//    Set the number of frames for the active animation.
//
//  Arguments:
//    frame     The number of frames.
//
//  Programmer: Eric Brugger
//  Creation:   August 31, 2000
//
// ****************************************************************************
void
ViewerMethods::AnimationSetNFrames(int nFrames)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::AnimationSetNFramesRPC);
    state->GetViewerRPC()->SetNFrames(nFrames);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::AnimationPlay
//
//  Purpose:
//    Play the active animation.
//
//  Programmer: Eric Brugger
//  Creation:   August 15, 2000
//
// ****************************************************************************
void
ViewerMethods::AnimationPlay()
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::AnimationPlayRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::AnimationReversePlay
//
//  Purpose:
//    Play the active animation in reverse.
//
//  Programmer: Eric Brugger
//  Creation:   August 15, 2000
//
// ****************************************************************************
void
ViewerMethods::AnimationReversePlay()
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::AnimationReversePlayRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::AnimationStop
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
ViewerMethods::AnimationStop()
{
#ifdef FROM_VIEWER_PROXY
    //
    // Send a special opcode to stop the animation.
    //
    xfer->SendSpecialOpcode(animationStopOpcode);
#endif

    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::AnimationStopRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::TimeSliderNextState
//
//  Purpose:
//    Advance the active time slider to the next state.
//
//  Programmer: Brad Whitlock
//  Creation:   Sun Jan 25 01:54:32 PDT 2004
//
// ****************************************************************************

void
ViewerMethods::TimeSliderNextState()
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::TimeSliderNextStateRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::TimeSliderPreviousState
//
//  Purpose:
//    Advance the active time slider to the previous state.
//
//  Programmer: Brad Whitlock
//  Creation:   Sun Jan 25 01:55:13 PDT 2004
//
// ****************************************************************************
void
ViewerMethods::TimeSliderPreviousState()
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::TimeSliderPreviousStateRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::SetTimeSliderState
//
//  Purpose:
//    Set the state for the active time slider.
//
//  Arguments:
//    state     The state number.
//
//  Programmer: Brad Whitlock
//  Creation:   Sun Jan 25 01:56:13 PDT 2004
//
// ****************************************************************************

void
ViewerMethods::SetTimeSliderState(int ts)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetTimeSliderStateRPC);
    state->GetViewerRPC()->SetStateNumber(ts);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::SetActiveTimeSlider
//
// Purpose: 
//   Tells the viewer to set the active time slider.
//
// Arguments:
//   ts : The index of the active time slider that we want to use.
//
// Programmer: Brad Whitlock
// Creation:   Sun Jan 25 01:57:56 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::SetActiveTimeSlider(const std::string &ts)
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetActiveTimeSliderRPC);
    state->GetViewerRPC()->SetDatabase(ts);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::SaveWindow
//
//  Purpose:
//    Save the current window.
//
//  Programmer: Hank Childs
//  Creation:   February 11, 2001
//
// ****************************************************************************
void
ViewerMethods::SaveWindow()
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SaveWindowRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::PrintWindow
//
// Purpose:
//   Print the current window.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 20 13:59:38 PST 2002
//
// Modifications:
//
// ****************************************************************************
void
ViewerMethods::PrintWindow()
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::PrintWindowRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::DisableRedraw
//
// Purpose: 
//   Disables updates for the VisWindow.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 19 14:41:38 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::DisableRedraw()
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::DisableRedrawRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::RedrawWindow
//
// Purpose: 
//   Redraws the VisWindow.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 19 14:42:36 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::RedrawWindow()
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::RedrawRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::ResizeWindow
//
// Purpose: 
//   Resize a window.
//
// Arguments:
//   win  : The window id.
//   w    : The new width.
//   h    : The new height.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 17 16:45:08 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::ResizeWindow(int win, int w, int h)
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ResizeWindowRPC);
    state->GetViewerRPC()->SetWindowId(win);
    state->GetViewerRPC()->SetIntArg1(w);
    state->GetViewerRPC()->SetIntArg2(h);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::MoveWindow
//
// Purpose: 
//   Move a window.
//
// Arguments:
//   win  : The window id.
//   x    : The new x.
//   y    : The new y.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 17 16:45:08 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::MoveWindow(int win, int x, int y)
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::MoveWindowRPC);
    state->GetViewerRPC()->SetWindowId(win);
    state->GetViewerRPC()->SetIntArg1(x);
    state->GetViewerRPC()->SetIntArg2(y);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::ResizeAndMoveWindow
//
// Purpose: 
//   Resize and move a window.
//
// Arguments:
//   win  : The window id.
//   x    : The new x.
//   y    : The new y.
//   w    : The new width.
//   h    : The new height.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 17 16:45:08 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::MoveAndResizeWindow(int win, int x, int y, int w, int h)
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::MoveAndResizeWindowRPC);
    state->GetViewerRPC()->SetWindowId(win);
    state->GetViewerRPC()->SetIntArg1(x);
    state->GetViewerRPC()->SetIntArg2(y);
    state->GetViewerRPC()->SetIntArg3(w);
    state->GetViewerRPC()->SetWindowLayout(h);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::HideToolbars
//
// Purpose: 
//   Hides the toolbars for the active vis window or for all vis windows.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 29 11:22:09 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::HideToolbars(bool forAllWindows)
{
    //
    // Set the rpc type and arguments.
    //
    if(forAllWindows)
        state->GetViewerRPC()->SetRPCType(ViewerRPC::HideToolbarsForAllWindowsRPC);
    else
        state->GetViewerRPC()->SetRPCType(ViewerRPC::HideToolbarsRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::ShowToolbars
//
// Purpose: 
//   Shows the toolbars for the active vis window or for all vis windows.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 29 11:22:09 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::ShowToolbars(bool forAllWindows)
{
    //
    // Set the rpc type and arguments.
    //
    if(forAllWindows)
        state->GetViewerRPC()->SetRPCType(ViewerRPC::ShowToolbarsForAllWindowsRPC);
    else
        state->GetViewerRPC()->SetRPCType(ViewerRPC::ShowToolbarsRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::AddPlot
//
//  Purpose:
//    Add a plot to the plot list.
//
//  Arguments:
//    type      The type of plot to add.
//    var       The variable to use for the plot.
//
//  Programmer: Eric Brugger
//  Creation:   August 4, 2000
//
//  Modifications:
//    Eric Brugger, Thu Mar  8 13:00:36 PST 2001
//    I changed the type of type to an integer.
//
// ****************************************************************************
void
ViewerMethods::AddPlot(int type, const std::string &var)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::AddPlotRPC);
    state->GetViewerRPC()->SetPlotType(type);
    state->GetViewerRPC()->SetVariable(var);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::CopyActivePlots
//
//  Purpose:
//    Copy current plot and add it to the plot list.
//
//  Arguments:
//
//  Programmer: Ellen Tarwater
//  Creation:   Sept 7, 2007
//
//  Modifications:
//
// ****************************************************************************
void
ViewerMethods::CopyActivePlots()
{
    int type;                  // type of plot to add
   
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::CopyActivePlotsRPC);
    type = state->GetViewerRPC()->GetPlotType();
    state->GetViewerRPC()->SetPlotType(type);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::SetPlotFrameRange
//
//  Purpose:
//    Set the frame range for the specified plot.
//
//  Arguments:
//    plotId    The id of the plot.
//    frame0    The start frame of the plot.
//    frame1    The end frame of the plot.
//
//  Programmer: Eric Brugger
//  Creation:   November 18, 2002
//
// ****************************************************************************
void
ViewerMethods::SetPlotFrameRange(int plotId, int frame0, int frame1)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetPlotFrameRangeRPC);
    state->GetViewerRPC()->SetIntArg1(plotId);
    state->GetViewerRPC()->SetIntArg2(frame0);
    state->GetViewerRPC()->SetIntArg3(frame1);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::DeletePlotKeyframe
//
//  Purpose:
//    Delete the keyframe for the specified plot.
//
//  Arguments:
//    plotId    The id of the plot.
//    frame     The keyframe to delete.
//
//  Programmer: Eric Brugger
//  Creation:   November 18, 2002
//
// ****************************************************************************
void
ViewerMethods::DeletePlotKeyframe(int plotId, int frame)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::DeletePlotKeyframeRPC);
    state->GetViewerRPC()->SetIntArg1(plotId);
    state->GetViewerRPC()->SetIntArg2(frame);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::MovePlotKeyframe
//
//  Purpose:
//    Move the the position of a keyframe for the specified plot.
//
//  Arguments:
//    plotId    The id of the plot.
//    oldFrame  The old location of the keyframe.
//    newFrame  The new location of the keyframe.
//
//  Programmer: Eric Brugger
//  Creation:   January 28, 2003
//
// ****************************************************************************
void
ViewerMethods::MovePlotKeyframe(int plotId, int oldFrame, int newFrame)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::MovePlotKeyframeRPC);
    state->GetViewerRPC()->SetIntArg1(plotId);
    state->GetViewerRPC()->SetIntArg2(oldFrame);
    state->GetViewerRPC()->SetIntArg3(newFrame);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::SetPlotDatabaseState
//
//  Purpose:
//    Set the database state associated with the specified frame and plot.
//
//  Arguments:
//    plotId    The id of the plot.
//    frame     The frame number.
//    state     The state to associate with the frame.
//
//  Programmer: Eric Brugger
//  Creation:   December 30, 2002
//
// ****************************************************************************
void
ViewerMethods::SetPlotDatabaseState(int plotId, int frame, int ts)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetPlotDatabaseStateRPC);
    state->GetViewerRPC()->SetIntArg1(plotId);
    state->GetViewerRPC()->SetIntArg2(frame);
    state->GetViewerRPC()->SetIntArg3(ts);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::DeletePlotDatabaseKeyframe
//
//  Purpose:
//    Delete the specified database keyframe for the specified plot.
//
//  Arguments:
//    plotId    The id of the plot.
//    frame     The keyframe to delete.
//
//  Programmer: Eric Brugger
//  Creation:   December 30, 2002
//
// ****************************************************************************
void
ViewerMethods::DeletePlotDatabaseKeyframe(int plotId, int frame)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::DeletePlotDatabaseKeyframeRPC);
    state->GetViewerRPC()->SetIntArg1(plotId);
    state->GetViewerRPC()->SetIntArg2(frame);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::MovePlotDatabaseKeyframe
//
//  Purpose:
//    Move the the position of a database keyframe for the specified plot.
//
//  Arguments:
//    plotId    The id of the plot.
//    oldFrame  The old location of the keyframe.
//    newFrame  The new location of the keyframe.
//
//  Programmer: Eric Brugger
//  Creation:   January 28, 2003
//
// ****************************************************************************
void
ViewerMethods::MovePlotDatabaseKeyframe(int plotId, int oldFrame, int newFrame)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::MovePlotDatabaseKeyframeRPC);
    state->GetViewerRPC()->SetIntArg1(plotId);
    state->GetViewerRPC()->SetIntArg2(oldFrame);
    state->GetViewerRPC()->SetIntArg3(newFrame);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::DeleteActivePlots
//
//  Purpose:
//    Delete the active plots from the plot list.
//
//  Programmer: Eric Brugger
//  Creation:   August 15, 2000
//
// ****************************************************************************
void
ViewerMethods::DeleteActivePlots()
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::DeleteActivePlotsRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::HideActivePlots
//
//  Purpose:
//    Hide the active plots from the plot list.
//
//  Programmer: Eric Brugger
//  Creation:   August 15, 2000
//
// ****************************************************************************
void
ViewerMethods::HideActivePlots()
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::HideActivePlotsRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::SetPlotFollowsTime
//
//  Purpose:
//    Disconnect the active plot from the time slider.
//
//  Programmer: Ellen Tarwater
//  Creation:   December 6, 2007
//
// ****************************************************************************
void
ViewerMethods::SetPlotFollowsTime()
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetPlotFollowsTimeRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}
// ****************************************************************************
//  Method: ViewerMethods::DrawPlots
//
//  Purpose:
//    Draw any undrawn plot in the plot list.
//
//  Programmer: Eric Brugger
//  Creation:   August 15, 2000
//
// Modifications:
//  Ellen Tarwater October 12, 2007
//  added drawAllPlots flag
//
// ****************************************************************************
void
ViewerMethods::DrawPlots(bool drawAllPlots)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::DrawPlotsRPC);
    state->GetViewerRPC()->SetBoolFlag(drawAllPlots);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::SetActivePlots
//
//  Purpose:
//    Set the active plots.
//
//  Arguments:
//    activePlotIds     : The indices of the new active plots.
//
//  Programmer: Eric Brugger
//  Creation:   August 15, 2000
//
//  Modifications:
//    Brad Whitlock, Fri Apr 11 11:34:15 PDT 2003
//    I added code to set a boolean flag.
//
// ****************************************************************************

void
ViewerMethods::SetActivePlots(const intVector &activePlotIds)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetActivePlotsRPC);
    state->GetViewerRPC()->SetActivePlotIds(activePlotIds);
    state->GetViewerRPC()->SetBoolFlag(false);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::SetActivePlots
//
//  Purpose:
//    Set the active plots.
//
//  Arguments:
//    activePlotIds     : The indices of the new active plots.
//    activeOperatorIds : The indices of the new active operators.
//    expandedPlots     : Whether the plots are expanded.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Apr 11 14:55:23 PST 2003
//
//  Modifications:
//
// ****************************************************************************

void
ViewerMethods::SetActivePlots(const intVector &activePlotIds,
    const intVector &activeOperatorIds, const intVector &expandedPlots)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetActivePlotsRPC);
    state->GetViewerRPC()->SetActivePlotIds(activePlotIds);
    state->GetViewerRPC()->SetActiveOperatorIds(activeOperatorIds);
    state->GetViewerRPC()->SetExpandedPlotIds(expandedPlots);
    state->GetViewerRPC()->SetBoolFlag(true);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::ChangeActivePlotsVar
//
//  Purpose:
//    Change the plot variable for the active plots.
//
//  Arguments:
//    var       The new variable name.
//
//  Programmer: Eric Brugger
//  Creation:   August 15, 2000
//
// ****************************************************************************
void
ViewerMethods::ChangeActivePlotsVar(const std::string &var)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ChangeActivePlotsVarRPC);
    state->GetViewerRPC()->SetVariable(var);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::AddOperator
//
//  Purpose:
//    Add the specified operator to the active plots.
//
//  Arguments:
//    oper      The operator to add.
//    fromDefault  Flag indicating whether the operator should be initialized
//                 from its DefaultAtts or from its Client Atts.
//
//  Programmer: Eric Brugger
//  Creation:   August 15, 2000
//
//  Modifications:
//    Jeremy Meredith, Thu Jul 26 03:13:36 PDT 2001
//    Removed all references to OperType.
//
//    Kathleen Bonnell,  
//    Added 'fromDefault' arg. Use it to set the bool flag in the rpc.
//
// ****************************************************************************
void
ViewerMethods::AddOperator(int oper, const bool fromDefault)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::AddOperatorRPC);
    state->GetViewerRPC()->SetOperatorType(oper);
    state->GetViewerRPC()->SetBoolFlag(fromDefault);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::AddInitializedOperator
//
// Purpose: 
//   Adds an operator, getting the initial attributes from the client atts.
//
// Arguments:
//  oper : The index of the operator to add.
//
// Programmer: Brad Whitlock
// Creation:   Tue May 8 16:49:46 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::AddInitializedOperator(int oper)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::AddInitializedOperatorRPC);
    state->GetViewerRPC()->SetOperatorType(oper);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::PromoteOperator
//
// Purpose: 
//   Tells the viewer to promote an operator for the specified plot. This means
//   that the operator is moved to later in the pipeline.
//
// Arguments:
//   operatorId : The index of the operator to promote.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 10 09:31:37 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::PromoteOperator(int operatorId)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::PromoteOperatorRPC);
    state->GetViewerRPC()->SetOperatorType(operatorId);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::DemoteOperator
//
// Purpose: 
//   Tells the viewer to demote an operator for the specified plot. This means
//   that the operator is moved closer to the start of the pipeline.
//
// Arguments:
//   operatorId : The index of the operator to demote.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 10 09:31:37 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::DemoteOperator(int operatorId)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::DemoteOperatorRPC);
    state->GetViewerRPC()->SetOperatorType(operatorId);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::RemoveOperator
//
// Purpose: 
//   Tells the viewer to remove an operator from the specified plot.
//
// Arguments:
//   operatorId : The index of the operator to remove.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 10 09:31:37 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::RemoveOperator(int operatorId)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::RemoveOperatorRPC);
    state->GetViewerRPC()->SetOperatorType(operatorId);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::RemoveLastOperator
//
//  Purpose:
//    Remove the last plot operator from the active plots.
//
//  Programmer: Eric Brugger
//  Creation:   August 15, 2000
//
// ****************************************************************************
void
ViewerMethods::RemoveLastOperator()
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::RemoveLastOperatorRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::RemoveAllOperators
//
//  Purpose:
//    Remove all the plot operators from the active plots.
//
//  Programmer: Eric Brugger
//  Creation:   August 15, 2000
//
// ****************************************************************************
void
ViewerMethods::RemoveAllOperators()
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::RemoveAllOperatorsRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::SetDefaultPlotOptions
//
//  Purpose:
//    Set the default options for the specified plot type.
//
//  Arguments:
//    type      The plot type to set the default options for.
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2000
//
//  Modifications:
//    Eric Brugger, Thu Mar  8 13:00:36 PST 2001
//    I changed the type of type to an integer.
//
// ****************************************************************************
void
ViewerMethods::SetDefaultPlotOptions(int type)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetDefaultPlotOptionsRPC);
    state->GetViewerRPC()->SetPlotType(type);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::SetPlotOptions
//
//  Purpose:
//    Set the plot options for the active plots of the specified plot type.
//
//  Arguments:
//    type      The plot type to set the options for.
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2000
//
//  Modifications:
//    Eric Brugger, Thu Mar  8 13:00:36 PST 2001
//    I changed the type of type to an integer.
//
// ****************************************************************************
void
ViewerMethods::SetPlotOptions(int type)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetPlotOptionsRPC);
    state->GetViewerRPC()->SetPlotType(type);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::ResetPlotOptions
//
//  Purpose:
//    Reset the plot options for the active plots of the specified plot type
//    to the default plot attributes.
//
//  Arguments:
//    type      The plot type for which to reset the attributes.
//
//  Programmer: Brad Whitlock, 
//  Creation:   Tue Aug 14 17:22:13 PST 2001
//
//  Modifications:
//
// ****************************************************************************
void
ViewerMethods::ResetPlotOptions(int type)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ResetPlotOptionsRPC);
    state->GetViewerRPC()->SetPlotType(type);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::SetDefaultOperatorOptions
//
//  Purpose:
//    Set the default options for the specified operator type.
//
//  Arguments:
//    oper      The operator type to set the default options for.
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2000
//
// ****************************************************************************
void
ViewerMethods::SetDefaultOperatorOptions(int oper)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetDefaultOperatorOptionsRPC);
    state->GetViewerRPC()->SetOperatorType(oper);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::SetOperatorOptions
//
//  Purpose:
//    Set the operator options for the operators of the active plots of
//    the specified operator type.
//
//  Arguments:
//    oper      The operator type to set the options for.
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2000
//
// ****************************************************************************
void
ViewerMethods::SetOperatorOptions(int oper)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetOperatorOptionsRPC);
    state->GetViewerRPC()->SetOperatorType(oper);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::ResetOperatorOptions
//
//  Purpose:
//    Reset the operator options for the operators of the active plots of
//    the specified operator type to the default operator attributes.
//
//  Arguments:
//    oper      The operator type for which to set the options.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Aug 14 17:20:53 PST 2001
//
// ****************************************************************************
void
ViewerMethods::ResetOperatorOptions(int oper)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ResetOperatorOptionsRPC);
    state->GetViewerRPC()->SetOperatorType(oper);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::SetAnimationAttributes
//
//  Purpose:
//    Applies the animation attributes.
//
//  Programmer: Eric Brugger
//  Creation:   November 19, 2001 
//
// ****************************************************************************

void
ViewerMethods::SetAnimationAttributes()
{
    //
    // Set the rpc type.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetAnimationAttributesRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::SetAnnotationAttributes
//
//  Purpose:
//    Applies the annotation attributes.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 18, 2001 
//
//  Modifications:
//    Brad Whitlock, Thu Aug 30 09:53:42 PDT 2001
//    Renamed the method to SetAnnotationAttributes.
//
// ****************************************************************************

void
ViewerMethods::SetAnnotationAttributes()
{
    //
    // Set the rpc type.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetAnnotationAttributesRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::SetDefaultAnnotationAttributes
//
// Purpose: 
//   Sets the default annotation attributes.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 30 09:54:29 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::SetDefaultAnnotationAttributes()
{
    //
    // Set the rpc type.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetDefaultAnnotationAttributesRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::ResetAnnotationAttributes
//
// Purpose: 
//   Reset the annotation attributes to the default values.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 30 09:55:07 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::ResetAnnotationAttributes()
{
    //
    // Set the rpc type.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ResetAnnotationAttributesRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::AddAnnotationObject
//
// Purpose: 
//   Tells the viewer to add a new annotation object of the specifed type.
//
// Arguments:
//   annotType : The type of annotation object to add. This argument corresponds
//               to the AnnotationType enum in AnnotationObject.h
//   annotName : The name of the new annotation object instance so we can
//               refer to it by name if we want.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 29 10:53:01 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::AddAnnotationObject(int annotType, const std::string &annotName)
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::AddAnnotationObjectRPC);
    state->GetViewerRPC()->SetIntArg1(annotType);
    state->GetViewerRPC()->SetStringArg1(annotName);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::HideActiveAnnotationObjects
//
// Purpose: 
//   Hides the active annotation objects.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 29 10:54:59 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::HideActiveAnnotationObjects()
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::HideActiveAnnotationObjectsRPC);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::DeleteActiveAnnotationObjects
//
// Purpose: 
//   Deletes the active annotation objects.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 29 10:54:59 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::DeleteActiveAnnotationObjects()
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::DeleteActiveAnnotationObjectsRPC);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::RaiseActiveAnnotationObjects
//
// Purpose: 
//   Raises the active annotation objects.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 29 10:54:59 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::RaiseActiveAnnotationObjects()
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::RaiseActiveAnnotationObjectsRPC);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::LowersActiveAnnotationObjects
//
// Purpose: 
//   Lowers the active annotation objects.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 29 10:54:59 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::LowerActiveAnnotationObjects()
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::LowerActiveAnnotationObjectsRPC);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::SetAnnotationObjectOptions
//
// Purpose: 
//   Tells the viewer to update the annotations using the options in the
//   annotation options list.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 29 10:54:59 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::SetAnnotationObjectOptions()
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetAnnotationObjectOptionsRPC);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::SetDefaultAnnotationObjectList
//
// Purpose: 
//   Tells the viewer to set the default annotation object list using the
//   client annotation object list.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 7 14:12:25 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::SetDefaultAnnotationObjectList()
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetDefaultAnnotationObjectListRPC);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::ResetAnnotationObjectList
//
// Purpose: 
//   Tells the viewer to set the client annotation object list using the
//   default annotation object list.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 7 14:12:25 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::ResetAnnotationObjectList()
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ResetAnnotationObjectListRPC);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::SetMaterialAttributes
//
//  Purpose:
//    Applies the material attributes.
//
//  Programmer: Jeremy Meredith
//  Creation:   October 24, 2002
//
//  Modifications:
//
// ****************************************************************************

void
ViewerMethods::SetMaterialAttributes()
{
    //
    // Set the rpc type.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetMaterialAttributesRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::SetDefaultMaterialAttributes
//
// Purpose: 
//   Sets the default material attributes.
//
// Programmer: Jeremy Meredith
// Creation:   October 24, 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::SetDefaultMaterialAttributes()
{
    //
    // Set the rpc type.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetDefaultMaterialAttributesRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::ResetMaterialAttributes
//
// Purpose: 
//   Reset the material attributes to the default values.
//
// Programmer: Jeremy Meredith
// Creation:   October 24, 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::ResetMaterialAttributes()
{
    //
    // Set the rpc type.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ResetMaterialAttributesRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::SetMeshManagementAttributes
//
//  Purpose: Applies the mesh management attributes.
//
//  Programmer: Mark C. Miller
//  Creation:   November 6, 2005 
//
// ****************************************************************************

void
ViewerMethods::SetMeshManagementAttributes()
{
    //
    // Set the rpc type.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetMeshManagementAttributesRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::SetDefaultMeshManagementAttributes
//
// Purpose: Sets the default mesh management attributes.
//
// Programmer: Mark C. Miller 
// Creation:   November 6, 2005 
//
// ****************************************************************************

void
ViewerMethods::SetDefaultMeshManagementAttributes()
{
    //
    // Set the rpc type.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetDefaultMeshManagementAttributesRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::ResetMeshManagementAttributes
//
// Purpose: Reset the mesh management attributes to the default values.
//
// Programmer: Mark C. Miller 
// Creation:   November 6, 2005 
//
// ****************************************************************************

void
ViewerMethods::ResetMeshManagementAttributes()
{
    //
    // Set the rpc type.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ResetMeshManagementAttributesRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::SetKeyframeAttributes
//
//  Purpose:
//    Set the current keyframe attributes.
//
//  Programmer: Jeremy Meredith
//  Creation:   May  8, 2002
//
//  Modifications:
//
// ****************************************************************************

void
ViewerMethods::SetKeyframeAttributes()
{
    //
    // Set the rpc type.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetKeyframeAttributesRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::SetLightList
//
//  Purpose:
//    Applies the light list to the window.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Sep 14 14:00:09 PST 2001 
//
//  Modifications:
//
// ****************************************************************************

void
ViewerMethods::SetLightList()
{
    //
    // Set the rpc type.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetLightListRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::SetDefaultLightList
//
// Purpose: 
//   Sets the default light list.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 14 13:50:47 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::SetDefaultLightList()
{
    //
    // Set the rpc type.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetDefaultLightListRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::ResetLightList
//
// Purpose: 
//   Reset the light list to the default values.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 14 13:50:47 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::ResetLightList()
{
    //
    // Set the rpc type.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ResetLightListRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::ResetPickLetter
//
// Purpose: 
//   Reset the pick attributes to default values.
//
// Programmer: Kathleen Bonnell 
// Creation:   December 9, 2003 
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::ResetPickLetter()
{
    //
    // Set the rpc type.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ResetPickLetterRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}


// ****************************************************************************
// Method: ViewerMethods::ResetPickAttributes
//
// Purpose: 
//   Reset the pick attributes to default values.
//
// Programmer: Kathleen Bonnell 
// Creation:   November 26, 2003 
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::ResetPickAttributes()
{
    //
    // Set the rpc type.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ResetPickAttributesRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}


// ****************************************************************************
// Method: ViewerMethods::SetActiveContinuousColorTable
//
// Purpose: 
//   Sets the active continuous color table. This is the color table that
//   is used for all new plots that need a continuous color table.
//
// Arguments:
//   colorTableName : The name of the continuous colortable to use.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 13 16:55:21 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::SetActiveContinuousColorTable(const std::string &colorTableName)
{
    // If it's a valid color table name, make it active.
    if(state->GetColorTableAttributes()->GetColorTableIndex(colorTableName) != -1)
    {
        state->GetColorTableAttributes()->SetActiveContinuous(colorTableName);
        state->GetColorTableAttributes()->Notify();

        // Update the color table. This has the effect of making all plots
        // use the default color table update to use the new active color
        // table.
        UpdateColorTable(colorTableName);
    }
}

// ****************************************************************************
// Method: ViewerMethods::SetActiveDiscreteColorTable
//
// Purpose: 
//   Sets the active discrete color table. This is the color table that
//   is used for all new plots that need a discrete color table.
//
// Arguments:
//   colorTableName : The name of the discrete colortable to use.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 13 16:55:21 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::SetActiveDiscreteColorTable(const std::string &colorTableName)
{
    // If it's a valid color table name, make it active.
    if(state->GetColorTableAttributes()->GetColorTableIndex(colorTableName) != -1)
    {
        state->GetColorTableAttributes()->SetActiveDiscrete(colorTableName);
        state->GetColorTableAttributes()->Notify();
    }
}

// ****************************************************************************
// Method: ViewerMethods::DeleteColorTable
//
// Purpose: 
//   Deletes the specified color table from the list of color tables. This
//   can cause all plots that use the deleted color table to be updated after
//   the color table is deleted.
//
// Arguments:
//   colorTableName : The name of the color table to delete.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 13 17:03:07 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::DeleteColorTable(const std::string &colorTableName)
{
    // If it's a valid color table name, make it active.
    int index = state->GetColorTableAttributes()->GetColorTableIndex(colorTableName);
    if(index != -1)
    {
        // Remove the color table from the list and update.
        state->GetColorTableAttributes()->RemoveColorTable(index);
        state->GetColorTableAttributes()->Notify();

        // Update the color table. The specified color table will no
        // longer exist in the list of color tables so all plots that used
        // that color table will have their color tables changed to something
        // else.
        UpdateColorTable(colorTableName);
    }
}

// ****************************************************************************
// Method: ViewerMethods::UpdateColorTable
//
// Purpose: 
//   Forces the viewer to update all plots that use the specified color table.
//   If the color table does not exist, then all plots that used it are reset
//   so they use the current color table.
//
// Arguments:
//   colorTableName : The name of the color table that's being updated.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 13 17:20:23 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::UpdateColorTable(const std::string &colorTableName)
{
    //
    // Set the RPC type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::UpdateColorTableRPC);
    state->GetViewerRPC()->SetColorTableName(colorTableName);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::ExportColorTable
//
// Purpose: 
//   Forces the viewer to export the named color table to a small XML file.
//
// Arguments:
//   colorTableName : The name of the color table that's being exported.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 1 16:48:09 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::ExportColorTable(const std::string &colorTableName)
{
    //
    // Set the RPC type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ExportColorTableRPC);
    state->GetViewerRPC()->SetColorTableName(colorTableName);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::InvertBackgroundColor
//
// Purpose: 
//   Tells the viewer to swap its background color with its foreground color.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 6 16:51:22 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::InvertBackgroundColor()
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::InvertBackgroundRPC);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::WriteConfigFile
//
//  Purpose:
//    Writes a config file containing the default values for the viewer's
//    state objects.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Sep 28 11:52:35 PDT 2000
//
// ****************************************************************************

void
ViewerMethods::WriteConfigFile()
{
    //
    // Set the rpc type.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::WriteConfigFileRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::ExportEntireState
//
// Purpose: 
//   Tells the viewer to dump its entire state to an XML file.
//
// Arguments:
//   filename : The name of the file used to write the state.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 9 11:58:00 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::ExportEntireState(const std::string &filename)
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ExportEntireStateRPC);
    state->GetViewerRPC()->SetVariable(filename);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::ImportEntireState
//
// Purpose: 
//   Tells the viewer to set its entire state using the values stored in
//   the named file.
//
// Arguments:
//   filename   : The name of the file to read for the state.
//   inVisItDir : Whether the session file is in the .visit directory.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 9 11:58:23 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::ImportEntireState(const std::string &filename, bool inVisItDir)
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ImportEntireStateRPC);
    state->GetViewerRPC()->SetVariable(filename);
    state->GetViewerRPC()->SetBoolFlag(inVisItDir);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::ImportEntireStateWithDifferentSources
//
// Purpose: 
//   Tells the viewer to set its entire state using the values stored in
//   the named file. It uses the sources that are provided instead of the
//   sources in the file.
//
// Arguments:
//   filename   : The name of the file to read for the state.
//   inVisItDir : Whether the session file is in the .visit directory.
//   sources    : The list of sources to use.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 10 09:32:48 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::ImportEntireStateWithDifferentSources(const std::string &filename, 
    bool inVisItDir, const stringVector &sources)
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ImportEntireStateWithDifferentSourcesRPC);
    state->GetViewerRPC()->SetVariable(filename);
    state->GetViewerRPC()->SetBoolFlag(inVisItDir);
    state->GetViewerRPC()->SetProgramOptions(sources);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::SetCenterOfRotation
//
// Purpose: 
//   Sets the center of rotation.
//
// Arguments:
//   x,y,z : The new center of rotation.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 29 09:21:31 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::SetCenterOfRotation(double x, double y, double z)
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetCenterOfRotationRPC);
    double pts[] = {x, y, z};
    state->GetViewerRPC()->SetQueryPoint1(pts);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::ChooseCenterOfRotation
//
// Purpose: 
//   Tells the viewer to use the point at the center of the screen as the
//   new center of rotation.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 29 09:22:02 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::ChooseCenterOfRotation()
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ChooseCenterOfRotationRPC);
    state->GetViewerRPC()->SetBoolFlag(false);
    state->GetViewerRPC()->Notify();
}

void
ViewerMethods::ChooseCenterOfRotation(double sx, double sy)
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ChooseCenterOfRotationRPC);
    state->GetViewerRPC()->SetBoolFlag(true);
    double pt[] = {sx, sy, 0.};
    state->GetViewerRPC()->SetQueryPoint1(pt);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::SetViewCurve
//
// Purpose: 
//   Tells the viewer to use the new curve view attributes.
//
// Programmer: Eric Brugger
// Creation:   August 20, 2003
//
// ****************************************************************************

void
ViewerMethods::SetViewCurve()
{
    //
    // Set the rpc type.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetViewCurveRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::SetView2D
//
// Purpose: 
//   Tells the viewer to use the new 2d view attributes.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 26 16:48:38 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::SetView2D()
{
    //
    // Set the rpc type.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetView2DRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::SetView3D
//
// Purpose: 
//   Tells the viewer to use the new 3d view attributes.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 26 16:48:38 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::SetView3D()
{
    //
    // Set the rpc type.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetView3DRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::ClearViewKeyframes
//
//  Purpose: 
//    Tells the viewer to clear the view keyframes.
//
//  Programmer: Eric Brugger
//  Creation:   January 3, 2003
//
//  Modifications:
//   
// ****************************************************************************

void
ViewerMethods::ClearViewKeyframes()
{
    //
    // Set the rpc type.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ClearViewKeyframesRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::DeleteViewKeyframe
//
//  Purpose: 
//    Tells the viewer to delete the specified view keyframe.
//
//  Arguments:
//    frame     The keyframe to delete.
//
//  Programmer: Eric Brugger
//  Creation:   January 3, 2003
//
//  Modifications:
//   
// ****************************************************************************

void
ViewerMethods::DeleteViewKeyframe(int frame)
{
    //
    // Set the rpc type.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::DeleteViewKeyframeRPC);
    state->GetViewerRPC()->SetFrame(frame);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::MoveViewKeyframe
//
//  Purpose:
//    Move the position of a view keyframe.
//
//  Arguments:
//    oldFrame  The old location of the keyframe.
//    newFrame  The new location of the keyframe.
//
//  Programmer: Eric Brugger
//  Creation:   January 28, 2003
//
// ****************************************************************************
void
ViewerMethods::MoveViewKeyframe(int oldFrame, int newFrame)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::MoveViewKeyframeRPC);
    state->GetViewerRPC()->SetIntArg1(oldFrame);
    state->GetViewerRPC()->SetIntArg2(newFrame);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::SetViewKeyframe
//
//  Purpose: 
//    Tells the viewer to set a view keyframe.
//
//  Programmer: Eric Brugger
//  Creation:   January 3, 2003
//
//  Modifications:
//   
// ****************************************************************************

void
ViewerMethods::SetViewKeyframe()
{
    //
    // Set the rpc type.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetViewKeyframeRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::ResetView
//
// Purpose: 
//   Tells the viewer to reset the view.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 6 16:40:01 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::ResetView()
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ResetViewRPC);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::RecenterView
//
// Purpose: 
//   Tells the viewer to recenter the view.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 6 16:40:01 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::RecenterView()
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::RecenterViewRPC);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::SetViewExtents
//
// Purpose: 
//   Tells the viewer to set the view extents.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 16 12:42:03 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::SetViewExtentsType(int t)
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetViewExtentsTypeRPC);
    state->GetViewerRPC()->SetWindowLayout(t);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::ToggleMaintainViewMode
//
// Purpose: 
//   Tells the viewer to toggle the maintain view mode.
//
// Programmer: Eric Brugger
// Creation:   April 18, 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::ToggleMaintainViewMode()
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ToggleMaintainViewModeRPC);
    state->GetViewerRPC()->Notify();
}


// ****************************************************************************
// Method: ViewerMethods::ToggleMaintainDataMode
//
// Purpose: 
//   Tells the viewer to toggle the maintain data mode.
//
// Programmer: Eric Brugger
// Creation:   March 29, 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::ToggleMaintainDataMode()
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ToggleMaintainDataModeRPC);
    state->GetViewerRPC()->Notify();
}


// ****************************************************************************
// Method: ViewerMethods::ToggleFullFrameMode
//
// Purpose: 
//   Tells the viewer to toggle the full frame mode.
//
// Programmer: Kathleen Bonnell 
// Creation:   May 13, 2003 
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::ToggleFullFrameMode()
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ToggleFullFrameRPC);
    state->GetViewerRPC()->Notify();
}


// ****************************************************************************
// Method: ViewerMethods::UndoView
//
// Purpose: 
//   Tells the viewer to undo the last view change.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 6 16:40:01 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::UndoView()
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::UndoViewRPC);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::RedoView
//
// Purpose: 
//   Tells the viewer to redo the last view change.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 7 16:36:56 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::RedoView()
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::RedoViewRPC);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::ToggleLockViewMode
//
// Purpose: 
//   Tells the viewer to lock the view.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 6 16:40:01 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::ToggleLockViewMode()
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ToggleLockViewModeRPC);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::ToggleLockTime
//
// Purpose: 
//   Toggles the viewer's lock time flag.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 11 11:47:51 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::ToggleLockTime()
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ToggleLockTimeRPC);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::ToggleLockTools
//
// Purpose: 
//   Toggles the viewer's lock tools flag.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 11 11:48:22 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::ToggleLockTools()
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ToggleLockToolsRPC);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::ToggleSpinMode
//
// Purpose: 
//   Tells the viewer to toggle the spin mode.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 6 16:40:01 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::ToggleSpinMode()
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ToggleSpinModeRPC);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::ToggleCameraViewMode
//
//  Purpose: 
//    Tells the viewer to toggle the camera view mode.
//
//  Programmer: Eric Brugger
//  Creation:   January 3, 2003
//
//  Modifications:
//   
// ****************************************************************************

void
ViewerMethods::ToggleCameraViewMode()
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ToggleCameraViewModeRPC);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::SetWindowMode
//
// Purpose: 
//   Sets the window mode.
//
// Arguments:
//   mode : The new window mode [0,2]
//
// Programmer: Brad Whitlock
// Creation:   Mon May 6 17:01:43 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::SetWindowMode(int mode)
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetWindowModeRPC);
    state->GetViewerRPC()->SetWindowMode(mode);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::ToggleBoundingBoxMode
//
// Purpose: 
//   Tells the viewer whether or not bbox mode should be used.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 6 17:02:12 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::ToggleBoundingBoxMode()
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ToggleBoundingBoxModeRPC);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::EnableTool
//
// Purpose: 
//   Tells the viewer to enable/disable a tool.
//
// Arguments:
//   tool    : The index of the tool.
//   enabled : Whether or not the tool is enabled.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 6 17:02:47 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::EnableTool(int tool, bool enabled)
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::EnableToolRPC);
    state->GetViewerRPC()->SetToolId(tool);
    state->GetViewerRPC()->SetBoolFlag(enabled);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::CopyViewToWindow
//
// Purpose: 
//   Copies the view from one window to another window.
//
// Arguments:
//   from : The source window.
//   to   : The destination window.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 27 16:17:06 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::CopyViewToWindow(int from, int to)
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::CopyViewToWindowRPC);
    // store from window in windowLayout
    state->GetViewerRPC()->SetWindowLayout(from);
    state->GetViewerRPC()->SetWindowId(to);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::CopyLightingToWindow
//
// Purpose: 
//   Copies the view from one window to another window.
//
// Arguments:
//   from : The source window.
//   to   : The destination window.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 27 16:17:06 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::CopyLightingToWindow(int from, int to)
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::CopyLightingToWindowRPC);
    // store from window in windowLayout
    state->GetViewerRPC()->SetWindowLayout(from);
    state->GetViewerRPC()->SetWindowId(to);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::CopyAnnotationsToWindow
//
// Purpose: 
//   Copies the annotations from one window to another window.
//
// Arguments:
//   from : The source window.
//   to   : The destination window.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 27 16:17:06 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::CopyAnnotationsToWindow(int from, int to)
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::CopyAnnotationsToWindowRPC);
    // store from window in windowLayout
    state->GetViewerRPC()->SetWindowLayout(from);
    state->GetViewerRPC()->SetWindowId(to);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::CopyPlotsToWindow
//
// Purpose: 
//   Copies the plots from one window to another window.
//
// Arguments:
//   from : The source window.
//   to   : The destination window.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 15 16:26:43 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::CopyPlotsToWindow(int from, int to)
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::CopyPlotsToWindowRPC);
    // store from window in windowLayout
    state->GetViewerRPC()->SetWindowLayout(from);
    state->GetViewerRPC()->SetWindowId(to);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::SetPlotSILRestriction
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
ViewerMethods::SetPlotSILRestriction()
{
    // Now that the new SIL restriction attributes have been sent to the
    // viewer, send the RPC that tells the viewer to apply them.
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetPlotSILRestrictionRPC);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::ProcessExpressions
//
// Purpose: 
//   Issues a ProcessExpressions RPC to the viewer. This RPC tells the viewer
//   to update its plots with new expressions.
//
// Programmer: Sean Ahern
// Creation:   Wed Sep 26 16:32:00 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::ProcessExpressions()
{
    // Set the rpc type.
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ProcessExpressionsRPC);

    // Issue the RPC.
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::SetAppearanceAttributes
//
// Purpose: 
//   Issues a SetAppearance RPC to the viewer. This RPC tells the viewer to
//   update its gui colors/font/style...
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 4 22:38:51 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::SetAppearanceAttributes()
{
    //
    // Set the rpc type.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetAppearanceRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::ClearPickPoints
//
// Purpose: 
//   Tells the viewer to clear its pick points.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 6 16:52:47 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::ClearPickPoints()
{
    //
    // Set the rpc type.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ClearPickPointsRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::ClearReferenceLines
//
// Purpose: 
//   Tells the viewer to clear its reference lines.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 6 16:52:47 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::ClearReferenceLines()
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ClearRefLinesRPC);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::SetRenderingAttributes
//
// Purpose: 
//   Tells the viewer to use the new rendering attributes.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 19 13:19:33 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::SetRenderingAttributes()
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetRenderingAttributesRPC);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::SetWindowArea
//
// Purpose: 
//   Sets the window area. This is the area used for the vis windows.
//
// Arguments:
//   x : The x location of the window area.
//   y : The y location of the window area.
//   w : The width of the window area.
//   h : The height of the window area.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 29 16:17:50 PST 2002
//
// Modifications:
//   Brad Whitlock, Fri Dec 20 14:31:04 PST 2002
//   Made it use the automatically generated viewerRPC object.
//
//   Brad Whitlock, Mon Feb 10 11:53:10 PDT 2003
//   I turned snprintf into SNPRINTF.
//
// ****************************************************************************

void
ViewerMethods::SetWindowArea(int x, int y, int w, int h)
{
    //
    // Set the rpc type.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetWindowAreaRPC);
    char str[50];
    SNPRINTF(str, 50, "%dx%d+%d+%d", w, h, x, y);
    state->GetViewerRPC()->SetWindowArea(str);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::DatabaseQuery
//
// Purpose:
//   Tells the viewer to perform the named database query.
//
// Arguments:
//   queryName : The name of the query to perform.
//   vars      : The variables that we're querying.
//   arg1      : Optional integer argument.
//   arg2      : Optional integer argument.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 9 16:35:17 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Wed Jul 23 17:04:10 PDT 2003
//   Added optional integer args.
//
//   Kathleen Bonnell, Wed Dec 15 17:12:47 PST 2004
//   Added optional bool globalflag.
//
//   Hank Childs, Mon Jul 10 17:37:14 PDT 2006
//   Added two double arguments.
//
// ****************************************************************************

void
ViewerMethods::DatabaseQuery(const std::string &queryName,
    const stringVector &vars, const bool bflag, const int arg1, const int arg2,
    const bool globalFlag, const doubleVector &darg1, const doubleVector &darg2)
{
    //
    // Set the rpc type.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::DatabaseQueryRPC);
    state->GetViewerRPC()->SetQueryName(queryName);
    state->GetViewerRPC()->SetQueryVariables(vars);
    state->GetViewerRPC()->SetIntArg1(arg1);
    state->GetViewerRPC()->SetIntArg2(arg2);
    state->GetViewerRPC()->SetBoolFlag(bflag);
    state->GetViewerRPC()->SetIntArg3((int)globalFlag);
    state->GetViewerRPC()->SetDoubleArg1(darg1);
    state->GetViewerRPC()->SetDoubleArg2(darg2);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::PointQuery
//
// Purpose: 
//   Tells the viewer to perform the named point query.
//
// Arguments:
//   queryName : The name of the query to perform.
//   pt        : The location of the point to query. It can be in screen 
//               or world coordinates.
//   vars      : The variables that we're querying.
//   arg1      : An optional int argument.
//   arg2      : An optional int argument.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 9 16:36:12 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Wed Nov 26 14:17:55 PST 2003
//   Added optional int args.
//   
//   Kathleen Bonnell, Thu Apr  1 19:13:59 PST 2004 
//   Added optional bool flag.
//   
//   Kathleen Bonnell, Wed Dec 15 17:12:47 PST 2004 
//   Added optional bool flag.
//   
// ****************************************************************************

void
ViewerMethods::PointQuery(const std::string &queryName, const double pt[3],
    const stringVector &vars, const bool time, const int arg1, const int arg2,
    const bool globalFlag) 
{
    //
    // Set the rpc type.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::PointQueryRPC);
    state->GetViewerRPC()->SetQueryName(queryName);
    state->GetViewerRPC()->SetQueryPoint1(pt);
    state->GetViewerRPC()->SetQueryVariables(vars);
    state->GetViewerRPC()->SetBoolFlag(time);
    state->GetViewerRPC()->SetIntArg1(arg1);
    state->GetViewerRPC()->SetIntArg2(arg2);
    state->GetViewerRPC()->SetIntArg3((int)globalFlag);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::LineQuery
//
// Purpose: 
//   Tells the viewer to perform the named line query.
//
// Arguments:
//   queryName : The name of the query to perform.
//   pt1       : The start location of the line to query. It can be in screen 
//               or world coordinates.
//   pt2       : The end location of the line to query. It can be in screen 
//               or world coordinates.
//   vars      : The variables that we're querying.
//   samples   : The number of samples to be used in the query. 
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 9 16:37:43 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Wed Jul 23 17:04:10 PDT 2003
//   Added samples arg.
//   
//   Kathleen Bonnell, Tue May 15 10:39:58 PDT 2007 
//   Added forceSampling arg.
//   
// ****************************************************************************

void
ViewerMethods::LineQuery(const std::string &queryName, const double pt1[3],
    const double pt2[3], const stringVector &vars, const int samples,
    const bool forceSampling)
{
    //
    // Set the rpc type.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::LineQueryRPC);
    state->GetViewerRPC()->SetQueryName(queryName);
    state->GetViewerRPC()->SetQueryPoint1(pt1);
    state->GetViewerRPC()->SetQueryPoint2(pt2);
    state->GetViewerRPC()->SetQueryVariables(vars);
    state->GetViewerRPC()->SetIntArg1(samples);
    state->GetViewerRPC()->SetBoolFlag(forceSampling);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::Pick
//
// Purpose: 
//   Tells the viewer to add a pick point at the specified screen location.
//
// Arguments:
//   x,y  : The pick point location in screen coordinates.
//   vars : The variables that we're querying.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 9 16:38:51 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Wed Sep  8 10:26:32 PDT 2004
//   Changed queryname passed to PointQuery form 'Pick' to 'ScreenZonePick'.
//   
// ****************************************************************************

void
ViewerMethods::Pick(int x, int y, const stringVector &vars)
{
    double pt[3];
    pt[0] = (double)x;
    pt[1] = (double)y;
    pt[2] = 0.;
    PointQuery("ScreenZonePick", pt, vars);
}


// ****************************************************************************
// Method: ViewerMethods::Pick
//
// Purpose: 
//   Tells the viewer to add a pick point at the specified world coordinate.
//
// Arguments:
//   xyz  : The pick point location in world coordinates.
//   vars   : The variables that we're querying.
//
// Programmer: Kathleen Bonnell 
// Creation:   July 23, 2003 
//
// Modifications:
//   Kathleen Bonnell, Wed Sep  8 10:26:32 PDT 2004
//   Changed queryname passed to PointQuery from 'WorldPick' to 'Pick'.
//
// ****************************************************************************

void
ViewerMethods::Pick(double xyz[3], const stringVector &vars)
{
    PointQuery("Pick", xyz, vars);
}

// ****************************************************************************
// Method: ViewerMethods::NodePick
//
// Purpose: 
//   Tells the viewer to add a pick point at the specified screen location.
//
// Arguments:
//   x,y  : The pick point location in screen coordinates.
//   vars : The variables that we're querying.
//
// Programmer: Kathleen Bonnell 
// Creation:   June 25, 2003 
//
// Modifications:
//   Kathleen Bonnell, Wed Sep  8 10:26:32 PDT 2004
//   Changed queryname passed to PointQuery from 'NodePick' to 
//   'ScreenNodePick'.
//   
// ****************************************************************************

void
ViewerMethods::NodePick(int x, int y, const stringVector &vars)
{
    double pt[3];
    pt[0] = (double)x;
    pt[1] = (double)y;
    pt[2] = 0.;
    PointQuery("ScreenNodePick", pt, vars);
}


// ****************************************************************************
// Method: ViewerMethods::NodePick
//
// Purpose: 
//   Tells the viewer to add a pick point at the specified world coordinate.
//
// Arguments:
//   xyz  : The pick point location in world coordinates.
//   vars   : The variables that we're querying.
//
// Programmer: Kathleen Bonnell 
// Creation:   July 23, 2003 
//
// Modifications:
//   Kathleen Bonnell, Wed Sep  8 10:26:32 PDT 2004
//   Changed queryname passed to PointQuery from 'WorldNodePick' to 'NodePick'.
//   
// ****************************************************************************

void
ViewerMethods::NodePick(double xyz[3], const stringVector &vars)
{
    PointQuery("NodePick", xyz, vars);
}

// ****************************************************************************
// Method: ViewerMethods::Lineout
//
// Purpose: 
//   Tells the viewer to add a reference line at the specified screen location.
//
// Arguments:
//   x0,y0   : The start location of the line in screen coordinates.
//   x1,y1   : The start location of the line in screen coordinates.
//   vars    : The variables that we're querying.
//   samples : The number of samples along the line. 
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 9 16:38:51 PST 2002
//
// Modifications:
//   Brad Whitlock, Fri Dec 27 11:43:46 PDT 2002
//   I changed the routine so it takes lineout endpoints in world space.
//
//   Kathleen Bonnell, Wed Jul 23 17:04:10 PDT 2003
//   Added samples arg.
//   
//   Kathleen Bonnell, Tue May 15 10:39:58 PDT 2007 
//   Added forceSampling arg.
//   
// ****************************************************************************

void
ViewerMethods::Lineout(const double p0[3], const double p1[3],
    const stringVector &vars, const int samples, const bool forceSampling)
{
    LineQuery("Lineout", p0, p1, vars, samples, forceSampling);
}



// ****************************************************************************
//  Method: ViewerMethods::SetGlobalLineotuAttributes
//
//  Purpose:
//    Applies the global lineout attributes.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 4, 2003 
//
//  Modifications:
//
// ****************************************************************************

void
ViewerMethods::SetGlobalLineoutAttributes()
{
    //
    // Set the rpc type.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetGlobalLineoutAttributesRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}


// ****************************************************************************
//  Method: ViewerMethods::SetDefaultPickAttributes
//
//  Purpose:
//    Applies the default pick attributes.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   December 9, 2003 
//
//  Modifications:
//
// ****************************************************************************

void
ViewerMethods::SetDefaultPickAttributes()
{
    //
    // Set the rpc type.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetDefaultPickAttributesRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}


// ****************************************************************************
//  Method: ViewerMethods::SetPickAttributes
//
//  Purpose:
//    Applies the pick attributes.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 30, 2003 
//
//  Modifications:
//
// ****************************************************************************

void
ViewerMethods::SetPickAttributes()
{
    //
    // Set the rpc type.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetPickAttributesRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}


// ****************************************************************************
// Method: ViewerMethods::ResetQueryOverTimeAttributes
//
// Purpose: 
//   Reset the time query attributes to default values.
//
// Programmer: Kathleen Bonnell 
// Creation:   March 24, 2004 
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::ResetQueryOverTimeAttributes()
{
    //
    // Set the rpc type.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ResetQueryOverTimeAttributesRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::SetDefaultQueryOverTimeAttributes
//
//  Purpose:
//    Applies the default time query attributes.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 24, 2004 
//
//  Modifications:
//
// ****************************************************************************

void
ViewerMethods::SetDefaultQueryOverTimeAttributes()
{
    //
    // Set the rpc type.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetDefaultQueryOverTimeAttributesRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}


// ****************************************************************************
//  Method: ViewerMethods::SetQueryOverTimeAttributes
//
//  Purpose:
//    Applies the time query attributes.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 24, 2004 
//
//  Modifications:
//
// ****************************************************************************

void
ViewerMethods::SetQueryOverTimeAttributes()
{
    //
    // Set the rpc type.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetQueryOverTimeAttributesRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::SetTryHarderCyclesTimes
//
//  Purpose: Tells viewer to try harder to get accurate cycles/times
//
//  Programmer: Mark C. Miller 
//  Creation:   May 27, 2005 
//
// ****************************************************************************

void
ViewerMethods::SetTryHarderCyclesTimes(int flag)
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetTryHarderCyclesTimesRPC);
    state->GetViewerRPC()->SetIntArg1(flag);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::SetTreatAllDBsAsTimeVarying
//
//  Purpose: Tells viewer to treat all databases as time varying 
//
//  Programmer: Mark C. Miller 
//  Creation:   June 11, 2007 
//
// ****************************************************************************

void
ViewerMethods::SetTreatAllDBsAsTimeVarying(int flag)
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetTreatAllDBsAsTimeVaryingRPC);
    state->GetViewerRPC()->SetIntArg1(flag);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::ResetLineoutColor
//
// Purpose: 
//   Reset the lineout color to default values.
//
// Programmer: Kathleen Bonnell 
// Creation:   August 5, 2004 
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::ResetLineoutColor()
{
    //
    // Set the rpc type.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ResetLineoutColorRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}


// ****************************************************************************
// Method: ViewerMethods::ResetInteractorAttributes
//
// Purpose: 
//   Reset the interactor attributes to default values.
//
// Programmer: Kathleen Bonnell 
// Creation:   August 16, 2004 
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::ResetInteractorAttributes()
{
    //
    // Set the rpc type.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::ResetInteractorAttributesRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::SetDefaultInteractorAttributes
//
//  Purpose:
//    Applies the default interactor attributes.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 16, 2004
//
//  Modifications:
//
// ****************************************************************************

void
ViewerMethods::SetDefaultInteractorAttributes()
{
    //
    // Set the rpc type.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetDefaultInteractorAttributesRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}


// ****************************************************************************
//  Method: ViewerMethods::SetInteractorAttributes
//
//  Purpose:
//    Applies the interactor attributes.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 16, 2004 
//
//  Modifications:
//
// ****************************************************************************

void
ViewerMethods::SetInteractorAttributes()
{
    //
    // Set the rpc type.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetInteractorAttributesRPC);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::QueryProcessAttributes
//
//  Purpose: Gets unix process information
//
//  Programmer: Mark C. Miller 
//  Creation:   January 5, 2005 
//
// ****************************************************************************

void
ViewerMethods::QueryProcessAttributes(int id,
    const std::string engineHostName, const std::string engineDbName)
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::GetProcInfoRPC);
    state->GetViewerRPC()->SetIntArg1(id);
    state->GetViewerRPC()->SetProgramHost(engineHostName);
    state->GetViewerRPC()->SetProgramSim(engineDbName);

    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method:  ViewerMethods::SendSimulationCommand
//
//  Purpose:
//    Send a command to the simulation that has no arguments.
//
//  Arguments:
//    hostName   the host for the simulation
//    simName    the sim filename for the simulation
//    command    the string for the command
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  4, 2005
//
// ****************************************************************************
void
ViewerMethods::SendSimulationCommand(const std::string &hostName,
                                   const std::string &simName,
                                   const std::string &command)
{
    //
    // Set the rpc type and arguments
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SendSimulationCommandRPC);
    state->GetViewerRPC()->SetProgramHost(hostName);
    state->GetViewerRPC()->SetProgramSim(simName);
    state->GetViewerRPC()->SetStringArg1(command);
    state->GetViewerRPC()->SetStringArg2("");

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}


// ****************************************************************************
//  Method:  ViewerMethods::SendSimulationCommand
//
//  Purpose:
//    Send a command to the simulation that has a single string argument.
//
//  Arguments:
//    hostName   the host for the simulation
//    simName    the sim filename for the simulation
//    command    the string for the command
//    argument   a string argument for the command
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  4, 2005
//
// ****************************************************************************
void
ViewerMethods::SendSimulationCommand(const std::string &hostName,
                                   const std::string &simName,
                                   const std::string &command,
                                   const std::string &argument)
{
    //
    // Set the rpc type and arguments
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SendSimulationCommandRPC);
    state->GetViewerRPC()->SetProgramHost(hostName);
    state->GetViewerRPC()->SetProgramSim(simName);
    state->GetViewerRPC()->SetStringArg1(command);
    state->GetViewerRPC()->SetStringArg2(argument);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::OpenClient
//
// Purpose: 
//   Tells the viewer to launch the specified VisIt client - a reverse launch!
//
// Arguments:
//   clientName : The name to display when we're connecting.
//   program    : The name of the program to launch.
//   args       : Any arguments that we want to the client program to have.
//
// Programmer: Brad Whitlock
// Creation:   Tue May 3 15:41:29 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::OpenClient(const std::string &clientName,
    const std::string &program, const stringVector &args)
{
    state->GetViewerRPC()->SetRPCType(ViewerRPC::OpenClientRPC);
    state->GetViewerRPC()->SetDatabase(clientName);
    state->GetViewerRPC()->SetProgramHost(program);
    state->GetViewerRPC()->SetProgramOptions(args);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::SuppressQueryOutput
//
// Purpose: 
//   Shows the toolbars for the active vis window or for all vis windows.
//
// Programmer: Kathleen Bonnell 
// Creation:   July 27, 2005 
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::SuppressQueryOutput(bool onOff)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SuppressQueryOutputRPC);
    state->GetViewerRPC()->SetBoolFlag(onOff);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}


// ****************************************************************************
// Method: ViewerMethods::SetQueryFloatFormat
//
// Purpose: 
//   Sets the floating point format string used by queries. 
//
// Programmer: Cyrus Harrison
// Creation:   September 18, 2007 
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::SetQueryFloatFormat(const std::string &format)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::SetQueryFloatFormatRPC);
    state->GetViewerRPC()->SetStringArg1(format);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
// Method: ViewerMethods::UpdatePlotInfoAtts
//
// Purpose: 
//   Update the plotInfoAttributes from the given plot and window.
//
// Programmer: Kathleen Bonnell 
// Creation:   June 20, 2006
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMethods::UpdatePlotInfoAtts(int plotID, int winID)
{
    //
    // Set the rpc type and arguments.
    //
    state->GetViewerRPC()->SetRPCType(ViewerRPC::UpdatePlotInfoAttsRPC);
    state->GetViewerRPC()->SetWindowId(winID);
    state->GetViewerRPC()->SetIntArg1(plotID);

    //
    // Issue the RPC.
    //
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::SetCreateMeshQualityExpressions
//
//  Purpose: Tells viewer to turn on/off automatic creation of MeshQuality
//           expressions. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 9, 2007 
//
// ****************************************************************************

void
ViewerMethods::SetCreateMeshQualityExpressions(int flag)
{
    state->GetViewerRPC()->SetRPCType(
        ViewerRPC::SetCreateMeshQualityExpressionsRPC);
    state->GetViewerRPC()->SetIntArg1(flag);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::SetCreateTimeDerivativeExpressions
//
//  Purpose: Tells viewer to turn on/off automatic creation of TimeDerivative
//           expressions. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 9, 2007 
//
// ****************************************************************************

void
ViewerMethods::SetCreateTimeDerivativeExpressions(int flag)
{
    state->GetViewerRPC()->SetRPCType(
        ViewerRPC::SetCreateTimeDerivativeExpressionsRPC);
    state->GetViewerRPC()->SetIntArg1(flag);
    state->GetViewerRPC()->Notify();
}

// ****************************************************************************
//  Method: ViewerMethods::SetCreateVectorMagnitudeExpressions
//
//  Purpose: Tells viewer to turn on/off automatic creation of vector 
//           magnitude expressions. 
//
//  Programmer: Cyrus Harrison
//  Creation:   November 28, 2007 
//
// ****************************************************************************

void
ViewerMethods::SetCreateVectorMagnitudeExpressions(int flag)
{
    state->GetViewerRPC()->SetRPCType(
        ViewerRPC::SetCreateVectorMagnitudeExpressionsRPC);
    state->GetViewerRPC()->SetIntArg1(flag);
    state->GetViewerRPC()->Notify();
}

