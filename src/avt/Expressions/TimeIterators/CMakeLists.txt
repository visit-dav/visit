# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#*****************************************************************************

visit_append_list(
    NAME  avtexpressions_ser_SOURCES
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtAverageOverTimeExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMaxOverTimeExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMinOverTimeExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSumOverTimeExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTimeIteratorDataTreeIteratorExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTimeIteratorExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtValueAtExtremaExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtWhenConditionIsTrueExpression.C)

visit_append_list(
    NAME  avtexpressions_ser_HEADERS
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtAverageOverTimeExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMaxOverTimeExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMinOverTimeExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSumOverTimeExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTimeIteratorDataTreeIteratorExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTimeIteratorExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtValueAtExtremaExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtWhenConditionIsTrueExpression.h)

visit_append_list(
    NAME  avtexpressions_ser_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/Expressions/TimeIterators>)

