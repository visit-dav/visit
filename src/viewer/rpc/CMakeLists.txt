# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

SET(VIEWERRPC_SOURCES
PostponedAction.C
ViewerMethods.C
ViewerRPC.C
ViewerState.C
)

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_COMMON_INCLUDES}
${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
)

LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY})

ADD_LIBRARY(viewerrpc ${VIEWERRPC_SOURCES})
TARGET_LINK_LIBRARIES(viewerrpc visitcommon avtdbatts)

VISIT_INSTALL_TARGETS(viewerrpc)
