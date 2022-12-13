# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#*****************************************************************************

set(CMFE_SOURCES
    avtCMFEExpression.C
    avtConnCMFEExpression.C
    avtCurveCMFEExpression.C
    avtEvalPlaneExpression.C
    avtEvalPointExpression.C
    avtEvalTransformExpression.C
    avtPosCMFEAlgorithm.C
    avtPosCMFEExpression.C
    avtSymmEvalExpression.C
    avtSymmPlaneExpression.C
    avtSymmPointExpression.C
    avtSymmTransformExpression.C)

target_sources(avtexpressions_ser PRIVATE ${CMFE_SOURCES})
target_include_directories(avtexpressions_ser PUBLIC .)

if(VISIT_PARALLEL)
    target_sources(avtexpressions_par PRIVATE ${CMFE_SOURCES})
    target_include_directories(avtexpressions_par PUBLIC .)
endif()

unset(CMFE_SOURCES)

