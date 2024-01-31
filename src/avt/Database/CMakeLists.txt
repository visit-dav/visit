# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#  Cyrus Harrison, Thu Apr 19 16:07:11 PDT 2012
#  Added Ghost/avtLocalStructuredDomainBoundaries.C
#
#  Kathleen Biagas, Tue Dec 13, 2022
#  Moved setting of sources to subdirs.
#
#****************************************************************************

add_library(avtdatabase_ser)
if(VISIT_PARALLEL)
    ADD_PARALLEL_LIBRARY(avtdatabase_par)
endif()

add_subdirectory(Database)
add_subdirectory(Formats)
add_subdirectory(Ghost)

include_directories(
    ${VISIT_COMMON_INCLUDES}
    ${VISIT_SOURCE_DIR}/avt/Math
    ${VISIT_SOURCE_DIR}/avt/VisWindow/VisWindow
    ${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
    ${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
    ${VISIT_SOURCE_DIR}/avt/MIR/Base
    ${VISIT_SOURCE_DIR}/avt/MIR/Tet
    ${VISIT_SOURCE_DIR}/avt/MIR/Youngs
    ${VISIT_SOURCE_DIR}/avt/MIR/Zoo
    ${VISIT_SOURCE_DIR}/avt/MIR/Discrete/
    ${VISIT_SOURCE_DIR}/avt/Pipeline/AbstractFilters
    ${VISIT_SOURCE_DIR}/avt/Pipeline/Data
    ${VISIT_SOURCE_DIR}/avt/Pipeline/Pipeline
    ${VISIT_SOURCE_DIR}/avt/Pipeline/Sinks
    ${VISIT_SOURCE_DIR}/avt/Pipeline/Sources
    ${VISIT_SOURCE_DIR}/third_party_builtin/tess2
    ${VISIT_SOURCE_DIR}/visit_vtk/full
    ${VISIT_SOURCE_DIR}/visit_vtk/lightweight
    ${VTK_INCLUDE_DIRS})

# Unless we're doing a VISIT_DBIO_ONLY build then we need avtmir
if(NOT VISIT_DBIO_ONLY)
    set(AVTMIR_SER avtmir_ser)
    set(AVTMIR_PAR avtmir_par)
endif()


#********************************* SERIAL ************************************
target_link_libraries(avtdatabase_ser visitcommon avtdbatts ${AVTMIR_SER} avtpipeline_ser visit_vtk tess2)
VISIT_INSTALL_TARGETS(avtdatabase_ser)

#********************************* PARALLEL **********************************
if(VISIT_PARALLEL)
    target_link_libraries(avtdatabase_par visitcommon avtdbatts ${AVTMIR_PAR} avtpipeline_par visit_vtk tess2)
    VISIT_INSTALL_TARGETS(avtdatabase_par)
endif()

