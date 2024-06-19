# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

visit_append_list(
    NAME  avtviswindow_ser_SOURCES
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtAxisRestrictionToolInterface.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtBoxToolInterface.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLineToolInterface.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPlaneToolInterface.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPointToolInterface.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSphereToolInterface.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtToolInterface.C
          ${CMAKE_CURRENT_SOURCE_DIR}/VisitAxisRestrictionTool.C
          ${CMAKE_CURRENT_SOURCE_DIR}/VisitBoxTool.C
          ${CMAKE_CURRENT_SOURCE_DIR}/VisitInteractiveTool.C
          ${CMAKE_CURRENT_SOURCE_DIR}/VisitLineTool.C
          ${CMAKE_CURRENT_SOURCE_DIR}/VisitPlaneTool.C
          ${CMAKE_CURRENT_SOURCE_DIR}/VisitPointTool.C
          ${CMAKE_CURRENT_SOURCE_DIR}/VisitSphereTool.C)

visit_append_list(
    NAME  avtviswindow_ser_HEADERS
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtAxisRestrictionToolInterface.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtBoxToolInterface.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLineToolInterface.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPlaneToolInterface.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPointToolInterface.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSphereToolInterface.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtToolInterface.h
          ${CMAKE_CURRENT_SOURCE_DIR}/VisitAxisRestrictionTool.h
          ${CMAKE_CURRENT_SOURCE_DIR}/VisitBoxTool.h
          ${CMAKE_CURRENT_SOURCE_DIR}/VisitInteractiveTool.h
          ${CMAKE_CURRENT_SOURCE_DIR}/VisitLineTool.h
          ${CMAKE_CURRENT_SOURCE_DIR}/VisitPlaneTool.h
          ${CMAKE_CURRENT_SOURCE_DIR}/VisitPointTool.h
          ${CMAKE_CURRENT_SOURCE_DIR}/VisitSphereTool.h)

visit_append_list(
    NAME  avtviswindow_ser_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/VisWindow/Tools>)

