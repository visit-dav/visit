# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#    Cyrus Harrison,Tue Feb  2 16:35:18 PST 2010
#    Added the python filter query.
#
#    Cyrus Harrison, Wed Mar 10 14:28:33 PST 2010   
#    Use proper python include path ${PYTHON_INCLUDE_PATH}, instead of 
#    ${PYTHON_INCLUDE_DIRS}
#
#    Eric Brugger, Wed Jun 30 13:37:54 PDT 2010
#    Added avtXRayQuery.
#
#    Hank Childs, Thu May 12 15:37:21 PDT 2011
#    Add avtAverageValueQuery.
#
#    Kevin Griffin, Thu Aug 11 10:53:13 PDT 2016
#    Add avtGyRadiusQuery.
#
#    Alister Maguire, Tue Sep 24 13:46:56 MST 2019
#    Added avtDirectDatabaseQOTFilter and avtTimeLoopQOTFilter.
#
#    Cyrus Harrison, Tue Dec 10 08:51:21 PST 2019
#    Add xml tools code gen targets
#
#    Cyrus Harrison, Mon May  9 14:01:56 PDT 2022
#    Added logic to use and link conduit in queries
#
#    Kathleen Biagas, Fri Sep 9, 2022
#    Link with 'rt' on Linux.
#
#    Kathleen Biagas, Tue Dec 13, 2022
#    Moved setting of sources to subdirs.
#
#****************************************************************************

add_library(avtquery_ser)

if(VISIT_PARALLEL)
    ADD_PARALLEL_LIBRARY(avtquery_par)
endif()

add_subdirectory(Abstract)
add_subdirectory(Misc)
add_subdirectory(Pick)
add_subdirectory(Queries)

include_directories(
    ${VISIT_COMMON_INCLUDES}
    ${VISIT_SOURCE_DIR}/avt/Math
    ${VISIT_SOURCE_DIR}/avt/VisWindow/VisWindow
    ${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
    ${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
    ${VISIT_SOURCE_DIR}/avt/Database/Database
    ${VISIT_SOURCE_DIR}/avt/Expressions/Abstract
    ${VISIT_SOURCE_DIR}/avt/Expressions/CMFE
    ${VISIT_SOURCE_DIR}/avt/Expressions/Conditional
    ${VISIT_SOURCE_DIR}/avt/Expressions/Derivations
    ${VISIT_SOURCE_DIR}/avt/Expressions/General
    ${VISIT_SOURCE_DIR}/avt/Expressions/ImageProcessing
    ${VISIT_SOURCE_DIR}/avt/Expressions/Management
    ${VISIT_SOURCE_DIR}/avt/Expressions/Math
    ${VISIT_SOURCE_DIR}/avt/Expressions/MeshQuality
    ${VISIT_SOURCE_DIR}/avt/Filters
    ${VISIT_SOURCE_DIR}/avt/MIR/Base
    ${VISIT_SOURCE_DIR}/avt/MIR/Tet
    ${VISIT_SOURCE_DIR}/avt/MIR/Zoo
    ${VISIT_SOURCE_DIR}/avt/Pipeline/AbstractFilters
    ${VISIT_SOURCE_DIR}/avt/Pipeline/Data
    ${VISIT_SOURCE_DIR}/avt/Pipeline/Pipeline
    ${VISIT_SOURCE_DIR}/avt/Pipeline/Sinks
    ${VISIT_SOURCE_DIR}/avt/Pipeline/Sources
    ${VISIT_SOURCE_DIR}/avt/Shapelets
    ${VISIT_SOURCE_DIR}/visit_vtk/full
    ${VISIT_SOURCE_DIR}/visit_vtk/lightweight
    ${VTK_INCLUDE_DIRS})

if(VISIT_PYTHON_FILTERS)
    include_directories(
        ${PYTHON_INCLUDE_PATH}
        ${VISIT_SOURCE_DIR}/avt/PythonFilters
        ${VISIT_SOURCE_DIR}/visitpy/common/)
endif()

if(CONDUIT_FOUND)
    # we will use conduit in queries
    include_directories(${CONDUIT_INCLUDE_DIR})
    link_directories(${CONDUIT_LIBRARY_DIR})
endif()

if(VISIT_PYTHON_FILTERS)
    link_directories(${PYTHON_LIBRARY_DIRS})
endif()

#********************************* SERIAL ************************************
target_link_libraries(avtquery_ser visitcommon avtmath avtshapelets
    avtexpressions_ser avtfilters_ser visit_vtk)

if(NOT WIN32 AND NOT APPLE)
    find_library(LIBRT rt)
    target_link_libraries(avtquery_ser ${LIBRT})
endif()

if(VISIT_PYTHON_FILTERS)
    target_link_libraries(avtquery_ser avtpythonfilters_ser)
endif()

# optionally link conduit
if(CONDUIT_FOUND)
    target_link_libraries(avtquery_ser ${CONDUIT_LIB})
endif()

VISIT_INSTALL_TARGETS(avtquery_ser)

#********************************* PARALLEL **********************************
if(VISIT_PARALLEL)
    target_link_libraries(avtquery_par visitcommon avtmath avtshapelets
        avtexpressions_par avtfilters_par visit_vtk)

    if(NOT WIN32 AND NOT APPLE)
        target_link_libraries(avtquery_par ${LIBRT})
    endif()

    if(VISIT_PYTHON_FILTERS)
        target_link_libraries(avtquery_par avtpythonfilters_par)
    endif()

    # optionally link conduit
    if(CONDUIT_FOUND)
        target_link_libraries(avtquery_par ${CONDUIT_LIB} ${CONDUIT_MPI_LIB})
    endif()

    VISIT_INSTALL_TARGETS(avtquery_par)
endif()

