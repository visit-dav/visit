# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#*****************************************************************************

visit_append_list(
    NAME  avtexpressions_ser_SOURCES
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtAbelInversionExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtConservativeSmoothingExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMeanFilterExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMedianFilterExpression.C)

visit_append_list(
    NAME  avtexpressions_ser_HEADERS
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtAbelInversionExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtConservativeSmoothingExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMeanFilterExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMedianFilterExpression.h)

visit_append_list(
    NAME  avtexpressions_ser_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/Expressions/ImageProcessing>)

