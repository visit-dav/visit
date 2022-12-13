# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

set(INTERACTORS_SOURCES
    Dolly3D.C
    FlyThrough.C
    Lineout2D.C
    Navigate2D.C
    Navigate3D.C
    NavigateAxisArray.C
    NavigateCurve.C
    NullInteractor.C
    Pick.C
    VisitHotPointInteractor.C
    VisitInteractor.C
    Zoom2D.C
    Zoom3D.C
    ZoomAxisArray.C
    ZoomCurve.C
    ZoomInteractor.C)

target_sources(avtviswindow_ser PRIVATE ${INTERACTORS_SOURCES})
target_include_directories(avtviswindow_ser PUBLIC .)

if(VISIT_PARALLEL)
    target_sources(avtviswindow_par PRIVATE ${INTERACTORS_SOURCES})
    target_include_directories(avtviswindow_par PUBLIC .)
endif()

unset(INTERACTORS_SOURCES)

