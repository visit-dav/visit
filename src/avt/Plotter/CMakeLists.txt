# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Brad Whitlock, Wed Aug 24 13:11:30 PDT 2011
#   Add vtkTexturedPointsPainter and remove some obsolete files.
#
#   Brad Whitlock, Wed Oct 12 22:42:12 PDT 2011
#   I moved some mappers to a different library since they may contain Qt. I
#   also cleaned up obsolete stuff in this file.
#
#   Carson Brownlee, Sun May  6 16:25:28 PDT 2012
#   Add support for Manta.
#
#   Kathleen Biagas, Tue Jul 31 12:08:32 MST 2012
#   Change CONFIGURE_FILE settings so things work correctly with
#   out-of-source builds.
#
#   Kathleen Biagas, Tue Sep  3 11:08:03 PDT 2013
#   Swap avtGLEWInitializer for avtOpenGLExtensionManager.
#
#   Kathleen Biagas, Wed Jan  8 14:19:28 PST 2014
#   Add vtkAxesActor2D.
#
#   Kathleen Biagas, Tue Mar 15 08:26:33 MST 2016
#   OSPRay is only required if requested (VISIT_OSPRAY is set to true/on).
#   Change OSPRay messages type to STATUS.
#
#   Kathleen Biagas, Mon Jul 18 10:04:22 PDT 2016
#   Remove SurfaceAndWireframeMapper, and specialized mappers for
#   Structured/Rectilinear grids.
#
#   Kathleen Biagas, Tue Feb 6 14:15:43 PST 2018
#   Moved avtLookupTable from avt/Pipeline/Data to here, and
#   visit_vtk/full/vtkDepthSortPolyData2 to vtk, due to their dependence on
#   vtkRendering.
#
#   Kathleen Biagas, Thu June 7, 2018
#   No longer add 'VTK_DEFINITIONS' to target. Add vtkRenderingVolumeOpenGL2
#   to link line.  Needed because the module is now init'd from
#   InitVTKRendering.
#
#   Kathleen Biagas, Fri Jun  8 12:17:45 PDT 2018
#   Remove vtkRenderingVolumeOpenGL2 from link line. Moved module init back
#   into volume plot.
#
#   Qi WU, Sat Jun 10 22:21:27 MST 2018
#   Remove the old VISIT_OSPRAY configurations because they are too old.
#   Additional Note: The FindOSPRay.cmake file is broken for a recent version
#                    of OSPRay. We should use the built-inosprayConfig.cmake
#                    file to find OSPRay instead.
#
#   Kathleen Biagas, Thu Sep 27 11:34:32 PDT 2018
#   Install InitVTKRenderingConfig.h when VISIT_INSTALL_THIRD_PARTY true.
#
#   Kathleen Biagas, Fri Jul 19 12:13:02 PDT 2019
#   Added vtkCommonExecutionModel, vtkFiltersHybrid, vtkRenderingCore in
#   support of building plugins vs install on Windows. They are direct
#   depencencies for this library, and CMake automagically pulls them in when
#   compiling VisIt even if not listed, but not when building the plugins.
#
#   Kathleen Biagas, Wed Jan  8 11:22:17 MST 2020
#   Removed InitVTKRenderingConfig.h.in, it only defined VISIT_MANTA which is
#   no longer used.
#
#   Kathleen Biagas, Tue Apr 13 2021
#   Add vtk/vtkVisItDatasetMapper.
#
#   Kathleen Biagas, Wed Aug 17, 2022
#   Incorporate ARSanderson's OSPRAY 2.8.0 work for VTK 9.
#
#   Kathleen Biagas, Tue Dec 13, 2022
#   Moved setting of vtk sources to subdir.
#
#****************************************************************************

add_subdirectory(vtk)

visit_append_list(
    NAME  avtplotter_ser_SOURCES
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtActor.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtBehavior.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCurveLegend.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCustomRenderer.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDecorationsDrawable.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDecorationsMapper.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDrawable.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtExternallyRenderedImagesActor.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtGeometryDrawable.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtImageDrawable.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtImageMapper.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLabelActor.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLabeledCurveMapper.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLegend.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLevelsLegend.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLevelsMapper.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLevelsPointGlyphMapper.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLightList.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLineoutActor.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLookupTable.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMapper.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMapperBase.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMapper2D.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPickActor.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPlot.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPlotMetaData.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPointMapper.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSoftwareShader.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTensorGlyphMapper.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTransparencyActor.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtUserDefinedMapper.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVariableLegend.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVariableMapper.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVariablePointGlyphMapper.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVectorGlyphMapper.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVolumeVariableLegend.C
          ${CMAKE_CURRENT_SOURCE_DIR}/RenderOrder.C)

visit_append_list(
    NAME  avtplotter_ser_HEADERS
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtActor.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtBehavior.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCurveLegend.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCustomRenderer.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDecorationsDrawable.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDecorationsMapper.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDrawable.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtExternallyRenderedImagesActor.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtGeometryDrawable.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtImageDrawable.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtImageMapper.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLabelActor.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLabeledCurveMapper.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLegend.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLevelsLegend.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLevelsMapper.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLevelsPointGlyphMapper.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLightList.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLineoutActor.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLookupTable.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMapper.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMapperBase.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMapper2D.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPickActor.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPlot.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPlotMetaData.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPointMapper.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSoftwareShader.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTensorGlyphMapper.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTransparencyActor.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtUserDefinedMapper.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVariableLegend.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVariableMapper.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVariablePointGlyphMapper.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVectorGlyphMapper.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVolumeVariableLegend.h
          ${CMAKE_CURRENT_SOURCE_DIR}/LineAttributes.h
          ${CMAKE_CURRENT_SOURCE_DIR}/plotter_exports.h
          ${CMAKE_CURRENT_SOURCE_DIR}/RenderOrder.h)

visit_append_list(
    NAME  avtplotter_ser_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/Plotter>)

visit_append_list(
    NAME  avtplotter_DEPENDS
    ITEMS visitcommon
          avtmath
          avtview
          visit_vtk
          lightweight_visit_vtk 
          VTK::CommonExecutionModel
          VTK::FiltersHybrid
          VTK::RenderingCore
          VTK::RenderingFreeType
          VTK::RenderingOpenGL2
          ${OPENGL_LIBRARIES})

if(TARGET VTK::RenderingRayTracing)
     # RenderingRayTracing available if VTK 9 was built with OSPRAY support
    list(APPEND avtplotter_DEPENDS VTK::RenderingRayTracing)
endif()

set(avtplotter_par_SOURCES  ${avtplotter_ser_SOURCES})
set(avtplotter_par_HEADERS  ${avtplotter_ser_HEADERS})
set(avtplotter_par_INCLUDES ${avtplotter_ser_INCLUDES})

#********************************* SERIAL ************************************
visit_add_library(
    NAME       avtplotter_ser
    SOURCES    ${avtplotter_ser_SOURCES}
    HEADERS    ${avtplotter_ser_HEADERS}
    INCLUDES   ${avtplotter_ser_INCLUDES}
    DEPENDS_ON ${avtplotter_DEPENDS}
               avtfilters_ser
               avtpipeline_ser)

#********************************* PARALLEL **********************************
if(VISIT_PARALLEL)
    visit_add_parallel_library(
        NAME       avtplotter_par
        SOURCES    ${avtplotter_par_SOURCES}
        HEADERS    ${avtplotter_par_HEADERS}
        INCLUDES   ${avtplotter_par_INCLUDES}
        DEPENDS_ON ${avtplotter_DEPENDS}
                   avtfilters_par
                   avtpipeline_par)
endif()

