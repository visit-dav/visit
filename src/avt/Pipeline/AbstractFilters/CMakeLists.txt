# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#*****************************************************************************

visit_append_list(
    NAME  avtpipeline_ser_SOURCES
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtCompactTreeFilter.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDataObjectToDatasetFilter.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDataObjectToImageFilter.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDataObjectToSamplePointsFilter.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDatasetToDataObjectFilter.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDatasetToDatasetFilter.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDatasetToImageFilter.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDatasetToSamplePointsFilter.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDataTreeIterator.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtFacadeFilter.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtFilter.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtImageToDataObjectFilter.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtImageToImageFilter.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMultipleInputToDataObjectFilter.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMultipleInputToDatasetFilter.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMultiResolutionPartitionStrategy.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPluginDatasetToDatasetFilter.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPluginDataTreeIterator.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPluginStructuredChunkDataTreeIterator.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSamplePointsToDataObjectFilter.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSamplePointsToDatasetFilter.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSamplePointsToImageFilter.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSamplePointsToSamplePointsFilter.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSIMODataTreeIterator.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSingleFilterFacade.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtStructuredChunkDataTreeIterator.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtStructuredMeshChunker.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtStructuredMeshPartitionStrategy.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSweepPlanePartitionStrategy.C)

visit_append_list(
    NAME  avtpipeline_ser_HEADERS
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtCompactTreeFilter.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDataObjectToDatasetFilter.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDataObjectToImageFilter.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDataObjectToSamplePointsFilter.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDatasetToDataObjectFilter.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDatasetToDatasetFilter.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDatasetToImageFilter.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDatasetToSamplePointsFilter.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDataTreeIterator.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtFacadeFilter.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtFilter.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtImageToDataObjectFilter.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtImageToImageFilter.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMultipleInputToDataObjectFilter.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMultipleInputToDatasetFilter.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMultiResolutionPartitionStrategy.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPluginDatasetToDatasetFilter.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPluginDataTreeIterator.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPluginFilter.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPluginStructuredChunkDataTreeIterator.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSamplePointsToDataObjectFilter.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSamplePointsToDatasetFilter.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSamplePointsToImageFilter.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSamplePointsToSamplePointsFilter.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSIMODataTreeIterator.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSingleFilterFacade.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtStructuredChunkDataTreeIterator.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtStructuredMeshChunker.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtStructuredMeshPartitionStrategy.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSweepPlanePartitionStrategy.h)

visit_append_list(
    NAME  avtpipeline_ser_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/Pipeline/AbstractFilters>)
