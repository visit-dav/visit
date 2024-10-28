# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#   Justin Privitera, Mon Oct 28 10:15:57 PDT 2024
#   Added avtGhostAwareUnaryMathExpression.C
#
#*****************************************************************************

set(ABSTRACT_SOURCES
    avtBinaryMathExpression.C
    avtComparisonExpression.C
    avtExpressionFilter.C
    avtExpressionDataTreeIterator.C
    avtMacroExpressionFilter.C
    avtMultipleInputExpressionFilter.C
    avtMultipleInputMathExpression.C
    avtSingleInputExpressionFilter.C
    avtUnaryMathExpression.C
    avtGhostAwareUnaryMathExpression.C)

target_sources(avtexpressions_ser PRIVATE ${ABSTRACT_SOURCES})
target_include_directories(avtexpressions_ser PUBLIC .)

if(VISIT_PARALLEL)
    target_sources(avtexpressions_par PRIVATE ${ABSTRACT_SOURCES})
    target_include_directories(avtexpressions_par PUBLIC .)
endif()

unset(ABSTRACT_SOURCES)

