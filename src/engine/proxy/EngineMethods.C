#include <EngineMethods.h>

#include <EngineState.h>

#include <AbortException.h>
#include <LostConnectionException.h>

// MCM -- 22Feb05: Hack to fix problem on SGI where reconstituting an
// exception would die in the throw. If we ever figure out why SGI needed
// this, we should remove this include directive
#include <InvalidVariableException.h>

#include <DebugStream.h>
#include <ExpressionList.h>
#include <ParentProcess.h>
#include <RemoteProcess.h>
#include <SocketConnection.h>
#include <StatusAttributes.h>
#include <TimingsManager.h>
#include <snprintf.h>


// ****************************************************************************
//  Method: EngineMethods::Constructor
//
//  Purpose:
//
//  Programmer:
//  Creation:
//
//  Modifications:
//
// ****************************************************************************
EngineMethods::EngineMethods(EngineState *_state)
{
    engineP     = 0;
    component   = 0;
    numNodes    = -1;
    state       = _state;
}

// ****************************************************************************
//  Method: EngineMethods::Destructor
//
//  Purpose:
//
//  Programmer:
//  Creation:
//
//  Modifications:
//
// ****************************************************************************
EngineMethods::~EngineMethods()
{
}

// ****************************************************************************
//  Method: EngineMethods::ReadDataObject
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
//      matopts
//      meshopts
//      treatAllDBsAsTimeVarying
//      ignoreExtents
//      selName   the selection to apply to the data.
//      windowID  the window id.
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
//
//    Brad Whitlock, Mon Aug 22 10:10:04 PDT 2011
//    I added a selName argument.
//
//    Eric Brugger, Mon Oct 31 10:37:19 PDT 2011
//    I added a window id argument.
//
// ****************************************************************************

void
EngineMethods::ReadDataObject(const std::string &format, const std::string &file,
                            const std::string &var, const int time,
                            avtSILRestriction_p silr,
                            const MaterialAttributes &matopts,
                            const ExpressionList &expressions,
                            const MeshManagementAttributes &meshopts,
                            bool treatAllDBsAsTimeVarying,
                            bool ignoreExtents,
                            const std::string &selName,
                            int windowID)
{
    // Make sure the engine knows about our current expression list.
    if (state->exprList != expressions)
    {
        state->exprList = expressions;
        state->exprList.Notify();
    }

    CompactSILRestrictionAttributes *atts = silr->MakeCompactAttributes();
    state->readRPC(format, file, var, time, *atts, matopts, meshopts,
                   treatAllDBsAsTimeVarying, ignoreExtents, selName, windowID);
    if (state->readRPC.GetStatus() == VisItRPC::error)
    {
        RECONSTITUTE_EXCEPTION(state->readRPC.GetExceptionType(),
                               state->readRPC.Message());
    }
    delete atts;
}

// ****************************************************************************
//  Method: EngineMethods::ApplyOperator
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
EngineMethods::ApplyOperator(const std::string &name, const AttributeSubject *atts)
{
    state->applyOperatorRPC(name, atts);
    if (state->applyOperatorRPC.GetStatus() == VisItRPC::error)
    {
        RECONSTITUTE_EXCEPTION(state->applyOperatorRPC.GetExceptionType(),
                               state->applyOperatorRPC.Message());
    }
}

// ****************************************************************************
//  Method: EngineMethods::MakePlot
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
EngineMethods::MakePlot(const std::string &plotName, const std::string &pluginID,
    const AttributeSubject *atts, const std::vector<double> &extents, int winID)
{
    int id;
    id = state->makePlotRPC(plotName, pluginID, atts, extents, winID);
    if (state->makePlotRPC.GetStatus() == VisItRPC::error)
    {
        RECONSTITUTE_EXCEPTION(state->makePlotRPC.GetExceptionType(),
                               state->makePlotRPC.Message());
    }
    return id;
}

// ****************************************************************************
//  Method:  EngineMethods::UseNetwork
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
EngineMethods::UseNetwork(int id)
{
    state->useNetworkRPC(id);
    if (state->useNetworkRPC.GetStatus() == VisItRPC::error)
    {
        RECONSTITUTE_EXCEPTION(state->useNetworkRPC.GetExceptionType(),
                               state->useNetworkRPC.Message());
    }
}


// ****************************************************************************
//  Method:  EngineMethods::UpdatePlotAttributes
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
EngineMethods::UpdatePlotAttributes(const std::string &name, int id,
                                  const AttributeSubject *atts)
{
    state->updatePlotAttsRPC(name, id, atts);
    if (state->updatePlotAttsRPC.GetStatus() == VisItRPC::error)
    {
        RECONSTITUTE_EXCEPTION(state->updatePlotAttsRPC.GetExceptionType(),
                               state->updatePlotAttsRPC.Message());
    }
}

// ****************************************************************************
//  Method:  EngineMethods::SetWinAnnotAtts
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
EngineMethods::SetWinAnnotAtts(const WindowAttributes *winAtts,
                             const AnnotationAttributes *annotAtts,
                             const AnnotationObjectList *aoList,
                             const std::string extStr,
                             const VisualCueList *visCues,
                             const int *frameAndState,
                             const double *viewExtents,
                             const std::string ctName,
                             const int winID)
{
    state->setWinAnnotAttsRPC(winAtts, annotAtts, aoList, extStr, visCues,
        frameAndState, viewExtents, ctName, winID);
    if (state->setWinAnnotAttsRPC.GetStatus() == VisItRPC::error)
    {
        RECONSTITUTE_EXCEPTION(state->setWinAnnotAttsRPC.GetExceptionType(),
                               state->setWinAnnotAttsRPC.Message());
    }
}


// ****************************************************************************
//  Method: EngineMethods::Execute
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
//    Brad Whitlock, Thu Apr  9 16:17:36 PDT 2009
//    I added reverse launch support.
//
// ****************************************************************************

avtDataObjectReader_p
EngineMethods::Execute(bool respondWithNull, void (*waitCB)(void *), void *cbData)
{
    // Send a status message indicating that we're starting to execute
    // the pipeline.
    Status("Executing pipeline.");

    // Do it!
    state->executeRPC(respondWithNull);

    // Get the reply and update the progress bar
    while (state->executeRPC.GetStatus() == VisItRPC::incomplete ||
           state->executeRPC.GetStatus() == VisItRPC::warning)
    {
        state->executeRPC.RecvReply();

        // Send a warning message if the status is a warning.
        if(state->executeRPC.GetStatus() == VisItRPC::incomplete)
        {
            // Send a status message.
            Status(state->executeRPC.GetPercent(), state->executeRPC.GetCurStageNum(),
                   state->executeRPC.GetCurStageName(), state->executeRPC.GetMaxStageNum());
        }
        else if(state->executeRPC.GetStatus() == VisItRPC::warning)
        {
            debug4 << "Warning: " << state->executeRPC.Message().c_str() << endl;
            Warning(state->executeRPC.Message().c_str());
        }

        // If we passed a callback function, execute it.
        if(waitCB)
            waitCB(cbData);
    }

    // Check for abort
    if (state->executeRPC.GetStatus() == VisItRPC::abort)
    {
        ClearStatus();
        EXCEPTION0(AbortException);
    }
    // Check for an error
    if (state->executeRPC.GetStatus() == VisItRPC::error)
    {
        RECONSTITUTE_EXCEPTION(state->executeRPC.GetExceptionType(),
                               state->executeRPC.Message());
    }

    // Send a status message that indicates the output of the engine is
    // being transferred across the network.
    Status("Reading engine output.");

    int readDelay = visitTimer->StartTimer();
    // Read the VTK data
    long size = state->executeRPC.GetReplyLen();
    char *buf = new char[size];

    if(engineP != NULL)
        engineP->GetReadConnection(1)->NeedsRead(true);
    else if(component != NULL)
        component->GetWriteConnection(1)->NeedsRead(true);
    visitTimer->StopTimer(readDelay, "Delay between read notification and actual read");

    int readData = visitTimer->StartTimer();
    if(engineP != NULL)
    {
        if (engineP->GetReadConnection(1)->DirectRead((unsigned char *)buf, size) < 0) 
        {
            debug1 << "Error reading VTK data!!!!\n";
        }
    }
    else if(component != NULL)
    {
        if (component->GetWriteConnection(1)->DirectRead((unsigned char *)buf, size) < 0) 
        {
            debug1 << "Error reading VTK data!!!!\n";
        }
    }

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
// Method: EngineMethods::ClearCache
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
EngineMethods::ClearCache()
{
    state->clearCacheRPC("none", true);
}

void
EngineMethods::ClearCache(const std::string &filename)
{
    state->clearCacheRPC(filename, false);
}

// ****************************************************************************
// Method: EngineMethods::OpenDatabase
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
EngineMethods::OpenDatabase(const std::string &format, const std::string &file,
                          int time, bool createMeshQualityExpressions,
                          bool createTimeDerivativeExpressions,
                          bool ignoreExtents)
{
    state->openDatabaseRPC(format, file, time, createMeshQualityExpressions,
                           createTimeDerivativeExpressions, ignoreExtents);
}

// ****************************************************************************
// Method: EngineMethods::DefineVirtualDatabase
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
//   Mark C. Miller, Wed Jun 17 16:10:27 PDT 2009
//   Added logic to recieve possible error message and re-constitute it.
// ****************************************************************************

void
EngineMethods::DefineVirtualDatabase(const std::string &fileFormat,
    const std::string &wholeDBName, const std::string &pathToFiles,
    const stringVector &files, int time, bool createMeshQualityExpressions,
    bool createTimeDerivativeExpressions)
{
    state->defineVirtualDatabaseRPC(fileFormat,wholeDBName, pathToFiles, files, time,
        createMeshQualityExpressions, createTimeDerivativeExpressions);

    while (state->defineVirtualDatabaseRPC.GetStatus() == VisItRPC::incomplete ||
           state->defineVirtualDatabaseRPC.GetStatus() == VisItRPC::warning)
    {
        state->defineVirtualDatabaseRPC.RecvReply();
    }

    // Check for an error
    if (state->defineVirtualDatabaseRPC.GetStatus() == VisItRPC::error)
    {
        RECONSTITUTE_EXCEPTION(state->defineVirtualDatabaseRPC.GetExceptionType(),
                               state->defineVirtualDatabaseRPC.Message());
    }

}

// ****************************************************************************
// Method: EngineMethods::Render
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
//
//    Brad Whitlock, Thu Apr  9 16:17:55 PDT 2009
//    I added reverse launch support.
//
// ****************************************************************************

avtDataObjectReader_p
EngineMethods::Render(bool sendZBuffer, const intVector& networkIDs,
    int annotMode, int windowID, bool leftEye,
    void (*waitCB)(void *), void *cbData)
{

    // Send a status message indicating that we're starting a scalable render
    Status("Scalable Rendering.");

    // Do it!
    state->renderRPC(networkIDs, sendZBuffer, annotMode, windowID, leftEye);

    // Get the reply and update the progress bar
    while (state->renderRPC.GetStatus() == VisItRPC::incomplete ||
           state->renderRPC.GetStatus() == VisItRPC::warning)
    {
        state->renderRPC.RecvReply();

        // Send a warning message if the status is a warning.
        if(state->renderRPC.GetStatus() == VisItRPC::incomplete)
        {
            // Send a status message.
            Status(state->renderRPC.GetPercent(), state->renderRPC.GetCurStageNum(),
                   state->renderRPC.GetCurStageName(), state->renderRPC.GetMaxStageNum());
        }
        else if(state->renderRPC.GetStatus() == VisItRPC::warning)
        {
            debug4 << "Warning: " << state->renderRPC.Message().c_str() << endl;
            Warning(state->renderRPC.Message().c_str());
        }
    }

    // Check for abort
    if (state->renderRPC.GetStatus() == VisItRPC::abort)
    {
        ClearStatus();
        EXCEPTION0(AbortException);
    }
    // Check for an error
    if (state->renderRPC.GetStatus() == VisItRPC::error)
    {
        RECONSTITUTE_EXCEPTION(state->renderRPC.GetExceptionType(),
                               state->renderRPC.Message());
    }

    // Send a status message that indicates the output of the engine is
    // being transferred across the network.
    if (sendZBuffer)
       Status("Reading engine output [with zbuffer]");
    else
       Status("Reading engine output.");

    // Read the VTK data
    long size = state->renderRPC.GetReplyLen();
    char *buf = new char[size];

    if(engineP != NULL)
    {
        if (engineP->GetReadConnection(1)->DirectRead((unsigned char *)buf, size) < 0) 
        {
            debug1 << "Error reading VTK data!!!!\n";
        }
    }
    else if(component != NULL)
    {
        if (component->GetWriteConnection(1)->DirectRead((unsigned char *)buf, size) < 0) 
        {
            debug1 << "Error reading VTK data!!!!\n";
        }
    }

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
// Method: EngineMethods::GetStatusAttributes
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
EngineMethods::GetStatusAttributes() const
{
    return state->statusAtts;
}

// ****************************************************************************
// Method: EngineMethods::Status
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
EngineMethods::Status(const char *message)
{
    state->statusAtts->SetClearStatus(false);
    state->statusAtts->SetMessageType(1);
    state->statusAtts->SetStatusMessage(message);
    state->statusAtts->Notify();
}

// ****************************************************************************
// Method: EngineMethods::Status
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
EngineMethods::Status(int percent, int curStage, const std::string &curStageName,
   int maxStage)
{
    state->statusAtts->SetClearStatus(false);
    state->statusAtts->SetMessageType(2);
    state->statusAtts->SetPercent(percent);
    state->statusAtts->SetCurrentStage(curStage);
    state->statusAtts->SetCurrentStageName(curStageName);
    state->statusAtts->SetMaxStage(maxStage);
    state->statusAtts->Notify();
}

// ****************************************************************************
// Method: EngineMethods::Warning
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
EngineMethods::Warning(const char *message)
{
    state->statusAtts->SetClearStatus(false);
    state->statusAtts->SetMessageType(3);
    state->statusAtts->SetStatusMessage(message);
    state->statusAtts->Notify();
}

// ****************************************************************************
// Method: EngineMethods::ClearStatus
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
EngineMethods::ClearStatus()
{
    state->statusAtts->SetClearStatus(true);
    state->statusAtts->Notify();
}

// ****************************************************************************
//  Method:  EngineMethods::Pick
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
EngineMethods::Pick(const int nid, const PickAttributes *atts,
                  PickAttributes &retAtts, int wid)
{
    retAtts = state->pickRPC(nid, atts, wid);

    if (state->pickRPC.GetStatus() == VisItRPC::error)
    {
        RECONSTITUTE_EXCEPTION(state->pickRPC.GetExceptionType(),
                             state->pickRPC.Message());
    }
}


// ****************************************************************************
//  Method:  EngineMethods::StartPick
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
EngineMethods::StartPick(const bool forZones, const bool flag, const int nid)
{

    state->startPickRPC(forZones, flag, nid);

    if (state->startPickRPC.GetStatus() == VisItRPC::error)
    {
        RECONSTITUTE_EXCEPTION(state->startPickRPC.GetExceptionType(),
                             state->startPickRPC.Message());
    }
}


// ****************************************************************************
//  Method:  EngineMethods::StartQuery
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
EngineMethods::StartQuery(const bool flag, const int nid)
{

    state->startQueryRPC(flag, nid);

    if (state->startQueryRPC.GetStatus() == VisItRPC::error)
    {
        RECONSTITUTE_EXCEPTION(state->startQueryRPC.GetExceptionType(),
                             state->startQueryRPC.Message());
    }
}


// ****************************************************************************
//  Method:  EngineMethods::Query
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
EngineMethods::Query(const std::vector<int> &nid, const QueryAttributes *atts,
                   QueryAttributes &retAtts)
{
    state->queryRPC(nid, atts);

    // Get the reply and update the progress bar
    while (state->queryRPC.GetStatus() == VisItRPC::incomplete ||
           state->queryRPC.GetStatus() == VisItRPC::warning)
    {
        state->queryRPC.RecvReply();

        // Send a warning message if the status is a warning.
        if(state->queryRPC.GetStatus() == VisItRPC::incomplete)
        {
            // Send a status message.
            Status(state->queryRPC.GetPercent(), state->queryRPC.GetCurStageNum(),
                   state->queryRPC.GetCurStageName(), state->queryRPC.GetMaxStageNum());
        }
        else if(state->queryRPC.GetStatus() == VisItRPC::warning)
        {
            debug4 << "Warning: " << state->queryRPC.Message().c_str() << endl;
            Warning(state->queryRPC.Message().c_str());
        }
    }

    // Check for abort
    if (state->queryRPC.GetStatus() == VisItRPC::abort)
    {
        ClearStatus();
        EXCEPTION0(AbortException);
    }

    // Check for an error
    if (state->queryRPC.GetStatus() == VisItRPC::error)
    {
        RECONSTITUTE_EXCEPTION(state->queryRPC.GetExceptionType(),
                               state->queryRPC.Message());
    }
    retAtts = state->queryRPC.GetReturnAtts();
    ClearStatus();
}


// ****************************************************************************
//  Method:  EngineMethods::GetQueryParameters
//
//  Purpose: Gets query parameters for the named query.
//
//  Programmer:  Kathleen Biagas
//  Creation:    July 15, 2011
//
// ****************************************************************************

std::string
EngineMethods::GetQueryParameters(const std::string &qName)
{
    std::string params = state->queryParametersRPC(qName);

    return params;
}


// ****************************************************************************
//  Method:  EngineMethods::GetProcInfo
//
//  Purpose: Gets unix process information from the engine
//
//  Programmer:  Mark C. Miller
//  Creation:    November 15, 2004
//
// ****************************************************************************

void
EngineMethods::GetProcInfo(ProcessAttributes &retAtts)
{
    state->procInfoRPC();

    // Get the reply and update the progress bar
    while (state->procInfoRPC.GetStatus() == VisItRPC::incomplete ||
           state->procInfoRPC.GetStatus() == VisItRPC::warning)
    {
        state->procInfoRPC.RecvReply();
    }

    // Check for abort
    if (state->procInfoRPC.GetStatus() == VisItRPC::abort)
    {
        ClearStatus();
        EXCEPTION0(AbortException);
    }

    // Check for an error
    if (state->procInfoRPC.GetStatus() == VisItRPC::error)
    {
        RECONSTITUTE_EXCEPTION(state->procInfoRPC.GetExceptionType(),
                               state->procInfoRPC.Message());
    }

    retAtts = state->procInfoRPC.GetReturnAtts();

}

// ****************************************************************************
//  Method:  EngineMethods::ReleaseData
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
EngineMethods::ReleaseData(const int id)
{
    state->releaseDataRPC(id);
    if (state->releaseDataRPC.GetStatus() == VisItRPC::error)
    {
        RECONSTITUTE_EXCEPTION(state->releaseDataRPC.GetExceptionType(),
                               state->releaseDataRPC.Message());
    }
}

// ****************************************************************************
//  Method:  EngineMethods::CloneNetwork
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
EngineMethods::CloneNetwork(const int id, const QueryOverTimeAttributes *qa)
{
    state->cloneNetworkRPC(id, qa);
    if (state->cloneNetworkRPC.GetStatus() == VisItRPC::error)
    {
        RECONSTITUTE_EXCEPTION(state->cloneNetworkRPC.GetExceptionType(),
                               state->cloneNetworkRPC.Message());
    }
}

// ****************************************************************************
// Method: EngineMethods::BlockForNamedSelectionOperation
//
// Purpose:
//   Block for named selection operations.
//
// Note:       This code is common to the various methods that use the
//             namedSelectionRPC to do operations. It's necessary since I
//             changed the namedSelectionRPC to be non-blocking so we could
//             get progress updates.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun  8 16:42:49 PDT 2011
//
// Modifications:
//
// ****************************************************************************

void
EngineMethods::BlockForNamedSelectionOperation()
{
    // Get the reply and update the progress bar
    while (state->namedSelectionRPC.GetStatus() == VisItRPC::incomplete ||
           state->namedSelectionRPC.GetStatus() == VisItRPC::warning)
    {
        state->namedSelectionRPC.RecvReply();

        // Send a warning message if the status is a warning.
        if(state->namedSelectionRPC.GetStatus() == VisItRPC::incomplete)
        {
            // Send a status message.
            Status(state->namedSelectionRPC.GetPercent(), state->namedSelectionRPC.GetCurStageNum(),
                   state->namedSelectionRPC.GetCurStageName(), state->namedSelectionRPC.GetMaxStageNum());
        }
        else if(state->namedSelectionRPC.GetStatus() == VisItRPC::warning)
        {
            debug4 << "Warning: " << state->namedSelectionRPC.Message().c_str() << endl;
            Warning(state->namedSelectionRPC.Message().c_str());
        }
    }

    ClearStatus();

    // Check for abort
    if (state->namedSelectionRPC.GetStatus() == VisItRPC::abort)
    {
        EXCEPTION0(AbortException);
    }

    if (state->namedSelectionRPC.GetStatus() == VisItRPC::error)
    {
        RECONSTITUTE_EXCEPTION(state->namedSelectionRPC.GetExceptionType(),
                               state->namedSelectionRPC.Message());
    }
}

// ****************************************************************************
//  Method:  EngineMethods::CreateNamedSelection
//
//  Purpose:
//      Create a named selection.
//
//  Arguments:
//    ids        the id of the network to create the selection from.
//    props      the properties of the named selection.
//
//  Programmer:  Hank Childs
//  Creation:    January 29, 2009
//
//  Modifications:
//    Kathleen Bonnell, Wed Mar 25 15:35:32 MST 2009
//    Renamed NamedSelectionRPC enum names to compile on windows.
//
//    Brad Whitlock, Tue Dec 14 12:00:36 PST 2010
//    I changed the code so we pass in the selection properties instead of
//    just the name. I made it return a selection summary.
//
// ****************************************************************************

const SelectionSummary &
EngineMethods::CreateNamedSelection(int id, const SelectionProperties &props)
{
    const SelectionSummary &s = state->namedSelectionRPC.CreateNamedSelection(id, props);

    BlockForNamedSelectionOperation();

    return s;
}

// ****************************************************************************
//  Method:  EngineMethods::UpdateNamedSelection
//
//  Purpose:
//      Update a named selection.
//
//  Arguments:
//    ids        the id of the network to create the selection from.
//    props      the properties of the named selection.
//
//  Programmer:  Brad Whitlock
//  Creation:    Wed Sep  7 14:30:38 PDT 2011
//
//  Modifications:
//
// ****************************************************************************

const SelectionSummary &
EngineMethods::UpdateNamedSelection(int id, const SelectionProperties &props, bool cache)
{
    const SelectionSummary &s = state->namedSelectionRPC.UpdateNamedSelection(id, props, cache);

    BlockForNamedSelectionOperation();

    return s;
}

// ****************************************************************************
//  Method:  EngineMethods::DeleteNamedSelection
//
//  Purpose:
//      Delete a named selection to a list of plots.
//
//  Arguments:
//    ids        the id of the network to create the selection from.
//    selName    the name of the named selection.
//
//  Programmer:  Hank Childs
//  Creation:    January 29, 2009
//
//  Modifications:
//    Kathleen Bonnell, Wed Mar 25 15:35:32 MST 2009
//    Renamed NamedSelectionRPC enum names to compile on windows.
//
// ****************************************************************************

void
EngineMethods::DeleteNamedSelection(const std::string selName)
{
    state->namedSelectionRPC.DeleteNamedSelection(selName);

    BlockForNamedSelectionOperation();
}


// ****************************************************************************
//  Method:  EngineMethods::LoadNamedSelection
//
//  Purpose:
//      Load a named selection to a list of plots.
//
//  Arguments:
//    ids        the id of the network to create the selection from.
//    selName    the name of the named selection.
//
//  Programmer:  Hank Childs
//  Creation:    January 29, 2009
//
//  Modifications:
//    Kathleen Bonnell, Wed Mar 25 15:35:32 MST 2009
//    Renamed NamedSelectionRPC enum names to compile on windows.
//
// ****************************************************************************

void
EngineMethods::LoadNamedSelection(const std::string selName)
{
    state->namedSelectionRPC.LoadNamedSelection(selName);

    BlockForNamedSelectionOperation();
}


// ****************************************************************************
//  Method:  EngineMethods::SaveNamedSelection
//
//  Purpose:
//      Save a named selection to a list of plots.
//
//  Arguments:
//    ids        the id of the network to create the selection from.
//    selName    the name of the named selection.
//
//  Programmer:  Hank Childs
//  Creation:    January 29, 2009
//
//  Modifications:
//    Kathleen Bonnell, Wed Mar 25 15:35:32 MST 2009
//    Renamed NamedSelectionRPC enum names to compile on windows.
//
// ****************************************************************************

void
EngineMethods::SaveNamedSelection(const std::string selName)
{
    state->namedSelectionRPC.SaveNamedSelection(selName);

    BlockForNamedSelectionOperation();
}


// ****************************************************************************
//  Method:  EngineMethods::ConstructDataBinning
//
//  Purpose:
//      Have the engine construct a derived data function.
//
//  Arguments:
//    id         the id of the network to be cloned.
//    atts       the attributes to construct the data binning
//
//  Programmer:  Hank Childs
//  Creation:    February 13, 2006
//
//  Modifications:
//
//    Hank Childs, Sat Aug 21 14:35:47 PDT 2010
//    Rename DDF to DataBinning.
//
// ****************************************************************************

void
EngineMethods::ConstructDataBinning(const int id, const ConstructDataBinningAttributes *atts)
{
    state->constructDataBinningRPC(id, atts);
    if (state->constructDataBinningRPC.GetStatus() == VisItRPC::error)
    {
        RECONSTITUTE_EXCEPTION(state->constructDataBinningRPC.GetExceptionType(),
                               state->constructDataBinningRPC.Message());
    }
}


// ****************************************************************************
//  Method:  EngineMethods::ExportDatabase
//
//  Purpose:
//      Have the engine export a database.
//
//  Arguments:
//    id         the id of the network to be cloned.
//    atts       the attributes to export the database.
//    timeSuffix a time state string that will be put into the exported filename.
//
//  Programmer:  Hank Childs
//  Creation:    May 26, 2005
//
//  Modifications:
//    Brad Whitlock, Fri Jan 24 16:37:14 PST 2014
//    Allow more than one network.
//    Work partially supported by DOE Grant SC0007548.
//
//    Brad Whitlock, Thu Jul 24 22:18:34 EDT 2014
//    Pass timeSuffix.
//
// ****************************************************************************

void
EngineMethods::ExportDatabases(const intVector &ids, const ExportDBAttributes &atts,
    const std::string &timeSuffix)
{
    state->exportDatabaseRPC(ids, atts, timeSuffix);
    if (state->exportDatabaseRPC.GetStatus() == VisItRPC::error)
    {
        RECONSTITUTE_EXCEPTION(state->exportDatabaseRPC.GetExceptionType(),
                               state->exportDatabaseRPC.Message());
    }
}

// ****************************************************************************
//  Method:  EngineMethods::UpdateExpressions
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
EngineMethods::UpdateExpressions(const ExpressionList &expressions)
{
    if (state->exprList != expressions)
    {
        state->exprList = expressions;
        state->exprList.Notify();
    }
}


// ****************************************************************************
//  Method:  EngineMethods::ExecuteSimulationControlCommand
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
EngineMethods::ExecuteSimulationControlCommand(const std::string &cmd)
{
    state->simulationCommandRPC(cmd, "");
}

// ****************************************************************************
//  Method:  EngineMethods::ExecuteSimulationControlCommand
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
EngineMethods::ExecuteSimulationControlCommand(const std::string &cmd,
                                             const std::string &arg)
{
    state->simulationCommandRPC(cmd, arg);
}

// ****************************************************************************
//  Method:  EngineMethods::SetDefaultFileOpenOptions
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
EngineMethods::SetDefaultFileOpenOptions(const FileOpenOptions &opts)
{
    state->setEFileOpenOptionsRPC(opts);
}

// ****************************************************************************
//  Method:  EngineMethods::SetPrecisionType
//
//  Purpose:
//    Tells the engine about the latest user-requested precision.
//
//  Arguments:
//    pType      The new precision.
//
//  Programmer:  Kathleen Biagas
//  Creation:    August 7, 2013
//
// ****************************************************************************

void
EngineMethods::SetPrecisionType(const int pType)
{
    state->setPrecisionTypeRPC(pType);
}

// ****************************************************************************
//  Method:  EngineMethods::SetBackendType
//
//  Purpose:
//    Tells the engine about the latest user-requested backend.
//
//  Arguments:
//    bType      The new backend.
//
//  Programmer:  Cameron Christensen
//  Creation:    June 10, 2014
//
// ****************************************************************************

void
EngineMethods::SetBackendType(const int bType)
{
    state->setBackendTypeRPC(bType);
}

// ****************************************************************************
// Method: EngineMethods::GetEngineProperties
//
// Purpose:
//   Return engine properties.
//
// Returns:    The engine properties.
//
// Note:       The number of nodes is retained from the command line used to
//             initiate the engine connection.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 10 12:16:05 PDT 2011
//
// Modifications:
//
// ****************************************************************************

EngineProperties
EngineMethods::GetEngineProperties()
{
    state->enginePropertiesRPC();

    // Get the reply and update the progress bar
    while (state->enginePropertiesRPC.GetStatus() == VisItRPC::incomplete ||
           state->enginePropertiesRPC.GetStatus() == VisItRPC::warning)
    {
        state->enginePropertiesRPC.RecvReply();
    }

    // Check for abort
    if (state->enginePropertiesRPC.GetStatus() == VisItRPC::abort)
    {
        ClearStatus();
        EXCEPTION0(AbortException);
    }

    // Check for an error
    if (state->enginePropertiesRPC.GetStatus() == VisItRPC::error)
    {
        RECONSTITUTE_EXCEPTION(state->enginePropertiesRPC.GetExceptionType(),
                               state->enginePropertiesRPC.Message());
    }

    EngineProperties props(state->enginePropertiesRPC.GetReturnAtts());
    if(numNodes > props.GetNumNodes())
        props.SetNumNodes(numNodes);

    return props;
}

// ****************************************************************************
// Method: EngineMethods::LaunchProcess
//
// Purpose:
//   Make the engine capable of launching a new process.
//
// Arguments:
//   args : The program arguments.
//
// Returns:
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 28 16:26:06 PST 2011
//
// Modifications:
//
// ****************************************************************************

void
EngineMethods::LaunchProcess(const stringVector &args)
{
    state->launchRPC(args);
}


// ****************************************************************************
//  Method:  EngineMethods::SetRemoveDuplicateNodes
//
//  Purpose:
//    Tells the engine about the removeDuplicateNodes.
//
//  Arguments:
//    flag      The new removeDuplicateNodes flag.
//
//  Programmer:  Kathleen Biagas
//  Creation:    December 22, 2014
//
// ****************************************************************************

void
EngineMethods::SetRemoveDuplicateNodes(bool flag)
{
    state->setRemoveDuplicateNodesRPC(flag);
}
