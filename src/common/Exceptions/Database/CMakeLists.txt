# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

target_include_directories(visitcommon PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

if(NOT VISIT_HEADERS_SKIP_INSTALL)
    target_include_directories(visitcommon PUBLIC
      $<INSTALL_INTERFACE:include/visit/common/Exceptions/Databases>)
endif()

target_sources(visitcommon PRIVATE
  BadPermissionException.C
  DBYieldedNoDataException.C
  FileDoesNotExistException.C
  InvalidDBTypeException.C
  InvalidFilesException.C
  InvalidSourceException.C
  InvalidTimeStepException.C
  InvalidVariableException.C
  InvalidZoneTypeException.C
  NonCompliantException.C
  NonCompliantFileException.C
  PointerNotInCacheException.C
  SiloException.C
)

