# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#   Hank Childs, Sat Aug 21 14:35:47 PDT 2010
#   Rename DDF to DataBinning.
#
#   Kathleen Biagas, Fri Jul 15 11:34:11 PDT 2011
#   Added QueryParametersRPC.
#
#   Brad Whitlock, Mon Oct 10 11:14:46 PDT 2011
#   Added EnginePropertiesRPC.
#
#   Kathleen Biagas, Wed Aug  7 15:56:48 PDT 2013
#   Added SetPrecisionTypeRPC.
#
#   Cameron Christensen, Tuesday, June 10, 2014
#   Added SetBackendTypeRPC.
#
#   Kathleen Biagas, Mon Dec 22 05:53:52 PST 2014
#   Added SetRemoveDuplicateNodesRPC.
#
#   Kathleen Biagas, Thu Oct 3, 2024
#   Utilize visit_add_library.
#
#****************************************************************************

set(ENGINERPC_SOURCES
    ApplyOperatorRPC.C
    ClearCacheRPC.C
    CloneNetworkRPC.C
    ConstructDataBinningRPC.C
    DefineVirtualDatabaseRPC.C
    EnginePropertiesRPC.C
    EngineState.C
    ExecuteRPC.C
    ExportDatabaseRPC.C
    MakePlotRPC.C
    NamedSelectionRPC.C
    OpenDatabaseRPC.C
    PickRPC.C
    ProcInfoRPC.C
    QueryParametersRPC.C
    QueryRPC.C
    ReadRPC.C
    ReleaseDataRPC.C
    RenderRPC.C
    SetBackendTypeRPC.C
    SetEFileOpenOptionsRPC.C
    SetPrecisionTypeRPC.C
    SetRemoveDuplicateNodesRPC.C
    SetWinAnnotAttsRPC.C
    SimulationCommandRPC.C
    StartPickRPC.C
    StartQueryRPC.C
    UpdatePlotAttsRPC.C
    UseNetworkRPC.C)

set(ENGINERPC_HEADERS
    ApplyOperatorRPC.h
    ClearCacheRPC.h
    CloneNetworkRPC.h
    ConstructDataBinningRPC.h
    DefineVirtualDatabaseRPC.h
    EnginePropertiesRPC.h
    EngineState.h
    ExecuteRPC.h
    ExportDatabaseRPC.h
    MakePlotRPC.h
    NamedSelectionRPC.h
    OpenDatabaseRPC.h
    PickRPC.h
    ProcInfoRPC.h
    QueryParametersRPC.h
    QueryRPC.h
    ReadRPC.h
    ReleaseDataRPC.h
    RenderRPC.h
    SetBackendTypeRPC.h
    SetEFileOpenOptionsRPC.h
    SetPrecisionTypeRPC.h
    SetRemoveDuplicateNodesRPC.h
    SetWinAnnotAttsRPC.h
    SimulationCommandRPC.h
    StartPickRPC.h
    StartQueryRPC.h
    UpdatePlotAttsRPC.h
    UseNetworkRPC.h
    engine_rpc_exports.h)

visit_add_library(
    NAME        enginerpc
    SOURCES     ${ENGINERPC_SOURCES}
    HEADERS     ${ENGINERPC_HEADERS}
    INCLUDES    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                $<BUILD_INTERFACE:${VISIT_SOURCE_DIR}/avt/Pipeline/Data>
                $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/engine/rpc>
    DEPENDS_ON  visitcommon avtdbatts)

