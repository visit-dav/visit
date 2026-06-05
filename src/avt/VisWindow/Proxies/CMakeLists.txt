# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

visit_append_list(
    NAME  avtviswindow_ser_SOURCES
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/VisWindowProtectionProxy.C)

visit_append_list(
    NAME  avtviswindow_ser_HEADERS
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/VisWindowColleagueProxy.h
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWindowInteractorProxy.h
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWindowProtectionProxy.h
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWindowRenderProxy.h
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWindowToolProxy.h)

visit_append_list(
    NAME  avtviswindow_ser_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/VisWindow/Proxies>)

