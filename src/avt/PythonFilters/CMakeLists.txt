# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#    Cyrus Harrison, Wed Mar 10 14:28:33 PST 2010   
#    Use proper python include path ${PYTHON_INCLUDE_PATH}, instead of 
#    ${PYTHON_INCLUDE_DIRS}
#
#****************************************************************************


set(avtpythonfilters_SOURCES
    avtPythonFilter.C
    avtPythonFilterEnvironment.C
    PyContract.C
    PyDataRequest.C
    PyDataSelection.C
    PySILRequest.C
    PythonInterpreter.C
    ${VISIT_SOURCE_DIR}/visitpy/common/PySILRestrictionBase.C)

set(avtpythonfilters_HEADERS
    avtPythonFilter.h
    avtPythonFilterEnvironment.h
    PyContract.h
    PyDataRequest.h
    PyDataSelection.h
    PySILRequest.h
    python_filters_exports.h
    PythonInterpreter.h)

set(avtpythonfilters_INCLUDES
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/PythonFilters>
    $<BUILD_INTERFACE:${VISIT_SOURCE_DIR}/visitpy/common>
    $<BUILD_INTERFACE:${PYTHON_INCLUDE_PATH}>)

set(avtpythonfilters_DEPENDS
    visitcommon
    avtdbatts
    ${PYTHON_LIBRARIES})

#********************************* SERIAL ************************************
visit_add_library(
    NAME       avtpythonfilters_ser
    SOURCES    ${avtpythonfilters_SOURCES}
    HEADERS    ${avtpythonfilters_HEADERS}
    INCLUDES   ${avtpythonfilters_INCLUDES}
    DEPENDS_ON ${avtpythonfilters_DEPENDS}
               avtpipeline_ser)
    target_link_directories(avtpythonfilters_ser PUBLIC ${PYTHON_DIR})

#********************************* PARALLEL **********************************
if(VISIT_PARALLEL)
    visit_add_parallel_library(
        NAME       avtpythonfilters_par
        SOURCES    ${avtpythonfilters_SOURCES}
        HEADERS    ${avtpythonfilters_HEADERS}
        INCLUDES   ${avtpythonfilters_INCLUDES}
        DEPENDS_ON ${avtpythonfilters_DEPENDS}
                   avtpipeline_par)
    target_link_directories(avtpythonfilters_par PUBLIC ${PYTHON_DIR})
endif()

