# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

SET(VIEWERPROXY_SOURCES
ViewerProxy.C
)

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_COMMON_INCLUDES}
${VISIT_SOURCE_DIR}/viewer/rpc
${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
)

LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY})

ADD_LIBRARY(viewerproxy ${VIEWERPROXY_SOURCES})
TARGET_LINK_LIBRARIES(viewerproxy viewerrpc visitcommon avtdbatts)

ADD_EXECUTABLE(proxyexample proxyexample.C)
set_target_properties(proxyexample PROPERTIES
                      FOLDER examples
                      EXCLUDE_FROM_ALL 1
                      EXCLUDE_FROM_DEFAULT_BUILD 1)
TARGET_LINK_LIBRARIES(proxyexample viewerrpc viewerproxy visitcommon avtdbatts)

VISIT_INSTALL_TARGETS(viewerproxy)
