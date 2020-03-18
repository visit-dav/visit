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

SET(AVTPYTHON_FILTERS_SOURCES
PythonInterpreter.C
PyContract.C
PyDataRequest.C
PyDataSelection.C
PySILRequest.C
${VISIT_SOURCE_DIR}/visitpy/common/PySILRestrictionBase.C
avtPythonFilterEnvironment.C
avtPythonFilter.C
)

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_COMMON_INCLUDES}
${VISIT_SOURCE_DIR}/avt/Pipeline/Data/
${VISIT_SOURCE_DIR}/avt/Pipeline/Pipeline/
${VISIT_SOURCE_DIR}/avt/DBAtts/
${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData/
${VISIT_SOURCE_DIR}/avt/DBAtts/SIL/
${VISIT_SOURCE_DIR}/visitpy/common/
${PYTHON_INCLUDE_PATH}
)

# Add link directories
LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY} ${PYTHON_DIR})


#********************************* SERIAL ************************************
ADD_LIBRARY(avtpythonfilters_ser ${AVTPYTHON_FILTERS_SOURCES})
TARGET_LINK_LIBRARIES(avtpythonfilters_ser visitcommon avtdbatts avtpipeline_ser ${PYTHON_LIBRARIES} )
VISIT_INSTALL_TARGETS(avtpythonfilters_ser)

#********************************* PARALLEL **********************************
IF(VISIT_PARALLEL)
    ADD_PARALLEL_LIBRARY(avtpythonfilters_par ${AVTPYTHON_FILTERS_SOURCES})
    TARGET_LINK_LIBRARIES(avtpythonfilters_par visitcommon avtdbatts avtpipeline_par ${PYTHON_LIBRARIES})
    VISIT_INSTALL_TARGETS(avtpythonfilters_par)
ENDIF(VISIT_PARALLEL)

