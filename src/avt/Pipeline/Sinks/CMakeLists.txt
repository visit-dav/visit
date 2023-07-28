# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

set(SINKS_SOURCES
    avtDatabaseWriter.C
    avtDataObjectSink.C
    avtDatasetSink.C
    avtImageSink.C
    avtMultipleInputSink.C
    avtNullDataSink.C
    avtSamplePointsSink.C
    avtTerminatingDatasetSink.C
    avtTerminatingImageSink.C
    avtTerminatingNullDataSink.C
    avtTerminatingSink.C)

target_sources(avtpipeline_ser PRIVATE ${SINKS_SOURCES})
target_include_directories(avtpipeline_ser PUBLIC .)

if(VISIT_PARALLEL)
    target_sources(avtpipeline_par PRIVATE ${SINKS_SOURCES})
    target_include_directories(avtpipeline_par PUBLIC .)
endif()

unset(SINKS_SOURCES)

