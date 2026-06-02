# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Thu Oct 24, 2024
#   Use visit_patch_target instead of blt_patch_target.
#
#****************************************************************************

set(VIEWERCORE_ACTION_SOURCES
    AnimationActions.C
    CopyActions.C
    DatabaseActions.C
    DatabaseStateActions.C
    EngineActions.C
    IOActions.C
    LockActions.C
    MDServerActions.C
    PlotAndOperatorActions.C
    QueryActions.C
    SelectionActions.C
    StateActions.C
    ViewActions.C
    WindowActions.C)

set(VIEWERCORE_ACTION_HEADERS
    AnimationActions.h
    CopyActions.h
    DatabaseActions.h
    DatabaseStateActions.h
    EngineActions.h
    IOActions.h
    LockActions.h
    MDServerActions.h
    PlotAndOperatorActions.h
    QueryActions.h
    SelectionActions.h
    StateActions.h
    ViewActions.h
    WindowActions.h)

visit_patch_target(
    NAME     viewercore_ser
    SOURCES  ${VIEWERCORE_ACTION_SOURCES}
    HEADERS  ${VIEWERCORE_ACTION_HEADERS}
    INCLUDES $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
             $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/viewer/core/actions>)

if(VISIT_PARALLEL)
    visit_patch_target(
        NAME     viewercore_par
        SOURCES  ${VIEWERCORE_ACTION_SOURCES}
        HEADERS  ${VIEWERCORE_ACTION_HEADERS}
        INCLUDES $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                 $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/viewer/core/actions>)
endif()

