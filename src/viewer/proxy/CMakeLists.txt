# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#  Kathleen Biagas, Thu Feb 27 2020
#  Only build examples if VISIT_ENABLE_EXAMPLES is ON (the default is ON).
#
#   Kathleen Biagas, Thu Oct 27, 2022
#   On Windows, viewerproxy has direct dependence on ws2_32.
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
if(WIN32)
    target_link_libraries(viewerproxy ws2_32)
endif()

if(VISIT_ENABLE_EXAMPLES)
  add_executable(proxyexample proxyexample.C)
  set_target_properties(proxyexample PROPERTIES
                        FOLDER examples
                        EXCLUDE_FROM_ALL 1
                        EXCLUDE_FROM_DEFAULT_BUILD 1)
  target_link_libraries(proxyexample viewerrpc viewerproxy visitcommon avtdbatts)
endif()

VISIT_INSTALL_TARGETS(viewerproxy)
