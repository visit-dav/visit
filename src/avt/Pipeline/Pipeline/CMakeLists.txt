# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

visit_append_list(
    NAME  avtpipeline_ser_SOURCES
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtContract.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDataObjectReader.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDataObjectString.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDataObjectWriter.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDataRequest.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDataSetReader.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDataSetWriter.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDebugDumpOptions.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtExecutionManager.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtImageReader.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtImageTiler.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtImageWriter.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMemory.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMultiWindowSaver.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtNamedSelection.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtNamedSelectionExtension.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtNamedSelectionManager.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtNullDataReader.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtNullDataWriter.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtParallel.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtParallelContext.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtWebpage.C
          ${CMAKE_CURRENT_SOURCE_DIR}/ThreadPool.C)

visit_append_list(
    NAME  avtpipeline_ser_HEADERS
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtContract.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDataObjectReader.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDataObjectString.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDataObjectWriter.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDataRequest.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDataSetReader.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDataSetWriter.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDebugDumpOptions.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtExecutionManager.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtImageReader.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtImageTiler.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtImageWriter.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMemory.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMultiWindowSaver.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtNamedSelection.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtNamedSelectionExtension.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtNamedSelectionManager.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtNullDataReader.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtNullDataWriter.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtParallel.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtParallelContext.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtWebpage.h
          ${CMAKE_CURRENT_SOURCE_DIR}/ThreadPool.h)

visit_append_list(
    NAME  avtpipeline_ser_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/Pipeline/Pipeline>)

