# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#****************************************************************************

set(GHOST_SOURCES
    avtDomainBoundaries.C
    avtGhostNodeGenerator.C
    avtIsenburgSGG.C
    avtLocalStructuredDomainBoundaries.C
    avtNekDomainBoundaries.C
    avtStreamingGhostGenerator.C
    avtStructuredDomainBoundaries.C
    avtStructuredDomainNesting.C
    avtUnstructuredDomainBoundaries.C
    avtUnstructuredPointBoundaries.C
    StructuredBoundary.C)

target_sources(avtdatabase_ser PRIVATE ${GHOST_SOURCES})
target_include_directories(avtdatabase_ser PUBLIC .)

if(VISIT_PARALLEL)
    target_sources(avtdatabase_par PRIVATE ${GHOST_SOURCES})
    target_include_directories(avtdatabase_par PUBLIC .)
endif()

unset(GHOST_SOURCES)

