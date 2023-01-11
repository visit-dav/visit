# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#  Kathleen Biagas, Tue Dec 13, 2022
#  Moved setting of sources to subdirs.
#
#****************************************************************************

add_library(avtmir_ser)
if(VISIT_PARALLEL)
    ADD_PARALLEL_LIBRARY(avtmir_par)
endif()

add_subdirectory(Base)
add_subdirectory(Discrete)
add_subdirectory(Tet)
add_subdirectory(Youngs)
add_subdirectory(Zoo)

include_directories(
    ${VISIT_COMMON_INCLUDES}
    ${VISIT_SOURCE_DIR}/visit_vtk/lightweight
    ${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
    ${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
    ${VISIT_SOURCE_DIR}/avt/Database/Database
    ${VISIT_SOURCE_DIR}/avt/Math
    ${VISIT_SOURCE_DIR}/avt/Pipeline/AbstractFilters
    ${VISIT_SOURCE_DIR}/avt/Pipeline/Data
    ${VISIT_SOURCE_DIR}/avt/Pipeline/Pipeline
    ${VISIT_SOURCE_DIR}/avt/Pipeline/Sinks
    ${VISIT_SOURCE_DIR}/avt/Pipeline/Sources
    ${VISIT_SOURCE_DIR}/avt/VisWindow/VisWindow
    ${VISIT_SOURCE_DIR}/third_party_builtin/verdict
    ${VTK_INCLUDE_DIRS})

#********************************* SERIAL ************************************
target_link_libraries(avtmir_ser visitcommon lightweight_visit_vtk
    visit_verdict avtpipeline_ser)
VISIT_INSTALL_TARGETS(avtmir_ser)

#********************************* PARALLEL **********************************
if(VISIT_PARALLEL)
    target_link_libraries(avtmir_par visitcommon lightweight_visit_vtk
        visit_verdict avtpipeline_par)
    VISIT_INSTALL_TARGETS(avtmir_par)
endif()

