# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#****************************************************************************

visit_append_list(
    NAME  avtmir_ser_SOURCES
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/CellInterface.C
          ${CMAKE_CURRENT_SOURCE_DIR}/CellInterface2D.C
          ${CMAKE_CURRENT_SOURCE_DIR}/CellInterface3D.C
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkCQS.C
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkDataSetGradient.C
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkYoungsMaterialInterface.C
          ${CMAKE_CURRENT_SOURCE_DIR}/YoungsMIR.C)

visit_append_list(
    NAME  avtmir_ser_HEADERS
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/CellInterface.h
          ${CMAKE_CURRENT_SOURCE_DIR}/CellInterface2D.h
          ${CMAKE_CURRENT_SOURCE_DIR}/CellInterface3D.h
          ${CMAKE_CURRENT_SOURCE_DIR}/CellInterfaceCommon.h
          ${CMAKE_CURRENT_SOURCE_DIR}/CellInterfaceCUDA.h
          ${CMAKE_CURRENT_SOURCE_DIR}/CellInterfaceMath.h
          ${CMAKE_CURRENT_SOURCE_DIR}/CellInterfaceTools2D.h
          ${CMAKE_CURRENT_SOURCE_DIR}/CellInterfaceTools2D_Axis.h
          ${CMAKE_CURRENT_SOURCE_DIR}/CellInterfaceTools3D.h
          ${CMAKE_CURRENT_SOURCE_DIR}/CellInterfaceTypes.h
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkCQS.h
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkDataSetGradient.h
          ${CMAKE_CURRENT_SOURCE_DIR}/vtkYoungsMaterialInterface.h
          ${CMAKE_CURRENT_SOURCE_DIR}/YoungsMIR.h)

visit_append_list(
    NAME  avtmir_ser_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/MIR/Youngs>)

