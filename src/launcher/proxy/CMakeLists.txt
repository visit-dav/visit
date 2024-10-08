# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Thu Oct 3, 2024
#   Utilize visit_add_library.
#
#****************************************************************************

visit_add_library(
    NAME        vclproxy
    SOURCES     LauncherProxy.C
    HEADERS     LauncherProxy.h
                vclproxy_exports.h
    INCLUDES    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/launcher/proxy>
    DEPENDS_ON  visitcommon vclrpc)

