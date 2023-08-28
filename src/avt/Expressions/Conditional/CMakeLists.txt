# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#*****************************************************************************

set(CONDITIONAL_SOURCES
    avtConditionalExpression.C
    avtLogicalAndExpression.C
    avtLogicalNegationExpression.C
    avtLogicalOrExpression.C
    avtTestEqualToExpression.C
    avtTestGreaterThanExpression.C
    avtTestGreaterThanOrEqualToExpression.C
    avtTestLessThanExpression.C
    avtTestLessThanOrEqualToExpression.C
    avtTestNotEqualToExpression.C)

target_sources(avtexpressions_ser PRIVATE ${CONDITIONAL_SOURCES})
target_include_directories(avtexpressions_ser PUBLIC .)

if(VISIT_PARALLEL)
    target_sources(avtexpressions_par PRIVATE ${CONDITIONAL_SOURCES})
    target_include_directories(avtexpressions_par PUBLIC .)
endif()

unset(CONDITIONAL_SOURCES)

