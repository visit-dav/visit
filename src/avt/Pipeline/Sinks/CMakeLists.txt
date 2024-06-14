# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

visit_append_list(
    NAME  avtpipeline_ser_SOURCES
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtDatabaseWriter.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDataObjectSink.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDatasetSink.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtImageSink.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMultipleInputSink.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtNullDataSink.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSamplePointsSink.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTerminatingDatasetSink.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTerminatingImageSink.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTerminatingNullDataSink.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTerminatingSink.C)

visit_append_list(
    NAME  avtpipeline_ser_HEADERS
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtDatabaseWriter.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDataObjectSink.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDatasetSink.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtImageSink.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMultipleInputSink.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtNullDataSink.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSamplePointsSink.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTerminatingDatasetSink.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTerminatingImageSink.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTerminatingNullDataSink.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTerminatingSink.h)

visit_append_list(
    NAME  avtpipeline_ser_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/Pipeline/Sinks>)

