# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

visit_append_list(
    NAME  avtviswindow_ser_SOURCES
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/VisWindow.C
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWindowTypes.C
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWindowWithInteractions.C)

visit_append_list(
    NAME  avtviswindow_ser_HEADERS
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/VisCallback.h
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWindow.h
          ${CMAKE_CURRENT_SOURCE_DIR}/viswindow_exports.h
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWindowTypes.h
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWindowWithInteractions.h)

visit_append_list(
    NAME  avtviswindow_ser_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/VisWindow/VisWindow>)

