# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#*****************************************************************************

visit_append_list(
    NAME  avtexpressions_ser_SOURCES
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtBinaryMathExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtComparisonExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtExpressionDataTreeIterator.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtExpressionFilter.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMacroExpressionFilter.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMultipleInputExpressionFilter.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMultipleInputMathExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSingleInputExpressionFilter.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtUnaryMathExpression.C)

visit_append_list(
    NAME  avtexpressions_ser_HEADERS
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtBinaryMathExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtComparisonExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtExpressionDataTreeIterator.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtExpressionFilter.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMacroExpressionFilter.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMultipleInputExpressionFilter.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMultipleInputMathExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSingleInputExpressionFilter.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtUnaryMathExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/expression_exports.h)

visit_append_list(
    NAME  avtexpressions_ser_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/Expressions/Abstract>)

