# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#*****************************************************************************

set(ABSTRACTFILTERS_SOURCES
    avtCompactTreeFilter.C
    avtDataObjectToDatasetFilter.C
    avtDataObjectToImageFilter.C
    avtDataObjectToSamplePointsFilter.C
    avtDataTreeIterator.C
    avtDatasetToDataObjectFilter.C
    avtDatasetToDatasetFilter.C
    avtDatasetToImageFilter.C
    avtDatasetToSamplePointsFilter.C
    avtFacadeFilter.C
    avtFilter.C
    avtImageToDataObjectFilter.C
    avtImageToImageFilter.C
    avtMultiResolutionPartitionStrategy.C
    avtMultipleInputToDataObjectFilter.C
    avtMultipleInputToDatasetFilter.C
    avtPluginDatasetToDatasetFilter.C
    avtPluginDataTreeIterator.C
    avtPluginStructuredChunkDataTreeIterator.C
    avtSIMODataTreeIterator.C
    avtSamplePointsToDataObjectFilter.C
    avtSamplePointsToDatasetFilter.C
    avtSamplePointsToImageFilter.C
    avtSamplePointsToSamplePointsFilter.C
    avtSingleFilterFacade.C
    avtStructuredChunkDataTreeIterator.C
    avtStructuredMeshChunker.C
    avtStructuredMeshPartitionStrategy.C
    avtSweepPlanePartitionStrategy.C)

target_sources(avtpipeline_ser PRIVATE ${ABSTRACTFILTERS_SOURCES})
target_include_directories(avtpipeline_ser PUBLIC .)

if(VISIT_PARALLEL)
    target_sources(avtpipeline_par PRIVATE ${ABSTRACTFILTERS_SOURCES})
    target_include_directories(avtpipeline_par PUBLIC .)
endif()

unset(ABSTRACTFILTERS_SOURCES)

