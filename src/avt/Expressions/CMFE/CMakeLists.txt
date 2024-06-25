# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#*****************************************************************************

visit_append_list(
    NAME  avtexpressions_ser_SOURCES
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtCMFEExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtConnCMFEExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCurveCMFEExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtEvalPlaneExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtEvalPointExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtEvalTransformExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPosCMFEAlgorithm.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPosCMFEExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSymmEvalExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSymmPlaneExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSymmPointExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSymmTransformExpression.C)

visit_append_list(
    NAME  avtexpressions_ser_HEADERS
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtCMFEExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtConnCMFEExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCurveCMFEExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtEvalPlaneExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtEvalPointExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtEvalTransformExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPosCMFEAlgorithm.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPosCMFEExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSymmEvalExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSymmPlaneExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSymmPointExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSymmTransformExpression.h)

visit_append_list(
    NAME  avtexpressions_ser_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/Expressions/CMFE>)

