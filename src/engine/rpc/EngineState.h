/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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
