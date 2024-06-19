# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#   Hank Childs, Sat Jun  5 19:03:22 CDT 2010
#   Add avtPICSFilter.
#
#   Hank Childs, Sun Jun  6 14:54:08 CDT 2010
#   Renamed avt*SLAlgorithm to avt*ICAlgorithm.
#
#   Eric Brugger, Wed Jun 30 12:59:41 PDT 2010
#   I added avtXRayFilter.
#
#   Hank Childs, Thu Aug 26 13:47:30 PDT 2010
#   Renamed avtCurrentExtentFilter to avtActualExtentsFilter.
#
#   Brad Whitlock, Fri Dec 10 16:21:23 PST 2010
#   I moved avtThresholdFilter to here from the Threshold plugin.
#
#   Eric Brugger, Tue Oct 25 13:55:27 PDT 2011
#   Add avtMultiresFilter.
#
#   Hank Childs, Tue Mar 13 12:59:07 PDT 2012
#   Move avtDatasetOnDemandFilter from Pipeline/AbstractFilters directory.
#
#   Kathleen Biagas, Tue May 28 17:02:23 MST 2013 
#   Move avtTimeLoopFilter and avtTimeLoopCollectorFilter from 
#   Pipeline/AbstractFilters directory.
#
#   Kevin Griffin, Mon Nov 3 12:41:11 PDT 2014
#   Add avtSubsetBlockMergeFilter.
#
#   Eric Brugger, Thu Jan 14 08:44:32 PST 2016
#   Add linking of vtkm library.
#
#   Cyrus Harrison, Mon Nov  7 10:03:35 PST 2016
#   Add avtPolylineCleanupFilter
#
#   Eric Brugger, Tue Aug 28 14:35:30 PDT 2018
#   I replaced support for vtkm with vtkh.
#
#   Kathleen Biagas, Fri Jul 19 12:13:02 PDT 2019
#   Added vtkCommonMath, vtkCommonTransforms, vtkFiltersGeneral in support
#   of building plugins vs install on Windows. They are direct depencencies
#   for this library, and CMake automagically pulls them in when compiling
#   VisIt even if not listed, but not when building the plugins.
#
#   Kathleen Biagas, Tue Nov  5 08:12:29 PST 2019
#   Remove avtPolylineToRibbonFilter, avtPolylineToTubeFilter,
#   avtPolylineAddEndPointsFilter, avtPointToGlyphFilter.
#
#   Kathleen Biagas, Thu Jun  4 16:30:37 PDT 2020
#   Add avtVertexExtractor, avtLineGlyphFilter.C
#
#   Eric Brugger, Fri Feb 24 14:57:15 PST 2023
#   I replaced vtkh with vtkm.
#
#   Kathleen Biagas, Wed Mar 15, 2023 
#   Only link with pthread if not on Windows.
#
#   Eric Brugger, Tue Jul 25 11:23:55 EDT 2023
#   Change pthread to Threads::Threads.
#
#****************************************************************************/

set(avtfilters_SOURCES
    avtActualExtentsFilter.C
    avtCondenseDatasetFilter.C
    avtContourFilter.C
    avtCoordSystemConvert.C
    avtCurveConstructorFilter.C
    avtCustomTransform.C
    avtDatasetOnDemandFilter.C
    avtDisplaceFilter.C
    avtFacelistFilter.C
    avtFeatureEdgesFilter.C
    avtGhostZoneAndFacelistFilter.C
    avtGhostZoneFilter.C
    avtImageCommunicator.C
    avtImageCompositer.C
    avtLinearTransformFilter.C
    avtLineGlyphFilter.C
    avtLineoutFilter.C
    avtLineScanFilter.C
    avtLineToPolylineFilter.C
    avtMassVoxelExtractor.C
    avtMeshLogFilter.C
    avtMissingDataFilter.C
    avtMultiresFilter.C
    avtNamedSelectionFilter.C
    avtPolylineCleanupFilter.C
    avtRayCompositer.C
    avtRayTracerBase.C
    avtRayTracer.C
    avtResampleFilter.C
    avtSamplePointCommunicator.C
    avtSamplePointExtractorBase.C
    avtSamplePointExtractor.C
    avtSamplePointToSurfaceFilter.C
    avtShiftCenteringFilter.C
    avtSimilarityTransformFilter.C
    avtSmoothPolyDataFilter.C
    avtStaggeringFilter.C
    avtSubsetBlockMergeFilter.C
    avtSummationFilter.C
    avtSurfaceFilter.C
    avtThresholdFilter.C
    avtTiledImageCompositor.C
    avtTimeLoopCollectorFilter.C
    avtTimeLoopFilter.C
    avtTransform.C
    avtValueImageCompositer.C
    avtVertexExtractor.C
    avtVertexNormalsFilter.C
    avtVoxelExtractor.C
    avtWarpFilter.C
    avtWholeImageCompositerNoZ.C
    avtWholeImageCompositerWithZ.C
    avtWorldSpaceToImageSpaceTransform.C
    avtXRayFilter.C)

set(avtfilters_HEADERS
    avtActualExtentsFilter.h
    avtCondenseDatasetFilter.h
    avtContourFilter.h
    avtCoordSystemConvert.h
    avtCurveConstructorFilter.h
    avtCustomTransform.h
    avtDatasetOnDemandFilter.h
    avtDisplaceFilter.h
    avtFacelistFilter.h
    avtFeatureEdgesFilter.h
    avtGhostZoneAndFacelistFilter.h
    avtGhostZoneFilter.h
    avtImageCommunicator.h
    avtImageCompositer.h
    avtLinearTransformFilter.h
    avtLineGlyphFilter.h
    avtLineoutFilter.h
    avtLineScanFilter.h
    avtLineToPolylineFilter.h
    avtMassVoxelExtractor.h
    avtMeshLogFilter.h
    avtMissingDataFilter.h
    avtMultiresFilter.h
    avtNamedSelectionFilter.h
    avtPolylineCleanupFilter.h
    avtRayCompositer.h
    avtRayTracerBase.h
    avtRayTracer.h
    avtResampleFilter.h
    avtSamplePointCommunicator.h
    avtSamplePointExtractorBase.h
    avtSamplePointExtractor.h
    avtSamplePointToSurfaceFilter.h
    avtShiftCenteringFilter.h
    avtSimilarityTransformFilter.h
    avtSmoothPolyDataFilter.h
    avtStaggeringFilter.h
    avtSubsetBlockMergeFilter.h
    avtSummationFilter.h
    avtSurfaceFilter.h
    avtThresholdFilter.h
    avtTiledImageCompositor.h
    avtTimeLoopCollectorFilter.h
    avtTimeLoopFilter.h
    avtTransform.h
    avtValueImageCompositer.h
    avtVertexExtractor.h
    avtVertexNormalsFilter.h
    avtVoxelExtractor.h
    avtWarpFilter.h
    avtWholeImageCompositer.h
    avtWholeImageCompositerNoZ.h
    avtWholeImageCompositerWithZ.h
    avtWorldSpaceToImageSpaceTransform.h
    avtXRayFilter.h
    filters_exports.h
    imgMetaData.h)

if(VISIT_SLIVR)
    list(APPEND avtfilters_SOURCES
         avtSLIVRCommon.C
         avtSLIVRImageCompositor.C
         avtSLIVRRayTracer.C
         avtSLIVRSamplePointExtractor.C
         avtSLIVRVoxelExtractor.C)
    list(APPEND avtfilters_HEADERS
         avtSLIVRCommon.h
         avtSLIVRImageCompositor.h
         avtSLIVRRayTracer.h
         avtSLIVRSamplePointExtractor.h
         avtSLIVRVoxelExtractor.h)
    list(APPEND avtfilters_DEFINES VISIT_SLIVR)
endif()

list(APPEND avtfilters_DEPENDS
    visitcommon
    lightweight_visit_vtk
    visit_vtk
    avtmath
    avtview
    avtdbatts
    VTK::CommonMath
    VTK::CommonTransforms
    VTK::FiltersGeneral
    VTK::FiltersModeling)

if(HAVE_LIBVTKM)
    list(APPEND avtfilters_DEPENDS
        vtkm_cont
        vtkm_filter_contour
        vtkm_filter_entity_extraction
        vtkm_filter_field_conversion)
    if(NOT WIN32)
        list(APPEND avtfilters_DEPENDS Threads::Threads)
    endif()
    if(VTKm_ENABLE_KOKKOS)
        list(APPEND avtfilters_DEPENDS amdhip64)
    endif()
endif()

set(avtfilters_INCLUDES
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/Filters>)

#********************************* SERIAL ************************************

visit_add_library(
    NAME       avtfilters_ser
    SOURCES    ${avtfilters_SOURCES}
    HEADERS    ${avtfilters_HEADERS}
    INCLUDES   ${avtfilters_INCLUDES}
    DEFINES    ${avtfilters_DEFINES}
    DEPENDS_ON ${avtfilters_DEPENDS}
               avtpipeline_ser
               avtdatabase_ser)

#********************************* PARALLEL **********************************

if(VISIT_PARALLEL)

    visit_add_parallel_library(
        NAME       avtfilters_par
        SOURCES    ${avtfilters_SOURCES}
        HEADERS    ${avtfilters_HEADERS}
        INCLUDES   ${avtfilters_INCLUDES}
        DEFINES    ${avtfilters_DEFINES}
        DEPENDS_ON ${avtfilters_DEPENDS}
                   avtpipeline_par
                   avtdatabase_par)
endif()

