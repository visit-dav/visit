#if !defined(_WIN32)
#include <strings.h>
#include <unistd.h>      // for alarm()
#endif
#include <iostream.h>

#include <ColorTableAttributes.h>
#include <ParentProcess.h>
#include <BufferConnection.h>
#include <SocketConnection.h>
#include <AbortException.h>
#include <CouldNotConnectException.h>
#include <DefineVirtualDatabaseRPC.h>
#include <LostConnectionException.h>
#include <IncompatibleVersionException.h>
#include <QuitRPC.h>
#include <ReadRPC.h>
#include <RenderRPC.h>
#include <ExecuteRPC.h>
#include <ApplyOperatorRPC.h>
#include <ApplyNamedFunctionRPC.h>
#include <DefineVirtualDatabaseRPC.h>
#include <ClearCacheRPC.h>
#include <SetFinalVariableNameRPC.h>
#include <UpdatePlotAttsRPC.h>
#include <UseNetworkRPC.h>
#include <OpenDatabaseRPC.h>
#include <PickRPC.h>
#include <QueryRPC.h>
#include <ReleaseDataRPC.h>
#include <StartPickRPC.h>
#include <SetWindowAttsRPC.h>
#include <MakePlotRPC.h>
#include <RPCExecutor.h>
#include <DebugStream.h>
#include <Init.h>
#include <InitVTK.h>
#include <QueryAttributes.h>

#include <avtStreamer.h>
#include <avtCallback.h>
#include <avtColorTables.h>
#include <avtDataset.h>
#include <avtDataObjectReader.h>
#include <avtDataObjectString.h>
#include <avtDataObjectWriter.h>
#include <avtFilter.h>
#include <avtVariableMapper.h>
#include <avtOriginatingSink.h>
#include <avtPlot.h>
#include <avtTerminatingSource.h>
#include <avtDataObjectQuery.h>
#include <DatabasePluginManager.h>
#include <LoadBalancer.h>
#include <NetworkManager.h>
#include <PlotPluginManager.h>
#include <PlotPluginInfo.h>
#include <OperatorPluginManager.h>
#include <OperatorPluginInfo.h>
#include <VisItException.h>
#include <TimingsManager.h>
#include <ImproperUseException.h>
#include <snprintf.h>

#include <vtkDataSetWriter.h>

#include <fstream.h>

#include <string>
using std::string;

#ifdef PARALLEL
#include <parallel.h>
#else
#include <Xfer.h>
#endif

// Prototypes.
void DefineColorTables(const ColorTableAttributes &ct);
void ProcessCommandLine(int argc, char *argv[]);
bool EventLoop(Xfer &xfer, QuitRPC &quit);
void WriteData(NonBlockingRPC *rpc, avtDataObjectWriter_p &polyData);
void ProcessInput(Xfer &xfer);
bool EngineAbortCallback(void *);
bool EngineAbortCallbackParallel(void *, bool);
void EngineUpdateProgressCallback(void *, const char *, const char *, int,int);
void EngineInitializeProgressCallback(void *, int);
void EngineWarningCallback(void *, const char *);
static void ResetTimeout(int timeout);
static bool ConnectViewer(ParentProcess &viewer, int *argc, char **argv[]);

#if !defined(_WIN32)
static void AlarmHandler(int signal);
#endif

Connection *vtkConnection = 0;
bool noFatalExceptions = true;
static int timeout = 0;

// The destination machine's type representation.
TypeRepresentation destinationFormat;

// Here's the network manager!
NetworkManager *netmgr;

#if defined(_WIN32)
// Get around a macro problem
#define GetMessageA GetMessage
#endif

// Initial connection timeout of 5 minutes (300 seconds)
#define INITIAL_CONNECTION_TIMEOUT 60

// ****************************************************************************
//  Method: RPCExecutor<QuitRPC>::Execute
//
//  Purpose:
//      Execute a QuitRPC.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 29, 2000
//
//  Modifications:
//
//    Hank Childs, Wed Jan  9 14:02:15 PST 2002
//    Add a call to ClearAllNetworks when debugging memory leaks.
//
// ****************************************************************************
template<>
void
RPCExecutor<QuitRPC>::Execute(QuitRPC *rpc)
{
    debug2 << "Executing QuitRPC" << endl;
    if (!rpc->GetQuit())
        rpc->SendError();
    else
    {
#ifdef DEBUG_MEMORY_LEAKS
        netmgr->ClearAllNetworks();
#endif
        rpc->SendReply();
    }
}

// ****************************************************************************
//  Method: RPCExecutor<ReadRPC>::Execute
//
//  Purpose:
//      Execute a ReadRPC.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 29, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Fri Apr 27 10:43:22 PDT 2001
//    Added try-catch block.
//
//    Hank Childs, Wed Jun 13 10:35:57 PDT 2001
//    Added SIL restriction when creating a network.
//
//    Brad Whitlock, Mon Oct 22 18:33:37 PST 2001
//    Changed the exception keywords to macros.
//
//    Hank Childs, Fri Dec 14 17:39:36 PST 2001
//    Use a more compact form of sil restriction.
//
//    Jeremy Meredith, Thu Oct 24 16:15:11 PDT 2002
//    Added material options.
//
// ****************************************************************************
template<>
void
RPCExecutor<ReadRPC>::Execute(ReadRPC *rpc)
{
    debug2 << "Executing ReadRPC" << endl;
    TRY
    {
        netmgr->AddDB(rpc->GetFile(), rpc->GetVar(), rpc->GetTime(),
                     rpc->GetCSRAttributes(), rpc->GetMaterialAttributes());
        rpc->SendReply();
    }
    CATCH2(VisItException, e)
    {
        rpc->SendError(e.GetMessage(), e.GetExceptionType());
    }
    ENDTRY
}

// ****************************************************************************
//  Method: RPCExecutor<PrepareOperatorRPC>::Execute
//
//  Purpose:
//      Allocate space for the attributes for an ApplyOperatorRPC.
//
//  Programmer: Jeremy Meredith
//  Creation:   March  4, 2001
//
//  Modifications:
//    Kathleen Bonnell, Fri Apr 27 10:43:22 PDT 2001
//    Added try-catch block.
//
//    Jeremy Meredith, Thu Jul 26 03:35:59 PDT 2001
//    Added support for the new (real) operator plugin manager.
//
//    Jeremy Meredith, Fri Sep 28 13:39:51 PDT 2001
//    Renamed GetName to GetID to reflect its new usage.
//    Made use of plugin manager ability to key off of ids.
//
//    Brad Whitlock, Mon Oct 22 18:33:37 PST 2001
//    Changed the exception keywords to macros.
//
//    Jeremy Meredith, Thu Nov 21 11:09:47 PST 2002
//    Added check for non-existent operator.
//
//    Brad Whitlock, Thu Jan 16 11:20:31 PDT 2003
//    I replaced the return in the TRY/CATCH block with CATCH_RETURN so
//    fake exceptions work.
//
// ****************************************************************************
template<>
void
RPCExecutor<PrepareOperatorRPC>::Execute(PrepareOperatorRPC *rpc)
{
    debug2 << "Executing PrepareOperatorRPC: " << rpc->GetID().c_str() << endl;
    TRY 
    {
        string id = rpc->GetID().c_str();

        if (!OperatorPluginManager::Instance()->PluginAvailable(id))
        {
            rpc->SendError("Requested operator does not exist for the engine",
                           "VisItException");
            CATCH_RETURN(1);
        }

        rpc->GetApplyOperatorRPC()->SetAtts(OperatorPluginManager::Instance()->
            GetEnginePluginInfo(id)->AllocAttributes());
        rpc->SendReply();
    }
    CATCH2(VisItException, e)
    {
        rpc->SendError(e.GetMessage(), e.GetExceptionType());
    }
    ENDTRY
}


// ****************************************************************************
//  Method: RPCExecutor<ApplyOperatorRPC>::Execute
//
//  Purpose:
//      Execute an ApplyOperatorRPC.
//
//  Programmer: Jeremy Meredith
//  Creation:   March  2, 2001
//
//  Modifications:
//    Kathleen Bonnell, Fri Apr 27 10:43:22 PDT 2001
//    Added try-catch block.
//
//    Jeremy Meredith, Fri Sep 28 13:39:51 PDT 2001
//    Renamed GetName to GetID to reflect its new usage.
//
// ****************************************************************************
template<>
void
RPCExecutor<ApplyOperatorRPC>::Execute(ApplyOperatorRPC *rpc)
{
    debug2 << "Executing ApplyOperatorRPC: " << rpc->GetID().c_str() << endl;
    TRY
    {
        netmgr->AddFilter(rpc->GetID().c_str(), rpc->GetAtts());
        rpc->SendReply();
    }
    CATCH2(VisItException, e)
    {
        rpc->SendError(e.GetMessage(), e.GetExceptionType());
    }
    ENDTRY
}

// ****************************************************************************
//  Method: RPCExecutor<ApplyNamedFunctionRPC>::Execute
//
//  Purpose:
//      Execute an ApplyNamedFunctionRPC.
//
//  Programmer: Sean Ahern
//  Creation:   Fri Apr 19 13:34:19 PDT 2002
//
//  Modifications:
//
// ****************************************************************************
template<>
void
RPCExecutor<ApplyNamedFunctionRPC>::Execute(ApplyNamedFunctionRPC *rpc)
{
    debug2 << "Executing ApplyNamedFunctionRPC" << endl;
    TRY
    {
        netmgr->AddNamedFunction(rpc->GetName(), rpc->GetNArgs());
        rpc->SendReply();
    }
    CATCH2(VisItException, e)
    {
        rpc->SendError(e.GetMessage(), e.GetExceptionType());
    }
    ENDTRY
}

// ****************************************************************************
//  Method: RPCExecutor<SetFinalVariableNameRPC>::Execute
//
//  Purpose:
//      Execute a SetFinalVariableNameRPC.
//
//  Programmer: Sean Ahern
//  Creation:   Thu Jun 13 16:18:38 PDT 2002
//
//  Modifications:
//
// ****************************************************************************
template<>
void
RPCExecutor<SetFinalVariableNameRPC>::Execute(SetFinalVariableNameRPC *rpc)
{
    debug2 << "Executing SetFinalVariableNameRPC" << endl;
    TRY
    {
        netmgr->SetFinalVariableName(rpc->GetName());
        rpc->SendReply();
    }
    CATCH2(VisItException, e)
    {
        rpc->SendError(e.GetMessage(), e.GetExceptionType());
    }
    ENDTRY
}

// ****************************************************************************
//  Method: RPCExecutor<PreparePlotPlotRPC>::Execute
//
//  Purpose:
//      Allocate space for the attributes for a MakePlotRPC.
//
//  Programmer: Jeremy Meredith
//  Creation:   March  4, 2001
//
//  Modifications:
//    Jeremy Meredith, Tue Mar 20 12:05:50 PST 2001
//    Made it use the new plugin manager.  For now we do a reverse lookup
//    of the name to an index.
//
//    Kathleen Bonnell, Fri Apr 27 10:43:22 PDT 2001
//    Added try-catch block.
//
//    Jeremy Meredith, Fri Sep 28 13:39:51 PDT 2001
//    Renamed GetName to GetID to reflect its new usage.
//    Made use of plugin manager ability to key off of ids.
//
//    Jeremy Meredith, Thu Nov 21 11:09:35 PST 2002
//    Added check for non-existent plot.
//
//    Brad Whitlock, Thu Jan 16 11:25:22 PDT 2003
//    I replaced the return from the TRY/CATCH block with CATCH_RETURN so
//    fake exceptions work.
//
// ****************************************************************************
template<>
void
RPCExecutor<PreparePlotRPC>::Execute(PreparePlotRPC *rpc)
{
    debug2 << "Executing PreparePlotRPC: " << rpc->GetID().c_str() << endl;
    TRY
    {
        string id = rpc->GetID().c_str();

        if (!PlotPluginManager::Instance()->PluginAvailable(id))
        {
            rpc->SendError("Requested plot does not exist for the engine",
                           "VisItException");
            CATCH_RETURN(1);
        }

        rpc->GetMakePlotRPC()->SetAtts(PlotPluginManager::Instance()->
            GetEnginePluginInfo(id)->AllocAttributes());
        rpc->SendReply();
    }
    CATCH2(VisItException, e)
    {
        rpc->SendError(e.GetMessage(), e.GetExceptionType());
    }
    ENDTRY
}

// ****************************************************************************
//  Method: RPCExecutor<MakePlotRPC>::Execute
//
//  Purpose:
//      Execute a MakePlotRPC.
//
//  Programmer: Jeremy Meredith
//  Creation:   March  4, 2001
//
//  Modifications:
//    Kathleen Bonnell, Fri Apr 27 10:43:22 PDT 2001
//    Added try-catch block.
//
//    Jeremy Meredith, Fri Sep 28 13:39:51 PDT 2001
//    Renamed GetName to GetID to reflect its new usage.
//
//    Brad Whitlock, Mon Oct 22 18:33:37 PST 2001
//    Changed the exception keywords to macros.
//
// ****************************************************************************
template<>
void
RPCExecutor<MakePlotRPC>::Execute(MakePlotRPC *rpc)
{
    debug2 << "Executing MakePlotRPC: " << rpc->GetID().c_str() << endl;
    TRY 
    {
        netmgr->MakePlot(rpc->GetID().c_str(), rpc->GetAtts());
        MakePlotRPC::NetworkID id(netmgr->EndNetwork());
        rpc->SendReply(&id);
    }
    CATCH2(VisItException, e)
    {
        rpc->SendError(e.GetMessage(), e.GetExceptionType());
    }
    ENDTRY
}

// ****************************************************************************
//  Method: RPCExecutor<UseNetworkRPC>::Execute
//
//  Purpose:
//      Execute a UseNetworkRPC.
//
//  Programmer: Jeremy Meredith
//  Creation:   November  7, 2001
//
//  Modifications:
// ****************************************************************************
template<>
void
RPCExecutor<UseNetworkRPC>::Execute(UseNetworkRPC *rpc)
{
    debug2 << "Executing UseNetworkRPC: " << rpc->GetID() << endl;
    TRY 
    {
        netmgr->UseNetwork(rpc->GetID());
        rpc->SendReply();
    }
    CATCH2(VisItException, e)
    {
        rpc->SendError(e.GetMessage(), e.GetExceptionType());
    }
    ENDTRY
}

// ****************************************************************************
//  Method: RPCExecutor<PrepareUpdatePlotAttsRPC>::Execute
//
//  Purpose:
//      Allocate space for the attributes for a UpdatePlotAttsRPC.
//
//  Programmer: Hank Childs
//  Creation:   November 30, 2001
//
// ****************************************************************************
template<>
void
RPCExecutor<PrepareUpdatePlotAttsRPC>::Execute(PrepareUpdatePlotAttsRPC *rpc)
{
    debug2 << "Executing PrepareUpdatePlotAttsRPC: " << rpc->GetID().c_str() << endl;
    TRY
    {
        string id = rpc->GetID().c_str();

        if (!PlotPluginManager::Instance()->PluginAvailable(id))
        {
            rpc->SendError("Requested plot does not exist for the engine",
                           "VisItException");
            CATCH_RETURN(1);
        }

        AttributeSubject *atts = PlotPluginManager::Instance()->
                                 GetEnginePluginInfo(id)->AllocAttributes();
        rpc->GetUpdatePlotAttsRPC()->SetAtts(atts);
        rpc->SendReply();
    }
    CATCH2(VisItException, e)
    {
        rpc->SendError(e.GetMessage(), e.GetExceptionType());
    }
    ENDTRY
}

// ****************************************************************************
//  Method: RPCExecutor<UpdatePlotAttsRPC>::Execute
//
//  Purpose:
//      Execute a UpdatePlotAttsRPC.
//
//  Programmer: Hank Childs
//  Creation:   November 30, 2001
//
// ****************************************************************************
template<>
void
RPCExecutor<UpdatePlotAttsRPC>::Execute(UpdatePlotAttsRPC *rpc)
{
    debug2 << "Executing UpdatePlotAttsRPC: " << rpc->GetID().c_str() << endl;
    TRY 
    {
        int plotIndex = rpc->GetPlotIndex();
        const AttributeSubject *atts = rpc->GetAtts();
        netmgr->UpdatePlotAtts(plotIndex, atts);
        rpc->SendReply();
    }
    CATCH2(VisItException, e)
    {
        rpc->SendError(e.GetMessage(), e.GetExceptionType());
    }
    ENDTRY
}

// ****************************************************************************
//  Method: RPCExecutor<PickRPC>::Execute
//
//  Purpose:
//
//  Programmer: Kathleen Bonnell 
//  Creation:   November 20, 2001
//
// ****************************************************************************
template<>
void
RPCExecutor<PickRPC>::Execute(PickRPC *rpc)
{
    debug2 << "Executing PickRPC: " << endl; 
    TRY 
    {
        netmgr->Pick(rpc->GetNetId(), rpc->GetPickAtts());
        rpc->SendReply(rpc->GetPickAtts());
    }
    CATCH2(VisItException, e)
    {
        rpc->SendError(e.GetMessage(), e.GetExceptionType());
    }
    ENDTRY
}


// ****************************************************************************
//  Method: RPCExecutor<StartPickRPC>::Execute
//
//  Purpose:
//
//  Programmer: Kathleen Bonnell 
//  Creation:   November 26, 2001
//
// ****************************************************************************
template<>
void
RPCExecutor<StartPickRPC>::Execute(StartPickRPC *rpc)
{
    debug2 << "Executing StartPickRPC: " << endl; 

    TRY 
    {
        if (rpc->GetStartFlag())
            netmgr->StartPickMode();
        else
            netmgr->StopPickMode();
        rpc->SendReply();
    }
    CATCH2(VisItException, e)
    {
        rpc->SendError(e.GetMessage(), e.GetExceptionType());
    }
    ENDTRY
}

// ****************************************************************************
//  Method: RPCExecutor<SetWindowAttsRPC>::Execute
//
//  Purpose:
//      Execute a SetWindowAttsRPC.
//
//  Programmer: Jeremy Meredith
//  Creation:   November  8, 2001
//
//  Modifications:
//    Brad Whitlock, Mon Dec 2 13:46:49 PST 2002
//    I added a method call to populate the color tables.
//
// ****************************************************************************
template<>
void
RPCExecutor<SetWindowAttsRPC>::Execute(SetWindowAttsRPC *rpc)
{
    debug2 << "Executing SetWindowAttsRPC "
           << rpc->GetWindowAtts().GetSize()[0] << "x"
           << rpc->GetWindowAtts().GetSize()[1] << endl;
    TRY 
    {
        DefineColorTables(rpc->GetWindowAtts().GetColorTables());
        netmgr->SetWindowAttributes(rpc->GetWindowAtts());
        rpc->SendReply();
    }
    CATCH2(VisItException, e)
    {
        rpc->SendError(e.GetMessage(), e.GetExceptionType());
    }
    ENDTRY
}


// ****************************************************************************
//  Method: RPCExecutor<ExecuteRPC>::Execute
//
//  Purpose:
//      Execute an ExecuteRPC.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 29, 2000
//
//  Modifications:
//    Kathleen Bonnell, Tue Nov  7 16:11:40 PST 2000
//    Made netmgr add the relevant points filter.
//
//    Jeremy Meredith, Thu Mar  1 13:55:26 PST 2001
//    Removed the facelist filter.  Plots now know how to add one themselves.
//
//    Brad Whitlock, Fri Mar 16 12:33:58 PDT 2001
//    Added code that catches the ImproperUseException that can be output by
//    NetworkManager::GetOutput and sets the global noFatalExceptions
//    variable to false to terminate the event loops.
//
//    Brad Whitlock, Tue May 1 13:15:17 PST 2001
//    Added code to send back an initial status.
//
//    Kathleen Bonnell, Tue May  1 16:57:02 PDT 2001 
//    Added code to catch generic VisItExceptions and send error via rpc. 
//
//    Jeremy Meredith, Fri Jun 29 14:50:37 PDT 2001
//    Added progress reporting.
//
//    Jeremy Meredith, Tue Jul  3 15:11:22 PDT 2001
//    Added abort capability.
//
//    Hank Childs, Mon Aug 13 15:14:50 PDT 2001
//    Changed location of progress/abort callbacks from avtFilter to
//    avtDataObjectSource.
//
//    Jeremy Meredith, Thu Sep 20 01:00:59 PDT 2001
//    Added registration of the progress callback with the LoadBalancer.
//
//    Hank Childs, Thu Oct 18 16:41:18 PDT 2001
//    Register a warning callback.
//
//    Brad Whitlock, Mon Oct 22 18:33:37 PST 2001
//    Changed the exception keywords to macros.
//
//    Eric Brugger, Mon Nov  5 13:50:49 PST 2001
//    Modified to always compile the timing code.
//
//    Hank Childs, Wed Sep 11 09:35:59 PDT 2002
//    Release the data from old networks.
//
//    Kathleen Bonnell, Mon Sep 30 14:38:33 PDT 2002 
//    Removed call to DoneWithNetwork that releases data.  This is now
//    handled by the ReleaseDataRPC. 
//
//    Hank Childs, Mon Dec  2 11:31:27 PST 2002
//    Prevent UMR when the operation was interrupted or failed.
//
// ****************************************************************************
template<>
void
RPCExecutor<ExecuteRPC>::Execute(ExecuteRPC *rpc)
{
    int gettingData = visitTimer->StartTimer();
    int writingData = -1;

    avtDataObjectSource::RegisterProgressCallback(EngineUpdateProgressCallback,
                                        (void*)rpc);
    LoadBalancer::RegisterProgressCallback(EngineUpdateProgressCallback,
                                        (void*)rpc);
    avtTerminatingSource::RegisterInitializeProgressCallback(
                                       EngineInitializeProgressCallback,
                                       (void*)rpc);
    avtCallback::RegisterWarningCallback(EngineWarningCallback, (void*)rpc);

    debug2 << "Executing ExecuteRPC with respondWithNullDataObject = " <<
       rpc->GetRespondWithNull() << endl;
    TRY
    {
        // Get the output of the network manager. This does the job of
        // executing the network.
        avtDataObjectWriter_p writer = netmgr->GetOutput(rpc->GetRespondWithNull());

        visitTimer->StopTimer(gettingData, "Executing network");
        writingData = visitTimer->StartTimer();

        // Send the data back to the viewer.
        WriteData(rpc, writer);
    }
    CATCH(ImproperUseException)
    {
        noFatalExceptions = false;
    }
    CATCH(AbortException)
    {
        rpc->SendAbort();
    }
    CATCH2(VisItException, e)
    {
        rpc->SendError(e.GetMessage(), e.GetExceptionType());
    }
    ENDTRY

    avtDataObjectSource::RegisterProgressCallback(EngineUpdateProgressCallback,
                                                  NULL);
    LoadBalancer::RegisterProgressCallback(EngineUpdateProgressCallback,
                                           NULL);
    avtTerminatingSource::RegisterInitializeProgressCallback(
                                       EngineInitializeProgressCallback, NULL);

    if (writingData >= 0)
    {
        visitTimer->StopTimer(writingData, "Writing data to viewer");
    }
    visitTimer->DumpTimings();
}

// ****************************************************************************
//  Method: RPCExecutor<ClearCacheRPC>::Execute
//
//  Purpose:
//      Execute a ClearCacheRPC.
//
//  Notes:      At present, this RPC executor forces the network manager to
//              clear all networks. This is not a good thing to do because
//              it potentially wastes a lot of work. Someone more knowledgeable
//              about the engine should make it clear the networks that use
//              the specified database and make the engine re-open the
//              database since it could have changed on disk if we're
//              doing this RPC.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Jul 30 13:10:26 PST 2002
//
//  Modifications:
//
// ****************************************************************************
template<>
void
RPCExecutor<ClearCacheRPC>::Execute(ClearCacheRPC *rpc)
{
    debug2 << "Executing ClearCacheRPC: file = " 
           << rpc->GetDatabaseName().c_str() << endl;
    TRY 
    {
        if(rpc->GetClearAll())
            netmgr->ClearAllNetworks();
        else
        {
            // Fix me. Make it only clear info related to the specified db and
            // also make it reopen the database.
            netmgr->ClearAllNetworks();
        }

        rpc->SendReply();
    }
    CATCH2(VisItException, e)
    {
        rpc->SendError(e.GetMessage(), e.GetExceptionType());
    }
    ENDTRY
}

 
// ****************************************************************************
//  Method: RPCExecutor<QueryRPC>::Execute
//
//  Purpose:  Execute a query.
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 16, 2002
//
// ****************************************************************************
template<>
void
RPCExecutor<QueryRPC>::Execute(QueryRPC *rpc)
{
    debug2 << "Executing QueryRPC: " << endl;

    avtDataObjectQuery::RegisterProgressCallback(EngineUpdateProgressCallback,
                                                  (void*) rpc);
    avtDataObjectSource::RegisterProgressCallback(EngineUpdateProgressCallback,
                                                  (void*) rpc);
    LoadBalancer::RegisterProgressCallback(EngineUpdateProgressCallback,
                                           (void*) rpc);
    avtDataObjectQuery::RegisterInitializeProgressCallback(
                                       EngineInitializeProgressCallback, 
                                       (void*) rpc);

 
    TRY
    {
        netmgr->Query(rpc->GetNetworkId(), rpc->GetQueryAtts());
        rpc->SendReply(rpc->GetQueryAtts());
    }
    CATCH2(VisItException, e)
    {
        rpc->SendError(e.GetMessage(), e.GetExceptionType());
    }
    ENDTRY

    avtDataObjectQuery::RegisterProgressCallback(EngineUpdateProgressCallback,
                                                  NULL);
    avtDataObjectSource::RegisterProgressCallback(EngineUpdateProgressCallback,
                                                  NULL);
    LoadBalancer::RegisterProgressCallback(EngineUpdateProgressCallback,
                                           NULL);
    avtDataObjectQuery::RegisterInitializeProgressCallback(
                                       EngineInitializeProgressCallback, 
                                       NULL);
}


// ****************************************************************************
//  Method: RPCExecutor<ReleaseDataRPC>::Execute
//
//  Purpose:
//      Execute a ReleaseDataRPC.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 18, 2002 
//
//  Modifications:
// ****************************************************************************
template<>
void
RPCExecutor<ReleaseDataRPC>::Execute(ReleaseDataRPC *rpc)
{
    debug2 << "Executing ReleaseDataRPC: " << rpc->GetID() << endl;
    TRY 
    {
        netmgr->DoneWithNetwork(rpc->GetID());
        rpc->SendReply();
    }
    CATCH2(VisItException, e)
    {
        rpc->SendError(e.GetMessage(), e.GetExceptionType());
    }
    ENDTRY
}


// ****************************************************************************
//  Method: RPCExecutor<OpenDatabaseRPC>::Execute
//
//  Purpose:
//      Execute an OpenDatabaseRPC.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Dec 10 14:34:01 PST 2002
//
//  Modifications:
//    Jeremy Meredith, Tue Mar  4 13:07:17 PST 2003
//    Removed the return reply since OpenDatabaseRPC is now non-blocking.
//
// ****************************************************************************
template<>
void
RPCExecutor<OpenDatabaseRPC>::Execute(OpenDatabaseRPC *rpc)
{
    debug2 << "Executing OpenDatabaseRPC: db=" << rpc->GetDatabaseName().c_str()
           << ", time=" << rpc->GetTime() << endl;

    netmgr->AddDB(rpc->GetDatabaseName(), rpc->GetTime());
}

// ****************************************************************************
//  Method: RPCExecutor<DefineVirtualDatabaseRPC>::Execute
//
//  Purpose:
//      Execute a DefineVirtualDatabaseRPC.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Mar 25 13:11:38 PST 2003
//
//  Modifications:
//
// ****************************************************************************
template<>
void
RPCExecutor<DefineVirtualDatabaseRPC>::Execute(DefineVirtualDatabaseRPC *rpc)
{
    debug2 << "Executing DefineVirtualDatabaseRPC: "
           << "db=" << rpc->GetDatabaseName().c_str()
           << "path=" << rpc->GetDatabasePath().c_str()
           << ", time=" << rpc->GetTime()
           << endl;
    for(int i = 0; i < rpc->GetDatabaseFiles().size(); ++i)
        debug5 << "file["<<i<<"]="<<rpc->GetDatabaseFiles()[i].c_str() << endl;

    netmgr->DefineDB(rpc->GetDatabaseName(), rpc->GetDatabasePath(),
                     rpc->GetDatabaseFiles(), rpc->GetTime());
}

// ****************************************************************************
//  Method: RPCExecutor<RenderRPC>::Execute
//
//  Purpose:
//      Execute a RenderRPC.
//
//  Programmer: Mark C. Miller 
//  Creation:   07Apr03 
//
// ****************************************************************************
template<>
void
RPCExecutor<RenderRPC>::Execute(RenderRPC *rpc)
{
    debug2 << "Executing RenderRPC for the following plots" << endl;
    debug2 << "   ";
    for (int i = 0; i < rpc->GetIDs().size(); i++)
       debug2 << rpc->GetIDs()[i] << ", ";
    debug2 << endl;

    avtDataObjectSource::RegisterProgressCallback(NULL, NULL);
    LoadBalancer::RegisterProgressCallback(NULL, NULL);
    avtTerminatingSource::RegisterInitializeProgressCallback(NULL, NULL);
    avtCallback::RegisterWarningCallback(NULL, NULL);
    TRY 
    {
        // do the render
        avtDataObjectWriter_p writer = netmgr->Render(rpc->GetIDs(),rpc->GetSendZBuffer());

        // Send the data back to the viewer.
        WriteData(rpc, writer);
    }
    CATCH2(VisItException, e)
    {
        rpc->SendError(e.GetMessage(), e.GetExceptionType());
    }
    ENDTRY
}

// ****************************************************************************
// Function: main
//
// Purpose:
//   This is the main function for a simple "engine" that runs in
//   parallel and gets state information from the viewer and prints
//   the information that changed to the console.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 12 15:20:19 PST 2000
//
// Modifications:
//
//    Jeremy Meredith, Wed Aug  9 14:41:24 PDT 2000
//    Switched out plotAtts for plotRPC.
//
//    Jeremy Meredith, Wed Aug  9 14:41:24 PDT 2000
//    Cleaned up the way RPCs are handled.
//    Switched out quitAtts for quitRPC.
//
//    Jeremy Meredith, Thu Sep  7 13:06:10 PDT 2000
//    Added the new RPC types for doing network-style computation.
//
//    Jeremy Meredith, Fri Sep 15 16:12:56 PDT 2000
//    Added slice RPC, fixed a bug with the previous ones.
//
//    Jeremy Meredith, Thu Sep 21 22:15:06 PDT 2000
//    Made it work in parallel again.
//
//    Kathleen Bonnell, Thu Oct 12 12:50:27 PDT 2000
//    Added OnionPeelRPC.
//
//    Brad Whitlock, Fri Oct 6 11:42:56 PDT 2000
//    Removed the SocketConnections. I also added code to set the
//    destination format from the connection back to the viewer.
//
//    Hank Childs, Thu Oct 26 10:13:55 PDT 2000
//    Added initialization of exceptions.
//
//    Jeremy Meredith, Fri Nov 17 16:15:04 PST 2000
//    Removed initialization of exceptions and added general initialization.
//
//    Kathleen Bonnell, Fri Nov 17 16:33:40 PST 2000 
//    Added MatPlotRPC.
//
//    Kathleen Bonnell, Fri Dec  1 14:51:50 PST 2000 
//    Added FilledBoundaryRPC.
//
//    Jeremy Meredith, Tue Dec 12 13:50:03 PST 2000
//    Added MaterialSelectRPC.
//
//    Hank Childs, Thu Jan 11 16:45:11 PST 2001
//    Added RangeVolumePlotRPC, IsoSurfaceVolumePlotRPC.
//
//    Kathleen Bonnell, Thu Feb 22 15:08:32 PST 2001 
//    Added ContourPlotRPC.
//
//    Jeremy Meredith, Sun Mar  4 16:50:49 PST 2001
//    Ripped out all plot and operator RPCs.
//    Created two new ones:  ApplyOperator and MakePlot.
//    Added manual initialization of (a new) PluginManager.
//
//    Kathleen Bonnell, Thu Mar  8 09:01:10 PST 2001 
//    Added registration of surface plot.
//
//    Brad Whitlock, Thu Mar 15 14:35:22 PST 2001
//    Modified how input connections are supplied to the xfer object. Changed
//    calls to the event loops.
//
//    Jeremy Meredith, Tue Mar 20 12:21:07 PST 2001
//    Removed registration of all plots since we are using the new
//    PlotPluginManager.
//
//    Hank Childs, Tue Apr 24 15:25:37 PDT 2001
//    Added initialization of VTK modules.
//
//    Brad Whitlock, Wed Apr 25 17:09:37 PST 2001
//    Added code to catch IncompatibleVersionException. The handlers set the
//    noFatalExceptions variable to false which causes the engine to terminate.
//
//    Jeremy Meredith, Thu May 10 15:00:40 PDT 2001
//    Added initialization of PlotPluginManager.
//
//    Kathleen Bonnell, Mon May  7 15:58:13 PDT 2001 
//    Added registration of Erase operator. 
//    
//    Hank Childs, Sun Jun 17 18:06:53 PDT 2001
//    Removed reference to avtLoadBalancer, initialized LoadBalancer.
//
//    Hank Childs, Wed Jun 20 18:15:44 PDT 2001
//    Initialize avt filters.
//
//    Jeremy Meredith, Tue Jul  3 15:10:52 PDT 2001
//    Added xfer parameter to EngineAbortCallback.
//
//    Hank Childs, Fri Jul 20 08:09:53 PDT 2001
//    Remove MaterialSelect.
//
//    Jeremy Meredith, Tue Jul 24 14:09:27 PDT 2001
//    Removed FacelistFilter.
//
//    Jeremy Meredith, Thu Jul 26 03:36:21 PDT 2001
//    Added support for the new (real) operator plugin manager.
//
//    Hank Childs, Mon Aug 13 15:14:50 PDT 2001
//    Changed location of progress/abort callbacks from avtFilter to
//    avtDataObjectSource.
//
//    Jeremy Meredith, Thu Sep 20 01:00:59 PDT 2001
//    Added registration of the progress callback with the LoadBalancer.
//
//    Jeremy Meredith, Fri Sep 21 14:45:14 PDT 2001
//    Added registration of the abort callback with the LoadBalancer.
//
//    Jeremy Meredith, Fri Sep 28 13:41:27 PDT 2001
//    Added load of plugins since they are not loaded anymore until
//    explicity told to do so.
//
//    Brad Whitlock, Mon Oct 22 18:33:37 PST 2001
//    Changed the exception keywords to macros.
//
//    Jeremy Meredith, Fri Nov  9 10:34:08 PST 2001
//    Added UseNetworkRPC and SetWindowAttsRPC.
//
//    Hank Childs, Thu Nov 29 16:22:37 PST 2001
//    Added UpdatePlotAttsRPC.
//
//    Kathleen Bonnell, Tue Nov 20 12:35:54 PST 2001 
//    Added PickRPC StartPickRPC.
//
//    Jeremy Meredith, Wed Jan 16 10:09:45 PST 2002
//    Do initialization of plugin managers with parallel flag.
//
//    Sean Ahern, Thu Mar 21 13:18:09 PST 2002
//    Added ApplyUnaryOperatorRPC.
//
//    Sean Ahern, Fri Apr 19 14:02:34 PDT 2002
//    Removed ApplyUnaryOperatorRPC.  Added ApplyNamedFunctionRPC.
//
//    Brad Whitlock, Tue Jul 30 13:13:42 PST 2002
//    I added ClearCacheRPC.
//
//    Jeremy Meredith, Wed Aug 21 12:51:28 PDT 2002
//    I renamed some plot/operator plugin manager methods for refactoring.
//
//    Jeremy Meredith, Thu Aug 22 14:31:44 PDT 2002
//    Added database plugins.
//
//    Hank Childs, Mon Sep 30 14:26:55 PDT 2002
//    Made the network manager be allocated off the heap.  That way we can
//    control whether or not we decide to clean up the memory associated with
//    it when we exit the program (it's faster not to).
//
//    Kathleen Bonnell, Mon Sep 16 14:28:09 PDT 2002  
//    Added QueryRPC, ReleaseDataRPC.
//
//    Brad Whitlock, Mon Sep 30 09:02:35 PDT 2002
//    The code to connect to the viewer became more complex so I moved it
//    to a new function. I made the new function return a bool that tells
//    whether or not the connection to the viewer was a success. The return
//    value is used to jump over the initializing of a lot of objects so the
//    engine can terminate faster.
//
//    Mark C. Miller, Mon Nov 11 14:45:16 PST 2002
//    Added a call to ForceMesa during initialization.
//
//    Brad Whitlock, Tue Dec 10 14:33:13 PST 2002
//    I added OpenDatabaseRPC.
//
//    Jeremy Meredith, Fri Feb 28 12:21:01 PST 2003
//    Renamed LoadPlugins to LoadPluginsNow.   There is now a corresponding
//    LoadPluginsOnDemand, and I made the Plot and Operator plugin managers
//    use that method instead.  At the moment, all Database plugins need
//    to be open for the avtDatabaseFactory to determine which one to use
//    when opening a file, but I expect this to change shortly.
//
//    Brad Whitlock, Tue Mar 25 13:13:53 PST 2003
//    I added a new rpc that lets us define virtual database files.
//
//    Jeremy Meredith, Wed May  7 15:49:53 PDT 2003
//    Force static load balancing.... for now.
//
// ****************************************************************************

int
main(int argc, char *argv[])
{
    LoadBalancer::ForceStatic();

    ParentProcess theViewer;

#ifdef PARALLEL
    MPIXfer xfer;

    //
    // Initialize for MPI and get the process rank & size.
    //
    PAR_Init(argc, argv);

    //
    // Initialize error logging
    //
    Init::Initialize(argc, argv, PAR_Rank(), PAR_Size());
    Init::SetComponentName("engine");
    debug1 << "ENGINE started\n";

    //
    // Connect to the viewer.
    //
    if(ConnectViewer(theViewer, &argc, &argv))
    {
        PlotPluginManager::Initialize(PlotPluginManager::Engine, true);
        OperatorPluginManager::Initialize(OperatorPluginManager::Engine, true);
        DatabasePluginManager::Initialize(DatabasePluginManager::Engine, true);
#else
    Xfer xfer;

    //
    // Initialize error logging
    //
    Init::Initialize(argc, argv);
    Init::SetComponentName("engine");
    debug1 << "ENGINE started\n";

    //
    // Connect to the viewer.
    //
    if(ConnectViewer(theViewer, &argc, &argv))
    {
        //
        // Initialize the plugin managers.
        //
        PlotPluginManager::Initialize(PlotPluginManager::Engine, false);
        OperatorPluginManager::Initialize(OperatorPluginManager::Engine, false);
        DatabasePluginManager::Initialize(DatabasePluginManager::Engine, false);
#endif
        InitVTK::Initialize();
        InitVTK::ForceMesa();
        avtCallback::SetNowinMode(true);

        //
        // Load plugins
        //
        PlotPluginManager::Instance()->LoadPluginsOnDemand();
        OperatorPluginManager::Instance()->LoadPluginsOnDemand();
        DatabasePluginManager::Instance()->LoadPluginsNow();

        vtkConnection = theViewer.GetReadConnection(1);

        //
        // Create the network manager.
        //
        netmgr = new NetworkManager;

        // Parse the command line.
        ProcessCommandLine(argc, argv);

#if !defined(_WIN32)
        // Set up the alarm signal handler.
        signal(SIGALRM, AlarmHandler);
#endif

        // Create some RPC objects and make Xfer observe them.
        QuitRPC                  quitRPC;
        ReadRPC                  readRPC;
        ApplyOperatorRPC         applyOperatorRPC;
        ApplyNamedFunctionRPC    applyNamedFunctionRPC;
        SetFinalVariableNameRPC  setFinalVariableNameRPC;
        MakePlotRPC              makePlotRPC;
        UseNetworkRPC            useNetworkRPC;
        UpdatePlotAttsRPC        updatePlotAttsRPC;
        SetWindowAttsRPC         setWindowAttsRPC;
        PickRPC                  pickRPC;
        StartPickRPC             startPickRPC;
        ExecuteRPC               executeRPC;
        ClearCacheRPC            clearCacheRPC;
        QueryRPC                 queryRPC;
        ReleaseDataRPC           releaseDataRPC;
        OpenDatabaseRPC          openDatabaseRPC;
        DefineVirtualDatabaseRPC defineVirtualDatabaseRPC;
        RenderRPC                renderRPC;

        xfer.Add(&quitRPC);
        xfer.Add(&readRPC);
        xfer.Add(&applyOperatorRPC);
        xfer.Add(&applyNamedFunctionRPC);
        xfer.Add(&setFinalVariableNameRPC);
        xfer.Add(&makePlotRPC);
        xfer.Add(&useNetworkRPC);
        xfer.Add(&updatePlotAttsRPC);
        xfer.Add(&setWindowAttsRPC);
        xfer.Add(&pickRPC);
        xfer.Add(&startPickRPC);
        xfer.Add(&executeRPC);
        xfer.Add(&clearCacheRPC);
        xfer.Add(&queryRPC);
        xfer.Add(&releaseDataRPC);
        xfer.Add(&openDatabaseRPC);
        xfer.Add(&defineVirtualDatabaseRPC);
        xfer.Add(&renderRPC);

        // Create an object to implement the RPCs
        RPCExecutor<QuitRPC>                  quitExecutor(&quitRPC);
        RPCExecutor<ReadRPC>                  readExecutor(&readRPC);
        RPCExecutor<ApplyOperatorRPC>         applyOperatorExecutor(&applyOperatorRPC);
        RPCExecutor<PrepareOperatorRPC>       prepareOperatorExecutor(&applyOperatorRPC.GetPrepareOperatorRPC());
        RPCExecutor<ApplyNamedFunctionRPC>    applyNamedFunctionExecutor(&applyNamedFunctionRPC);
        RPCExecutor<SetFinalVariableNameRPC>  setFinalVariableNameExecutor(&setFinalVariableNameRPC);
        RPCExecutor<MakePlotRPC>              makePlotExecutor(&makePlotRPC);
        RPCExecutor<PreparePlotRPC>           preparePlotExecutor(&makePlotRPC.GetPreparePlotRPC());
        RPCExecutor<UseNetworkRPC>            useNetworkExecutor(&useNetworkRPC);
        RPCExecutor<UpdatePlotAttsRPC>        updatePlotAttsExecutor(&updatePlotAttsRPC);
        RPCExecutor<PrepareUpdatePlotAttsRPC> prepareUpdatePlotAttsExecutor(
                               &updatePlotAttsRPC.GetPrepareUpdatePlotAttsRPC());

        RPCExecutor<SetWindowAttsRPC>         setWindowAttsExecutor(&setWindowAttsRPC);
        RPCExecutor<PickRPC>                  pickRPCExecutor(&pickRPC);
        RPCExecutor<StartPickRPC>             startPickRPCExecutor(&startPickRPC);
        RPCExecutor<ExecuteRPC>               executeExecutor(&executeRPC);
        RPCExecutor<ClearCacheRPC>            clearCacheExecutor(&clearCacheRPC);
        RPCExecutor<QueryRPC>                 queryExecutor(&queryRPC);
        RPCExecutor<ReleaseDataRPC>           releaseDataExecutor(&releaseDataRPC);
        RPCExecutor<OpenDatabaseRPC>          openDatabaseExecutor(&openDatabaseRPC);
        RPCExecutor<DefineVirtualDatabaseRPC> defineVirtualDatabaseExecutor(&defineVirtualDatabaseRPC);
        RPCExecutor<RenderRPC>                renderExecutor(&renderRPC);

        //
        // Hook up the viewer connections to Xfer
        //
#ifdef PARALLEL
        if(PAR_UIProcess())
            xfer.SetInputConnection(theViewer.GetWriteConnection());
#else
        xfer.SetInputConnection(theViewer.GetWriteConnection());
#endif
        xfer.SetOutputConnection(theViewer.GetReadConnection());

        //
        // Set the global destination format. This only happens on the UI-Process
        // when running in parallel since non-UI processes have no SocketConnections.
        //
        if(theViewer.GetReadConnection() != 0)
            destinationFormat = theViewer.GetReadConnection()->GetDestinationFormat();

        //
        // Create the network manager and the load balancer.
        //
#ifdef PARALLEL
        LoadBalancer lb(PAR_Size(), PAR_Rank());
#else
        LoadBalancer lb(1, 0);
#endif
        netmgr->SetLoadBalancer(&lb);

        //
        // Initialize some callback functions.
        //
        avtDataObjectSource::RegisterAbortCallback(EngineAbortCallback, &xfer);
        avtDataObjectSource::RegisterProgressCallback(EngineUpdateProgressCallback,
                                                      NULL);
        LoadBalancer::RegisterAbortCallback(EngineAbortCallbackParallel, &xfer);
        LoadBalancer::RegisterProgressCallback(EngineUpdateProgressCallback,
                                               NULL);
        avtTerminatingSource::RegisterInitializeProgressCallback(
                                         EngineInitializeProgressCallback, NULL);

        //
        // Begin the engine's event processing loop.
        //
#ifdef PARALLEL
        PAR_EventLoop(xfer, quitRPC);
#else
        EventLoop(xfer, quitRPC);
#endif

#ifdef DEBUG_MEMORY_LEAKS
        delete visitTimer;
        delete netmgr;
#endif
    }
    else
    {
        debug1 << "The engine could not connect to the viewer." << endl;
    }

    debug1 << "ENGINE exited." << endl;
#ifdef PARALLEL
    PAR_Exit();
#endif

    return 0;
}

// ****************************************************************************
// Function: ConnectViewer
//
// Purpose: 
//   Connects to the viewer.
//
// Arguments:
//   viewer : The viewer object that we want to connect.
//
// Returns:    True if we connect to the viewer, false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 30 08:47:46 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

bool
ConnectViewer(ParentProcess &viewer, int *argc, char **argv[])
{
    // Connect to the viewer.
    TRY
    {
#ifdef PARALLEL
        viewer.Connect(argc, argv, PAR_UIProcess());
#else
        viewer.Connect(argc, argv, true);
#endif
    }
    CATCH(IncompatibleVersionException)
    {
        debug1 << "The engine has a different version than its client." << endl;
        noFatalExceptions = false;
    }
    CATCH(CouldNotConnectException)
    {
        noFatalExceptions = false;
    }
    ENDTRY

#ifdef PARALLEL
    //
    // Tell the other processes if they should exit if we had an error when
    // connecting to the viewer.
    //
    int shouldExit = noFatalExceptions ? 0 : 1;
    MPI_Bcast((void *)&shouldExit, 1, MPI_INT, 0, MPI_COMM_WORLD);
    noFatalExceptions = (shouldExit == 0);
#endif

    return noFatalExceptions;
}

#ifdef PARALLEL
// ****************************************************************************
// Function: PAR_EventLoop
//
// Purpose:
//   This is the main event loop for the engine. The master process executes
//   the serial event loop and its xfer object broadcasts the viewer's control
//   data to the xfer objects on the other processors.
//
// Notes:      
//   The MPI_BCast call for the UI process happens in the MPIXfer
//   object's process method. This was done so broadcasts are done
//   only when there are complete messages. This avoids LOTS of
//   extra broadcasts when the socket read tends to happen in small
//   increments.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 12 15:58:20 PST 2000
//
// Modifications:
//    Jeremy Meredith, Thu Sep 21 22:42:19 PDT 2000
//    Added an extern for the appropriate socket to read from.
//    Note that this is a small hack.   
//
//    Brad Whitlock, Thu Mar 15 14:32:33 PST 2001
//    Rewrote it so the master process uses the new & improved serial
//    event loop. It is also set up to tell the other processes to die if
//    we lost the connection to the viewer.
//
//    Sean Ahern, Thu Feb 21 16:18:03 PST 2002
//    Added timeout support.
//
//    Brad Whitlock, Tue Apr 9 13:43:12 PST 2002
//    Ported to Windows.
//
//    Hank Childs, Tue Jun 24 18:02:01 PDT 2003
//    Allow for timeouts during network executions.
//
// ****************************************************************************

void
PAR_EventLoop(MPIXfer &xfer, QuitRPC &quit)
{
    PAR_StateBuffer  buf;
    BufferConnection conn;

    if(PAR_UIProcess())
    {
        // The master process executes the serial event loop since it
        // communicates with the viewer.
        bool errFlag = EventLoop(xfer, quit);

        // If the errFlag is set, we exited the event loop because we lost
        // the connection to the viewer. We need to send a quit signal
        // to all other processes.
        if(errFlag || !noFatalExceptions)
        {
            quit.Write(conn);
            xfer.SetInputConnection(&conn);
            xfer.SetEnableReadHeader(false);
            xfer.Process();
        }
    }
    else
    {
        // Set the xfer object's input connection to be the buffer connection
        // that was declared at the top of this routine.
        xfer.SetInputConnection(&conn);

        // Non-UI Process
        while(!quit.GetQuit() && noFatalExceptions)
        {
            // Reset the alarm
            ResetTimeout(timeout * 60);

            // Get state information from the UI process.
            MPI_Bcast((void *)&buf, 1, PAR_STATEBUFFER, 0, MPI_COMM_WORLD);

            // We have work to do, so cancel the alarm.
            int num_seconds_in_half_hour = 30*60;
            ResetTimeout(num_seconds_in_half_hour);

            // Add the state information to the connection.
            conn.Append((unsigned char *)buf.buffer, buf.nbytes);

            // Process the state information.
            xfer.Process();

            ResetTimeout(timeout * 60);
        }
    }
}
#endif

// ****************************************************************************
// Function: EventLoop
//
// Purpose:
//   This is the serial event loop for the engine. It reads from the
//   socket attached to the viewer and processes the state information
//   that it reads.
//
// Notes:      Executes the event loop until the quit State object
//             says to quit.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 12 16:08:27 PST 2000
//
// Modifications:
//    Jeremy Meredith, Wed Aug  9 14:42:44 PDT 2000
//    Changed the read socket to a real variable.
//
//    Brad Whitlock, Thu Mar 15 13:44:41 PST 2001
//    I rewrote the event loop so it uses a call to select. This allowed me
//    to add code that tests to see if we've lost a connection. If we lost
//    the connection, quit the program.
//
//    Brad Whitlock, Mon Oct 22 18:33:37 PST 2001
//    Changed the exception keywords to macros.
//
//    Sean Ahern, Thu Feb 21 16:14:52 PST 2002
//    Added timeout support.
//
//    Brad Whitlock, Mon Mar 25 16:03:54 PST 2002
//    Made the connection and timeout code more general.
//
//    Hank Childs, Tue Jun 24 18:02:01 PDT 2003
//    Allow for timeouts during network executions.
//
// ****************************************************************************

bool
EventLoop(Xfer &xfer, QuitRPC &quit)
{
    bool errFlag = false;

    // The application's main loop
    while(!quit.GetQuit() && noFatalExceptions)
    {
        // Reset the timeout alarm
        ResetTimeout(timeout * 60);

        //
        // Block until the connection needs to be read. Then process its
        // new input.
        //
        if(xfer.GetInputConnection()->NeedsRead(true))
        {
            TRY
            {
                // We've got some work to do.  Disable the alarm.
                int num_seconds_in_half_hour = 30*60;
                ResetTimeout(num_seconds_in_half_hour);

                // Process input.
                ProcessInput(xfer);

                ResetTimeout(timeout * 60);
            }
            CATCH(LostConnectionException)
            {
                // Indicate that we want to quit the application.
                quit.SetQuit(true);
                errFlag = true;
            }
            ENDTRY
        }
    }

    return errFlag;
}

// ****************************************************************************
// Function: ProcessInput
//
// Purpose:
//   Reads socket input from the viewer and adds it to the xfer object's
//   input. After doing that, the xfer object is called upon to process its
//   input.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 15 14:08:30 PST 2001
//
// Modifications:
//   Brad Whitlock, Wed Mar 20 17:53:20 PST 2002
//   I abstracted the read code.
//
// ****************************************************************************

void
ProcessInput(Xfer &xfer)
{    
    // Try reading from the viewer.  
    int amountRead = xfer.GetInputConnection()->Fill();

    // If we got input, process it. Otherwise, start counting errors.
    if(amountRead > 0)
    {
        // Process the new information.
        xfer.Process();
    }
}

// ****************************************************************************
// Function: ProcessCommandLine
//
// Purpose:
//   Reads the command line arguments for the engine.
//
// Programmer: Jeremy Meredith
// Creation:   September 21, 2001
//
// Modifications:
//   Eric Brugger, Wed Nov  7 12:40:56 PST 2001
//   I added the command line argument -timing.
//
//   Sean Ahern, Thu Feb 21 16:12:43 PST 2002
//   Added timeout support.
//
//   Sean Ahern, Tue Dec  3 09:58:28 PST 2002
//   Added -dump support for streamer debugging.
//
//   Hank Childs, Mon May 12 19:44:50 PDT 2003
//   Add support for -lb-block, -lb-stride, and -lb-random.
//
// ****************************************************************************
void
ProcessCommandLine(int argc, char **argv)
{
    // process arguments.
    for (int i=1; i<argc; i++)
    {
        if (strcmp(argv[i], "-forcestatic") == 0)
            LoadBalancer::ForceStatic();
        else if (strcmp(argv[i], "-forcedynamic") == 0)
            LoadBalancer::ForceDynamic();
        else if (strcmp(argv[i], "-timing") == 0)
            visitTimer->Enable();
        else if (strcmp(argv[i], "-timeout") == 0)
        {
            timeout = atol(argv[i+1]);
            i++;
        }
        else if (strcmp(argv[i], "-dump") == 0)
        {
            avtStreamer::DebugDump(true);
        }
        else if (strcmp(argv[i], "-lb-block") == 0)
        {
            LoadBalancer::SetScheme(LOAD_BALANCE_CONTIGUOUS_BLOCKS_TOGETHER);
        }
        else if (strcmp(argv[i], "-lb-stride") == 0)
        {
            LoadBalancer::SetScheme(LOAD_BALANCE_STRIDE_ACROSS_BLOCKS);
        }
        else if (strcmp(argv[i], "-lb-random") == 0)
        {
            LoadBalancer::SetScheme(LOAD_BALANCE_RANDOM_ASSIGNMENT);
        }
    }
}

// ****************************************************************************
// Function: AlarmHandler
//
// Purpose:
//   Gracefully exits the engine if an SIGALRM signal was received.
//
// Programmer: Sean Ahern
// Creation:   Thu Feb 21 16:13:43 PST 2002
//
// Modifications:
//   Brad Whitlock, Tue Apr 9 13:46:32 PST 2002
//   Disabled on Windows.
//
// ****************************************************************************
#if !defined(_WIN32)
void
AlarmHandler(int signal)
{
    debug1 << "ENGINE exited due to an inactivity timeout of "
           << timeout << " minutes." << endl;
    exit(0);
}
#endif

// ****************************************************************************
// Function: WriteData
//
// Purpose:
//   Writes a vtkDataSet object back to the viewer
//
// Notes:      
//
// Programmer: Jeremy Meredith
// Creation:   September 5, 2000
//
// Modifications:
//    Jeremy Meredith, Thu Sep 21 22:17:42 PDT 2000
//    I made this append the output from all processors in parallel
//    before sending it back to the viewer.
//
//    Jeremy Meredith, Tue Sep 26 16:41:09 PDT 2000
//    Made this use an avtDataSetWriter in serial.  Parallel is
//    temporarily broken.
//
//    Jeremy Meredith, Thu Sep 28 12:44:22 PDT 2000
//    Fixed for parallel.
//
//    Hank Childs, Tue Oct 17 09:38:12 PDT 2000
//    Made argument be a reference counted avtDataset instead of a normal
//    avtDataset.
//
//    Brad Whitlock, Wed Oct 18 14:48:29 PST 2000
//    I fixed a bug with using the ref_ptr. I also added code to set
//    the destination format in the avtDataSetWriter used by the UI-process
//    so the avtDataSet it sends back is in the right format. Note that
//    I didn't set the destination format in the non-UI processes because
//    the string created by non-UI processes is sent to the UI process
//    where it is read and resent to the viewer.
//
//    Hank Childs, Wed Oct 18 16:57:03 PDT 2000
//    Cleaned up memory leak.
//
//    Hank Childs, Wed Jan 17 11:37:36 PST 2001
//    Made input be a data object writer rather than data.
//
//    Hank Childs, Mon Feb 12 07:55:47 PST 2001
//    Fix logic for parallel.
//
//    Brad Whitlock, Tue May 1 13:45:31 PST 2001
//    Added code to send back status.
//
//    Hank Childs, Sat May 26 10:31:14 PDT 2001
//    Made use of avtDataObjectString to prevent bottlenecking at proc 0.
//
//    Jeremy Meredith, Fri Jun 29 14:50:18 PDT 2001
//    Added progress reporting, even in parallel.
//
//    Jeremy Meredith, Tue Aug 14 14:45:12 PDT 2001
//    Made the final progress update occur before the SendReply because
//    SendStatus is for reporting *incomplete* progress.  If it got sent
//    too soon, it overwrote the complete status in the viewer and 
//    the viewer never realized it was done.  Thus, it hung.
//
//    Hank Childs, Sun Sep 16 14:55:48 PDT 2001
//    Add timing information.
//
//    Hank Childs, Sun Sep 16 17:30:43 PDT 2001
//    Reflect new interface for data object strings.
//
//    Hank Childs, Mon Sep 17 11:20:10 PDT 2001
//    Have the data object reader handle deleting data strings.
//
//    Hank Childs, Mon Oct  1 11:31:27 PDT 2001
//    Stop assumptions about data objects.
//
//    Eric Brugger, Mon Nov  5 13:50:49 PST 2001
//    Modified to always compile the timing code.
//
//    Hank Childs, Mon Jan  7 19:39:32 PST 2002
//    Fix memory leak.
//
//    Brad Whitlock, Tue Mar 26 10:56:38 PDT 2002
//    Changed the communication code so it uses connection objects.
//
//    Sean Ahern, Thu Jun 13 10:51:17 PDT 2002
//    Removed broken, unused code.
//
//    Hank Childs, Sun Aug 18 21:41:29 PDT 2002
//    Removed progress message that said "Transferring Data Set" as we were
//    really waiting for the processors to synchronize.
//
//    Hank Childs (Mark Miller), Mon May 12 18:04:51 PDT 2003
//    Corrected slightly misleading debug statement.
//
//    Hank Childs, Fri May 16 16:26:03 PDT 2003
//    Detect if there was a failure in the pipeline and send a message to
//    the viewer if so.
//
//    Kathleen Bonnell, Thu Jun 12 10:57:11 PDT 2003 
//    Split timing code to time Serialization separately from write. 
//    
// ****************************************************************************
void
WriteData(NonBlockingRPC *rpc, avtDataObjectWriter_p &writer)
{
#ifdef PARALLEL
    if (PAR_UIProcess())
    {
        int collectAndWriteData = visitTimer->StartTimer();
        int collectData = visitTimer->StartTimer();

        // Send a second stage for the RPC.
        rpc->SendStatus(0,
                        rpc->GetCurStageNum(),
                        "Synchronizing",
                        rpc->GetMaxStageNum());

        avtDataObject_p ui_dob = writer->GetInput();
       
        // Create a writer to write across the network.
        avtDataObjectWriter_p networkwriter = ui_dob->InstantiateWriter();
        networkwriter->SetDestinationFormat(destinationFormat);

        if (writer->MustMergeParallelStreams())
        {
            for (int i=1; i<PAR_Size(); i++)
            {
                MPI_Status stat;
                int size;
                MPI_Recv(&size,1,MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG,
                         MPI_COMM_WORLD, &stat);
                debug5 << "receiving " << size << " bytes from MPI_SOURCE "
                       << stat.MPI_SOURCE << endl;
                char *str = new char[size];
                MPI_Recv(str,size,MPI_CHAR, stat.MPI_SOURCE, MPI_ANY_TAG,
                         MPI_COMM_WORLD, &stat);
    
                // The data object reader will delete the string.
                avtDataObjectReader *avtreader = new avtDataObjectReader;
                avtreader->Read(size, str);
                avtDataObject_p proc_i_dob = avtreader->GetOutput();

                // We can't tell the reader to read (Update) unless we tell it
                // what we want it to read.  Fortunately, we can just ask it
                // for a general specification.
                avtTerminatingSource *src = proc_i_dob->GetTerminatingSource();
                avtPipelineSpecification_p spec
                    = src->GetGeneralPipelineSpecification();
                proc_i_dob->Update(spec);

                ui_dob->Merge(*proc_i_dob);
                delete avtreader;

                rpc->SendStatus(100. * float(i)/float(PAR_Size()),
                                rpc->GetCurStageNum(),
                                "Synchronizing",
                                rpc->GetMaxStageNum());
            }
        }
        visitTimer->StopTimer(collectData, "Collecting data");

        //
        // See if there was an error on another processor.
        //
        avtDataValidity &v = ui_dob->GetInfo().GetValidity();
        if (!v.HasErrorOccurred())
        {
            int serializeData = visitTimer->StartTimer();
            networkwriter->SetInput(ui_dob);
    
            avtDataObjectString do_str;
            networkwriter->Write(do_str);
    
            rpc->SendStatus(100,
                            rpc->GetCurStageNum(),
                            "Synchronizing",
                            rpc->GetMaxStageNum());
    
            visitTimer->StopTimer(serializeData, "Serializing data for writer");

            int totalSize = do_str.GetTotalLength();
            int nStrings = do_str.GetNStrings();
            debug5 << "sending " << totalSize << " bytes in "
                   << nStrings << " DirectWrites to viewer" << endl;
            rpc->SendReply(totalSize);
            int writeData = visitTimer->StartTimer();
            if (PAR_UIProcess())
            {
                for (int i = 0 ; i < nStrings ; i++)
                {
                    int size;
                    char *str;
                    do_str.GetString(i, str, size);
                    vtkConnection->DirectWrite((const unsigned char *)str,
                                               long(size));
                }
            }
            char info[124];
            SNPRINTF(info, 124, "Writing %d bytes to socket", totalSize);     
            visitTimer->StopTimer(writeData, info);
        }
        else
        {
            rpc->SendError(v.GetErrorMessage());
        }

        char *descStr = "Collecting data and writing it to viewer";
        visitTimer->StopTimer(collectAndWriteData, descStr);
    }
    else
    {
        if (writer->MustMergeParallelStreams())
        {
            char *str;
            int   size;
            avtDataObjectString do_str;
            writer->Write(do_str);
            do_str.GetWholeString(str, size);

            MPI_Send(&size,1,MPI_INT, 0, PAR_Rank(), MPI_COMM_WORLD);
            debug5 << "sending " << size << " bytes to proc 0" << endl;
            MPI_Send(str,size,MPI_CHAR, 0, PAR_Rank(), MPI_COMM_WORLD);
        }
        else
        {
            debug5 << "not sending data to proc 0 because the data object "
                   << "does not require parallel streams." << endl;
        }
    }

#else // serial
    avtDataObject_p dob = writer->GetInput();
    avtDataValidity &v = dob->GetInfo().GetValidity();
    if (!v.HasErrorOccurred())
    {
        // Send a second stage for the RPC.
        rpc->SendStatus(0,
                        rpc->GetCurStageNum(),
                        "Transferring Data Set",
                        rpc->GetMaxStageNum());

        writer->SetDestinationFormat(destinationFormat);
        avtDataObjectString  do_str;
        writer->Write(do_str);

        rpc->SendStatus(100,
                        rpc->GetCurStageNum(),
                        "Transferring Data Set",
                        rpc->GetMaxStageNum());

        int wholeSize = do_str.GetTotalLength();
        int nStrings = do_str.GetNStrings();
        debug5 << "sending " << wholeSize << " bytes in " << nStrings 
               << " DirectWrites to viewer" << endl;
        rpc->SendReply(wholeSize);
        for (int i = 0 ; i < nStrings ; i++)
        {
            char *str;
            int   size;
            do_str.GetString(i, str, size);
            vtkConnection->DirectWrite((const unsigned char *)str, long(size));
        }
    }
    else
    {
        rpc->SendError(v.GetErrorMessage());
    }
#endif
}


// ****************************************************************************
//  Function: EngineAbortCallback
//
//  Purpose:
//      Tells filters whether or not they should abort execution.
//
//  Arguments:
//      data     the xfer object to use
//
//  Programmer:   Jeremy Meredith
//  Creation:     July 3, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Sep 20 18:28:22 PDT 2001
//    Added support for parallel interruption.
//
//    Brad Whitlock, Mon Mar 25 15:51:39 PST 2002
//    Made it more general.
//
// ****************************************************************************

bool
EngineAbortCallbackParallel(void *data, bool informSlaves)
{
    Xfer *xfer = (Xfer*)data;
    if (!xfer)
        EXCEPTION1(VisItException,
                   "EngineAbortCallback called with no Xfer set.");

#ifdef PARALLEL
    // non-ui processes must do something entirely different
    if (!PAR_UIProcess())
    {
        int flag;
        MPI_Status status;
        MPI_Iprobe(0, 456, MPI_COMM_WORLD, &flag, &status);
        if (flag)
        {
            char buf[1];
            MPI_Recv(buf, 1, MPI_CHAR, 0, 456, MPI_COMM_WORLD, &status);
            return true;
        }
        return false;
    }
#endif

    //
    // Check to see if the connection has any input that should be read.
    //
    if(xfer->GetInputConnection()->NeedsRead())
    {
        xfer->GetInputConnection()->Fill();
    }

    bool abort = xfer->ReadPendingMessages();

#ifdef PARALLEL
    // If needed, tell the non-ui processes to abort as well
    if (abort && informSlaves)
        xfer->SendInterruption();
#endif
    return abort;
}

// ****************************************************************************
//  Method:  EngineAbortCallback
//
//  Purpose:
//    This is the normal callback which interfaces to the parallel
//    callback with the default arguments for an unknown code type.
//
//  Arguments:
//    data       the callback data
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 20, 2001
//
// ****************************************************************************

bool
EngineAbortCallback(void *data)
{
    return EngineAbortCallbackParallel(data, true);
}

// ****************************************************************************
//  Function: EngineUpdateProgressCallback
//
//  Purpose:
//      Updates the progress of the filter execution.
//
//  Arguments:
//      data       The current rpc (if it exists)
//      type       The filter type.
//      desc       A description from the filter (this may be NULL).
//      cur        The current node that finished execution.
//      total      The total number of nodes to execute on.
//
//  Notes:         When a filter starts or finishes entirely, it call this
//                 routine with total == 0 to indicate that, with cur == 0
//                 for starting and cur == 1 for finishing.
//
//  Programmer:   Hank Childs
//  Creation:     June 20, 2001
//
//  Modifications:
//    Jeremy Meredith, Fri Jun 29 14:44:59 PDT 2001
//    Added real status updates, and added cur==0/1 for starting/finishing.
//
//    Jeremy Meredith, Wed Aug 29 15:30:58 PDT 2001
//    Fixed domain indexing for partial stage completion.
//
//    Jeremy Meredith, Mon Sep 17 21:37:31 PDT 2001
//    Fixed previous fix....
//
//    Jeremy Meredith, Thu Sep 20 00:59:47 PDT 2001
//    Changed the way stage-finish updates are sent to the client.
//
// ****************************************************************************

void
EngineUpdateProgressCallback(void *data, const char *type, const char *desc,
                             int cur, int total)
{
    NonBlockingRPC *rpc = (NonBlockingRPC*)data;
    if (!rpc)
        EXCEPTION1(VisItException,
                   "EngineUpdateProgressCallback called with no RPC set.");

    if (total == 0)
    {
        if (cur == 0)
        {
            // Starting the new stage
            rpc->SendStatus(0,
                            rpc->GetCurStageNum(),
                            desc ? desc : type,  
                            rpc->GetMaxStageNum());
        }
        else
        {
            // Ending this stage...
            rpc->SendStatus(100,
                            rpc->GetCurStageNum(),
                            desc ? desc : type,
                            rpc->GetMaxStageNum());
            // And moving to the next one
            rpc->SendStatus(0,
                            rpc->GetCurStageNum() + 1,
                            desc ? desc : type,
                            rpc->GetMaxStageNum());
        }
    }
    else
    {
        rpc->SendStatus(100. * float(cur)/float(total),
                        rpc->GetCurStageNum(),
                        desc ? desc : type,
                        rpc->GetMaxStageNum());
    }
}


// ****************************************************************************
//  Function: EngineInitializeProgressCallback
//
//  Purpose:
//      Informs this module how many total filters will be executing.
//
//  Arguments:
//      data       The current rpc (if it exists)
//      nStages    The total number of stages in the pipeline.
//
//  Programmer:   Hank Childs
//  Creation:     June 20, 2001
//
//  Modifications:
//    Jeremy Meredith, Fri Jun 29 14:44:59 PDT 2001
//    Added real status updates.
//
// ****************************************************************************

void
EngineInitializeProgressCallback(void *data, int nStages)
{
    NonBlockingRPC *rpc = (NonBlockingRPC*)data;
    if (!rpc)
        EXCEPTION1(VisItException,
                   "EngineInitializeProgressCallback called with no RPC set.");

    if (nStages > 0)
        rpc->SendStatus(0, 1, "Starting execution", nStages+1);
    else
        debug1 << "ERROR: EngineInitializeProgressCallback called "
               << "with nStages == 0" << endl;
}


// ****************************************************************************
//  Function: EngineWarningCallback
//
//  Purpose:
//      Issues a warning through an rpc.
//
//  Arguments:
//      data      The current rpc (if it exists)
//      msg       The warning message.
//
//  Programmer:   Hank Childs
//  Creation:     October 18, 2001
//
// ****************************************************************************

void
EngineWarningCallback(void *data, const char *msg)
{
    NonBlockingRPC *rpc = (NonBlockingRPC*)data;
    if (!rpc)
        EXCEPTION1(VisItException,
                   "EngineInitializeProgressCallback called with no RPC set.");

    rpc->SendWarning(msg);
}

// ****************************************************************************
// Function: ResetTimeout
//
// Purpose: 
//   Resets the engine timeout.
//
// Arguments:
//   timeout : The number of seconds until we want another alarm.
//
// Programmer: Sean Ahern, Brad Whitlock
// Creation:   Tue Apr 9 13:41:29 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ResetTimeout(int timeout)
{
#if !defined(_WIN32)
    alarm(timeout);
#endif    
}

// ****************************************************************************
// Function: DefineColorTables
//
// Purpose: 
//   Takes the contents of the color table attributes and creates matching
//   color tables in avtColorTables which is used to color plots.
//
// Arguments:
//   ctAtts : The color table attributes.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 2 13:49:46 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
DefineColorTables(const ColorTableAttributes &ctAtts)
{
    avtColorTables *ct = avtColorTables::Instance();
    ct->SetColorTables(ctAtts);
}

