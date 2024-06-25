# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

visit_append_list(
    NAME  avtviswindow_ser_SOURCES
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/BadColleagueException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/BadInteractorException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/BadPlotException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/BadWindowModeException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/PlotDimensionalityException.C)

visit_append_list(
    NAME  avtviswindow_ser_HEADERS
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/BadColleagueException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/BadInteractorException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/BadPlotException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/BadWindowModeException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/PlotDimensionalityException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/VisWindowException.h)

visit_append_list(
    NAME  avtviswindow_ser_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/VisWindow/Exceptions>)

