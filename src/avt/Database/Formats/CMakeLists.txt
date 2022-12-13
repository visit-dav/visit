# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#****************************************************************************

set(FORMATS_SOURCES
    avtFileFormat.C
    avtFileFormatInterface.C
    avtMTMDFileFormat.C
    avtMTMDFileFormatInterface.C
    avtMTSDFileFormat.C
    avtMTSDFileFormatInterface.C
    avtSTMDFileFormat.C
    avtSTMDFileFormatInterface.C
    avtSTSDFileFormat.C
    avtSTSDFileFormatInterface.C)

target_sources(avtdatabase_ser PRIVATE ${FORMATS_SOURCES})
target_include_directories(avtdatabase_ser PUBLIC .)

if(VISIT_PARALLEL)
    target_sources(avtdatabase_par PRIVATE ${FORMATS_SOURCES})
    target_include_directories(avtdatabase_par PUBLIC .)
endif()

unset(FORMATS_SOURCES)

