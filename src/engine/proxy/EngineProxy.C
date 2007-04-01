// ************************************************************************* //
//                               EngineProxy.C                               //
// ************************************************************************* //

#include <EngineProxy.h>

#include <AbortException.h>
#include <LostConnectionException.h>
#include <RemoteProcess.h>
#include <SocketConnection.h>
#include <StatusAttributes.h>
#include <ParsingExprList.h>
#include <ExpressionList.h>
#include <DebugStream.h>
#include <TimingsManager.h>
#include <snprintf.h>

#include <stdio.h>

#if defined(_WIN32)
// Windows compiler kludge
#define GetMessageA GetMessage
#endif

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
// ****************************************************************************

EngineProxy::~EngineProxy()
{
    delete statusAtts;
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
    xfer.Add(&executeRPC);
    xfer.Add(&clearCacheRPC);
    xfer.Add(&queryRPC);
    xfer.Add(&releaseDataRPC);
    xfer.Add(&openDatabaseRPC);
    xfer.Add(&defineVirtualDatabaseRPC);
    xfer.Add(&renderRPC);
    xfer.Add(&setWinAnnotAttsRPC);
    xfer.Add(&cloneNetworkRPC);

    //
    // Add other state objects to the transfer object
    //
    xfer.Add(&exprList);
    xfer.Add(metaData);
    xfer.Add(silAtts);

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
// ****************************************************************************
void
EngineProxy::ReadDataObject(const string &format, const string &file, 
                            const string &var, const int time,
                            avtSILRestriction_p silr,
                            const MaterialAttributes &matopts)
{
    // Make sure the engine knows about our current expression list.
    ExpressionList *vel = ParsingExprList::Instance()->GetList();
    if (exprList != *vel)
    {
        exprList.CopyAttributes(vel);
        exprList.Notify();
    }

    CompactSILRestrictionAttributes *atts = silr->MakeCompactAttributes();
    readRPC(format, file, var, time, *atts, matopts);
    if (readRPC.GetStatus() == VisItRPC::error)
    {
        RECONSTITUTE_EXCEPTION(readRPC.GetExceptionType(),
                               readRPC.GetMessage());
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
                               applyOperatorRPC.GetMessage());
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
// ****************************************************************************
int
EngineProxy::MakePlot(const string &name, const AttributeSubject *atts,
                      const vector<double> &extents)
{
    int id;
    id = makePlotRPC(name, atts, extents);
    if (makePlotRPC.GetStatus() == VisItRPC::error)
    {
        RECONSTITUTE_EXCEPTION(makePlotRPC.GetExceptionType(),
                               makePlotRPC.GetMessage());
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
                               useNetworkRPC.GetMessage());
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
                               updatePlotAttsRPC.GetMessage());
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
// ****************************************************************************

void
EngineProxy::SetWinAnnotAtts(const WindowAttributes *winAtts,
                             const AnnotationAttributes *annotAtts,
                             const AnnotationObjectList *aoList,
                             const string extStr,
                             const VisualCueList *visCues,
                             const int *frameAndState,
                             const double *viewExtents)
{
    setWinAnnotAttsRPC(winAtts, annotAtts, aoList, extStr, visCues,
        frameAndState, viewExtents);
    if (setWinAnnotAttsRPC.GetStatus() == VisItRPC::error)
    {
        RECONSTITUTE_EXCEPTION(setWinAnnotAttsRPC.GetExceptionType(),
                               setWinAnnotAttsRPC.GetMessage());
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
            debug4 << "Warning: " << executeRPC.GetMessage().c_str() << endl;
            Warning(executeRPC.GetMessage().c_str());
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
                               executeRPC.GetMessage());
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
// ****************************************************************************

void
EngineProxy::OpenDatabase(const std::string &format, const std::string &file,
                          int time)
{
    openDatabaseRPC(format, file, time);
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
// ****************************************************************************

void
EngineProxy::DefineVirtualDatabase(const std::string &fileFormat,
    const std::string &wholeDBName, const std::string &pathToFiles, 
    const stringVector &files, int time)
{
    defineVirtualDatabaseRPC(fileFormat,wholeDBName, pathToFiles, files, time);
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
// ****************************************************************************

avtDataObjectReader_p
EngineProxy::Render(bool sendZBuffer, const intVector& networkIDs,
    int annotMode)
{

    // Send a status message indicating that we're starting a scalable render 
    Status("Scalable Rendering.");

    // Do it!
    renderRPC(networkIDs, sendZBuffer, annotMode);

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
            debug4 << "Warning: " << renderRPC.GetMessage().c_str() << endl;
            Warning(renderRPC.GetMessage().c_str());
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
                               renderRPC.GetMessage());
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
    statusAtts->SetMessage(message);
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
    statusAtts->SetMessage(message);
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
// ****************************************************************************

void 
EngineProxy::Pick(const int nid, const PickAttributes *atts,
                  PickAttributes &retAtts)
{
    retAtts = pickRPC(nid, atts);

    if (pickRPC.GetStatus() == VisItRPC::error)
    {
        RECONSTITUTE_EXCEPTION(pickRPC.GetExceptionType(),
                             pickRPC.GetMessage());
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
                             startPickRPC.GetMessage());
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
            debug4 << "Warning: " << queryRPC.GetMessage().c_str() << endl;
            Warning(queryRPC.GetMessage().c_str());
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
                               queryRPC.GetMessage());
    }
    retAtts = queryRPC.GetReturnAtts();
    ClearStatus();
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
                               releaseDataRPC.GetMessage());
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
                               cloneNetworkRPC.GetMessage());
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

