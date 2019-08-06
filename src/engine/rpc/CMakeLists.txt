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
#****************************************************************************

SET(ENGINERPC_SOURCES
ApplyOperatorRPC.C
ClearCacheRPC.C
CloneNetworkRPC.C
DBQueryOverTimeRPC.C
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
QueryRPC.C
QueryParametersRPC.C
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
UseNetworkRPC.C
)

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_COMMON_INCLUDES}
${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
${VISIT_SOURCE_DIR}/avt/Math
${VISIT_SOURCE_DIR}/avt/Pipeline/AbstractFilters
${VISIT_SOURCE_DIR}/avt/Pipeline/Data
${VISIT_SOURCE_DIR}/avt/Pipeline/Pipeline
${VISIT_SOURCE_DIR}/avt/Pipeline/Sinks
${VISIT_SOURCE_DIR}/avt/Pipeline/Sources
${VISIT_SOURCE_DIR}/avt/VisWindow/VisWindow
${VISIT_SOURCE_DIR}/engine/rpc
)

LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY})

ADD_LIBRARY(enginerpc ${ENGINERPC_SOURCES})
TARGET_LINK_LIBRARIES(enginerpc visitcommon avtdbatts)

VISIT_INSTALL_TARGETS(enginerpc)
