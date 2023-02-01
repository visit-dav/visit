# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

set(PIPELINE_SOURCES
    avtContract.C
    avtDataObjectReader.C
    avtDataObjectString.C
    avtDataObjectWriter.C
    avtDataRequest.C
    avtDataSetReader.C
    avtDataSetWriter.C
    avtDebugDumpOptions.C
    avtExecutionManager.C
    avtImageReader.C
    avtImageTiler.C
    avtImageWriter.C
    avtMemory.C
    avtMultiWindowSaver.C
    avtNamedSelection.C
    avtNamedSelectionExtension.C
    avtNamedSelectionManager.C
    avtNullDataReader.C
    avtNullDataWriter.C
    avtParallel.C
    avtParallelContext.C
    avtWebpage.C
    ThreadPool.C)

target_sources(avtpipeline_ser PRIVATE ${PIPELINE_SOURCES})
target_include_directories(avtpipeline_ser PUBLIC .)

if(VISIT_PARALLEL)
    target_sources(avtpipeline_par PRIVATE ${PIPELINE_SOURCES})
    target_include_directories(avtpipeline_par PUBLIC .)
endif()

unset(PIPELINE_SOURCES)

