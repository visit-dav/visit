# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Mon Dec 12, 2022
#   Moved from ../CMakeLists.txt
#****************************************************************************


set(AVTPLOTTER_VTK_SOURCES
    InitVTKRendering.C
    vtkAxesActor2D.C
    vtkBackgroundActor.C
    vtkLineLegend.C
    vtkMultiFontVectorText.C
    vtkOpenGLPointMapper.C
    vtkOpenGLTexturedBackgroundMapper.C
    vtkParallelImageSpaceRedistributor.C
    vtkPointGlyphMapper.C
    vtkPointMapper.C
    vtkTexturedBackgroundActor.C
    vtkTimeSliderActor.C
    vtkTriad2D.C
    vtkVectorFontCharacterData.C
    vtkVisItAxisActor.C
    vtkVisItAxisActor2D.C
    vtkVisItCubeAxesActor.C
    vtkVisItDataSetMapper.C
    vtkVisItFullFrameMapper2D.C
    vtkVisItScalarBarActor.C
    vtkVisItScalarBarWithOpacityActor.C
    vtkVisItTextActor.C)

if(VTK_VERSION VERSION_EQUAL "8.1.0" AND VISIT_OSPRAY)
    list(APPEND AVTPLOTTER_VTK_SOURCES
         vtkOSPRayVisItAxisActorNode.C
         vtkOSPRayVisItCubeAxesActorNode.C
         vtkVisItViewNodeFactory.C)
elseif(VTK_VERSION VERSION_GREATER_EQUAL "9.1.0" AND HAVE_LIBOSPRAY)
    list(APPEND AVTPLOTTER_VTK_SOURCES
         vtkOSPRayVisItAxisActorNode.C
         vtkOSPRayVisItCubeAxesActorNode.C
         vtkOSPRayVisItDataSetMapperNode.C
         vtkVisItViewNodeFactory.C)
endif()

target_sources(avtplotter_ser PRIVATE ${AVTPLOTTER_VTK_SOURCES})
target_include_directories(avtplotter_ser PUBLIC .)

if(VISIT_PARALLEL)
    target_include_directories(avtplotter_par PUBLIC .)
    target_sources(avtplotter_par PRIVATE ${AVTPLOTTER_VTK_SOURCES})
endif()

unset(AVTPLOTTER_VTK_SOURCES)

