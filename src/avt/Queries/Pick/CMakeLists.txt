# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

visit_append_list(
    NAME  avtquery_ser_SOURCES
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtActualCoordsQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtActualNodeCoordsQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtActualZoneCoordsQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCurvePickQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLocateCellQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLocateNodeQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLocateQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtNodePickQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPickByNodeQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPickByZoneQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPickQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtZonePickQuery.C)

visit_append_list(
    NAME  avtquery_ser_HEADERS
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtActualCoordsQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtActualNodeCoordsQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtActualZoneCoordsQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCurvePickQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLocateCellQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLocateNodeQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLocateQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtNodePickQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPickByNodeQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPickByZoneQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPickQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtZonePickQuery.h)

visit_append_list(
    NAME  avtquery_ser_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/Queries/Pick>)

