# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#*****************************************************************************

set(IMAGEPROCESSING_SOURCES
    avtAbelInversionExpression.C
    avtConservativeSmoothingExpression.C
    avtMeanFilterExpression.C
    avtMedianFilterExpression.C)

target_sources(avtexpressions_ser PRIVATE ${IMAGEPROCESSING_SOURCES})
target_include_directories(avtexpressions_ser PUBLIC .)

if(VISIT_PARALLEL)
    target_sources(avtexpressions_par PRIVATE ${IMAGEPROCESSING_SOURCES})
    target_include_directories(avtexpressions_par PUBLIC .)
endif()

unset(IMAGEPROCESSING_SOURCES)

