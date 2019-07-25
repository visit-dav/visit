# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Wed Oct 10 10:39:15 PDT 2012
#   Add ddtsim only if VISIT_DDT is chosen.
#
#   Brad Whitlock, Fri May 17 13:43:48 PDT 2013
#   Define LIBSIM_SOURCES.
#
#   Brad Whitlock, Fri Aug 14 12:28:12 PDT 2015
#   Add VisIt_OptionList.
#
#****************************************************************************

# We define this value here so we can use it in multiple CMakeLists.txt under
# this directory.
SET(LIBSIM_SOURCES
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisItControlInterface_V2.c 
${CMAKE_CURRENT_SOURCE_DIR}/lib/SimUI.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/SimV2Tracing.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisItFortran.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_CommandMetaData.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_CSGMesh.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_CurveData.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_CurveMetaData.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_CurvilinearMesh.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_DomainBoundaries.c 
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_DomainList.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_DomainNesting.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_ExpressionMetaData.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_MaterialData.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_MaterialMetaData.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_MeshMetaData.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_MessageMetaData.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_NameList.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_OptionList.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_PointMesh.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_RectilinearMesh.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_SimulationMetaData.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_SpeciesData.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_SpeciesMetaData.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_UnstructuredMesh.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_VariableData.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_VariableMetaData.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_View2D.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_View3D.c
)

ADD_SUBDIRECTORY(lib)
ADD_SUBDIRECTORY(runtime)
IF(VISIT_DDT)
    ADD_SUBDIRECTORY(ddtsim)
ENDIF(VISIT_DDT)
# Disable swig bindings on Windows for now. Generated code has some errors.
IF(UNIX)
    ADD_SUBDIRECTORY(swig)
ENDIF(UNIX)
