# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

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
    PointerNotInCacheException.C)

target_include_directories(visitcommon PUBLIC .)

