# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#*****************************************************************************

visit_append_list(
    NAME  avtexpressions_ser_SOURCES
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtConditionalExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLogicalAndExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLogicalNegationExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLogicalOrExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTestEqualToExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTestGreaterThanExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTestGreaterThanOrEqualToExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTestLessThanExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTestLessThanOrEqualToExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTestNotEqualToExpression.C)

visit_append_list(
    NAME  avtexpressions_ser_HEADERS
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtConditionalExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLogicalAndExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLogicalNegationExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLogicalOrExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTestEqualToExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTestGreaterThanExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTestGreaterThanOrEqualToExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTestLessThanExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTestLessThanOrEqualToExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTestNotEqualToExpression.h)

visit_append_list(
    NAME  avtexpressions_ser_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/Expressions/Conditional>)

