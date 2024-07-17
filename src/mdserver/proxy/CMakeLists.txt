# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#  Kathleen Biagas, Wed Jul 17, 2024
#  Utilize visit_add_library.
#
#****************************************************************************

visit_add_library(
    NAME        mdserverproxy
    SOURCES     MDServerManager.C
                MDServerProxy.C
    HEADERS     MDServerManager.h
                MDServerProxy.h
                mdsproxy_exports.h
    INCLUDES    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERISON_INCLUDE}/visit/mdserver/proxy>
    DEPENDS_ON  mdserverrpc
                visitcommon
                avtdbatts)

