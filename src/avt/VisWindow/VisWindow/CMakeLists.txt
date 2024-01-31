# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

set(VISWINDOW_SOURCES
    VisWindow.C
    VisWindowTypes.C
    VisWindowWithInteractions.C)

target_sources(avtviswindow_ser PRIVATE ${VISWINDOW_SOURCES})
target_include_directories(avtviswindow_ser PUBLIC .)

if(VISIT_PARALLEL)
    target_sources(avtviswindow_par PRIVATE ${VISWINDOW_SOURCES})
    target_include_directories(avtviswindow_par PUBLIC .)
endif()

unset(VISWINDOW_SOURCES)

