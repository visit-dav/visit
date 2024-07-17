# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

visit_append_list(
    NAME  avtquery_ser_SOURCES
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtCurveComparisonQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCurveQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDataObjectQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDatasetQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtGeneralQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMultipleInputQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTwoPassDatasetQuery.C)

visit_append_list(
    NAME  avtquery_ser_HEADERS
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtCurveComparisonQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCurveQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDataObjectQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDatasetQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtGeneralQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMultipleInputQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTwoPassDatasetQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/query_exports.h)

visit_append_list(
    NAME  avtquery_ser_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/Queries/Abstract>)

