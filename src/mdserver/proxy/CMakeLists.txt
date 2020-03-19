# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

SET(MDSERVERPROXY_SOURCES
MDServerProxy.C
MDServerManager.C
)

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_COMMON_INCLUDES}
${VISIT_SOURCE_DIR}/mdserver/rpc
${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
)

LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY})

ADD_LIBRARY(mdserverproxy ${MDSERVERPROXY_SOURCES})
TARGET_LINK_LIBRARIES(mdserverproxy mdserverrpc visitcommon avtdbatts)

VISIT_INSTALL_TARGETS(mdserverproxy)
