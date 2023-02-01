# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

# This flag controls whether we try to build the simv2 runtime with 
# viewercore support.
SET(SIMV2_VIEWER_INTEGRATION ON)

IF(SIMV2_VIEWER_INTEGRATION)
    SET(LIBSIM_RUNTIME_VIEWER_SOURCES SimEngineManager.C SimFileServer.C)
    SET(LIBSIM_RUNTIME_VIEWER_LIBS_SER viewercore_ser)
    SET(LIBSIM_RUNTIME_VIEWER_LIBS_PAR viewercore_par)
    SET(VISIT_VIEWER_DIRS
        ${VISIT_SOURCE_DIR}/viewer/core
        ${VISIT_SOURCE_DIR}/viewer/rpc
    )
    ADD_DEFINITIONS(-DSIMV2_VIEWER_INTEGRATION)
ENDIF(SIMV2_VIEWER_INTEGRATION)

SET(LIBSIM_RUNTIME_SOURCES 
${LIBSIM_RUNTIME_VIEWER_SOURCES}
SimEngine.C
SimOperatorPluginManager.C
SimPlotPluginManager.C
VisItControlInterfaceRuntime.C 
VisItDataInterfaceRuntime.C 
VisItDataInterfaceRuntimeP.C 
simv2_CommandMetaData.C
simv2_CurveData.C
simv2_CurveMetaData.C
simv2_CurvilinearMesh.C
simv2_CSGMesh.C
simv2_DomainBoundaries.C
simv2_DomainList.C 
simv2_DomainNesting.C
simv2_ExpressionMetaData.C
simv2_MaterialData.C
simv2_MaterialMetaData.C
simv2_MeshMetaData.C
simv2_MessageMetaData.C
simv2_NameList.C
simv2_OptionList.C
simv2_PointMesh.C
simv2_RectilinearMesh.C
simv2_SimulationMetaData.C
simv2_SpeciesData.C
simv2_SpeciesMetaData.C
simv2_UnstructuredMesh.C
simv2_VariableData.C
simv2_VariableMetaData.C
simv2_View2D.C
simv2_View3D.C
)

# If VisIt is static then we need some static symbol lookup functions
IF(VISIT_STATIC)
    SET(SIMV2_PLUGIN_DIR ${VISIT_SOURCE_DIR}/databases/SimV2)

    SET(SIMV2_PLUGIN_SOURCES
        ${SIMV2_PLUGIN_DIR}/SimV2CommonPluginInfo.C
        ${SIMV2_PLUGIN_DIR}/SimV2EnginePluginInfo.C
        ${SIMV2_PLUGIN_DIR}/SimV2GetMesh.C
        ${SIMV2_PLUGIN_DIR}/SimV2PluginInfo.C
        ${SIMV2_PLUGIN_DIR}/avtSimV2FileFormat.C
        ${SIMV2_PLUGIN_DIR}/avtSimV2Writer.C
     )

    SET(LIBSIM_STATIC_SOURCES ${SIMV2_PLUGIN_SOURCES} SimStaticSymbolLocator.C)
ENDIF(VISIT_STATIC)

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_COMMON_INCLUDES}
${VTK_INCLUDE_DIRS}
${VISIT_VIEWER_DIRS}
${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
${VISIT_SOURCE_DIR}/avt/DataBinning
${VISIT_SOURCE_DIR}/avt/Database/Database
${VISIT_SOURCE_DIR}/avt/Database/Formats
${VISIT_SOURCE_DIR}/avt/Database/Ghost
${VISIT_SOURCE_DIR}/avt/Expressions/Abstract
${VISIT_SOURCE_DIR}/avt/Expressions/CMFE
${VISIT_SOURCE_DIR}/avt/Expressions/Conditional
${VISIT_SOURCE_DIR}/avt/Expressions/Derivations
${VISIT_SOURCE_DIR}/avt/Expressions/General
${VISIT_SOURCE_DIR}/avt/Expressions/ImageProcessing
${VISIT_SOURCE_DIR}/avt/Expressions/Management
${VISIT_SOURCE_DIR}/avt/Expressions/Math
${VISIT_SOURCE_DIR}/avt/Expressions/MeshQuality
${VISIT_SOURCE_DIR}/avt/FileWriter
${VISIT_SOURCE_DIR}/avt/Filters
${VISIT_SOURCE_DIR}/avt/Math
${VISIT_SOURCE_DIR}/avt/VisWindow/VisWindow
${VISIT_SOURCE_DIR}/avt/Pipeline/AbstractFilters
${VISIT_SOURCE_DIR}/avt/Pipeline/Data
${VISIT_SOURCE_DIR}/avt/Pipeline/Pipeline
${VISIT_SOURCE_DIR}/avt/Pipeline/Sinks
${VISIT_SOURCE_DIR}/avt/Pipeline/Sources
${VISIT_SOURCE_DIR}/avt/Plotter
${VISIT_SOURCE_DIR}/avt/Plotter/vtk
${VISIT_SOURCE_DIR}/avt/Queries/Abstract
${VISIT_SOURCE_DIR}/avt/Queries/Misc
${VISIT_SOURCE_DIR}/avt/Queries/Pick
${VISIT_SOURCE_DIR}/avt/Queries/Queries
${VISIT_SOURCE_DIR}/avt/View
${VISIT_SOURCE_DIR}/avt/VisWindow/Colleagues
${VISIT_SOURCE_DIR}/avt/VisWindow/Proxies
${VISIT_SOURCE_DIR}/avt/VisWindow/Tools
${VISIT_SOURCE_DIR}/avt/VisWindow/VisWindow
${VISIT_SOURCE_DIR}/engine/main
${VISIT_SOURCE_DIR}/engine/proxy
${VISIT_SOURCE_DIR}/engine/rpc
${VISIT_SOURCE_DIR}/sim/V2/lib
${VISIT_SOURCE_DIR}/visit_vtk/full
${VISIT_SOURCE_DIR}/visit_vtk/lightweight
${SIMV2_PLUGIN_DIR}
)

LINK_DIRECTORIES(
${LIBRARY_OUTPUT_DIRECTORY}
${VTK_LIBRARY_DIRS}
${ICET_LIBRARY_DIR}
${AVT_REQUIRED_LIB_DIR}
${ALL_THIRDPARTY_IO_LIBRARY_DIR}
)


ADD_LIBRARY(simV2runtime_ser ${LIBSIM_RUNTIME_SOURCES} ${LIBSIM_STATIC_SOURCES})
TARGET_LINK_LIBRARIES(simV2runtime_ser engine_ser ${LIBSIM_RUNTIME_VIEWER_LIBS_SER})

IF(NOT APPLE)
    SET_TARGET_PROPERTIES(simV2runtime_ser PROPERTIES INSTALL_RPATH "$ORIGIN")
ENDIF(NOT APPLE)
IF(VISIT_ENABLE_XDB)
    ADD_TARGET_DEFINITIONS(simV2runtime_ser HAVE_XDB)
ENDIF(VISIT_ENABLE_XDB)

VISIT_INSTALL_TARGETS(simV2runtime_ser)

IF(VISIT_PARALLEL)
    ADD_PARALLEL_LIBRARY(simV2runtime_par ${LIBSIM_RUNTIME_SOURCES} ${LIBSIM_STATIC_SOURCES})

    TARGET_LINK_LIBRARIES(simV2runtime_par engine_par ${LIBSIM_RUNTIME_VIEWER_LIBS_PAR})

    if(NOT WIN32)
        target_link_libraries(simV2runtime_par ${VTK_LIBRARIES})
    endif()

    IF(NOT APPLE)
        SET_TARGET_PROPERTIES(simV2runtime_par PROPERTIES INSTALL_RPATH "$ORIGIN")
    ENDIF(NOT APPLE)
    IF(VISIT_ENABLE_XDB)
        ADD_TARGET_DEFINITIONS(simV2runtime_par HAVE_XDB)
    ENDIF(VISIT_ENABLE_XDB)

    VISIT_INSTALL_TARGETS(simV2runtime_par)
ENDIF(VISIT_PARALLEL)
