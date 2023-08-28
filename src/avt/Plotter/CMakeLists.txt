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

set(AVTPLOTTER_SOURCES
    avtActor.C
    avtBehavior.C
    avtCurveLegend.C
    avtCustomRenderer.C
    avtDecorationsDrawable.C
    avtDecorationsMapper.C
    avtDrawable.C
    avtExternallyRenderedImagesActor.C
    avtGeometryDrawable.C
    avtImageDrawable.C
    avtImageMapper.C
    avtLabelActor.C
    avtLabeledCurveMapper.C
    avtLegend.C
    avtLevelsLegend.C
    avtLevelsMapper.C
    avtLevelsPointGlyphMapper.C
    avtLightList.C
    avtLineoutActor.C
    avtLookupTable.C
    avtMapper.C
    avtMapperBase.C
    avtMapper2D.C
    avtPickActor.C
    avtPlot.C
    avtPlotMetaData.C
    avtPointMapper.C
    avtSoftwareShader.C
    avtTensorGlyphMapper.C
    avtTransparencyActor.C
    avtUserDefinedMapper.C
    avtVariableLegend.C
    avtVariableMapper.C
    avtVariablePointGlyphMapper.C
    avtVectorGlyphMapper.C
    avtVolumeVariableLegend.C
    RenderOrder.C)

include_directories(
    ${VISIT_COMMON_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
    ${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
    ${VISIT_SOURCE_DIR}/avt/Database/Database
    ${VISIT_SOURCE_DIR}/avt/Filters
    ${VISIT_SOURCE_DIR}/avt/Math
    ${VISIT_SOURCE_DIR}/avt/VisWindow/VisWindow
    ${VISIT_SOURCE_DIR}/avt/Pipeline/AbstractFilters
    ${VISIT_SOURCE_DIR}/avt/Pipeline/Data
    ${VISIT_SOURCE_DIR}/avt/Pipeline/Pipeline
    ${VISIT_SOURCE_DIR}/avt/Pipeline/Sinks
    ${VISIT_SOURCE_DIR}/avt/Pipeline/Sources
    ${VISIT_SOURCE_DIR}/avt/View
    ${VISIT_SOURCE_DIR}/visit_vtk/full
    ${VISIT_SOURCE_DIR}/visit_vtk/lightweight
    ${PROJECT_BINARY_DIR}/avt/Plotter/vtk/
    ${VTK_INCLUDE_DIRS})

if(VTK_VERSION VERSION_EQUAL "8.1.0")
    set(vtklibs vtkCommonExecutionModel vtkFiltersHybrid
            vtkRenderingCore vtkRenderingFreeType vtkRenderingOpenGL2)
    if(VISIT_OSPRAY)
        list(APPEND vtklibs vtkRenderingOSPRay)
    endif()
else()
    set(vtklibs VTK::CommonExecutionModel VTK::FiltersHybrid
            VTK::RenderingCore VTK::RenderingFreeType VTK::RenderingOpenGL2)
    if(TARGET VTK::RenderingRayTracing)
         # RenderingRayTracing available if VTK 9 was built with OSPRAY support
        list(APPEND vtklibs VTK::RenderingRayTracing)
    endif()
endif()
#********************************* SERIAL ************************************
add_library(avtplotter_ser ${AVTPLOTTER_SOURCES})
target_link_libraries(avtplotter_ser
    visitcommon avtmath avtview avtfilters_ser avtpipeline_ser visit_vtk
    lightweight_visit_vtk ${vtklibs} ${OPENGL_LIBRARIES})
VISIT_INSTALL_TARGETS(avtplotter_ser)

#********************************* PARALLEL **********************************
if(VISIT_PARALLEL)
    ADD_PARALLEL_LIBRARY(avtplotter_par ${AVTPLOTTER_SOURCES})
    target_link_libraries(avtplotter_par
        visitcommon avtmath avtview avtfilters_par avtpipeline_par visit_vtk
        lightweight_visit_vtk ${vtklibs} ${OPENGL_LIBRARIES})
    VISIT_INSTALL_TARGETS(avtplotter_par)
endif()

add_subdirectory(vtk)

