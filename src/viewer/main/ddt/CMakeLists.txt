# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

target_sources(viewer PRIVATE
    DDTActions.C
    DDTActionsUI.C
    DDTManager.C
    DDTSession.C
    DDTActions.h
    DDTActionsUI.h
    DDTManager.h
    DDTSession.h)

target_include_directories(viewer PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/viewer/main/ddt>)

