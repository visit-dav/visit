# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#****************************************************************************

set(DATABASE_SOURCES
    avtDatabase.C
    avtDatabaseFactory.C
    avtDatasetCollection.C
    avtDatasetDatabase.C
    avtFileDescriptorManager.C
    avtGenericDatabase.C
    avtIOInformation.C
    avtPolygonToTrianglesTesselator.C
    avtPolyhedralSplit.C
    avtSILGenerator.C
    avtSourceFromDatabase.C
    avtTransformManager.C
    avtVariableCache.C)

target_sources(avtdatabase_ser PRIVATE ${DATABASE_SOURCES})
target_include_directories(avtdatabase_ser PUBLIC .)

if(VISIT_PARALLEL)
    target_sources(avtdatabase_par PRIVATE ${DATABASE_SOURCES})
    target_include_directories(avtdatabase_par PUBLIC .)
endif()

unset(DATABASE_SOURCES)

