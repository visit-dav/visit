# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#*****************************************************************************

visit_append_list(
    NAME  avtexpressions_ser_SOURCES
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtGlobalAvgExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtGlobalMaxExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtGlobalMinExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtGlobalRMSExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtGlobalStdDevExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtGlobalSumExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtGlobalVarianceExpression.C)

visit_append_list(
    NAME  avtexpressions_ser_HEADERS
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtGlobalAvgExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtGlobalMaxExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtGlobalMinExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtGlobalRMSExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtGlobalStdDevExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtGlobalSumExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtGlobalVarianceExpression.h)

visit_append_list(
    NAME  avtexpressions_ser_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/Expressions/GlobalConstant>)

