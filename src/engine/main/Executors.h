#ifndef EXECUTORS_H
#define EXECUTORS_H

#include <Engine.h>

#include <RPCExecutor.h>

#include <AbortException.h>
#include <DatabasePluginManager.h>
#include <DebugStream.h>
#include <ImproperUseException.h>
#include <LoadBalancer.h>
#include <NetworkManager.h>
#include <OperatorPluginInfo.h>
#include <OperatorPluginManager.h>
#include <PlotPluginInfo.h>
#include <PlotPluginManager.h>
#include <TimingsManager.h>
#include <VisItException.h>
#include <avtCallback.h>
#include <avtColorTables.h>
#include <avtDataObjectQuery.h>
#include <avtNullData.h>


#include <ApplyOperatorRPC.h>
#include <ClearCacheRPC.h>
#include <CloneNetworkRPC.h>
#include <DefineVirtualDatabaseRPC.h>
#include <ExecuteRPC.h>
#include <KeepAliveRPC.h>
#include <MakePlotRPC.h>
#include <OpenDatabaseRPC.h>
#include <PickRPC.h>
#include <QueryRPC.h>
#include <QuitRPC.h>
#include <ReadRPC.h>
#include <ReleaseDataRPC.h>
#include <RenderRPC.h>
#include <SetWinAnnotAttsRPC.h>
#include <StartPickRPC.h>
#include <UpdatePlotAttsRPC.h>
#include <UseNetworkRPC.h>

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
//    Hank Childs, Wed Jan  9 14:02:15 PST 2002
//    Add a call to ClearAllNetworks when debugging memory leaks.
//
//    Jeremy Meredith, Thu Jul 10 11:37:48 PDT 2003
//    Made the engine an object.
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
        Engine         *engine = Engine::Instance();
        NetworkManager *netmgr = engine->GetNetMgr();
        netmgr->ClearAllNetworks();
#endif
        rpc->SendReply();
    }
}

// ****************************************************************************
// Method: RPCExecutor<KeepAliveRPC>::Execute
//
// Purpose: 
//   Handles the KeepAliveRPC.
//
// Arguments:
//   rpc : A pointer to the KeepAlive RPC object.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 12 11:36:11 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

template<>
void
RPCExecutor<KeepAliveRPC>::Execute(KeepAliveRPC *rpc)
{
    //
    // Now send back some data on the command and data sockets.
    //
    Engine *engine = Engine::Instance();
    engine->SendKeepAliveReply();
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
//    Sean Ahern, Mon Dec 23 12:51:43 PST 2002
//    Renamed AddDB to StartNetwork.
//
//    Jeremy Meredith, Thu Jul 10 11:37:48 PDT 2003
//    Made the engine an object.
//
//    Jeremy Meredith, Thu Nov  6 13:21:05 PST 2003
//    Added a call to cancel the current network in case of an error.
//    That way future calls do not fail due to a failed pre-existing network.
//
//    Hank Childs, Tue Mar  9 14:27:31 PST 2004
//    Load the database plugin before reading.
//
//    Jeremy Meredith, Tue Mar 23 14:41:33 PST 2004
//    Added the file format as an argument to StartNetwork.
//
// ****************************************************************************
template<>
void
RPCExecutor<ReadRPC>::Execute(ReadRPC *rpc)
{
    Engine         *engine = Engine::Instance();
    NetworkManager *netmgr = engine->GetNetMgr();

    debug2 << "Executing ReadRPC" << endl;
    TRY
    {
        DatabasePluginManager::Instance()->PluginAvailable(rpc->GetFormat());
        netmgr->StartNetwork(rpc->GetFile(), rpc->GetFormat(),
                             rpc->GetVar(), rpc->GetTime(),
                             rpc->GetCSRAttributes(),
                             rpc->GetMaterialAttributes());
        rpc->SendReply();
    }
    CATCH2(VisItException, e)
    {
        netmgr->CancelNetwork();
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
//    Jeremy Meredith, Thu Jul 10 11:37:48 PDT 2003
//    Made the engine an object.
//
//    Jeremy Meredith, Thu Nov  6 13:21:05 PST 2003
//    Added a call to cancel the current network in case of an error.
//    That way future calls do not fail due to a failed pre-existing network.
//
// ****************************************************************************
template<>
void
RPCExecutor<PrepareOperatorRPC>::Execute(PrepareOperatorRPC *rpc)
{
    Engine         *engine = Engine::Instance();
    NetworkManager *netmgr = engine->GetNetMgr();

    debug2 << "Executing PrepareOperatorRPC: " << rpc->GetID().c_str() << endl;
    TRY 
    {
        string id = rpc->GetID().c_str();

        if (!OperatorPluginManager::Instance()->PluginAvailable(id))
        {
            netmgr->CancelNetwork();
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
        netmgr->CancelNetwork();
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
//    Jeremy Meredith, Thu Jul 10 11:37:48 PDT 2003
//    Made the engine an object.
//
//    Jeremy Meredith, Thu Nov  6 13:21:05 PST 2003
//    Added a call to cancel the current network in case of an error.
//    That way future calls do not fail due to a failed pre-existing network.
//
// ****************************************************************************
template<>
void
RPCExecutor<ApplyOperatorRPC>::Execute(ApplyOperatorRPC *rpc)
{
    Engine         *engine = Engine::Instance();
    NetworkManager *netmgr = engine->GetNetMgr();

    debug2 << "Executing ApplyOperatorRPC: " << rpc->GetID().c_str() << endl;
    TRY
    {
        netmgr->AddFilter(rpc->GetID().c_str(), rpc->GetAtts());
        rpc->SendReply();
    }
    CATCH2(VisItException, e)
    {
        netmgr->CancelNetwork();
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
//    Jeremy Meredith, Thu Jul 10 11:37:48 PDT 2003
//    Made the engine an object.
//
//    Jeremy Meredith, Thu Nov  6 13:21:05 PST 2003
//    Added a call to cancel the current network in case of an error.
//    That way future calls do not fail due to a failed pre-existing network.
//
// ****************************************************************************
template<>
void
RPCExecutor<PreparePlotRPC>::Execute(PreparePlotRPC *rpc)
{
    Engine         *engine = Engine::Instance();
    NetworkManager *netmgr = engine->GetNetMgr();

    debug2 << "Executing PreparePlotRPC: " << rpc->GetID().c_str() << endl;
    TRY
    {
        string id = rpc->GetID().c_str();

        if (!PlotPluginManager::Instance()->PluginAvailable(id))
        {
            rpc->SendError("Requested plot does not exist for the engine",
                           "VisItException");
            netmgr->CancelNetwork();
            CATCH_RETURN(1);
        }

        rpc->GetMakePlotRPC()->SetAtts(PlotPluginManager::Instance()->
            GetEnginePluginInfo(id)->AllocAttributes());
        rpc->SendReply();
    }
    CATCH2(VisItException, e)
    {
        netmgr->CancelNetwork();
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
//    Jeremy Meredith, Thu Jul 10 11:37:48 PDT 2003
//    Made the engine an object.
//
//    Jeremy Meredith, Thu Nov  6 13:21:05 PST 2003
//    Added a call to cancel the current network in case of an error.
//    That way future calls do not fail due to a failed pre-existing network.
//
//    Eric Brugger, Fri Mar 19 15:19:13 PST 2004
//    Modified the rpc to pass the data extents to the engine.
//
// ****************************************************************************
template<>
void
RPCExecutor<MakePlotRPC>::Execute(MakePlotRPC *rpc)
{
    Engine         *engine = Engine::Instance();
    NetworkManager *netmgr = engine->GetNetMgr();

    debug2 << "Executing MakePlotRPC: " << rpc->GetID().c_str() << endl;
    TRY 
    {
        netmgr->MakePlot(rpc->GetID().c_str(), rpc->GetAtts(),
                         rpc->GetDataExtents());
        MakePlotRPC::NetworkID id(netmgr->EndNetwork());
        rpc->SendReply(&id);
    }
    CATCH2(VisItException, e)
    {
        netmgr->CancelNetwork();
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
//    Jeremy Meredith, Thu Jul 10 11:37:48 PDT 2003
//    Made the engine an object.
//
// ****************************************************************************
template<>
void
RPCExecutor<UseNetworkRPC>::Execute(UseNetworkRPC *rpc)
{
    Engine         *engine = Engine::Instance();
    NetworkManager *netmgr = engine->GetNetMgr();

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
//  Modifications:
//    Jeremy Meredith, Thu Jul 10 11:37:48 PDT 2003
//    Made the engine an object.
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
//  Modifications:
//    Jeremy Meredith, Thu Jul 10 11:37:48 PDT 2003
//    Made the engine an object.
//
// ****************************************************************************
template<>
void
RPCExecutor<UpdatePlotAttsRPC>::Execute(UpdatePlotAttsRPC *rpc)
{
    Engine         *engine = Engine::Instance();
    NetworkManager *netmgr = engine->GetNetMgr();

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
//  Modifications:
//    Jeremy Meredith, Thu Jul 10 11:37:48 PDT 2003
//    Made the engine an object.
//
//    Hank Childs, Tue Aug 19 21:15:58 PDT 2003
//    Set up callbacks before doing the pick.
//
//    Kathleen Bonnell, Fri Oct 10 10:58:10 PDT 2003 
//    Set up callbacks for DataObjectQuery.
//
// ****************************************************************************
template<>
void
RPCExecutor<PickRPC>::Execute(PickRPC *rpc)
{
    Engine         *engine = Engine::Instance();
    NetworkManager *netmgr = engine->GetNetMgr();

    debug2 << "Executing PickRPC: " << endl; 
    TRY 
    {
        avtDataObjectSource::RegisterProgressCallback(NULL, NULL);
        avtDataObjectQuery::RegisterProgressCallback(NULL, NULL);
        LoadBalancer::RegisterProgressCallback(NULL, NULL);
        avtTerminatingSource::RegisterInitializeProgressCallback(NULL, NULL);
        avtDataObjectQuery::RegisterInitializeProgressCallback(NULL, NULL);
        netmgr->Pick(rpc->GetNetId(), rpc->GetPickAtts());
        rpc->SendReply(rpc->GetPickAtts());
    }
    CATCH2(VisItException, e)
    {
        rpc->SendError(e.GetMessage(), e.GetExceptionType());
    }
    ENDTRY

    avtDataObjectSource::RegisterProgressCallback(
                               Engine::EngineUpdateProgressCallback, NULL);
    avtDataObjectQuery::RegisterProgressCallback(
                               Engine::EngineUpdateProgressCallback, NULL);
    LoadBalancer::RegisterProgressCallback(
                               Engine::EngineUpdateProgressCallback, NULL);
    avtTerminatingSource::RegisterInitializeProgressCallback(
                               Engine::EngineInitializeProgressCallback, NULL);
    avtDataObjectQuery::RegisterInitializeProgressCallback(
                               Engine::EngineInitializeProgressCallback, NULL);
}


// ****************************************************************************
//  Method: RPCExecutor<StartPickRPC>::Execute
//
//  Purpose:
//
//  Programmer: Kathleen Bonnell 
//  Creation:   November 26, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Jul 10 11:37:48 PDT 2003
//    Made the engine an object.
//
//    Kathleen Bonnell, Wed Jun  2 09:48:29 PDT 2004 
//    Added ForZones flag to rpc and call to StartPickMode. 
//
// ****************************************************************************
template<>
void
RPCExecutor<StartPickRPC>::Execute(StartPickRPC *rpc)
{
    Engine         *engine = Engine::Instance();
    NetworkManager *netmgr = engine->GetNetMgr();

    debug2 << "Executing StartPickRPC: " << endl; 

    TRY 
    {
        if (rpc->GetStartFlag())
            netmgr->StartPickMode(rpc->GetForZones());
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
//  Method: RPCExecutor<SetWinAnnotAttsRPC>::Execute
//
//  Purpose:
//      Execute a SetWinAnnotAttsRPC.
//
//  Programmer: Jeremy Meredith
//  Creation:   November  8, 2001
//
//  Modifications:
//    Brad Whitlock, Mon Dec 2 13:46:49 PST 2002
//    I added a method call to populate the color tables.
//
//    Jeremy Meredith, Thu Jul 10 11:37:48 PDT 2003
//    Made the engine an object.
//
//    Mark C. Miller, Wed Apr 14 16:41:32 PDT 2004
//    Added code to pass extents type string to SetWindowAttributes
//
//    Mark C. Miller, Tue May 25 17:15:10 PDT 2004
//    Relocated code to set color tables to NetworkManager::SetWindowAtts
//    Added argument to SetAnnotationAttributes to accomodate an
//    AnnotationObjectList
//
//    Mark C. Miller, Tue May 25 20:44:10 PDT 2004
//    Added code to pass annotation object list along
//
//    Mark C. Miller, Wed Jun  9 17:44:38 PDT 2004
//    Added code to pass visual cue list along
//
//    Mark C. Miller, Tue Jul 27 15:11:11 PDT 2004
//    Added code to pass frame and state info along
//
// ****************************************************************************
template<>
void
RPCExecutor<SetWinAnnotAttsRPC>::Execute(SetWinAnnotAttsRPC *rpc)
{
    Engine         *engine = Engine::Instance();
    NetworkManager *netmgr = engine->GetNetMgr();

    debug2 << "Executing SetWinAnnotAttsRPC "
           << rpc->GetWindowAtts().GetSize()[0] << "x"
           << rpc->GetWindowAtts().GetSize()[1] << endl;
    TRY 
    {
        netmgr->SetWindowAttributes(rpc->GetWindowAtts(),
                                    rpc->GetExtentTypeString());
        netmgr->SetAnnotationAttributes(rpc->GetAnnotationAtts(),
                                        rpc->GetAnnotationObjectList(),
                                        rpc->GetVisualCueList(),
                                        rpc->GetFrameAndState());
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
//    Jeremy Meredith, Thu Jul 10 11:37:48 PDT 2003
//    Made the engine an object.
//
//    Kathleen Bonnell, Wed Mar 31 16:53:03 PST 2004 
//    Set up callbacks for DataObjectQuery.
//
//    Mark C. Miller, Mon May 24 18:36:13 PDT 2004
//    Modified to support new WriteData interface
//
//    Mark C. Miller, Wed Jul  7 11:42:09 PDT 2004
//    Wrapped call to GetOutput in TRY/CATCH block to catch possible
//    abort exception and do the right thing
//
//    Mark C. Miller, Wed Aug 11 23:42:18 PDT 2004
//    Added code to pass cellCountMultiplier to WriteData
//
//    Mark C. Miller, Mon Aug 23 20:24:31 PDT 2004
//    Moved code to get cellCountMultiplier to inside GetOutput
//
// ****************************************************************************
template<>
void
RPCExecutor<ExecuteRPC>::Execute(ExecuteRPC *rpc)
{
    Engine         *engine = Engine::Instance();
    NetworkManager *netmgr = engine->GetNetMgr();

    int gettingData = visitTimer->StartTimer();
    int writingData = -1;

    avtDataObjectSource::RegisterProgressCallback(Engine::EngineUpdateProgressCallback,
                                        (void*)rpc);
    LoadBalancer::RegisterProgressCallback(Engine::EngineUpdateProgressCallback,
                                        (void*)rpc);
    avtTerminatingSource::RegisterInitializeProgressCallback(
                                       Engine::EngineInitializeProgressCallback,
                                       (void*)rpc);
    avtCallback::RegisterWarningCallback(Engine::EngineWarningCallback, (void*)rpc);

    avtDataObjectQuery::RegisterProgressCallback(
                         Engine::EngineUpdateProgressCallback, (void*) rpc);
    avtDataObjectQuery::RegisterInitializeProgressCallback(
                         Engine::EngineInitializeProgressCallback,(void*) rpc);



    debug2 << "Executing ExecuteRPC with respondWithNullDataObject = " <<
       rpc->GetRespondWithNull() << endl;
    TRY
    {
        // save the current network id for later
        bool shouldSendAbort = false;
        float cellCountMultiplier;
        int netId = netmgr->GetCurrentNetworkId();
        avtNullData abortDob(NULL);

        // Get the output of the network manager. This does the job of
        // executing the network.
        avtDataObjectWriter_p writer;
        TRY
        {
            writer = netmgr->GetOutput(rpc->GetRespondWithNull(), false,
                                       &cellCountMultiplier);
        }
        CATCH(AbortException)
        {
            shouldSendAbort = true;

            // make a dummy dataobject writer for the call to WriteData
            abortDob.SetWriterShouldMergeParallelStreams();
            writer = abortDob.InstantiateWriter();
        }
        ENDTRY

        visitTimer->StopTimer(gettingData, "Executing network");
        writingData = visitTimer->StartTimer();

        // set params influencing scalable rendering 
        int scalableThreshold = netmgr->GetScalableThreshold();
        int currentTotalGlobalCellCount = netmgr->GetTotalGlobalCellCounts();
        int currentNetworkGlobalCellCount = 0;
        bool scalableThresholdExceeded = false;

        // Send the data back to the viewer.
        engine->WriteData(rpc, writer, rpc->GetRespondWithNull(),
                    scalableThreshold, &scalableThresholdExceeded,
                    currentTotalGlobalCellCount, cellCountMultiplier,
                    &currentNetworkGlobalCellCount);

        // re-set the network if we exceeded the scalable threshold
        if (scalableThresholdExceeded && !rpc->GetRespondWithNull())
            netmgr->UseNetwork(netId);

        // only update cell count if we're not here asking for null data
        if (!rpc->GetRespondWithNull())
            netmgr->SetGlobalCellCount(netId, currentNetworkGlobalCellCount);

        // send an abort message if we decided we needed to
        if (shouldSendAbort)
            rpc->SendAbort();
    }
    CATCH(ImproperUseException)
    {
        engine->SetNoFatalExceptions(false);
    }
    CATCH2(VisItException, e)
    {
        rpc->SendError(e.GetMessage(), e.GetExceptionType());
    }
    ENDTRY

    avtDataObjectSource::RegisterProgressCallback(Engine::EngineUpdateProgressCallback,
                                                  NULL);
    LoadBalancer::RegisterProgressCallback(Engine::EngineUpdateProgressCallback,
                                           NULL);
    avtTerminatingSource::RegisterInitializeProgressCallback(
                               Engine::EngineInitializeProgressCallback, NULL);
    avtDataObjectQuery::RegisterProgressCallback(
                               Engine::EngineUpdateProgressCallback, NULL);
    avtDataObjectQuery::RegisterInitializeProgressCallback(
                               Engine::EngineInitializeProgressCallback, NULL);

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
//  Programmer: Brad Whitlock
//  Creation:   Tue Jul 30 13:10:26 PST 2002
//
//  Modifications:
//    Jeremy Meredith, Thu Jul 10 11:37:48 PDT 2003
//    Made the engine an object.
//
//    Hank Childs, Mon Jan  5 15:54:26 PST 2004
//    When a database is involved, only clear the networks that reference
//    that database.
//
// ****************************************************************************
template<>
void
RPCExecutor<ClearCacheRPC>::Execute(ClearCacheRPC *rpc)
{
    Engine         *engine = Engine::Instance();
    NetworkManager *netmgr = engine->GetNetMgr();

    debug2 << "Executing ClearCacheRPC: file = " 
           << rpc->GetDatabaseName().c_str() << endl;
    TRY 
    {
        if (rpc->GetClearAll())
            netmgr->ClearAllNetworks();
        else
        {
            netmgr->ClearNetworksWithDatabase(rpc->GetDatabaseName());
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
//  Modifications:
//    Jeremy Meredith, Thu Jul 10 11:37:48 PDT 2003
//    Made the engine an object.
//
//    Hank Childs, Tue Aug 19 20:57:57 PDT 2003
//    Added a callback for the terminating source.
//
//    Hank Childs, Thu Oct  2 16:29:36 PDT 2003
//    Renamed GetNetworkId to GetNetworkIds.
//
// ****************************************************************************
template<>
void
RPCExecutor<QueryRPC>::Execute(QueryRPC *rpc)
{
    Engine         *engine = Engine::Instance();
    NetworkManager *netmgr = engine->GetNetMgr();

    debug2 << "Executing QueryRPC: " << endl;

    avtDataObjectQuery::RegisterProgressCallback(
                         Engine::EngineUpdateProgressCallback, (void*) rpc);
    avtDataObjectSource::RegisterProgressCallback(
                         Engine::EngineUpdateProgressCallback, (void*) rpc);
    LoadBalancer::RegisterProgressCallback(
                         Engine::EngineUpdateProgressCallback, (void*) rpc);
    avtDataObjectQuery::RegisterInitializeProgressCallback(
                         Engine::EngineInitializeProgressCallback,(void*) rpc);
    avtTerminatingSource::RegisterInitializeProgressCallback(
                         Engine::EngineInitializeProgressCallback,(void*) rpc);
 
    TRY
    {
        netmgr->Query(rpc->GetNetworkIds(), rpc->GetQueryAtts());
        rpc->SendReply(rpc->GetQueryAtts());
    }
    CATCH2(VisItException, e)
    {
        rpc->SendError(e.GetMessage(), e.GetExceptionType());
    }
    ENDTRY

    avtDataObjectQuery::RegisterProgressCallback(
                               Engine::EngineUpdateProgressCallback, NULL);
    avtDataObjectSource::RegisterProgressCallback(
                               Engine::EngineUpdateProgressCallback, NULL);
    LoadBalancer::RegisterProgressCallback(
                               Engine::EngineUpdateProgressCallback, NULL);
    avtDataObjectQuery::RegisterInitializeProgressCallback(
                               Engine::EngineInitializeProgressCallback, NULL);
    avtTerminatingSource::RegisterInitializeProgressCallback(
                               Engine::EngineInitializeProgressCallback, NULL);
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
//    Jeremy Meredith, Thu Jul 10 11:37:48 PDT 2003
//    Made the engine an object.
//
// ****************************************************************************
template<>
void
RPCExecutor<ReleaseDataRPC>::Execute(ReleaseDataRPC *rpc)
{
    Engine         *engine = Engine::Instance();
    NetworkManager *netmgr = engine->GetNetMgr();

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
//    Sean Ahern, Mon Dec 23 11:24:18 PST 2002
//    Changed AddDB to GetDBFromCache.
//
//    Jeremy Meredith, Thu Jul 10 11:37:48 PDT 2003
//    Made the engine an object.
//
//    Hank Childs, Fri Mar  5 11:46:09 PST 2004
//    Load database plugins before trying to instantiate the DB of that type.
//
//    Hank Childs, Mon Mar 22 11:16:47 PST 2004
//    Specify the file format type explicitly.
//
//    Jeremy Meredith, Wed Aug 25 12:01:15 PDT 2004
//    Send metadata to the client if we are a simulation.
//
// ****************************************************************************
template<>
void
RPCExecutor<OpenDatabaseRPC>::Execute(OpenDatabaseRPC *rpc)
{
    Engine         *engine = Engine::Instance();
    NetworkManager *netmgr = engine->GetNetMgr();

    debug2 << "Executing OpenDatabaseRPC: db=" <<rpc->GetDatabaseName().c_str()
           << ", time=" << rpc->GetTime() << endl;
    DatabasePluginManager::Instance()->PluginAvailable(rpc->GetFileFormat());

    netmgr->GetDBFromCache(rpc->GetDatabaseName(), rpc->GetTime(),
                           rpc->GetFileFormat().c_str());

    engine->PopulateSimulationMetaData(rpc->GetDatabaseName(),
                                       rpc->GetFileFormat());
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
//    Jeremy Meredith, Thu Jul 10 11:37:48 PDT 2003
//    Made the engine an object.
//
//    Hank Childs, Fri Mar  5 11:46:09 PST 2004
//    Load database plugins before trying to instantiate the DB of that type.
//
//    Hank Childs, Mon Mar 22 11:10:43 PST 2004
//    Explicitly tell NetworkManager what file format type to open the file 
//    with.
//
// ****************************************************************************
template<>
void
RPCExecutor<DefineVirtualDatabaseRPC>::Execute(DefineVirtualDatabaseRPC *rpc)
{
    Engine         *engine = Engine::Instance();
    NetworkManager *netmgr = engine->GetNetMgr();

    debug2 << "Executing DefineVirtualDatabaseRPC: "
           << "db=" << rpc->GetDatabaseName().c_str()
           << ", path=" << rpc->GetDatabasePath().c_str()
           << ", time=" << rpc->GetTime()
           << ", numStates=" << rpc->GetDatabaseFiles().size()
           << endl;
    for (int i = 0; i < rpc->GetDatabaseFiles().size(); ++i)
        debug5 << "file["<<i<<"]="<<rpc->GetDatabaseFiles()[i].c_str() << endl;
    DatabasePluginManager::Instance()->PluginAvailable(rpc->GetFileFormat());

    netmgr->DefineDB(rpc->GetDatabaseName(), rpc->GetDatabasePath(),
                rpc->GetDatabaseFiles(), rpc->GetTime(), rpc->GetFileFormat());
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
//  Modifications:
//    Jeremy Meredith, Thu Jul 10 11:37:48 PDT 2003
//    Made the engine an object.
//
//    Mark C. Miller, Mon Mar 29 14:27:10 PST 200
//    Added stuff to pass knowledge of annotations to render method
//
//    Mark C. Miller, Tue Apr 20 07:44:34 PDT 2004
//    Added code to set the warning call back
//
//    Mark C. Miller, Wed Jul  7 11:42:09 PDT 2004
//    Added code to re-register various callbacks like other RPC's in this
//    file
//
// ****************************************************************************
template<>
void
RPCExecutor<RenderRPC>::Execute(RenderRPC *rpc)
{
    Engine         *engine = Engine::Instance();
    NetworkManager *netmgr = engine->GetNetMgr();

    debug2 << "Executing RenderRPC for the following plots" << endl;
    debug2 << "   ";
    for (int i = 0; i < rpc->GetIDs().size(); i++)
       debug2 << rpc->GetIDs()[i] << ", ";
    debug2 << endl;

    avtDataObjectSource::RegisterProgressCallback(NULL, NULL);
    LoadBalancer::RegisterProgressCallback(NULL, NULL);
    avtTerminatingSource::RegisterInitializeProgressCallback(NULL, NULL);
    avtCallback::RegisterWarningCallback(Engine::EngineWarningCallback, (void*)rpc);
    TRY 
    {
        // do the render
        avtDataObjectWriter_p writer =
            netmgr->Render(rpc->GetIDs(),rpc->GetSendZBuffer(),
                           rpc->GetDo3DAnnotsOnly());

        // Send the data back to the viewer.
        engine->WriteData(rpc, writer);
    }
    CATCH2(VisItException, e)
    {
        rpc->SendError(e.GetMessage(), e.GetExceptionType());
    }
    ENDTRY

    avtDataObjectSource::RegisterProgressCallback(
                               Engine::EngineUpdateProgressCallback, NULL);
    LoadBalancer::RegisterProgressCallback(
                               Engine::EngineUpdateProgressCallback, NULL);
    avtTerminatingSource::RegisterInitializeProgressCallback(
                               Engine::EngineInitializeProgressCallback, NULL);
    avtCallback::RegisterWarningCallback(Engine::EngineWarningCallback, NULL);
}

// ****************************************************************************
//  Method: RPCExecutor<CloneNetworkRPC>::Execute
//
//  Purpose:
//      Execute a CloneNetworkRPC.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 18, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Thu Apr 15 14:07:53 PDT 2004
//    Send rpc->GetID() as arg to AddQueryOverTimeFilter.
//
// ****************************************************************************
template<>
void
RPCExecutor<CloneNetworkRPC>::Execute(CloneNetworkRPC *rpc)
{
    Engine         *engine = Engine::Instance();
    NetworkManager *netmgr = engine->GetNetMgr();

    debug2 << "Executing CloneNetworkRPC: " << rpc->GetID() << endl;
    TRY
    {
        netmgr->CloneNetwork(rpc->GetID());
        if (rpc->GetQueryOverTimeAtts() != NULL)
            netmgr->AddQueryOverTimeFilter(rpc->GetQueryOverTimeAtts(),
                                           rpc->GetID());
        rpc->SendReply();
    }
    CATCH2(VisItException, e)
    {
        netmgr->CancelNetwork();
        rpc->SendError(e.GetMessage(), e.GetExceptionType());
    }
    ENDTRY
}

#endif
