# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Mon Dec 12, 2022
#   Moved from ../CMakeLists.txt
#****************************************************************************


visit_append_list(
    NAME  avtplotter_ser_SOURCES
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/InitVTKRendering.C
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkAxesActor2D.C
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkBackgroundActor.C
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkLineLegend.C
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkMultiFontVectorText.C
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkOpenGLPointMapper.C
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkOpenGLTexturedBackgroundMapper.C
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkParallelImageSpaceRedistributor.C
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkPointGlyphMapper.C
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkPointMapper.C
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkTexturedBackgroundActor.C
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkTimeSliderActor.C
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkTriad2D.C
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkVectorFontCharacterData.C
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkVisItAxisActor.C
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkVisItAxisActor2D.C
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkVisItCubeAxesActor.C
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkVisItDataSetMapper.C
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkVisItFullFrameMapper2D.C
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkVisItScalarBarActor.C
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkVisItScalarBarWithOpacityActor.C
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkVisItTextActor.C)

visit_append_list(
    NAME  avtplotter_ser_HEADERS
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/InitVTKRendering.h
          ${CMAKE_CURRENT_SOURCE_DIR}/visit_arial.h
          ${CMAKE_CURRENT_SOURCE_DIR}/visit_arial_bold.h
          ${CMAKE_CURRENT_SOURCE_DIR}/visit_arial_italic.h
          ${CMAKE_CURRENT_SOURCE_DIR}/visit_courier.h
          ${CMAKE_CURRENT_SOURCE_DIR}/visit_courier_bold.h
          ${CMAKE_CURRENT_SOURCE_DIR}/visit_courier_italic.h
          ${CMAKE_CURRENT_SOURCE_DIR}/visit_times.h
          ${CMAKE_CURRENT_SOURCE_DIR}/visit_times_bold.h
          ${CMAKE_CURRENT_SOURCE_DIR}/visit_times_italic.h
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkAxesActor2D.h
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkBackgroundActor.h
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkLineLegend.h
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkMultiFontVectorText.h
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkOpenGLPointMapper.h
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkOpenGLTexturedBackgroundMapper.h
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkParallelImageSpaceRedistributor.h
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkPointGlyphMapper.h
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkPointMapper.h
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkTexturedBackgroundActor.h
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkTimeSliderActor.h
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkTriad2D.h
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkVectorFontCharacterData.h
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkVisItAxisActor.h
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkVisItAxisActor2D.h
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkVisItCubeAxesActor.h
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkVisItDataSetMapper.h
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkVisItFullFrameMapper2D.h
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkVisItScalarBarActor.h
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkVisItScalarBarWithOpacityActor.h
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkVisItTextActor.h)

if(HAVE_LIBOSPRAY)
    visit_append_list(
        NAME  avtplotter_ser_SOURCES
        ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/vtkOSPRayVisItAxisActorNode.C
              ${CMAKE_CURRENT_SOURCE_DIR}/vtkOSPRayVisItCubeAxesActorNode.C
              ${CMAKE_CURRENT_SOURCE_DIR}/vtkOSPRayVisItDataSetMapperNode.C
              ${CMAKE_CURRENT_SOURCE_DIR}/vtkVisItViewNodeFactory.C)

    visit_append_list(
        NAME  avtplotter_ser_HEADERS
        ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/vtkOSPRayVisItAxisActorNode.h
              ${CMAKE_CURRENT_SOURCE_DIR}/vtkOSPRayVisItCubeAxesActorNode.h
              ${CMAKE_CURRENT_SOURCE_DIR}/vtkOSPRayVisItDataSetMapperNode.h
              ${CMAKE_CURRENT_SOURCE_DIR}/vtkVisItViewNodeFactory.h)
endif()


visit_append_list(
    NAME  avtplotter_ser_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/Plotter/vtk>)

