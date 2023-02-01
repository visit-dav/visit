# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

set(ABSTRACT_SOURCES
    avtCurveComparisonQuery.C
    avtCurveQuery.C
    avtDataObjectQuery.C
    avtDatasetQuery.C
    avtGeneralQuery.C
    avtMultipleInputQuery.C
    avtTwoPassDatasetQuery.C)

target_sources(avtquery_ser PRIVATE ${ABSTRACT_SOURCES})
target_include_directories(avtquery_ser PUBLIC .)

if(VISIT_PARALLEL)
    target_sources(avtquery_par PRIVATE ${ABSTRACT_SOURCES})
    target_include_directories(avtquery_par PUBLIC .)
endif()

unset(ABSTRACT_SOURCES)

