# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Thu May 16, 2024
#   Using blt, so create lists for sources, headers, includes.
#
#****************************************************************************

visit_append_list(
    NAME  visitcommon_SOURCES
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/BadPermissionException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/DBYieldedNoDataException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/FileDoesNotExistException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/InvalidDBTypeException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/InvalidFilesException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/InvalidSourceException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/InvalidTimeStepException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/InvalidVariableException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/InvalidZoneTypeException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/NonCompliantException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/NonCompliantFileException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/PointerNotInCacheException.C
          ${CMAKE_CURRENT_SOURCE_DIR}/SiloException.C)

visit_append_list(
    NAME  visitcommon_HEADERS
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/BadPermissionException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/DatabaseException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/DBYieldedNoDataException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/FileDoesNotExistException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/InvalidDBTypeException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/InvalidFilesException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/InvalidSourceException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/InvalidTimeStepException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/InvalidVariableException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/InvalidZoneTypeException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/NonCompliantException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/NonCompliantFileException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/PointerNotInCacheException.h
          ${CMAKE_CURRENT_SOURCE_DIR}/SiloException.h)

visit_append_list(
    NAME  visitcommon_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/common/Exceptions/Database>)

