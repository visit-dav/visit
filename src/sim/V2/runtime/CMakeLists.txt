# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Fri Jun 13, 2025
#   Utilize visit_add_library.
#
#****************************************************************************

# This flag controls whether we try to build the simv2 runtime with
# viewercore support.
set(SIMV2_VIEWER_INTEGRATION ON)

if(SIMV2_VIEWER_INTEGRATION)
    set(LIBSIM_RUNTIME_VIEWER_SOURCES SimEngineManager.C SimFileServer.C)
    set(LIBSIM_RUNTIME_VIEWER_HEADERS SimEngineManager.h SimFileServer.h)
endif()

set(LIBSIM_RUNTIME_SOURCES
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
    simv2_View3D.C)


set(LIBSIM_RUNTIME_HEADERS
    ${LIBSIM_RUNTIME_VIEWER_HEADERS}
    SimEngine.h
    SimOperatorPluginManager.h
    SimPlotPluginManager.h
    VisItControlInterfaceRuntime.h
    VisItDataInterfaceRuntime.h
    VisItDataInterfaceRuntimeP.h
    simv2_CommandMetaData.h
    simv2_CurveData.h
    simv2_CurveMetaData.h
    simv2_CurvilinearMesh.h
    simv2_CSGMesh.h
    simv2_DomainBoundaries.h
    simv2_DomainList.h
    simv2_DomainNesting.h
    simv2_ExpressionMetaData.h
    simv2_MaterialData.h
    simv2_MaterialMetaData.h
    simv2_MeshMetaData.h
    simv2_MessageMetaData.h
    simv2_NameList.h
    simv2_OptionList.h
    simv2_PointMesh.h
    simv2_RectilinearMesh.h
    simv2_SimulationMetaData.h
    simv2_SpeciesData.h
    simv2_SpeciesMetaData.h
    simv2_TypeTraits.hxx
    simv2_UnstructuredMesh.h
    simv2_VariableData.h
    simv2_VariableMetaData.h
    simv2_View2D.h
    simv2_View3D.h)

# If VisIt is static then we need some static symbol lookup functions
if(VISIT_STATIC)
    set(SIMV2_PLUGIN_DIR ${VISIT_SOURCE_DIR}/databases/SimV2)

    set(SIMV2_PLUGIN_SOURCES
        ${SIMV2_PLUGIN_DIR}/SimV2CommonPluginInfo.C
        ${SIMV2_PLUGIN_DIR}/SimV2EnginePluginInfo.C
        ${SIMV2_PLUGIN_DIR}/SimV2GetMesh.C
        ${SIMV2_PLUGIN_DIR}/SimV2PluginInfo.C
        ${SIMV2_PLUGIN_DIR}/avtSimV2FileFormat.C
        ${SIMV2_PLUGIN_DIR}/avtSimV2Writer.C)

    set(LIBSIM_STATIC_SOURCES ${SIMV2_PLUGIN_SOURCES} SimStaticSymbolLocator.C)
endif()

visit_add_library(
    NAME        simV2runtime_ser
    SOURCES     ${LIBSIM_RUNTIME_SOURCES}
                ${LIBSIM_STATIC_SOURCES}
    HEADERS     ${LIBSIM_RUNTIME_HEADERS}
    INCLUDES    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                $<BUILD_INTERFACE:${SIMV2_PLUGIN_DIR}>
                $<BUILD_INTERFACE:${VISIT_SOURCE_DIR}/sim/V2/lib>
    DEFINES     $<$<BOOL:${SIMV2_VIEWER_INTEGRATION}>:SIMV2_VIEWER_INTEGRATION>
                $<$<BOOL:${VISIT_ENABLE_XDB}>:HAVE_XDB>
    DEPENDS_ON  engine_ser
                $<$<BOOL:${SIMV2_VIEWER_INTEGRATION}>:viewercore_ser>)

if(NOT APPLE)
    set_target_properties(simV2runtime_ser PROPERTIES INSTALL_RPATH "$ORIGIN")
endif()

if(VISIT_STATIC)
    set_target_properties(simV2runtime_ser PROPERTIES
        LINK_DIRECTORIES ${ALL_THIRDPARTY_IO_LIBRARY_DIR})
endif()

if(VISIT_PARALLEL)
    visit_add_parallel_library(
        NAME        simV2runtime_par
        SOURCES     ${LIBSIM_RUNTIME_SOURCES}
                    ${LIBSIM_STATIC_SOURCES}
        HEADERS     ${LIBSIM_RUNTIME_HEADERS}
        INCLUDES    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                    $<BUILD_INTERFACE:${SIMV2_PLUGIN_DIR}>
                    $<BUILD_INTERFACE:${VISIT_SOURCE_DIR}/sim/V2/lib>
        DEFINES     $<$<BOOL:${SIMV2_VIEWER_INTEGRATION}>:SIMV2_VIEWER_INTEGRATION>
                    $<$<BOOL:${VISIT_ENABLE_XDB}>:HAVE_XDB>
        DEPENDS_ON  engine_par
                    $<$<BOOL:${SIMV2_VIEWER_INTEGRATION}>:viewercore_par>)

    if(NOT APPLE)
        set_target_properties(simV2runtime_par PROPERTIES INSTALL_RPATH "$ORIGIN")
    endif()

    if(VISIT_STATIC)
        set_target_properties(simV2runtime_par PROPERTIES
            LINK_DIRECTORIES ${ALL_THIRDPARTY_IO_LIBRARY_DIR})
    endif()
endif()
