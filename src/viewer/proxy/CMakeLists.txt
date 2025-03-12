# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#  Kathleen Biagas, Thu Feb 27 2020
#  Only build examples if VISIT_ENABLE_EXAMPLES is ON (the default is ON).
#
#  Kathleen Biagas, Thu Oct 3, 2024
#  Utilize visit_add_library.
#
#****************************************************************************

visit_add_library(
    NAME        viewerproxy
    SOURCES     ViewerProxy.C
    HEADERS     ViewerProxy.h
                viewerproxy_exports.h
    INCLUDES    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/viewer/proxy>
    DEPENDS_ON  viewerrpc visitcommon avtdbatts)


if(VISIT_ENABLE_EXAMPLES)
  visit_add_executable(
      NAME        proxyexample
      SOURCES     proxyexample.C
      DEPENDS_ON  viewerrpc
                  viewerproxy
                  visitcommon
                  avtdbatts
      FOLDER      examples
      SKIP_INSTALL)
  set_target_properties(proxyexample PROPERTIES
                        EXCLUDE_FROM_ALL 1
                        EXCLUDE_FROM_DEFAULT_BUILD 1)
endif()

