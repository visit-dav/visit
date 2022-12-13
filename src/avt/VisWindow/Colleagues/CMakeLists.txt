# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

set(COLLEAGUES_SOURCES
    avtAnnotationColleague.C
    avtAnnotationWithTextColleague.C
    avtImageColleague.C
    avtLegendAttributesColleague.C
    avtLine2DColleague.C
    avtLine3DColleague.C
    avtText2DColleague.C
    avtText3DColleague.C
    avtTimeSliderColleague.C
    VisWinAnnotations.C
    VisWinAxes.C
    VisWinAxes3D.C
    VisWinAxesArray.C
    VisWinBackground.C
    VisWinColleague.C
    VisWinFrame.C
    VisWinInteractions.C
    VisWinLegends.C
    VisWinLighting.C
    VisWinParallelAxes.C
    VisWinPathTracker.C
    VisWinPlots.C
    VisWinQuery.C
    VisWinRendering.C
    VisWinRenderingWithoutWindow.C
    VisWinRenderingWithoutWindowWithInteractions.C
    VisWinTools.C
    VisWinTriad.C
    VisWinUserInfo.C
    VisWinView.C)

target_sources(avtviswindow_ser PRIVATE ${COLLEAGUES_SOURCES})
target_include_directories(avtviswindow_ser PUBLIC .)

if(VISIT_PARALLEL)
    target_sources(avtviswindow_par PRIVATE ${COLLEAGUES_SOURCES})
    target_include_directories(avtviswindow_par PUBLIC .)
endif()

unset(COLLEAGUES_SOURCES)

