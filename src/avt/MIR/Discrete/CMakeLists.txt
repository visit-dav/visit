# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#****************************************************************************

visit_append_list(
    NAME  avtmir_ser_SOURCES
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/DiscreteMIR.C)

visit_append_list(
    NAME  avtmir_ser_HEADERS
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/DiscreteMIR.h)

visit_append_list(
    NAME  avtmir_ser_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/MIR/Discrete>)

