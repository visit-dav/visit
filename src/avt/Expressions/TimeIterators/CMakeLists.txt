# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#*****************************************************************************

set(TIMEITERATORS_SOURCES
    avtAverageOverTimeExpression.C
    avtMaxOverTimeExpression.C
    avtMinOverTimeExpression.C
    avtSumOverTimeExpression.C
    avtTimeIteratorDataTreeIteratorExpression.C
    avtTimeIteratorExpression.C
    avtValueAtExtremaExpression.C
    avtWhenConditionIsTrueExpression.C)

target_sources(avtexpressions_ser PRIVATE ${TIMEITERATORS_SOURCES})
target_include_directories(avtexpressions_ser PUBLIC .)

if(VISIT_PARALLEL)
    target_sources(avtexpressions_par PRIVATE ${TIMEITERATORS_SOURCES})
    target_include_directories(avtexpressions_par PUBLIC .)
endif()

unset(TIMEITERATORS_SOURCES)

