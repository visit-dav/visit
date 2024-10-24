# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#  Kathleen Biagas, Thu Oct 24, 2024
#  Use visit_patch_target instead of blt_patch_target.
#
#****************************************************************************


visit_patch_target(
    NAME     viewer
    SOURCES  DDTActions.C
             DDTActionsUI.C
             DDTManager.C
             DDTSession.C
    HEADERS  DDTActions.h
             DDTActionsUI.h
             DDTManager.h
             DDTSession.h
    INCLUDES $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
             $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/viewer/main/ddt>)

