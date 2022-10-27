# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Wed Oct 22 14:20:27 MST 2014
#   Send 'diagnostics/networktest' type argument to VISIT_TOOLS_ADD_FOLDER.
#
#   Kathleen Biagas, Wed Oct 22 14:20:27 MST 2014
#   On Windows, networktest client has direct dependency on ws2_32 library.
#
#****************************************************************************

SET(TARGETS networktestclient networktestserver)

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_COMMON_INCLUDES}
)

LINK_DIRECTORIES(
${LIBRARY_OUTPUT_DIRECTORY}
)

ADD_EXECUTABLE(networktestclient networktestclient.cpp)
TARGET_LINK_LIBRARIES(networktestclient ${VISIT_EXE_LINKER_FLAGS} visitcommon)
if(WIN32)
   target_link_libraries(networktestclient ws2_32)
endif()

ADD_EXECUTABLE(networktestserver networktestserver.cpp)
TARGET_LINK_LIBRARIES(networktestserver ${VISIT_EXE_LINKER_FLAGS} visitcommon)

IF(VISIT_PARALLEL)
    ADD_PARALLEL_EXECUTABLE(networktestserver_par networktestserver.cpp)
    PARALLEL_EXECUTABLE_LINK_LIBRARIES(networktestserver_par visitcommon)
    SET(TARGETS ${TARGETS} networktestserver_par)
ENDIF(VISIT_PARALLEL)

VISIT_INSTALL_TARGETS(${TARGETS})
VISIT_TOOLS_ADD_FOLDER(diagnostics/networktest ${TARGETS})
