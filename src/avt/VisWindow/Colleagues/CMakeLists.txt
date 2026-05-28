# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

visit_append_list(
    NAME  avtviswindow_ser_SOURCES
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtAnnotationColleague.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtAnnotationWithTextColleague.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtImageColleague.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLegendAttributesColleague.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLine2DColleague.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLine3DColleague.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtText2DColleague.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtText3DColleague.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTimeSliderColleague.C
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWinAnnotations.C
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWinAxes.C
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWinAxes3D.C
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWinAxesArray.C
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWinBackground.C
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWinColleague.C
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWinFrame.C
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWinInteractions.C
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWinLegends.C
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWinLighting.C
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWinParallelAxes.C
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWinPathTracker.C
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWinPlots.C
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWinQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWinRendering.C
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWinRenderingWithoutWindow.C
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWinRenderingWithoutWindowWithInteractions.C
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWinTools.C
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWinTriad.C
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWinUserInfo.C
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWinView.C)

visit_append_list(
    NAME  avtviswindow_ser_HEADERS
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtAnnotationColleague.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtAnnotationWithTextColleague.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtImageColleague.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLegendAttributesColleague.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLine2DColleague.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLine3DColleague.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtText2DColleague.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtText3DColleague.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTimeSliderColleague.h
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWinAnnotations.h
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWinAxes.h
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWinAxes3D.h
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWinAxesArray.h
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWinBackground.h
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWinColleague.h
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWinFrame.h
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWinInteractions.h
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWinLegends.h
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWinLighting.h
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWinParallelAxes.h
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWinPathTracker.h
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWinPlots.h
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWinQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWinRendering.h
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWinRenderingWithoutWindow.h
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWinRenderingWithoutWindowWithInteractions.h
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWinTools.h
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWinTriad.h
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWinUserInfo.h
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWinView.h)

visit_append_list(
    NAME  avtviswindow_ser_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/VisWindow/Colleagues>)

