# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#****************************************************************************

visit_append_list(
    NAME  avtmir_ser_SOURCES
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/BitUtils.C
          ${CMAKE_CURRENT_SOURCE_DIR}/CellReconstructor.C
          ${CMAKE_CURRENT_SOURCE_DIR}/IsovolumeCellReconstructor.C
          ${CMAKE_CURRENT_SOURCE_DIR}/RecursiveCellReconstructor.C
          ${CMAKE_CURRENT_SOURCE_DIR}/ResampledMat.C
          ${CMAKE_CURRENT_SOURCE_DIR}/ZooMIR.C)

visit_append_list(
    NAME  avtmir_ser_HEADERS
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/BitUtils.h
          ${CMAKE_CURRENT_SOURCE_DIR}/CellReconstructor.h
          ${CMAKE_CURRENT_SOURCE_DIR}/IsovolumeCellReconstructor.h
          ${CMAKE_CURRENT_SOURCE_DIR}/RecursiveCellReconstructor.h
          ${CMAKE_CURRENT_SOURCE_DIR}/ResampledMat.h
          ${CMAKE_CURRENT_SOURCE_DIR}/ZooMIR.h)

visit_append_list(
    NAME  avtmir_ser_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/MIR/Zoo>)

