# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

set(MISC_SOURCES
    avtDirectDatabaseQOTFilter.C
    avtQueryFactory.C
    avtQueryOverTimeFilter.C
    avtTimeLoopQOTFilter.C)

target_sources(avtquery_ser PRIVATE ${MISC_SOURCES})
target_include_directories(avtquery_ser PUBLIC .)

if(VISIT_PARALLEL)
    target_sources(avtquery_par PRIVATE ${MISC_SOURCES})
    target_include_directories(avtquery_par PUBLIC .)
endif()

unset(MISC_SOURCES)

