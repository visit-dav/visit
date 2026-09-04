# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

visit_append_list(
    NAME  avtquery_ser_SOURCES
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtDirectDatabaseQOTFilter.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtQueryFactory.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtQueryOverTimeFilter.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTimeLoopQOTFilter.C)

visit_append_list(
    NAME  avtquery_ser_HEADERS
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtDirectDatabaseQOTFilter.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtQueryFactory.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtQueryOverTimeFilter.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTimeLoopQOTFilter.h)

visit_append_list(
    NAME  avtquery_ser_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/Queries/Misc>)

