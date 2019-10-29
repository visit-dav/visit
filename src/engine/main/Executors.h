// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef EXECUTORS_H
#define EXECUTORS_H

#include <Engine.h>

#include <EngineRPCExecutor.h>

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
#include <avtDatabaseFactory.h>
#include <avtDataObjectQuery.h>
#include <avtNamedSelectionManager.h>
#include <avtNullData.h>


#include <ApplyOperatorRPC.h>
#include <ClearCacheRPC.h>
#include <CloneNetworkRPC.h>
#include <ConstructDataBinningRPC.h>
#include <DefineVirtualDatabaseRPC.h>
#include <EnginePropertiesRPC.h>
#include <ExecuteRPC.h>
#include <ExportDatabaseRPC.h>
#include <KeepAliveRPC.h>
#include <LaunchRPC.h>
#include <MakePlotRPC.h>
#include <NamedSelectionRPC.h>
#include <OpenDatabaseRPC.h>
#include <PickRPC.h>
#include <ProcInfoRPC.h>
#include <QueryRPC.h>
#include <QueryParametersRPC.h>
#include <QuitRPC.h>
#include <ReadRPC.h>
#include <ReleaseDataRPC.h>
#include <RenderRPC.h>
#include <SetEFileOpenOptionsRPC.h>
#include <SetPrecisionTypeRPC.h>
#include <SetBackendTypeRPC.h>
#include <SetWinAnnotAttsRPC.h>
#include <SimulationCommandRPC.h>
#include <StartPickRPC.h>
#include <StartQueryRPC.h>
#include <UpdatePlotAttsRPC.h>
#include <UseNetworkRPC.h>

#include <avtParallel.h>
#include <avtDataObjectString.h>

#include <string>

// ****************************************************************************
//  Method: EngineRPCExecutor<QuitRPC>::Execute
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
EngineRPCExecutor<QuitRPC>::Execute(QuitRPC *rpc)
{

    debug2 << "Executing QuitRPC" << endl;
    if (!rpc->GetQuit())
        rpc->SendError();
    else
    {
#ifdef DEBUG_MEMORY_LEAKS
        GetEngine()->GetNetMgr()->ClearAllNetworks();
#endif
        rpc->SendReply();
    }
}

// ****************************************************************************
// Method: EngineRPCExecutor<KeepAliveRPC>::Execute
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
//   Burlen Loring, Fri Oct 16 13:35:24 PDT 2015
//   Fix a warning
//
// ****************************************************************************

template<>
void
EngineRPCExecutor<KeepAliveRPC>::Execute(KeepAliveRPC *rpc)
{
    (void) rpc;
    //
    // Now send back some data on the command and data sockets.
    //
    GetEngine()->SendKeepAliveReply();
}

// ****************************************************************************
//  Method: EngineRPCExecutor<ReadRPC>::Execute
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
//    Brad Whitlock, Tue Feb 22 12:38:39 PDT 2005
//    I changed the interface to StartNetwork.
//
//    Mark C. Miller, Wed Nov 16 10:46:36 PST 2005
//    Passed MeshManagement attributes 
//
//    Mark C. Miller, Thu Jun 14 10:26:37 PDT 2007
//    Added support to treat all databases as time varying
//
//    Mark C. Miller, Tue Jun 10 15:57:15 PDT 2008
//    Added support for ignoring extents
//
//    Brad Whitlock, Tue Jun 24 15:57:34 PDT 2008
//    Changed how the database plugin manager is accessed.
//
//    Brad Whitlock, Mon Aug 22 10:22:18 PDT 2011
//    Pass the selection name to StartNetwork.
//
//    Eric Brugger, Mon Oct 31 09:51:17 PDT 2011
//    Added the window id to StartNetwork.
//
// ****************************************************************************

template<>
void
EngineRPCExecutor<ReadRPC>::Execute(ReadRPC *rpc)
{
    NetworkManager *netmgr = GetEngine()->GetNetMgr();

    debug2 << "Executing ReadRPC" << endl;
    TRY
    {
        netmgr->GetDatabasePluginManager()->PluginAvailable(rpc->GetFormat());

        netmgr->StartNetwork(rpc->GetFormat(),
                             rpc->GetFile(), 
                             rpc->GetVar(),
                             rpc->GetTime(),
                             rpc->GetCSRAttributes(),
                             rpc->GetMaterialAttributes(),
                             rpc->GetMeshManagementAttributes(),
                             rpc->GetTreatAllDBsAsTimeVarying(),
                             rpc->GetIgnoreExtents(),
                             rpc->GetSelectionName(),
                             rpc->GetWindowID());
        rpc->SendReply();
    }
    CATCH2(VisItException, e)
    {
        netmgr->CancelNetwork();
        rpc->SendError(e.Message(), e.GetExceptionType());
    }
    ENDTRY
}

// ****************************************************************************
//  Method: EngineRPCExecutor<PrepareOperatorRPC>::Execute
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
//    Brad Whitlock, Tue Jun 24 15:58:13 PDT 2008
//    Changed how the operator plugin manager is accessed.
//
// ****************************************************************************
template<>
void
EngineRPCExecutor<PrepareOperatorRPC>::Execute(PrepareOperatorRPC *rpc)
{
    NetworkManager *netmgr = GetEngine()->GetNetMgr();

    debug2 << "Executing PrepareOperatorRPC: " << rpc->GetID().c_str() << endl;
    TRY 
    {
        std::string id = rpc->GetID().c_str();

        if (!netmgr->GetOperatorPluginManager()->PluginAvailable(id))
        {
            netmgr->CancelNetwork();
            rpc->SendError("Requested operator does not exist for the engine",
                           "VisItException");
            CATCH_RETURN(1);
        }

        rpc->GetApplyOperatorRPC()->SetAtts(
            netmgr->GetOperatorPluginManager()->GetEnginePluginInfo(id)->
                AllocAttributes());
        rpc->SendReply();
    }
    CATCH2(VisItException, e)
    {
        netmgr->CancelNetwork();
        rpc->SendError(e.Message(), e.GetExceptionType());
    }
    ENDTRY
}


// ****************************************************************************
//  Method: EngineRPCExecutor<ApplyOperatorRPC>::Execute
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
EngineRPCExecutor<ApplyOperatorRPC>::Execute(ApplyOperatorRPC *rpc)
{
    NetworkManager *netmgr = GetEngine()->GetNetMgr();

    debug2 << "Executing ApplyOperatorRPC: " << rpc->GetID() << endl;
    TRY
    {
        netmgr->AddFilter(rpc->GetID(), rpc->GetAtts());
        rpc->SendReply();
    }
    CATCH2(VisItException, e)
    {
        netmgr->CancelNetwork();
        rpc->SendError(e.Message(), e.GetExceptionType());
    }
    ENDTRY
}

// ****************************************************************************
//  Method: EngineRPCExecutor<PreparePlotPlotRPC>::Execute
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
//    Brad Whitlock, Tue Jun 24 15:59:35 PDT 2008
//    Changed how the plot plugin manager is accessed.
//
// ****************************************************************************
template<>
void
EngineRPCExecutor<PreparePlotRPC>::Execute(PreparePlotRPC *rpc)
{
    NetworkManager *netmgr = GetEngine()->GetNetMgr();

    debug2 << "Executing PreparePlotRPC: " << rpc->GetID().c_str() << endl;
    TRY
    {
        std::string id = rpc->GetID().c_str();

        if (!netmgr->GetPlotPluginManager()->PluginAvailable(id))
        {
            rpc->SendError("Requested plot does not exist for the engine",
                           "VisItException");
            netmgr->CancelNetwork();
            CATCH_RETURN(1);
        }

        rpc->GetMakePlotRPC()->SetAtts(netmgr->GetPlotPluginManager()->
            GetEnginePluginInfo(id)->AllocAttributes());
        rpc->SendReply();
    }
    CATCH2(VisItException, e)
    {
        netmgr->CancelNetwork();
        rpc->SendError(e.Message(), e.GetExceptionType());
    }
    ENDTRY
}

// ****************************************************************************
//  Method: EngineRPCExecutor<MakePlotRPC>::Execute
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
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Added code to pass window id
//
//    Brad Whitlock, Wed Mar 21 22:56:37 PST 2007
//    Added code to pass plotName.
//
// ****************************************************************************
template<>
void
EngineRPCExecutor<MakePlotRPC>::Execute(MakePlotRPC *rpc)
{
    NetworkManager *netmgr = GetEngine()->GetNetMgr();

    debug2 << "Executing MakePlotRPC: " << rpc->GetID().c_str() << endl;
    TRY 
    {
        netmgr->MakePlot(rpc->GetName().c_str(), rpc->GetID().c_str(), 
                         rpc->GetAtts(), rpc->GetDataExtents());
        MakePlotRPC::NetworkID id(netmgr->EndNetwork(rpc->GetWindowID()));
        rpc->SendReply(&id);
    }
    CATCH2(VisItException, e)
    {
        netmgr->CancelNetwork();
        rpc->SendError(e.Message(), e.GetExceptionType());
    }
    ENDTRY
}

// ****************************************************************************
//  Method: EngineRPCExecutor<UseNetworkRPC>::Execute
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
EngineRPCExecutor<UseNetworkRPC>::Execute(UseNetworkRPC *rpc)
{
    NetworkManager *netmgr = GetEngine()->GetNetMgr();

    debug2 << "Executing UseNetworkRPC: " << rpc->GetID() << endl;
    TRY 
    {
        netmgr->UseNetwork(rpc->GetID());
        rpc->SendReply();
    }
    CATCH2(VisItException, e)
    {
        rpc->SendError(e.Message(), e.GetExceptionType());
    }
    ENDTRY
}

// ****************************************************************************
//  Method: EngineRPCExecutor<PrepareUpdatePlotAttsRPC>::Execute
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
//    Brad Whitlock, Tue Jun 24 16:00:58 PDT 2008
//    Changed how the plugin manager is accessed.
//
// ****************************************************************************
template<>
void
EngineRPCExecutor<PrepareUpdatePlotAttsRPC>::Execute(PrepareUpdatePlotAttsRPC *rpc)
{
    debug2 << "Executing PrepareUpdatePlotAttsRPC: " << rpc->GetID().c_str() << endl;
    TRY
    {
        NetworkManager *netmgr = GetEngine()->GetNetMgr();
        std::string id = rpc->GetID().c_str();

        if (!netmgr->GetPlotPluginManager()->PluginAvailable(id))
        {
            rpc->SendError("Requested plot does not exist for the engine",
                           "VisItException");
            CATCH_RETURN(1);
        }

        AttributeSubject *atts = netmgr->GetPlotPluginManager()->
                                 GetEnginePluginInfo(id)->AllocAttributes();
        rpc->GetUpdatePlotAttsRPC()->SetAtts(atts);
        rpc->SendReply();
    }
    CATCH2(VisItException, e)
    {
        rpc->SendError(e.Message(), e.GetExceptionType());
    }
    ENDTRY
}

// ****************************************************************************
//  Method: EngineRPCExecutor<UpdatePlotAttsRPC>::Execute
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
EngineRPCExecutor<UpdatePlotAttsRPC>::Execute(UpdatePlotAttsRPC *rpc)
{
    NetworkManager *netmgr = GetEngine()->GetNetMgr();

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
        rpc->SendError(e.Message(), e.GetExceptionType());
    }
    ENDTRY
}

// ****************************************************************************
//  Method: EngineRPCExecutor<PickRPC>::Execute
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
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Added window id to pick rpc
//
//    Kathleen Bonnell, Tue Mar  7 08:27:25 PST 2006 
//    Added call to PickForIntersection.
//
// ****************************************************************************
template<>
void
EngineRPCExecutor<PickRPC>::Execute(PickRPC *rpc)
{
    NetworkManager *netmgr = GetEngine()->GetNetMgr();

    debug2 << "Executing PickRPC: " << endl; 
    TRY 
    {
        avtDataObjectSource::RegisterProgressCallback(NULL, NULL);
        avtDataObjectQuery::RegisterProgressCallback(NULL, NULL);
        LoadBalancer::RegisterProgressCallback(NULL, NULL);
        avtOriginatingSource::RegisterInitializeProgressCallback(NULL, NULL);
        avtDataObjectQuery::RegisterInitializeProgressCallback(NULL, NULL);
        if (rpc->GetNetId() >= 0)
            netmgr->Pick(rpc->GetNetId(), rpc->GetWinId(), rpc->GetPickAtts());
        else
            netmgr->PickForIntersection(rpc->GetWinId(), rpc->GetPickAtts());
        rpc->SendReply(rpc->GetPickAtts());
    }
    CATCH2(VisItException, e)
    {
        rpc->SendError(e.Message(), e.GetExceptionType());
    }
    ENDTRY

    avtDataObjectSource::RegisterProgressCallback(
                               Engine::EngineUpdateProgressCallback, NULL);
    avtDataObjectQuery::RegisterProgressCallback(
                               Engine::EngineUpdateProgressCallback, NULL);
    LoadBalancer::RegisterProgressCallback(
                               Engine::EngineUpdateProgressCallback, NULL);
    avtOriginatingSource::RegisterInitializeProgressCallback(
                               Engine::EngineInitializeProgressCallback, NULL);
    avtDataObjectQuery::RegisterInitializeProgressCallback(
                               Engine::EngineInitializeProgressCallback, NULL);
}


// ****************************************************************************
//  Method: EngineRPCExecutor<StartPickRPC>::Execute
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
EngineRPCExecutor<StartPickRPC>::Execute(StartPickRPC *rpc)
{
    NetworkManager *netmgr = GetEngine()->GetNetMgr();

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
        rpc->SendError(e.Message(), e.GetExceptionType());
    }
    ENDTRY
}


// ****************************************************************************
//  Method: EngineRPCExecutor<StartQueryRPC>::Execute
//
//  Purpose:
//      Indicates that we should start query mode or stop query mode.
//      This is important to the network manager, because it cannot do DLB
//      while in query mode.
//
//  Programmer: Hank Childs
//  Creation:   February 28, 2005
//
// ****************************************************************************
template<>
void
EngineRPCExecutor<StartQueryRPC>::Execute(StartQueryRPC *rpc)
{
    NetworkManager *netmgr = GetEngine()->GetNetMgr();

    debug2 << "Executing StartQueryRPC: " << endl; 

    TRY 
    {
        if (rpc->GetStartFlag())
            netmgr->StartQueryMode();
        else
            netmgr->StopQueryMode();
        rpc->SendReply();
    }
    CATCH2(VisItException, e)
    {
        rpc->SendError(e.Message(), e.GetExceptionType());
    }
    ENDTRY
}

// ****************************************************************************
//  Method: EngineRPCExecutor<SetWinAnnotAttsRPC>::Execute
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
//    Mark C. Miller, Wed Oct  6 18:12:29 PDT 2004
//    Added code to pass view extents
//
//    Mark C. Miller, Tue Oct 19 19:44:00 PDT 2004
//    Added arg to pass changed color table name
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Added window id to SetWinAnnotAttsRPC
// ****************************************************************************
template<>
void
EngineRPCExecutor<SetWinAnnotAttsRPC>::Execute(SetWinAnnotAttsRPC *rpc)
{
    NetworkManager *netmgr = GetEngine()->GetNetMgr();

    debug2 << "Executing SetWinAnnotAttsRPC "
           << rpc->GetWindowAtts().GetSize()[0] << "x"
           << rpc->GetWindowAtts().GetSize()[1] << endl;
    TRY 
    {
        netmgr->SetWindowAttributes(rpc->GetWindowAtts(),
                                    rpc->GetExtentTypeString(),
                                    rpc->GetViewExtents(),
                                    rpc->GetChangedCtName(),
                                    rpc->GetWindowID());
        netmgr->SetAnnotationAttributes(rpc->GetAnnotationAtts(),
                                        rpc->GetAnnotationObjectList(),
                                        rpc->GetVisualCueList(),
                                        rpc->GetFrameAndState(),
                                        rpc->GetWindowID());
        rpc->SendReply();
    }
    CATCH2(VisItException, e)
    {
        rpc->SendError(e.Message(), e.GetExceptionType());
    }
    ENDTRY
}


// ****************************************************************************
//  Method: EngineRPCExecutor<ExecuteRPC>::Execute
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
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Added code to operate on specific window id
//
//    Hank Childs, Sun Mar 27 14:02:22 PST 2005
//    Use OutputAllTimings, in case timings are being withheld.
//
//    Mark C. Miller, Thu Nov  3 16:59:41 PST 2005
//    Added compression control
//
//    Mark C. Miller, Wed Nov 16 14:17:01 PST 2005
//    Changed use compression settin of window instead of always setting false 
//
//    Mark C. Miller, Wed Dec 14 16:43:07 PST 2005
//    Changed to pass compression bool to Engine::WriteData
//
//    Mark C. Miller, Thu Jun 14 10:26:37 PDT 2007
//    Minor alteration to debug2 statement output.
//
//    Brad Whitlock, Mon Sep 22 21:24:54 PDT 2014
//    Use some callback functions to handle status and writing the data back
//    to the engine socket.
// 
// ****************************************************************************

static void
Execute_statusCB(int percent, const char *msg, void *cbdata)
{
    NonBlockingRPC *rpc = (NonBlockingRPC *)cbdata;
    rpc->SendStatus(percent,
                    rpc->GetCurStageNum(),
                    msg,
                    rpc->GetMaxStageNum());
}

static void
Execute_writeCB(avtDataObjectString &do_str, void *cbdata)
{
    NonBlockingRPC *rpc = (NonBlockingRPC *)cbdata;

    // Set the return data size so we know the size of the data
    // message to read.
    int totalSize = do_str.GetTotalLength();
    rpc->SendReply(totalSize);

    // Send the data back to the viewer.
    Engine::GetEngine()->WriteByteStreamToSocket(do_str);
}

template<>
void
EngineRPCExecutor<ExecuteRPC>::Execute(ExecuteRPC *rpc)
{
    NetworkManager *netmgr = GetEngine()->GetNetMgr();

    int gettingData = visitTimer->StartTimer();
    int writingData = -1;

    avtDataObjectSource::RegisterProgressCallback(Engine::EngineUpdateProgressCallback,
                                        (void*)rpc);
    LoadBalancer::RegisterProgressCallback(Engine::EngineUpdateProgressCallback,
                                        (void*)rpc);
    avtOriginatingSource::RegisterInitializeProgressCallback(
                                       Engine::EngineInitializeProgressCallback,
                                       (void*)rpc);
    avtCallback::RegisterWarningCallback(Engine::EngineWarningCallback, (void*)rpc);

    avtDataObjectQuery::RegisterProgressCallback(
                         Engine::EngineUpdateProgressCallback, (void*) rpc);
    avtDataObjectQuery::RegisterInitializeProgressCallback(
                         Engine::EngineInitializeProgressCallback,(void*) rpc);



    debug2 << "Executing ExecuteRPC with "
           << "respondWithNullDataObject = " << rpc->GetRespondWithNull()
           << endl;
    TRY
    {
        // save the current network id for later
        bool shouldSendAbort = false;
        float cellCountMultiplier;
        int netId = netmgr->GetCurrentNetworkId();
        int winId = netmgr->GetCurrentWindowId();
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
        int scalableThreshold = netmgr->GetScalableThreshold(winId);
        int currentTotalGlobalCellCount = netmgr->GetTotalGlobalCellCounts(winId);
        int currentNetworkGlobalCellCount = 0;
        bool scalableThresholdExceeded = false;
        bool useCompression = netmgr->GetShouldUseCompression(winId);

        // Gather the data for the viewer into the data object string.
        std::string errorMessage;
        bool success = GetEngine()->GatherData(writer,
                           useCompression,
                           rpc->GetRespondWithNull(),
                           scalableThreshold, 
                           currentTotalGlobalCellCount, cellCountMultiplier,
                           Execute_statusCB, (void *)rpc,
                           Execute_writeCB, (void*)rpc,
                           errorMessage,
                           &scalableThresholdExceeded,
                           &currentNetworkGlobalCellCount);

        if (PAR_UIProcess() && !success)
            rpc->SendError(errorMessage);

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
        GetEngine()->SetNoFatalExceptions(false);
    }
    CATCH2(VisItException, e)
    {
        rpc->SendError(e.Message(), e.GetExceptionType());
    }
    ENDTRY

    avtDataObjectSource::RegisterProgressCallback(Engine::EngineUpdateProgressCallback,
                                                  NULL);
    LoadBalancer::RegisterProgressCallback(Engine::EngineUpdateProgressCallback,
                                           NULL);
    avtOriginatingSource::RegisterInitializeProgressCallback(
                               Engine::EngineInitializeProgressCallback, NULL);
    avtDataObjectQuery::RegisterProgressCallback(
                               Engine::EngineUpdateProgressCallback, NULL);
    avtDataObjectQuery::RegisterInitializeProgressCallback(
                               Engine::EngineInitializeProgressCallback, NULL);

    if (writingData >= 0)
    {
        visitTimer->StopTimer(writingData, "Writing data to viewer");
    }
    visitTimer->OutputAllTimings();
}

// ****************************************************************************
//  Method: EngineRPCExecutor<ClearCacheRPC>::Execute
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
EngineRPCExecutor<ClearCacheRPC>::Execute(ClearCacheRPC *rpc)
{
    NetworkManager *netmgr = GetEngine()->GetNetMgr();

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
        rpc->SendError(e.Message(), e.GetExceptionType());
    }
    ENDTRY
}

 
// ****************************************************************************
//  Method: EngineRPCExecutor<ProcInfoRPC>::Execute
//
//  Purpose:  Execute a request for process infor.
//
//  Programmer: Mark C. Miller
//  Creation:   November 15, 2004
//
// ****************************************************************************
template<>
void
EngineRPCExecutor<ProcInfoRPC>::Execute(ProcInfoRPC *rpc)
{
    debug2 << "Executing ProcInfoRPC: " << endl;

    TRY
    {
        rpc->SendReply(GetEngine()->GetProcessAttributes());
    }
    CATCH2(VisItException, e)
    {
        rpc->SendError(e.Message(), e.GetExceptionType());
    }
    ENDTRY
}

// ****************************************************************************
//  Method: EngineRPCExecutor<EnginePropertiesRPC>::Execute
//
//  Purpose:  Execute a request for process infor.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Oct 10 11:17:03 PDT 2011
//
// ****************************************************************************
template<>
void
EngineRPCExecutor<EnginePropertiesRPC>::Execute(EnginePropertiesRPC *rpc)
{
    debug2 << "Executing EnginePropertiesRPC: " << endl;

    TRY
    {
        EngineProperties props(GetEngine()->GetEngineProperties());
        rpc->SendReply(&props);
    }
    CATCH2(VisItException, e)
    {
        rpc->SendError(e.Message(), e.GetExceptionType());
    }
    ENDTRY
}

// ****************************************************************************
//  Method: EngineRPCExecutor<QueryRPC>::Execute
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
EngineRPCExecutor<QueryRPC>::Execute(QueryRPC *rpc)
{
    NetworkManager *netmgr = GetEngine()->GetNetMgr();

    debug2 << "Executing QueryRPC: " << endl;

    avtDataObjectQuery::RegisterProgressCallback(
                         Engine::EngineUpdateProgressCallback, (void*) rpc);
    avtDataObjectSource::RegisterProgressCallback(
                         Engine::EngineUpdateProgressCallback, (void*) rpc);
    LoadBalancer::RegisterProgressCallback(
                         Engine::EngineUpdateProgressCallback, (void*) rpc);
    avtDataObjectQuery::RegisterInitializeProgressCallback(
                         Engine::EngineInitializeProgressCallback,(void*) rpc);
    avtOriginatingSource::RegisterInitializeProgressCallback(
                         Engine::EngineInitializeProgressCallback,(void*) rpc);
 
    TRY
    {
        netmgr->Query(rpc->GetNetworkIds(), rpc->GetQueryAtts());
        rpc->SendReply(rpc->GetQueryAtts());
    }
    CATCH2(VisItException, e)
    {
        rpc->SendError(e.Message(), e.GetExceptionType());
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
    avtOriginatingSource::RegisterInitializeProgressCallback(
                               Engine::EngineInitializeProgressCallback, NULL);
}


// ****************************************************************************
//  Method: EngineRPCExecutor<QueryParametersRPC>::Execute
//
//  Purpose:  Retrieve query parameters.
//
//  Programmer: Kathleen Biagas
//  Creation:   July 15, 2011 
//
//  Modifications:
//
// ****************************************************************************
template<>
void
EngineRPCExecutor<QueryParametersRPC>::Execute(QueryParametersRPC *rpc)
{
    NetworkManager *netmgr = GetEngine()->GetNetMgr();

    debug2 << "Executing QueryParametersRPC: " << endl;

    TRY
    {
        QueryParametersRPC::MapNodeString s(
            netmgr->GetQueryParameters(rpc->GetQueryName()));
        rpc->SendReply(&s);
    }
    CATCH2(VisItException, e)
    {
        rpc->SendError(e.Message(), e.GetExceptionType());
    }
    ENDTRY
}


// ****************************************************************************
//  Method: EngineRPCExecutor<ReleaseDataRPC>::Execute
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
EngineRPCExecutor<ReleaseDataRPC>::Execute(ReleaseDataRPC *rpc)
{
    NetworkManager *netmgr = GetEngine()->GetNetMgr();

    debug2 << "Executing ReleaseDataRPC: " << rpc->GetID() << endl;
    TRY 
    {
        netmgr->DoneWithNetwork(rpc->GetID());
        rpc->SendReply();
    }
    CATCH2(VisItException, e)
    {
        rpc->SendError(e.Message(), e.GetExceptionType());
    }
    ENDTRY
}


// ****************************************************************************
//  Method: EngineRPCExecutor<OpenDatabaseRPC>::Execute
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
//    Hank Childs, Wed May 17 16:14:56 PDT 2006
//    Added exception handling.
//
//    Kathleen Bonnell, Tue Oct  9 14:40:10 PDT 2007
//    Use rpc flags for creation of MeshQuality and TimeDerivative 
//    expressions to set same flags in the DatabaseFactory. 
//
//    Brad Whitlock, Tue Jun 24 16:02:10 PDT 2008
//    Changed how the database plugin manager is accessed.
//
//    Brad Whitlock, Thu Feb 26 14:05:27 PST 2009
//    I added code to send engine error messages back to the viewer. It's
//    done differently than some other RPC's because this RPC is non-blocking.
//    This means we can't send the message back to the viewer via RPC results.
//    This mechanism is probably only good for simulations presently.
//
// ****************************************************************************
template<>
void
EngineRPCExecutor<OpenDatabaseRPC>::Execute(OpenDatabaseRPC *rpc)
{
    TRY
    {
        NetworkManager *netmgr = GetEngine()->GetNetMgr();

        debug2 << "Executing OpenDatabaseRPC: db=" 
               << rpc->GetDatabaseName().c_str()
               << ", time=" << rpc->GetTime() << endl;
        netmgr->GetDatabasePluginManager()->PluginAvailable(rpc->GetFileFormat());
   
        avtDatabaseFactory::SetCreateMeshQualityExpressions(
                            rpc->GetCreateMeshQualityExpressions()); 
        avtDatabaseFactory::SetCreateTimeDerivativeExpressions(
                            rpc->GetCreateTimeDerivativeExpressions()); 

        bool treatAllDBsAsTimeVarying = false;
        bool fileMayHaveUnloadedPlugin = false;
        netmgr->GetDBFromCache(rpc->GetDatabaseName(), rpc->GetTime(),
                               rpc->GetFileFormat().c_str(),
                               treatAllDBsAsTimeVarying,
                               fileMayHaveUnloadedPlugin,
                               rpc->GetIgnoreExtents());

        GetEngine()->PopulateSimulationMetaData(rpc->GetDatabaseName(),
                                           rpc->GetFileFormat());
    }
    CATCH2(VisItException, e)
    {
        GetEngine()->Error((e.GetExceptionType() + ": ") + e.Message());
        debug1 << "An error occurred while opening the database." << endl;
    }
    ENDTRY
}

// ****************************************************************************
//  Method: EngineRPCExecutor<DefineVirtualDatabaseRPC>::Execute
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
//    Kathleen Bonnell, Tue Oct  9 14:40:10 PDT 2007
//    Use rpc flags for creation of MeshQuality and TimeDerivative 
//    expressions to set same flags in the DatabaseFactory. 
//
//    Brad Whitlock, Tue Jun 24 16:02:35 PDT 2008
//    Changed how the database plugin manager is accessed.
//
//    Mark C. Miller, Wed Jun 17 16:10:59 PDT 2009
//    Added logic to send replies and catch exceptions.
// ****************************************************************************
template<>
void
EngineRPCExecutor<DefineVirtualDatabaseRPC>::Execute(DefineVirtualDatabaseRPC *rpc)
{
    NetworkManager *netmgr = GetEngine()->GetNetMgr();

    debug2 << "Executing DefineVirtualDatabaseRPC: "
           << "db=" << rpc->GetDatabaseName().c_str()
           << ", path=" << rpc->GetDatabasePath().c_str()
           << ", time=" << rpc->GetTime()
           << ", numStates=" << rpc->GetDatabaseFiles().size()
           << endl;
    for (size_t i = 0; i < rpc->GetDatabaseFiles().size(); ++i)
        debug5 << "file["<<i<<"]="<<rpc->GetDatabaseFiles()[i].c_str() << endl;

    TRY
    {
        netmgr->GetDatabasePluginManager()->PluginAvailable(rpc->GetFileFormat());

        avtDatabaseFactory::SetCreateMeshQualityExpressions(
            rpc->GetCreateMeshQualityExpressions()); 
        avtDatabaseFactory::SetCreateTimeDerivativeExpressions(
            rpc->GetCreateTimeDerivativeExpressions()); 

        netmgr->DefineDB(rpc->GetDatabaseName(), rpc->GetDatabasePath(),
            rpc->GetDatabaseFiles(), rpc->GetTime(), rpc->GetFileFormat());

        rpc->SendReply();
    }
    CATCH2(VisItException, e)
    {
        rpc->SendError(e.Message(), e.GetExceptionType());
    }
    ENDTRY
}

// ****************************************************************************
//  Method: EngineRPCExecutor<RenderRPC>::Execute
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
//    Mark C. Miller, Wed Oct  6 18:12:29 PDT 2004
//    Changed bool flag for 3D annots to integer mode
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Added code to operate on specific window id
//
//    Mark C. Miller, Thu Nov  3 16:59:41 PST 2005
//    Added compression controls
//
//    Hank Childs, Sun Dec  4 16:51:20 PST 2005
//    Allow SR mode to give progress during rendering.
//
//    Mark C. Miller, Wed Dec 14 16:43:07 PST 2005
//    Changed to pass compression bool to Engine::WriteData
//
//    Hank Childs, Sat Jan 28 11:40:35 PST 2006
//    Added OutputAllTimings.
//
//    Hank Childs, Tue Mar  7 15:13:33 PST 2006
//    Register one more callback, which can lead to an exception if we don't
//    do this, when a warning is issued during SR.
//
//    Mark C. Miller, Sat Jul 22 23:21:09 PDT 2006
//    Added leftEye to Render call to support stereo SR
//
//    Brad Whitlock, Mon Mar  2 16:35:50 PST 2009
//    I changed the code to account for NetworkManager::Render now returning
//    an avtDataObject_p.
//
//    Brad Whitlock, Mon Sep 22 21:26:20 PDT 2014
//    Handle data writing and status differently.
//
//    Brad Whitlock, Thu Sep 21 16:45:34 PDT 2017
//    Add alpha support. Add support for luminance image, value images.
//
// ****************************************************************************

template<>
void
EngineRPCExecutor<RenderRPC>::Execute(RenderRPC *rpc)
{
    NetworkManager *netmgr = GetEngine()->GetNetMgr();

    debug2 << "Executing RenderRPC for the following plots" << endl;
    debug2 << "   ";
    for (size_t i = 0; i < rpc->GetIDs().size(); i++)
       debug2 << rpc->GetIDs()[i] << ", ";
    debug2 << endl;

    avtDataObjectSource::RegisterProgressCallback(
                             Engine::EngineUpdateProgressCallback, (void*)rpc);
    avtOriginatingSource::RegisterInitializeProgressCallback(NULL, NULL);
    LoadBalancer::RegisterProgressCallback(NULL, NULL);
    avtCallback::RegisterWarningCallback(Engine::EngineWarningCallback,
                                         (void*)rpc);
    NetworkManager::RegisterInitializeProgressCallback(
                        Engine::EngineInitializeProgressCallback, (void*)rpc);
    NetworkManager::RegisterProgressCallback(
                           Engine::EngineUpdateProgressCallback, (void*)rpc);
    TRY 
    {
        // do the render
        avtDataObject_p image = NULL;

        if(rpc->GetImageType() == ValueImage)
        {
            image = netmgr->RenderValues(rpc->GetIDs(),rpc->GetSendZBuffer(),
                                         rpc->GetWindowID(), rpc->GetLeftEye());
        }
        else
        {
            int outImgWidth = 0, outImgHeight = 0;
            bool checkThreshold = true;
            image = netmgr->Render(rpc->GetImageType(), rpc->GetSendZBuffer(),
                        rpc->GetIDs(), checkThreshold,
                        rpc->GetAnnotMode(), rpc->GetWindowID(), rpc->GetLeftEye(),
                        outImgWidth, outImgHeight);
        }

        avtDataObjectWriter_p writer;
        if(*image != NULL)
        {
            writer = image->InstantiateWriter();
            writer->SetInput(image);
        }
        else
        {
            writer = netmgr->CreateNullDataWriter();
        }

        // Send the data back to the viewer.
        bool useCompression = netmgr->GetShouldUseCompression(rpc->GetWindowID());

        std::string errorMessage;
        bool  respondWithNull = false;
        int   scalableThreshold = -1;
        bool  scalableThresholdExceeded = false;
        int   currentTotalGlobalCellCount = 0;
        float cellCountMultiplier = 1.f;
        int   currentNetworkGlobalCellCount = 0;

        bool success = GetEngine()->GatherData(writer,
                           useCompression,
                           respondWithNull,
                           scalableThreshold, 
                           currentTotalGlobalCellCount, cellCountMultiplier,
                           Execute_statusCB, (void *)rpc,
                           Execute_writeCB, (void *)rpc,
                           errorMessage,
                           &scalableThresholdExceeded,
                           &currentNetworkGlobalCellCount);

        if (PAR_UIProcess() && !success)
            rpc->SendError(errorMessage);

        visitTimer->OutputAllTimings();
    }
    CATCH2(VisItException, e)
    {
        rpc->SendError(e.Message(), e.GetExceptionType());
    }
    ENDTRY

    avtDataObjectSource::RegisterProgressCallback(
                               Engine::EngineUpdateProgressCallback, NULL);
    LoadBalancer::RegisterProgressCallback(
                               Engine::EngineUpdateProgressCallback, NULL);
    avtOriginatingSource::RegisterInitializeProgressCallback(
                               Engine::EngineInitializeProgressCallback, NULL);
    avtCallback::RegisterWarningCallback(Engine::EngineWarningCallback, NULL);
    NetworkManager::RegisterInitializeProgressCallback(NULL, NULL);
    NetworkManager::RegisterProgressCallback(NULL, NULL);
}

// ****************************************************************************
//  Method: EngineRPCExecutor<CloneNetworkRPC>::Execute
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
//    Alister Maguire, Tue Oct 29 14:46:38 MST 2019
//    Moved the actual cloning procedure to AddQueryOverTimeFilter so
//    that the correct type of clone can be chosen.
//
// ****************************************************************************
template<>
void
EngineRPCExecutor<CloneNetworkRPC>::Execute(CloneNetworkRPC *rpc)
{
    NetworkManager *netmgr = GetEngine()->GetNetMgr();

    debug2 << "Executing CloneNetworkRPC: " << rpc->GetID() << endl;
    TRY
    {
        if (rpc->GetQueryOverTimeAtts() != NULL)
            netmgr->AddQueryOverTimeFilter(rpc->GetQueryOverTimeAtts(),
                                           rpc->GetID());
        rpc->SendReply();
    }
    CATCH2(VisItException, e)
    {
        netmgr->CancelNetwork();
        rpc->SendError(e.Message(), e.GetExceptionType());
    }
    ENDTRY
}

// ****************************************************************************
//  Method:  EngineRPCExecutor<SimulationCommandRPC>::Execute
//
//  Purpose:
//    Execute a simulation command.
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 18, 2005
//
//  Modifications:
//    Brad Whitlock, Fri Mar 27 11:31:31 PDT 2009
//    I made the arguments only be type string.
//
// ****************************************************************************
template<>
void
EngineRPCExecutor<SimulationCommandRPC>::Execute(SimulationCommandRPC *rpc)
{
    debug2 << "Executing SimulationCommandRPC: " << rpc->GetCommand().c_str() << endl;

    TRY
    {
        GetEngine()->ExecuteSimulationCommand(rpc->GetCommand(),
                                         rpc->GetStringData());
        rpc->SendReply();
    }
    CATCH2(VisItException, e)
    {
        rpc->SendError(e.Message(), e.GetExceptionType());
    }
    ENDTRY
}

// ****************************************************************************
//  Method:  EngineRPCExecutor<NamedSelectionRPC>::Execute
//
//  Purpose:
//      Handles a NamedSelection RPC.
//
//  Programmer:  Hank Childs
//  Creation:    January 29, 2009
//
//  Modifications:
//    Kathleen Bonnell, Wed Mar 25 15:35:32 MST 2009
//    Renamed NamedSelectionRPC enum names to compile on windows.
//
//    Brad Whitlock, Tue Dec 14 14:05:04 PST 2010
//    Pass selection properties to CreateNamedSelection. I also added status
//    updates since the RPC is now non-blocking.
//
//    Brad Whitlock, Mon Aug 22 10:21:17 PDT 2011
//    I changed how named selections get applied.
//
//    Brad Whitlock, Wed Sep  7 14:33:01 PDT 2011
//    I added NS_UPDATE and code to clear the NSM's cache.
//
// ****************************************************************************
template<>
void
EngineRPCExecutor<NamedSelectionRPC>::Execute(NamedSelectionRPC *rpc)
{
    NetworkManager *netmgr = GetEngine()->GetNetMgr();

    const char *mName = "Executing NamedSelectionRPC: ";
    debug2 << mName << "0" << endl;

    avtDataObjectSource::RegisterProgressCallback(
                               Engine::EngineUpdateProgressCallback, (void*)rpc);
    LoadBalancer::RegisterProgressCallback(
                               Engine::EngineUpdateProgressCallback, (void*)rpc);
    avtOriginatingSource::RegisterInitializeProgressCallback(
                               Engine::EngineInitializeProgressCallback, (void*)rpc);
    avtCallback::RegisterWarningCallback(Engine::EngineWarningCallback, (void*)rpc);

    TRY
    {
        SelectionSummary summary;
        summary.SetName(rpc->GetSelectionName());

        switch (rpc->GetNamedSelectionOperation())
        {
        case NamedSelectionRPC::NS_CREATE:
            avtNamedSelectionManager::GetInstance()->ClearCache(rpc->GetSelectionName());
            summary = netmgr->CreateNamedSelection(
                rpc->GetPlotID(), rpc->GetSelectionProperties());
            break;
        case NamedSelectionRPC::NS_UPDATE:
            if(!rpc->GetAllowCache())
                avtNamedSelectionManager::GetInstance()->ClearCache(rpc->GetSelectionName());
            netmgr->DeleteNamedSelection(rpc->GetSelectionName());
            summary = netmgr->CreateNamedSelection(
                rpc->GetPlotID(), rpc->GetSelectionProperties());
            break;
        case NamedSelectionRPC::NS_DELETE:
            avtNamedSelectionManager::GetInstance()->ClearCache(rpc->GetSelectionName());
            netmgr->DeleteNamedSelection(rpc->GetSelectionName());
            break;
        case NamedSelectionRPC::NS_LOAD:
            netmgr->LoadNamedSelection(rpc->GetSelectionName());
            break;
        case NamedSelectionRPC::NS_SAVE:
            netmgr->SaveNamedSelection(rpc->GetSelectionName());
            break;
        }
    debug2 << mName << "1" << endl;
        rpc->SendReply(&summary);
    }
    CATCH2(VisItException, e)
    {
    debug2 << mName << "2" << endl;

        rpc->SendError(e.Message(), e.GetExceptionType());
    }
    ENDTRY

    avtDataObjectSource::RegisterProgressCallback(
                               Engine::EngineUpdateProgressCallback, NULL);
    LoadBalancer::RegisterProgressCallback(
                               Engine::EngineUpdateProgressCallback, NULL);
    avtOriginatingSource::RegisterInitializeProgressCallback(
                               Engine::EngineInitializeProgressCallback, NULL);
    avtCallback::RegisterWarningCallback(Engine::EngineWarningCallback, NULL);

    debug2 << mName << "3,end" << endl;
}

// ****************************************************************************
//  Method:  EngineRPCExecutor<ConstructDataBinningRPC>::Execute
//
//  Purpose:
//      Constructs a data binning.
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
template<>
void
EngineRPCExecutor<ConstructDataBinningRPC>::Execute(ConstructDataBinningRPC *rpc)
{
    NetworkManager *netmgr = GetEngine()->GetNetMgr();

    debug2 << "Executing ConstructDataBinningRPC." << endl;

    avtDataObjectSource::RegisterProgressCallback(NULL, NULL);
    LoadBalancer::RegisterProgressCallback(NULL, NULL);
    avtOriginatingSource::RegisterInitializeProgressCallback(NULL, NULL);
    avtCallback::RegisterWarningCallback(Engine::EngineWarningCallback, (void*)rpc);
    TRY
    {
        netmgr->ConstructDataBinning(rpc->GetID(), rpc->GetConstructDataBinningAtts());
        rpc->SendReply();
    }
    CATCH2(VisItException, e)
    {
        rpc->SendError(e.Message(), e.GetExceptionType());
    }
    ENDTRY

    avtDataObjectSource::RegisterProgressCallback(
                               Engine::EngineUpdateProgressCallback, NULL);
    LoadBalancer::RegisterProgressCallback(
                               Engine::EngineUpdateProgressCallback, NULL);
    avtOriginatingSource::RegisterInitializeProgressCallback(
                               Engine::EngineInitializeProgressCallback, NULL);
    avtCallback::RegisterWarningCallback(Engine::EngineWarningCallback, NULL);
}

// ****************************************************************************
//  Method:  EngineRPCExecutor<ExportDatabaseRPC>::Execute
//
//  Purpose:
//      Exports a database.
//
//  Programmer:  Hank Childs
//  Creation:    May 26, 2005
//
//  Modifications:
//    Brad Whitlock, Mon Apr  7 15:37:46 PDT 2014
//    Support multiple plots.
//    Work partially supported by DOE Grant SC0007548.
//
// ****************************************************************************
template<>
void
EngineRPCExecutor<ExportDatabaseRPC>::Execute(ExportDatabaseRPC *rpc)
{
    NetworkManager *netmgr = GetEngine()->GetNetMgr();

    debug2 << "Executing ExportDatabaseRPC." << endl;

    avtDataObjectSource::RegisterProgressCallback(NULL, NULL);
    LoadBalancer::RegisterProgressCallback(NULL, NULL);
    avtOriginatingSource::RegisterInitializeProgressCallback(NULL, NULL);
    avtCallback::RegisterWarningCallback(Engine::EngineWarningCallback, (void*)rpc);
    TRY
    {
        netmgr->ExportDatabases(rpc->GetIDs(), rpc->GetExportDBAtts(), rpc->GetTimeSuffix());
        rpc->SendReply();
    }
    CATCH2(VisItException, e)
    {
        rpc->SendError(e.Message(), e.GetExceptionType());
    }
    ENDTRY

    avtDataObjectSource::RegisterProgressCallback(
                               Engine::EngineUpdateProgressCallback, NULL);
    LoadBalancer::RegisterProgressCallback(
                               Engine::EngineUpdateProgressCallback, NULL);
    avtOriginatingSource::RegisterInitializeProgressCallback(
                               Engine::EngineInitializeProgressCallback, NULL);
    avtCallback::RegisterWarningCallback(Engine::EngineWarningCallback, NULL);
}

// ****************************************************************************
//  Method:  EngineRPCExecutor<SetEFileOpenOptionsRPC>::Execute
//
//  Purpose:
//    Set the new default file opening options in the database factory.
//
//  Arguments:
//    opts       the new options
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2008
//
// ****************************************************************************
template<>
void
EngineRPCExecutor<SetEFileOpenOptionsRPC>::Execute(SetEFileOpenOptionsRPC *rpc)
{
    avtDatabaseFactory::SetDefaultFileOpenOptions(rpc->GetFileOpenOptions());
    rpc->SendReply();
}

// ****************************************************************************
//  Method:  EngineRPCExecutor<SetPrecisionTypeRPC>::Execute
//
//  Purpose:
//    Set the new precision type in the database factory.
//
//  Programmer:  Kathleen Biagas
//  Creation:    August 1, 2013
//
// ****************************************************************************
template<>
void
EngineRPCExecutor<SetPrecisionTypeRPC>::Execute(SetPrecisionTypeRPC *rpc)
{
    avtDatabaseFactory::SetPrecisionType(rpc->GetPrecisionType());
    rpc->SendReply();
}

// ****************************************************************************
//  Method:  EngineRPCExecutor<SetBackendTypeRPC>::Execute
//
//  Purpose:
//    Set the new backend type.
//
//  Programmer:  Cameron Christensen
//  Creation:    June 10, 2014
//
// ****************************************************************************
template<>
void
EngineRPCExecutor<SetBackendTypeRPC>::Execute(SetBackendTypeRPC *rpc)
{
    avtDatabaseFactory::SetBackendType(rpc->GetBackendType());
    avtCallback::SetBackendType((GlobalAttributes::BackendType)rpc->GetBackendType());
    rpc->SendReply();
}


// ****************************************************************************
//  Method:  EngineRPCExecutor<SetRemoveDuplicateNodesRPC>::Execute
//
//  Purpose:
//    Set the flag specifying removal of duplicate nodes in the database
//    factory.
//
//  Programmer:  Kathleen Biagas
//  Creation:    December 22, 2014
//
// ****************************************************************************
template<>
void
EngineRPCExecutor<SetRemoveDuplicateNodesRPC>::Execute(SetRemoveDuplicateNodesRPC *rpc)
{
    avtDatabaseFactory::SetRemoveDuplicateNodes(rpc->GetRemoveDuplicateNodes());
    rpc->SendReply();
}

// ****************************************************************************
//  Method: EngineRPCExecutor<LaunchRPC>::Execute
//
//  Purpose:  Launch a process.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Nov 28 16:31:49 PST 2011
//
// ****************************************************************************
template<>
void
EngineRPCExecutor<LaunchRPC>::Execute(LaunchRPC *rpc)
{
    debug2 << "Executing LaunchRPC: " << endl;

    TRY
    {
        GetEngine()->LaunchProcess(rpc->GetLaunchArgs());
        rpc->SendReply();
    }
    CATCH2(VisItException, e)
    {
        rpc->SendError(e.Message(), e.GetExceptionType());
    }
    ENDTRY
}

#endif
