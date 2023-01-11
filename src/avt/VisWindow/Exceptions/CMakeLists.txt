# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

set(EXCEPTIONS_SOURCES
    BadColleagueException.C
    BadInteractorException.C
    BadPlotException.C
    BadWindowModeException.C
    PlotDimensionalityException.C)

target_sources(avtviswindow_ser PRIVATE ${EXCEPTIONS_SOURCES})
target_include_directories(avtviswindow_ser PUBLIC .)

if(VISIT_PARALLEL)
    target_sources(avtviswindow_par PRIVATE ${EXCEPTIONS_SOURCES})
    target_include_directories(avtviswindow_par PUBLIC .)
endif()

unset(EXCEPTIONS_SOURCES)

