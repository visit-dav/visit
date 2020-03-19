# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Brad Whitlock, Mon Nov 28 16:21:26 PST 2011
#   I moved LaunchRPC to common/state.
#
#****************************************************************************

SET(VCLRPC_SOURCES
ConnectSimRPC.C
LauncherMethods.C
LauncherState.C
)

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_COMMON_INCLUDES}
)

LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY})

ADD_LIBRARY(vclrpc ${VCLRPC_SOURCES})
TARGET_LINK_LIBRARIES(vclrpc visitcommon)

VISIT_INSTALL_TARGETS(vclrpc)
