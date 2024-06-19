# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#*****************************************************************************

visit_append_list(
    NAME  avtexpressions_ser_SOURCES
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtConstantCreatorExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDeferExpressionBaseFilter.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtExecuteThenTimeLoopFilter.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtExpressionEvaluatorFilter.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtExprNode.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtExprNodeFactory.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtFunctionExpr_CreateCMFEFilters.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtFunctionExpr_CreateConditionalFilters.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtFunctionExpr_CreateImageProcessingFilters.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtFunctionExpr_CreateMaterialFilters.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtFunctionExpr_CreateMathFilters.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtFunctionExpr_CreateMeshFilters.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtFunctionExpr_CreateMeshQualityFilters.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtFunctionExpr_CreateTimeAndValueFilters.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtFunctionExpr_CreateVectorMatrixFilters.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTimeExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/ExprPipelineState.C)

visit_append_list(
    NAME  avtexpressions_ser_HEADERS
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtConstantCreatorExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDeferExpressionBaseFilter.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtExecuteThenTimeLoopFilter.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtExpressionEvaluatorFilter.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtExprNode.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtExprNodeFactory.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTimeExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/ExprPipelineState.h)

visit_append_list(
    NAME  avtexpressions_ser_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/Expressions/Management>)

