# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#****************************************************************************

visit_append_list(
    NAME  avtdatabase_ser_SOURCES
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtFileFormat.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtFileFormatInterface.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMTMDFileFormat.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMTMDFileFormatInterface.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMTSDFileFormat.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMTSDFileFormatInterface.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSTMDFileFormat.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSTMDFileFormatInterface.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSTSDFileFormat.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSTSDFileFormatInterface.C)

visit_append_list(
    NAME  avtdatabase_ser_HEADERS
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtFileFormat.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtFileFormatInterface.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMTMDFileFormat.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMTMDFileFormatInterface.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMTSDFileFormat.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMTSDFileFormatInterface.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSTMDFileFormat.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSTMDFileFormatInterface.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSTSDFileFormat.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSTSDFileFormatInterface.h)

visit_append_list(
    NAME  avtdatabase_ser_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/Database/Formats>)
