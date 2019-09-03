// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef ENGINE_STATE_H
#define ENGINE_STATE_H

#include <engine_rpc_exports.h>
#include <ReadRPC.h>
#include <RenderRPC.h>
#include <ExecuteRPC.h>
#include <ApplyOperatorRPC.h>
#include <ClearCacheRPC.h>
#include <CloneNetworkRPC.h>
#include <ConstructDataBinningRPC.h>
#include <DefineVirtualDatabaseRPC.h>
#include <EnginePropertiesRPC.h>
#include <ExportDatabaseRPC.h>
#include <LaunchRPC.h>
#include <MakePlotRPC.h>
#include <NamedSelectionRPC.h>
#include <OpenDatabaseRPC.h>
#include <PickRPC.h>
#include <ProcInfoRPC.h>
#include <QueryRPC.h>
#include <QueryParametersRPC.h>
#include <ReleaseDataRPC.h>
#include <SetWinAnnotAttsRPC.h>
#include <SimulationCommand.h>
#include <SimulationCommandRPC.h>
#include <SILAttributes.h>
#include <StartPickRPC.h>
#include <StartQueryRPC.h>
#include <UpdatePlotAttsRPC.h>
#include <UseNetworkRPC.h>
#include <ExpressionList.h>
#include <FileOpenOptions.h>
#include <SetEFileOpenOptionsRPC.h>
#include <SetPrecisionTypeRPC.h>
#include <SetRemoveDuplicateNodesRPC.h>
#include <SetBackendTypeRPC.h>
#include <SelectionProperties.h>
#include <StatusAttributes.h>

class ENGINE_RPC_API EngineState
{
public:
    EngineState();
    ~EngineState();
    size_t                     GetNumStateObjects();
    VisItRPC*                  GetStateObject(int i);

    void                       SetupComponentRPCs(Xfer* xfer);

    ReadRPC&                    GetReadRPC() { return readRPC; }
    ApplyOperatorRPC&           GetApplyOperatorRPC() { return applyOperatorRPC; }
    MakePlotRPC&                GetMakePlotRPC() { return makePlotRPC; }
    UseNetworkRPC&              GetUseNetworkRPC() { return useNetworkRPC; }
    UpdatePlotAttsRPC&          GetUpdatePlotAttsRPC() { return updatePlotAttsRPC; }
    ExecuteRPC&                 GetExecuteRPC() { return executeRPC; }
    PickRPC&                    GetPickRPC() { return pickRPC; }
    StartPickRPC&               GetStartPickRPC(){ return startPickRPC; }
    StartQueryRPC&              GetStartQueryRPC() { return startQueryRPC; }
    ClearCacheRPC&              GetClearCacheRPC() { return clearCacheRPC; }
    QueryRPC&                   GetQueryRPC() { return queryRPC; }
    QueryParametersRPC&         GetQueryParametersRPC() { return queryParametersRPC; }
    ReleaseDataRPC&             GetReleaseDataRPC() { return releaseDataRPC; }
    OpenDatabaseRPC&            GetOpenDatabaseRPC() { return openDatabaseRPC; }
    DefineVirtualDatabaseRPC&   GetDefineVirtualDatabaseRPC() { return defineVirtualDatabaseRPC; }
    RenderRPC&                  GetRenderRPC() { return renderRPC; }
    SetWinAnnotAttsRPC&         GetSetWinAnnotAttsRPC() { return setWinAnnotAttsRPC; }
    CloneNetworkRPC&            GetCloneNetworkRPC() { return cloneNetworkRPC; }
    ProcInfoRPC&                GetProcInfoRPC() { return procInfoRPC; }
    SimulationCommandRPC&       GetSimulationCommandRPC() { return simulationCommandRPC; }
    ExportDatabaseRPC&          GetExportDatabaseRPC() { return exportDatabaseRPC; }
    ConstructDataBinningRPC&    GetConstructDataBinningRPC() { return constructDataBinningRPC; }
    NamedSelectionRPC&          GetNamedSelectionRPC() { return namedSelectionRPC; }
    SetEFileOpenOptionsRPC&     GetSetEFileOpenOptionsRPC() { return setEFileOpenOptionsRPC; }
    SetPrecisionTypeRPC&        GetSetPrecisionTypeRPC() { return setPrecisionTypeRPC; }
    SetBackendTypeRPC&          GetSetBackendTypeRPC() { return setBackendTypeRPC; }
    SetRemoveDuplicateNodesRPC& GetSetRemoveDuplicateNodesRPC() { return setRemoveDuplicateNodesRPC; }
    EnginePropertiesRPC&        GetEnginePropertiesRPC() { return enginePropertiesRPC; }
    LaunchRPC&                  GetLaunchRPC() { return launchRPC; }
private:

    ReadRPC                  readRPC;
    ApplyOperatorRPC         applyOperatorRPC;
    MakePlotRPC              makePlotRPC;
    UseNetworkRPC            useNetworkRPC;
    UpdatePlotAttsRPC        updatePlotAttsRPC;
    ExecuteRPC               executeRPC;
    PickRPC                  pickRPC;
    StartPickRPC             startPickRPC;
    StartQueryRPC            startQueryRPC;
    ClearCacheRPC            clearCacheRPC;
    QueryRPC                 queryRPC;
    QueryParametersRPC       queryParametersRPC;
    ReleaseDataRPC           releaseDataRPC;
    OpenDatabaseRPC          openDatabaseRPC;
    DefineVirtualDatabaseRPC defineVirtualDatabaseRPC;
    RenderRPC                renderRPC;
    SetWinAnnotAttsRPC       setWinAnnotAttsRPC;
    CloneNetworkRPC          cloneNetworkRPC;
    ProcInfoRPC              procInfoRPC;
    SimulationCommandRPC     simulationCommandRPC;
    ExportDatabaseRPC        exportDatabaseRPC;
    ConstructDataBinningRPC  constructDataBinningRPC;
    NamedSelectionRPC        namedSelectionRPC;
    SetEFileOpenOptionsRPC   setEFileOpenOptionsRPC;
    SetPrecisionTypeRPC      setPrecisionTypeRPC;
    SetBackendTypeRPC        setBackendTypeRPC;
    SetRemoveDuplicateNodesRPC setRemoveDuplicateNodesRPC;
    EnginePropertiesRPC      enginePropertiesRPC;
    LaunchRPC                launchRPC;
    StatusAttributes        *statusAtts;

    //void SetupComponentRPCs(Xfer* xfer);
    std::vector<VisItRPC*> objVector;
    friend class EngineMethods;
public:
    ExpressionList           exprList;
};

#endif
