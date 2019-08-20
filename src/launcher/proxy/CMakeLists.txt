# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

SET(VCLPROXY_SOURCES
LauncherProxy.C
)

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_COMMON_INCLUDES}
${VISIT_SOURCE_DIR}/launcher/rpc
)

LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY} )

ADD_LIBRARY(vclproxy ${VCLPROXY_SOURCES})
TARGET_LINK_LIBRARIES(vclproxy vclrpc visitcommon)

VISIT_INSTALL_TARGETS(vclproxy)
