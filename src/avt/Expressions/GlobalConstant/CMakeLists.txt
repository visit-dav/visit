# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#*****************************************************************************

set(GLOBAL_CONSTANT_SOURCES
    avtGlobalAvgExpression.C
    avtGlobalMaxExpression.C
    avtGlobalMinExpression.C
    avtGlobalRMSExpression.C
    avtGlobalStdDevExpression.C
    avtGlobalSumExpression.C
    avtGlobalVarianceExpression.C)

target_sources(avtexpressions_ser PRIVATE ${GLOBAL_CONSTANT_SOURCES})
target_include_directories(avtexpressions_ser PUBLIC .)

if(VISIT_PARALLEL)
    target_sources(avtexpressions_par PRIVATE ${GLOBAL_CONSTANT_SOURCES})
    target_include_directories(avtexpressions_par PUBLIC .)
endif()

unset(GLOBAL_CONSTANT_SOURCES)

