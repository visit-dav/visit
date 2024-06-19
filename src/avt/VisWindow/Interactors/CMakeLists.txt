# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

visit_append_list(
    NAME  avtviswindow_ser_SOURCES
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/Dolly3D.C
          ${CMAKE_CURRENT_SOURCE_DIR}/FlyThrough.C
          ${CMAKE_CURRENT_SOURCE_DIR}/Lineout2D.C
          ${CMAKE_CURRENT_SOURCE_DIR}/Navigate2D.C
          ${CMAKE_CURRENT_SOURCE_DIR}/Navigate3D.C
          ${CMAKE_CURRENT_SOURCE_DIR}/NavigateAxisArray.C
          ${CMAKE_CURRENT_SOURCE_DIR}/NavigateCurve.C
          ${CMAKE_CURRENT_SOURCE_DIR}/NullInteractor.C
          ${CMAKE_CURRENT_SOURCE_DIR}/Pick.C
          ${CMAKE_CURRENT_SOURCE_DIR}/VisitHotPointInteractor.C
          ${CMAKE_CURRENT_SOURCE_DIR}/VisitInteractor.C
          ${CMAKE_CURRENT_SOURCE_DIR}/Zoom2D.C
          ${CMAKE_CURRENT_SOURCE_DIR}/Zoom3D.C
          ${CMAKE_CURRENT_SOURCE_DIR}/ZoomAxisArray.C
          ${CMAKE_CURRENT_SOURCE_DIR}/ZoomCurve.C
          ${CMAKE_CURRENT_SOURCE_DIR}/ZoomInteractor.C)

visit_append_list(
    NAME  avtviswindow_ser_HEADERS
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/Dolly3D.h
          ${CMAKE_CURRENT_SOURCE_DIR}/FlyThrough.h
          ${CMAKE_CURRENT_SOURCE_DIR}/Lineout2D.h
          ${CMAKE_CURRENT_SOURCE_DIR}/Navigate2D.h
          ${CMAKE_CURRENT_SOURCE_DIR}/Navigate3D.h
          ${CMAKE_CURRENT_SOURCE_DIR}/NavigateAxisArray.h
          ${CMAKE_CURRENT_SOURCE_DIR}/NavigateCurve.h
          ${CMAKE_CURRENT_SOURCE_DIR}/NullInteractor.h
          ${CMAKE_CURRENT_SOURCE_DIR}/Pick.h
          ${CMAKE_CURRENT_SOURCE_DIR}/VisitHotPointInteractor.h
          ${CMAKE_CURRENT_SOURCE_DIR}/VisitInteractor.h
          ${CMAKE_CURRENT_SOURCE_DIR}/Zoom2D.h
          ${CMAKE_CURRENT_SOURCE_DIR}/Zoom3D.h
          ${CMAKE_CURRENT_SOURCE_DIR}/ZoomAxisArray.h
          ${CMAKE_CURRENT_SOURCE_DIR}/ZoomCurve.h
          ${CMAKE_CURRENT_SOURCE_DIR}/ZoomInteractor.h)

visit_append_list(
    NAME  avtviswindow_ser_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/VisWindow/Interactors>)

