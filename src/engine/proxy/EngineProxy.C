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

// ************************************************************************* //
//                               EngineProxy.C                               //
// ************************************************************************* //

#include <EngineProxy.h>

#include <AbortException.h>
#include <LostConnectionException.h>

// MCM -- 22Feb05: Hack to fix problem on SGI where reconstituting an
// exception would die in the throw. If we ever figure out why SGI needed
// this, we should remove this include directive
#include <InvalidVariableException.h>

#include <RemoteProcess.h>
#include <SocketConnection.h>
#include <StatusAttributes.h>
#include <ExpressionList.h>
#include <DebugStream.h>
#include <TimingsManager.h>
#include <snprintf.h>

#include <stdio.h>

// ****************************************************************************
//  Method: EngineProxy constructor
//
//  Programmer: Eric Brugger
//  Creation:   July 26, 2000
//
//  Modifications:
//     Brad Whitlock, Fri Oct 20 12:50:32 PDT 2000
//     Added intialization of remoteUserName.
//
//     Brad Whitlock, Mon Apr 30 17:43:51 PST 2001
//     Added code to create the status attributes.
//
//     Brad Whitlock, Thu Sep 26 17:28:54 PST 2002
//     Initialized the progress callbacks.
//
//     Brad Whitlock, Wed Nov 27 14:10:03 PST 2002
//     I added numProcs, numNodes, and loadBalancing.
//
//     Brad Whitlock, Fri May 2 15:32:27 PST 2003
//     I made it inherit from RemoteProxyBase.
//
//     Jeremy Meredith, Tue Aug 24 22:12:21 PDT 2004
//     Added metadata and sil atts for simulations.
//
//     Brad Whitlock, Thu Jan 25 13:53:15 PST 2007
//     Added commandFromSim.
//
// ****************************************************************************

EngineProxy::EngineProxy() : RemoteProxyBase("-engine")
{
    // Indicate that we want 2 write sockets from the engine.
    nWrite = 2;

    // Initialize the engine information that we can query.
    numProcs = 1;
    numNodes = -1;
    loadBalancing = 0;

    // Create the status attributes that we use to communicate status
    // information to the client.
    statusAtts = new StatusAttributes;
    metaData = new avtDatabaseMetaData;
    silAtts = new SILAttributes;
    commandFromSim = new SimulationCommand;
}

// ****************************************************************************
//  Method: EngineProxy destructor
//
//  Programmer: Eric Brugger
//  Creation:   July 26, 2000
//
//  Modifications:
//    Brad Whitlock, Fri Sep 29 19:18:02 PST 2000
//    Added code to delete command line arguments.
//
//    Hank Childs, Mon Oct 16 11:18:58 PDT 2000
//    Fixed up memory leak.
//
//    Brad Whitlock, Fri Oct 6 11:32:23 PDT 2000
//    I removed the SocketConnections.
//
//    Brad Whitlock, Fri Oct 20 12:51:14 PDT 2000
//    Added code to delete remoteUserName.
//
//    Brad Whitlock, Tue Apr 24 12:48:21 PDT 2001
//    Added code to delete the engine.
//
//    Brad Whitlock, Mon Apr 30 17:43:23 PST 2001
//    Added code to delete the status attributes.
//
//    Brad Whitlock, Fri May 2 15:33:21 PST 2003
//    I removed some members since they are now deleted in the base class.
//
//    Kathleen Bonnell, Tue Jan 11 16:06:33 PST 2005 
//    Delete metaData and silAtts.
//
//    Brad Whitlock, Thu Jan 25 13:53:36 PST 2007
//    Delete commandFromSim.
//
// ****************************************************************************

EngineProxy::~EngineProxy()
{
    delete statusAtts;
    delete metaData;
    delete silAtts;
    delete commandFromSim;
}

// ****************************************************************************
// Method: EngineProxy::SetupComponentRPCs
//
// Purpose: 
//   Hook up the engine's RPC's to the xfer object. This is called after the
//   engine is launched.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 15:34:54 PST 2003
//
// Modifications:
//
//    Sean Ahern, Thu Nov 21 20:14:01 PST 2002
//    Removed AddNamedFunction (no longer needed).  Added observation of
//    the ExpressionList.
//
//    Sean Ahern, Wed Feb  5 15:34:04 PST 2003
//    Changed the interface to expression lists.
//
//    Sean Ahern, Tue Jul 29 12:58:08 PDT 2003
//    Added a notification for the expression list.
//   
//    Jeremy Meredith, Mon Sep 15 17:16:20 PDT 2003
//    Removed SetFinalVariableNameRPC.
//
//    Kathleen Bonnell, Wed Mar 31 17:23:01 PST 2004 
//    Added CloneNetworkRPC.
//
//    Jeremy Meredith, Tue Aug 24 22:12:21 PDT 2004
//    Added metadata and sil atts for simulations.
//
//    Hank Childs, Mon Feb 28 17:26:21 PST 2005
//    Added StartQuery.
//
//    Mark C. Miller, Tue Mar  8 17:59:40 PST 2005
//    Added ProcInfoRPC
//
//    Jeremy Meredith, Mon Apr  4 16:01:21 PDT 2005
//    Added simulationCommandRPC.
//
//    Hank Childs, Thu May 26 11:42:40 PDT 2005
//    Added exportDatabaseRPC.
//
//    Hank Childs, Mon Feb 13 22:21:42 PST 2006
//    Add constructDDFRPC.
//
//    Brad Whitlock, Thu Jan 25 13:54:02 PST 2007
//    Added commandFromSim.
//
//    Jeremy Meredith, Wed Jan 23 16:11:41 EST 2008
//    Added setEFileOpenOptionsRPC.
//
// ****************************************************************************
void
EngineProxy::SetupComponentRPCs()
{
    //
    // Add RPCs to the transfer object.
    //
    xfer.Add(&readRPC);
    xfer.Add(&applyOperatorRPC);
    xfer.Add(&makePlotRPC);
    xfer.Add(&useNetworkRPC);
    xfer.Add(&updatePlotAttsRPC);
    xfer.Add(&pickRPC);
    xfer.Add(&startPickRPC);
    xfer.Add(&startQueryRPC);
    xfer.Add(&executeRPC);
    xfer.Add(&clearCacheRPC);
    xfer.Add(&queryRPC);
    xfer.Add(&releaseDataRPC);
    xfer.Add(&openDatabaseRPC);
    xfer.Add(&defineVirtualDatabaseRPC);
    xfer.Add(&renderRPC);
    xfer.Add(&setWinAnnotAttsRPC);
    xfer.Add(&cloneNetworkRPC);
    xfer.Add(&procInfoRPC);
    xfer.Add(&simulationCommandRPC);
    xfer.Add(&exportDatabaseRPC);
    xfer.Add(&constructDDFRPC);
    xfer.Add(&setEFileOpenOptionsRPC);

    //
    // Add other state objects to the transfer object
    //
    xfer.Add(&exprList);
    xfer.Add(metaData);
    xfer.Add(silAtts);
    xfer.Add(commandFromSim);

    // Extract some information about the engine from the command line
    // arguments that were used to create it.
    ExtractEngineInformation();
}

// ****************************************************************************
// Method: EngineProxy::ExtractEngineInformation
//
// Purpose: 
//   Extracts information about the engine from its command line arguments.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 15:38:22 PST 2003
//
// Modifications:
//
// ****************************************************************************

void
EngineProxy::ExtractEngineInformation()
{
    //
    // Look for certain key arguments in the command line arguments
    // and parse them out so we can query their values in the
    // engine proxy.
    //
    for(int i = 0; i < argv.size(); ++i)
    {
        if(argv[i] == "-np" && (i+1) < argv.size())
        {
           int np = 1;
           if(sscanf(argv[i+1].c_str(), "%d", &np) == 1)
           {
              if(np >= 1)
                  numProcs = np;
           }
           ++i;
        }
        else if(argv[i] == "-nn" && (i+1) < argv.size())
        {
           int nn = 1;
           if(sscanf(argv[i+1].c_str(), "%d", &nn) == 1)
           {
              if(nn >= 1)
                  numNodes = nn;
           }
           ++i;
        }
        else if(argv[i] == "-forcestatic")
        {
           loadBalancing = 0;
        }
        else if(argv[i] == "-forcedynamic")
        {
           loadBalancing = 1;
        }
    }
}

// ****************************************************************************
// Method: EngineProxy::GetComponentName
//
// Purpose: 
//   Returns the name of this component.
//
// Returns:    The name of the component.
//
// Programmer: Brad Whitlock
// Creation:   Tue May 6 10:47:35 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

std::string
EngineProxy::GetComponentName() const
{
    return "compute engine";
}

// ****************************************************************************
// Method: EngineProxy::SendKeepAlive
//
// Purpose: 
//   Sends a KeepAlive RPC to the compute engine and waits for a response.
//
// Note:       This method can cause a LostConnectionException if the engine
//             cannot be contacted.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 12 11:04:57 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
EngineProxy::SendKeepAlive()
{
    debug3 << "Sending KeepAlive RPC to compute engine." << endl;

    //
    // Call the base class's SendKeepAlive method so the command sockets
    // will be exercised.
    //
    RemoteProxyBase::SendKeepAlive();

    //
    // Now read some input back from the engine's data socket.
    //
#define KEEPALIVE_SIZE 10
    unsigned char buf[KEEPALIVE_SIZE];
    if (component->GetWriteConnection(1)->DirectRead(buf, KEEPALIVE_SIZE) < 0)
        debug1 << "Error reading keep alive data from engine!!!!\n";
}

// ****************************************************************************
//  Method: EngineProxy::ReadDataObject
//
//  Purpose:
//      Start a new network and open a variable/time from a database
//
//  Arguments:
//      format    the file format type.
//      file      the file to read from
//      var       the variable to read
//      time      the time step to read
//      silr      the sil restriction to use.
//
//  Returns:    
//
//  Programmer: Jeremy Meredith
//  Creation:   September 25, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Wed May 2 10:43:22 PDT 2001
//    Added reconstruction and throw of VisItException upon error status. 
//
//    Hank Childs, Wed Jun 13 10:50:35 PDT 2001
//    Added SIL restriction.
//
//    Brad Whitlock, Tue Oct 23 14:11:29 PST 2001
//    Modified to use a macro for doing exceptions.
//
//    Jeremy Meredith, Thu Oct 24 16:15:11 PDT 2002
//    Added material options.
//
//    Sean Ahern, Tue Jul 29 15:47:42 PDT 2003
//    Made the proxy have its own expression list.  When it's different
//    from the main one in the viewer, copy it, and send it to the engine
//    before asking for the data object.
//
//    Hank Childs, Tue Mar  9 14:37:46 PST 2004
//    Add the file format type.
//
//    Brad Whitlock, Fri Feb 18 09:40:52 PDT 2005
//    Added expressions argument so we don't have to rely on what's actually
//    stored in ParsingExprList since that can be unreliable with respect
//    to database expressions.
//
//    Mark C. Miller, Wed Nov 16 10:46:36 PST 2005
//    Added mesh management attributes 
//
//    Mark C. Miller, Thu Jun 14 10:26:37 PDT 2007
//    Added support to treat all databases as time varying
//
//    Mark C. Miller, Tue Jun 10 15:57:15 PDT 2008
//    Added support for ignoring extents
// ****************************************************************************

void
EngineProxy::ReadDataObject(const string &format, const string &file, 
                            const string &var, const int time,
                            avtSILRestriction_p silr,
                            const MaterialAttributes &matopts,
                            const ExpressionList &expressions,
                            const MeshManagementAttributes &meshopts,
                            bool treatAllDBsAsTimeVarying,
                            bool ignoreExtents)
{
    // Make sure the engine knows about our current expression list.
    if (exprList != expressions)
    {
        exprList = expressions;
        exprList.Notify();
    }

    CompactSILRestrictionAttributes *atts = silr->MakeCompactAttributes();
    readRPC(format, file, var, time, *atts, matopts, meshopts,
        treatAllDBsAsTimeVarying, ignoreExtents);
    if (readRPC.GetStatus() == VisItRPC::error)
    {
        RECONSTITUTE_EXCEPTION(readRPC.GetExceptionType(),
                               readRPC.Message());
    }
    delete atts;
}

// ****************************************************************************
//  Method: EngineProxy::ApplyOperator
//
//  Purpose:
//      Apply an operator to the end of the current pipeline
//
//  Arguments:
//      name       the name of the operator
//      atts       the attributes to apply
//
//  Returns:    
//
//  Programmer: Jeremy Meredith
//  Creation:   March  2, 2001
//
//  Modifications:
//    Kathleen Bonnell, Wed May 2 10:43:22 PDT 2001
//    Added reconstruction and throw of VisItException upon error status. 
//
//    Brad Whitlock, Tue Oct 23 14:11:29 PST 2001
//    Modified to use a macro for doing exceptions.
//
// ****************************************************************************
void
EngineProxy::ApplyOperator(const string &name, const AttributeSubject *atts)
{
    applyOperatorRPC(name, atts);
    if (applyOperatorRPC.GetStatus() == VisItRPC::error)
    {
        RECONSTITUTE_EXCEPTION(applyOperatorRPC.GetExceptionType(),
                               applyOperatorRPC.Message());
    }
}

// ****************************************************************************
//  Method: EngineProxy::MakePlot
//
//  Purpose:
//      Create a plot from the current pipeline
//
//  Arguments:
//      name       the name of the plot
//      atts       the attributes to apply
//
//  Returns:    
//
//  Programmer: Jeremy Meredith
//  Creation:   March  4, 2001
//
//  Modifications:
//    Kathleen Bonnell, Wed May 2 10:43:22 PDT 2001
//    Added reconstruction and throw of VisItException upon error status. 
//
//    Brad Whitlock, Tue Oct 23 14:11:29 PST 2001
//    Modified to use a macro for doing exceptions.
//
//    Jeremy Meredith, Fri Nov  9 10:21:15 PST 2001
//    Made it return the network id.
//
//    Eric Brugger, Fri Mar 19 15:14:34 PST 2004
//    I modified the rpc to pass the data limits to the engine.
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Added window id
//
//    Brad Whitlock, Wed Mar 21 22:55:12 PST 2007
//    Added plotName.
//
// ****************************************************************************
int
EngineProxy::MakePlot(const std::string &plotName, const string &pluginID,
    const AttributeSubject *atts, const vector<double> &extents, int winID)
{
    int id;
    id = makePlotRPC(plotName, pluginID, atts, extents, winID);
    if (makePlotRPC.GetStatus() == VisItRPC::error)
    {
        RECONSTITUTE_EXCEPTION(makePlotRPC.GetExceptionType(),
                               makePlotRPC.Message());
    }
    return id;
}

// ****************************************************************************
//  Method:  EngineProxy::UseNetwork
//
//  Purpose:
//    Set up the engine to re-use an existing network.
//
//  Arguments:
//    id         the id of the network to reuse
//
//  Programmer:  Jeremy Meredith
//  Creation:    November  7, 2001
//
// ****************************************************************************

void
EngineProxy::UseNetwork(int id)
{
    useNetworkRPC(id);
    if (useNetworkRPC.GetStatus() == VisItRPC::error)
    {
        RECONSTITUTE_EXCEPTION(useNetworkRPC.GetExceptionType(),
                               useNetworkRPC.Message());
    }
}


// ****************************************************************************
//  Method:  EngineProxy::UpdatePlotAttributes
//
//  Purpose:
//      Tell the engine to update the attributes for a plot.
//
//  Arguments:
//    id         the id of the network to reuse
//    atts       the attributes for a plot.
//
//  Programmer:  Hank Childs
//  Creation:    November 28, 2001
//
// ****************************************************************************

void
EngineProxy::UpdatePlotAttributes(const string &name, int id,
                                  const AttributeSubject *atts)
{
    updatePlotAttsRPC(name, id, atts);
    if (updatePlotAttsRPC.GetStatus() == VisItRPC::error)
    {
        RECONSTITUTE_EXCEPTION(updatePlotAttsRPC.GetExceptionType(),
                               updatePlotAttsRPC.Message());
    }
}

// ****************************************************************************
//  Method:  EngineProxy::SetWinAnnotAtts
//
//  Purpose:
//    Set up the current window and annotation attributes for the engine.
//
//  Arguments:
//    winAtts       the current window's attributes
//    annotAtts     the current window's annotation attributes
//
//  Programmer:  Mark C. Miller 
//  Creation:    15Jul03 
//
//  Modifications:
//
//    Mark C. Miller Wed Apr 14 16:41:32 PDT 2004
//    Added argument for extents type string
//
//    Mark C. Miller, Tue May 25 17:25:55 PDT 2004
//    Added argument for AnnotationObjectList
//
//    Mark C. Miller, Wed Jun  9 17:44:38 PDT 2004
//    Added arguement for visual cues
//
//    Mark C. Miller, Tue Jul 27 15:11:11 PDT 2004
//    Added argument for frame and state
//
//    Mark C. Miller, Wed Oct  6 18:12:29 PDT 2004
//    Added argument for view extents
//
//    Mark C. Miller, Tue Oct 19 20:18:22 PDT 2004
//    Added argument for color table name
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Added winID
//
// ****************************************************************************

void
EngineProxy::SetWinAnnotAtts(const WindowAttributes *winAtts,
                             const AnnotationAttributes *annotAtts,
                             const AnnotationObjectList *aoList,
                             const string extStr,
                             const VisualCueList *visCues,
                             const int *frameAndState,
                             const double *viewExtents,
                             const string ctName,
                             const int winID)
{
    setWinAnnotAttsRPC(winAtts, annotAtts, aoList, extStr, visCues,
        frameAndState, viewExtents, ctName, winID);
    if (setWinAnnotAttsRPC.GetStatus() == VisItRPC::error)
    {
        RECONSTITUTE_EXCEPTION(setWinAnnotAttsRPC.GetExceptionType(),
                               setWinAnnotAttsRPC.Message());
    }
}


// ****************************************************************************
//  Method: EngineProxy::Execute
//
//  Purpose:
//      Execute the current pipeline and return the result
//
//  Arguments:
//
//  Returns:    The avtDataObjectReader
//
//  Programmer: Jeremy Meredith
//  Creation:   September 25, 2000
//
//  Modifications:
//
//    Jeremy Meredith, Tue Sep 26 16:40:08 PDT 2000
//    Switched to use avt reader instead of vtk reader.
//
//    Hank Childs, Thu Sep 28 22:14:46 PDT 2000
//    Made return type be an avtDataset.
//
//    Hank Childs, Tue Oct 17 08:36:36 PDT 2000
//    Made return type be an avtDataSetReader.
//
//    Hank Childs, Fri Dec 29 08:48:12 PST 2000
//    Made return type be an avtDataObjectReader.
//
//    Brad Whitlock, Tue May 1 14:29:22 PST 2001
//    Added code to send status updates.
//
//    Kathleen Bonnell, Wed May 2 10:43:22 PDT 2001
//    Added reconstruction and throw of VisItException upon error status. 
//
//    Jeremy Meredith, Tue Jul  3 15:03:43 PDT 2001
//    Added handling of execution aborting.
//
//    Hank Childs, Mon Sep 17 11:22:30 PDT 2001
//    Allow avtDataObjectReader to manage memory to prevent unneed copies.
//
//    Brad Whitlock, Wed Oct 17 15:55:40 PST 2001
//    Added support for warnings coming from the RPC.
//
//    Brad Whitlock, Tue Oct 23 14:11:29 PST 2001
//    Modified to use a macro for doing exceptions.
//
//    Jeremy Meredith, Fri Dec 14 12:35:10 PST 2001
//    Removed some useless debug statements.
//
//    Brad Whitlock, Mon Mar 25 10:44:19 PDT 2002
//    Modified communication a little.
//
//    Kathleen Bonnell, Thu Jun 12 10:53:29 PDT 2003 
//    Added timing code for delay before read, and read. 
//
// ****************************************************************************

avtDataObjectReader_p
EngineProxy::Execute(bool respondWithNull, void (*waitCB)(void *), void *cbData)
{
    // Send a status message indicating that we're starting to execute
    // the pipeline.
    Status("Executing pipeline.");

    // Do it!
    executeRPC(respondWithNull);

    // Get the reply and update the progress bar
    while (executeRPC.GetStatus() == VisItRPC::incomplete ||
           executeRPC.GetStatus() == VisItRPC::warning)
    {
        executeRPC.RecvReply();

        // Send a warning message if the status is a warning.
        if(executeRPC.GetStatus() == VisItRPC::incomplete)
        {
            // Send a status message.
            Status(executeRPC.GetPercent(), executeRPC.GetCurStageNum(),
                   executeRPC.GetCurStageName(), executeRPC.GetMaxStageNum());
        }
        else if(executeRPC.GetStatus() == VisItRPC::warning)
        {
            debug4 << "Warning: " << executeRPC.Message().c_str() << endl;
            Warning(executeRPC.Message().c_str());
        }

        // If we passed a callback function, execute it.
        if(waitCB)
            waitCB(cbData);
    }

    // Check for abort
    if (executeRPC.GetStatus() == VisItRPC::abort)    
    {
        ClearStatus();
        EXCEPTION0(AbortException);
    }
    // Check for an error
    if (executeRPC.GetStatus() == VisItRPC::error)    
    {
        RECONSTITUTE_EXCEPTION(executeRPC.GetExceptionType(),
                               executeRPC.Message());
    }

    // Send a status message that indicates the output of the engine is
    // being transferred across the network.
    Status("Reading engine output.");

    int readDelay = visitTimer->StartTimer();
    // Read the VTK data
    long size = executeRPC.GetReplyLen();
    char *buf = new char[size];

    component->GetWriteConnection(1)->NeedsRead(true);
    visitTimer->StopTimer(readDelay, "Delay between read notification and actual read");

    int readData = visitTimer->StartTimer();
    if (component->GetWriteConnection(1)->DirectRead((unsigned char *)buf, size) < 0)
        debug1 << "Error reading VTK data!!!!\n";

    char msg[128];
    SNPRINTF(msg, 128, "Reading %ld bytes from socket", size);
    visitTimer->StopTimer(readData, msg);
    visitTimer->DumpTimings();

    // The data object reader will clean up the memory with buf.
    avtDataObjectReader_p avtreader  = new avtDataObjectReader;
    avtreader->Read(size, buf);

    // Output a message that indicates we're done reading the engine's output.
    ClearStatus();

    // Whoever called Execute will own the reader when this is returned since
    // the reference count will be decremented when avtreader goes out of
    // scope.
    return avtreader;
}

// ****************************************************************************
// Method: EngineProxy::ClearCache
//
// Purpose: 
//   This method calls an RPC that clears the engine's cache for the specified
//   database.
//
// Arguments:
//   filename : The name of the database to clear out.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 30 13:05:31 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
EngineProxy::ClearCache()
{
    clearCacheRPC("none", true);
}

void
EngineProxy::ClearCache(const std::string &filename)
{
    clearCacheRPC(filename, false);
}

// ****************************************************************************
// Method: EngineProxy::OpenDatabase
//
// Purpose: 
//   Tells the engine to open the specified database.
//
// Arguments:
//   file : The database name.
//   time : The timestep to open.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 10 14:24:21 PST 2002
//
// Modifications:
//    Jeremy Meredith, Tue Mar  4 13:07:17 PST 2003
//    Removed the check for status since OpenDatabaseRPC is now non-blocking.
//   
//    Hank Childs, Fri Mar  5 11:41:12 PST 2004
//    Add file format type.
//
//    Kathleen Bonnell, Tue Oct  9 14:40:10 PDT 2007 
//    Added createMeshQualityExpressions, createTimeDerivativeExpressions.
//
//    Mark C. Miller, Tue Jun 10 22:36:25 PDT 2008
//    Added support for ignoring bad extents from dbs.
// ****************************************************************************

void
EngineProxy::OpenDatabase(const std::string &format, const std::string &file,
                          int time, bool createMeshQualityExpressions,
                          bool createTimeDerivativeExpressions,
                          bool ignoreExtents)
{
    openDatabaseRPC(format, file, time, createMeshQualityExpressions,
                    createTimeDerivativeExpressions, ignoreExtents);
}

// ****************************************************************************
// Method: EngineProxy::DefineVirtualDatabase
//
// Purpose: 
//   Tells the engine to define the specified virtual database.
//
// Arguments:
//   fileFormat : The file format type.
//   dbName : The database name.
//   files  : The files in the database.
//   time   : The timestep to open.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 25 13:59:32 PST 2003
//
// Modifications:
//   
//   Hank Childs, Fri Mar  5 16:03:46 PST 2004
//   Added fileFormat argument.
//
//   Kathleen Bonnell, Tue Oct  9 14:40:10 PDT 2007 
//   Added createMeshQualityExpressions, createTimeDerivativeExpressions.
//
// ****************************************************************************

void
EngineProxy::DefineVirtualDatabase(const std::string &fileFormat,
    const std::string &wholeDBName, const std::string &pathToFiles, 
    const stringVector &files, int time, bool createMeshQualityExpressions,
    bool createTimeDerivativeExpressions)
{
    defineVirtualDatabaseRPC(fileFormat,wholeDBName, pathToFiles, files, time,
        createMeshQualityExpressions, createTimeDerivativeExpressions);
}

// ****************************************************************************
// Method: EngineProxy::Render
//
// Purpose: 
//   Tells the engine to render the plots specified in the list of ids 
//
// Arguments:
//   networkIDs : the network IDs of the plots to render 
//
// Programmer: Mark C. Miller 
// Creation:   07Apr03 
//
// Modifications:
//    Mark C. Miller, Mon Mar 29 15:01:58 PST 2004
//    Added new bool arg for controlling 3D annoations
//
//    Mark C. Miller, Tue Apr 20 07:44:34 PDT 2004
//    Added new waitCB and cbData args as well as a call to the waitCB in the
//    inner loop to support warning messages comming from engine
//
//    Mark C. Miller, Mon Jul 12 19:46:32 PDT 2004
//    Removed waitCB and cbData arguments
//   
//    Mark C. Miller, Wed Oct  6 18:12:29 PDT 2004
//    Changed bool arg for doing 3D annots to annotMode
//
//    Mark C. Miller, Sat Nov 13 09:35:51 PST 2004
//    Disabled callback to waitCB
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Added windowID
//
//    Mark C. Miller, Sat Jul 22 23:21:09 PDT 2006
//    Added leftEye to support stereo SR
//
//    Mark C. Miller, Mon Apr 14 15:41:21 PDT 2008
//    Removed conditionally removed code block conditioned on 'MCM_FIX'
// ****************************************************************************

avtDataObjectReader_p
EngineProxy::Render(bool sendZBuffer, const intVector& networkIDs,
    int annotMode, int windowID, bool leftEye,
    void (*waitCB)(void *), void *cbData)
{

    // Send a status message indicating that we're starting a scalable render 
    Status("Scalable Rendering.");

    // Do it!
    renderRPC(networkIDs, sendZBuffer, annotMode, windowID, leftEye);

    // Get the reply and update the progress bar
    while (renderRPC.GetStatus() == VisItRPC::incomplete ||
           renderRPC.GetStatus() == VisItRPC::warning)
    {
        renderRPC.RecvReply();

        // Send a warning message if the status is a warning.
        if(renderRPC.GetStatus() == VisItRPC::incomplete)
        {
            // Send a status message.
            Status(renderRPC.GetPercent(), renderRPC.GetCurStageNum(),
                   renderRPC.GetCurStageName(), renderRPC.GetMaxStageNum());
        }
        else if(renderRPC.GetStatus() == VisItRPC::warning)
        {
            debug4 << "Warning: " << renderRPC.Message().c_str() << endl;
            Warning(renderRPC.Message().c_str());
        }
    }

    // Check for abort
    if (renderRPC.GetStatus() == VisItRPC::abort)    
    {
        ClearStatus();
        EXCEPTION0(AbortException);
    }
    // Check for an error
    if (renderRPC.GetStatus() == VisItRPC::error)    
    {
        RECONSTITUTE_EXCEPTION(renderRPC.GetExceptionType(),
                               renderRPC.Message());
    }

    // Send a status message that indicates the output of the engine is
    // being transferred across the network.
    if (sendZBuffer)
       Status("Reading engine output [with zbuffer]");
    else
       Status("Reading engine output.");

    // Read the VTK data
    long size = renderRPC.GetReplyLen();
    char *buf = new char[size];

    if (component->GetWriteConnection(1)->DirectRead((unsigned char *)buf, size) < 0)
        debug1 << "Error reading VTK data!!!!\n";

    // The data object reader will clean up the memory with buf.
    avtDataObjectReader_p avtreader  = new avtDataObjectReader;
    avtreader->Read(size, buf);

    // Output a message that indicates we're done reading the engine's output.
    ClearStatus();

    // Whoever called Execute will own the reader when this is returned since
    // the reference count will be decremented when avtreader goes out of
    // scope.
    return avtreader;
}


// ****************************************************************************
// Method: EngineProxy::GetStatusAttributes
//
// Purpose: 
//   Returns a pointer to the status attributes.
//
// Returns:    A pointer to the status attributes.
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 30 17:45:01 PST 2001
//
// Modifications:
//   
// ****************************************************************************

StatusAttributes *
EngineProxy::GetStatusAttributes() const
{
    return statusAtts;
}

// ****************************************************************************
// Method: EngineProxy::Status
//
// Purpose: 
//   Sends a status message to the observers of the proxy's status attributes.
//
// Arguments:
//   message : The message to send.
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 30 17:53:51 PST 2001
//
// Modifications:
//    Jeremy Meredith, Fri Jun 29 15:14:45 PDT 2001
//    Added MessageType field.
//
// ****************************************************************************

void
EngineProxy::Status(const char *message)
{
    statusAtts->SetClearStatus(false);
    statusAtts->SetMessageType(1);
    statusAtts->SetStatusMessage(message);
    statusAtts->Notify();
}

// ****************************************************************************
// Method: EngineProxy::Status
//
// Purpose: 
//   Sends a status message to the observers of the proxy's status attributes.
//
// Arguments:
//   percent      : The percent complete.
//   curStage     : The current stage number,
//   curStageName : The name of the current stage.
//   maxStage     : The max number of stages.
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 30 17:53:51 PST 2001
//
// Modifications:
//    Jeremy Meredith, Fri Jun 29 15:14:45 PDT 2001
//    Added MessageType field.
//
// ****************************************************************************

void
EngineProxy::Status(int percent, int curStage, const std::string &curStageName,
   int maxStage)
{
    statusAtts->SetClearStatus(false);
    statusAtts->SetMessageType(2);
    statusAtts->SetPercent(percent);
    statusAtts->SetCurrentStage(curStage);
    statusAtts->SetCurrentStageName(curStageName);
    statusAtts->SetMaxStage(maxStage);
    statusAtts->Notify();
}

// ****************************************************************************
// Method: EngineProxy::Warning
//
// Purpose: 
//   Sends a warning message to the observer's of the proxy's status attributes.
//
// Arguments:
//   message : A pointer to the message string to send.
//
// Note:       I'm using the status attributes to send a message. Note the
//             message type is a new one (3).
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 17 16:01:20 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
EngineProxy::Warning(const char *message)
{
    statusAtts->SetClearStatus(false);
    statusAtts->SetMessageType(3);
    statusAtts->SetStatusMessage(message);
    statusAtts->Notify();
}

// ****************************************************************************
// Method: EngineProxy::ClearStatus
//
// Purpose: 
//   Clears the status.
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 30 17:56:08 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
EngineProxy::ClearStatus()
{
    statusAtts->SetClearStatus(true);
    statusAtts->Notify();
}


// ****************************************************************************
//  Method: EngineProxy::Interrupt
//
//  Purpose: 
//    Interrupt execution of the current pipeline in the engine.
//
//  Programmer: Jeremy Meredith
//  Creation:   July  2, 2001
//
//  Modifications:
//
// ****************************************************************************

void
EngineProxy::Interrupt()
{
    xfer.SendInterruption();
}

// ****************************************************************************
//  Method:  EngineProxy::Pick
//
//  Purpose:
//    Set up the engine to re-use an existing network.
//
//  Arguments:
//    atts       the pick attributes 
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    November 20, 2001
//
//  Modifications:
//    Kathleen Bonnell, Tue Mar  5 09:27:51 PST 2002  
//    Remove unnecessary debug lines.
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Added wid
//
// ****************************************************************************

void 
EngineProxy::Pick(const int nid, const PickAttributes *atts,
                  PickAttributes &retAtts, int wid)
{
    retAtts = pickRPC(nid, atts, wid);

    if (pickRPC.GetStatus() == VisItRPC::error)
    {
        RECONSTITUTE_EXCEPTION(pickRPC.GetExceptionType(),
                             pickRPC.Message());
    }
}


// ****************************************************************************
//  Method:  EngineProxy::StartPick
//
//  Purpose:
//
//  Arguments:
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    November 26, 2001
//
//  Modifications:
//    Kathleen Bonnell, Wed Jun  2 09:43:07 PDT 2004
//    Added forZones arg.
//
// ****************************************************************************

void 
EngineProxy::StartPick(const bool forZones, const bool flag, const int nid)
{

    startPickRPC(forZones, flag, nid);

    if (startPickRPC.GetStatus() == VisItRPC::error)
    {
        RECONSTITUTE_EXCEPTION(startPickRPC.GetExceptionType(),
                             startPickRPC.Message());
    }
}


// ****************************************************************************
//  Method:  EngineProxy::StartQuery
//
//  Purpose:
//
//  Arguments:
//
//  Programmer:  Hank Childs
//  Creation:    February 28, 2005
//
// ****************************************************************************

void 
EngineProxy::StartQuery(const bool flag, const int nid)
{

    startQueryRPC(flag, nid);

    if (startQueryRPC.GetStatus() == VisItRPC::error)
    {
        RECONSTITUTE_EXCEPTION(startQueryRPC.GetExceptionType(),
                             startQueryRPC.Message());
    }
}


// ****************************************************************************
//  Method:  EngineProxy::Query
//
//  Purpose:
//    Set up the engine to perform a query.
//
//  Arguments:
//    nid        The network Id this query should use.
//    atts       Attributes to be used by this query.
//    retAtts    Attributes to be returned by this query.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    September 6, 2002 
//
//  Modifications:
//
//    Hank Childs, Thu Oct  2 16:20:17 PDT 2003
//    Allow for a query to have multiple network ids.
//
// ****************************************************************************

void 
EngineProxy::Query(const std::vector<int> &nid, const QueryAttributes *atts,
                   QueryAttributes &retAtts)
{
    queryRPC(nid, atts);

    // Get the reply and update the progress bar
    while (queryRPC.GetStatus() == VisItRPC::incomplete ||
           queryRPC.GetStatus() == VisItRPC::warning)
    {
        queryRPC.RecvReply();
 
        // Send a warning message if the status is a warning.
        if(queryRPC.GetStatus() == VisItRPC::incomplete)
        {
            // Send a status message.
            Status(queryRPC.GetPercent(), queryRPC.GetCurStageNum(),
                   queryRPC.GetCurStageName(), queryRPC.GetMaxStageNum());
        }
        else if(queryRPC.GetStatus() == VisItRPC::warning)
        {
            debug4 << "Warning: " << queryRPC.Message().c_str() << endl;
            Warning(queryRPC.Message().c_str());
        }
    }
 
    // Check for abort
    if (queryRPC.GetStatus() == VisItRPC::abort)
    {
        ClearStatus();
        EXCEPTION0(AbortException);
    }

    // Check for an error
    if (queryRPC.GetStatus() == VisItRPC::error)    
    {
        RECONSTITUTE_EXCEPTION(queryRPC.GetExceptionType(),
                               queryRPC.Message());
    }
    retAtts = queryRPC.GetReturnAtts();
    ClearStatus();
}

// ****************************************************************************
//  Method:  EngineProxy::GetProcInfo
//
//  Purpose: Gets unix process information from the engine
//
//  Programmer:  Mark C. Miller 
//  Creation:    November 15, 2004 
//
// ****************************************************************************

void 
EngineProxy::GetProcInfo(ProcessAttributes &retAtts)
{
    procInfoRPC();

    // Get the reply and update the progress bar
    while (procInfoRPC.GetStatus() == VisItRPC::incomplete ||
           procInfoRPC.GetStatus() == VisItRPC::warning)
    {
        procInfoRPC.RecvReply();
    }
 
    // Check for abort
    if (procInfoRPC.GetStatus() == VisItRPC::abort)
    {
        ClearStatus();
        EXCEPTION0(AbortException);
    }

    // Check for an error
    if (procInfoRPC.GetStatus() == VisItRPC::error)    
    {
        RECONSTITUTE_EXCEPTION(procInfoRPC.GetExceptionType(),
                               procInfoRPC.Message());
    }

    retAtts = procInfoRPC.GetReturnAtts();

}

// ****************************************************************************
//  Method:  EngineProxy::ReleaseData
//
//  Purpose:
//    Set up the engine to have an existing network release its data.
//
//  Arguments:
//    id         the id of the network to release data.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    September 18, 2002 
//
// ****************************************************************************

void
EngineProxy::ReleaseData(const int id)
{
    releaseDataRPC(id);
    if (releaseDataRPC.GetStatus() == VisItRPC::error)
    {
        RECONSTITUTE_EXCEPTION(releaseDataRPC.GetExceptionType(),
                               releaseDataRPC.Message());
    }
}

// ****************************************************************************
//  Method:  EngineProxy::CloneNetwork
//
//  Purpose:
//    Set up the engine to have an existing network cloned.
//
//  Arguments:
//    id         the id of the network to be cloned.
//    origData   the type of input to use for the cloned network. 
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 18, 2004 
//
// ****************************************************************************

void
EngineProxy::CloneNetwork(const int id, const QueryOverTimeAttributes *qa)
{
    cloneNetworkRPC(id, qa);
    if (cloneNetworkRPC.GetStatus() == VisItRPC::error)
    {
        RECONSTITUTE_EXCEPTION(cloneNetworkRPC.GetExceptionType(),
                               cloneNetworkRPC.Message());
    }
}


// ****************************************************************************
//  Method:  EngineProxy::ConstructDDF
//
//  Purpose:
//      Have the engine construct a derived data function.
//
//  Arguments:
//    id         the id of the network to be cloned.
//    atts       the attributes to construct the DDF
//
//  Programmer:  Hank Childs
//  Creation:    February 13, 2006
//
// ****************************************************************************

void
EngineProxy::ConstructDDF(const int id, const ConstructDDFAttributes *atts)
{
    constructDDFRPC(id, atts);
    if (constructDDFRPC.GetStatus() == VisItRPC::error)
    {
        RECONSTITUTE_EXCEPTION(constructDDFRPC.GetExceptionType(),
                               constructDDFRPC.Message());
    }
}


// ****************************************************************************
//  Method:  EngineProxy::ExportDatabase
//
//  Purpose:
//      Have the engine export a database.
//
//  Arguments:
//    id         the id of the network to be cloned.
//    atts       the attributes to export the database.
//
//  Programmer:  Hank Childs
//  Creation:    May 26, 2005
//
// ****************************************************************************

void
EngineProxy::ExportDatabase(const int id, const ExportDBAttributes *atts)
{
    exportDatabaseRPC(id, atts);
    if (exportDatabaseRPC.GetStatus() == VisItRPC::error)
    {
        RECONSTITUTE_EXCEPTION(exportDatabaseRPC.GetExceptionType(),
                               exportDatabaseRPC.Message());
    }
}


// ****************************************************************************
//  Method:  EngineProxy::GetWriteSocket
//
//  Purpose:
//    Get the socket to receive input from the engine
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 24, 2004
//
// ****************************************************************************

int
EngineProxy::GetWriteSocket()
{
    if (xfer.GetInputConnection())
        return xfer.GetInputConnection()->GetDescriptor();
    else
        return -1;
}

// ****************************************************************************
//  Method:  EngineProxy::ReadDataAndProcess
//
//  Purpose:
//    Get some data fom the engine and process it.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 24, 2004
//
// ****************************************************************************

void
EngineProxy::ReadDataAndProcess()
{
    int amountRead = xfer.GetInputConnection()->Fill();
    if (amountRead > 0)
        xfer.Process();
    else
        EXCEPTION0(LostConnectionException);
}

// ****************************************************************************
//  Method:  EngineProxy::GetSimulationMetaData
//
//  Purpose:
//    Return the engine proxy metadata attributes for simulations.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 24, 2004
//
// ****************************************************************************

avtDatabaseMetaData *
EngineProxy::GetSimulationMetaData()
{
    return metaData;
}

// ****************************************************************************
//  Method:  EngineProxy::GetSimulationSILAtts
//
//  Purpose:
//    Return the engine proxy SIL attributes for simulations.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 24, 2004
//
// ****************************************************************************

SILAttributes *
EngineProxy::GetSimulationSILAtts()
{
    return silAtts;
}

// ****************************************************************************
//  Method:  EngineProxy::GetCommandFromSimulation
//
//  Purpose:
//    Return a command object that simulations can use to send the viewer 
//    commands.
//
//  Arguments:
//    none
//
//  Programmer:  Brad Whitlock
//  Creation:    Thu Jan 25 13:55:06 PST 2007
//
// ****************************************************************************

SimulationCommand *
EngineProxy::GetCommandFromSimulation()
{
    return commandFromSim;
}

// ****************************************************************************
//  Method:  EngineProxy::UpdateExpressions
//
//  Purpose:
//    Make sure the engine knows about the current expression list.
//
//  Arguments:
//    expressions  The new expression list.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 1, 2005 
//
// ****************************************************************************

void
EngineProxy::UpdateExpressions(const ExpressionList &expressions)
{
    if (exprList != expressions)
    {
        exprList = expressions;
        exprList.Notify();
    }
}


// ****************************************************************************
//  Method:  EngineProxy::ExecuteSimulationControlCommand
//
//  Purpose:
//    Execute a simulation control command that accepts no arguments.
//
//  Arguments:
//    cmd        the command string to execute
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 18, 2005
//
// ****************************************************************************

void
EngineProxy::ExecuteSimulationControlCommand(const std::string &cmd)
{
    simulationCommandRPC(cmd, 0,0,"");
}

// ****************************************************************************
//  Method:  EngineProxy::ExecuteSimulationControlCommand
//
//  Purpose:
//    Execute a simulation control command that takes a string argument.
//
//  Arguments:
//    cmd        the command string to execute
//    arg        a string argument to the command
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 21, 2005
//
// ****************************************************************************

void
EngineProxy::ExecuteSimulationControlCommand(const std::string &cmd,
                                             const std::string &arg)
{
    simulationCommandRPC(cmd, 0,0,arg);
}

// ****************************************************************************
//  Method:  EngineProxy::SetDefaultFileOpenOptions
//
//  Purpose:
//    Tells the engine about the latest default file opening options.
//
//  Arguments:
//    opts       the new options
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2008
//
// ****************************************************************************

void
EngineProxy::SetDefaultFileOpenOptions(const FileOpenOptions &opts)
{
    setEFileOpenOptionsRPC(opts);
}
