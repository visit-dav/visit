# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

visit_append_list(
    NAME  avtpipeline_ser_SOURCES
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtDataObjectSource.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDatasetSource.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtImageSource.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtInlinePipelineSource.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtNullDataSource.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtOriginatingDatasetSource.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtOriginatingImageSource.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtOriginatingNullDataSource.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtOriginatingSource.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtQueryableSource.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSamplePointsSource.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSourceFromAVTDataset.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSourceFromDataset.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSourceFromImage.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSourceFromNullData.C)

visit_append_list(
    NAME  avtpipeline_ser_HEADERS
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtDataObjectSource.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDatasetSource.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtImageSource.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtInlinePipelineSource.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtNullDataSource.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtOriginatingDatasetSource.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtOriginatingImageSource.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtOriginatingNullDataSource.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtOriginatingSource.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtQueryableSource.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSamplePointsSource.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSourceFromAVTDataset.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSourceFromDataset.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSourceFromImage.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSourceFromNullData.h)

visit_append_list(
    NAME  avtpipeline_ser_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/Pipeline/Sources>)

