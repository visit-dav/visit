# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Brad Whitlock, Mon Nov 28 16:21:26 PST 2011
#   I moved LaunchRPC to common/state.
#
#   Kathleen Biagas, Thu Oct 3, 2024
#   Utilize visit_add_library.
#
#****************************************************************************


visit_add_library(
    NAME        vclrpc
    SOURCES     ConnectSimRPC.C
                LauncherMethods.C
                LauncherState.C
    HEADERS     ConnectSimRPC.h
                LauncherMethods.h
                LauncherState.h
                vclrpc_exports.h
    INCLUDES    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/launcher/rpc>
    DEPENDS_ON  visitcommon)

