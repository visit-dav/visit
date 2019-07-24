# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Wed Oct 22 14:20:27 MST 2014
#   Send 'diagnostics/exceptiontest' type argument to VISIT_TOOLS_ADD_FOLDER.
#
#****************************************************************************

SET(TARGETS exceptiontest)

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_COMMON_INCLUDES}
)

LINK_DIRECTORIES(
${LIBRARY_OUTPUT_DIRECTORY}
)

ADD_EXECUTABLE(exceptiontest exceptiontest.cpp)
TARGET_LINK_LIBRARIES(exceptiontest ${VISIT_EXE_LINKER_FLAGS} visitcommon)

IF(VISIT_PARALLEL)
    ADD_PARALLEL_EXECUTABLE(exceptiontest_par exceptiontest.cpp)
    PARALLEL_EXECUTABLE_LINK_LIBRARIES(exceptiontest_par visitcommon)
    SET(TARGETS ${TARGETS} exceptiontest_par)
ENDIF(VISIT_PARALLEL)

VISIT_INSTALL_TARGETS(${TARGETS})
VISIT_TOOLS_ADD_FOLDER(diagnostics/exceptiontest ${TARGETS})
