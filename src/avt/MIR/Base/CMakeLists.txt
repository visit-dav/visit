# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#****************************************************************************

visit_append_list(
    NAME  avtmir_ser_SOURCES
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/MIR.C
          ${CMAKE_CURRENT_SOURCE_DIR}/MIRConnectivity.C
          ${CMAKE_CURRENT_SOURCE_DIR}/MIROptions.C)

visit_append_list(
    NAME  avtmir_ser_HEADERS
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/MIR.h
          ${CMAKE_CURRENT_SOURCE_DIR}/MIRConnectivity.h
          ${CMAKE_CURRENT_SOURCE_DIR}/MIROptions.h
          ${CMAKE_CURRENT_SOURCE_DIR}/mir_exports.h
          ${CMAKE_CURRENT_SOURCE_DIR}/VisItArray.h)

visit_append_list(
    NAME  avtmir_ser_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/MIR/Base>)

