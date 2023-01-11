# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#*****************************************************************************

set(MANAGEMENT_SOURCES
    avtConstantCreatorExpression.C
    avtDeferExpressionBaseFilter.C
    avtExecuteThenTimeLoopFilter.C
    avtExpressionEvaluatorFilter.C
    avtExprNode.C
    avtExprNodeFactory.C
    avtFunctionExpr_CreateCMFEFilters.C
    avtFunctionExpr_CreateConditionalFilters.C
    avtFunctionExpr_CreateImageProcessingFilters.C
    avtFunctionExpr_CreateMaterialFilters.C
    avtFunctionExpr_CreateMathFilters.C
    avtFunctionExpr_CreateMeshFilters.C
    avtFunctionExpr_CreateMeshQualityFilters.C
    avtFunctionExpr_CreateTimeAndValueFilters.C
    avtFunctionExpr_CreateVectorMatrixFilters.C
    avtTimeExpression.C
    ExprPipelineState.C)

target_sources(avtexpressions_ser PRIVATE ${MANAGEMENT_SOURCES})
target_include_directories(avtexpressions_ser PUBLIC .)

if(VISIT_PARALLEL)
    target_sources(avtexpressions_par PRIVATE ${MANAGEMENT_SOURCES})
    target_include_directories(avtexpressions_par PUBLIC .)
endif()

unset(MANAGEMENT_SOURCES)

