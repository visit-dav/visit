# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

set(SOURCES_SOURCES
    avtDataObjectSource.C
    avtDatasetSource.C
    avtImageSource.C
    avtInlinePipelineSource.C
    avtNullDataSource.C
    avtOriginatingDatasetSource.C
    avtOriginatingImageSource.C
    avtOriginatingNullDataSource.C
    avtOriginatingSource.C
    avtQueryableSource.C
    avtSamplePointsSource.C
    avtSourceFromAVTDataset.C
    avtSourceFromDataset.C
    avtSourceFromImage.C
    avtSourceFromNullData.C)

target_sources(avtpipeline_ser PRIVATE ${SOURCES_SOURCES})
target_include_directories(avtpipeline_ser PUBLIC .)

if(VISIT_PARALLEL)
    target_sources(avtpipeline_par PRIVATE ${SOURCES_SOURCES})
    target_include_directories(avtpipeline_par PUBLIC .)
endif()

unset(SOURCES_SOURCES)

