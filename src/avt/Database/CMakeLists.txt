# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#  Cyrus Harrison, Thu Apr 19 16:07:11 PDT 2012
#  Added Ghost/avtLocalStructuredDomainBoundaries.C
#
#  Kathleen Biagas, Tue Dec 13, 2022
#  Moved setting of sources to subdirs.
#
#****************************************************************************

add_subdirectory(Database)
add_subdirectory(Formats)
add_subdirectory(Ghost)

set(avtdatabase_ser_DEPENDS
    visitcommon
    avtdbatts
    visit_vtk
    tess2)

set(avtdatabase_par_SOURCES  ${avtdatabase_ser_SOURCES})
set(avtdatabase_par_HEADERS  ${avtdatabase_ser_HEADERS})
set(avtdatabase_par_INCLUDES ${avtdatabase_ser_INCLUDES})
set(avtdatabase_par_DEPENDS  ${avtdatabase_ser_DEPENDS})

if(NOT VISIT_DBIO_ONLY)
    list(APPEND avtdatabase_ser_DEPENDS avtmir_ser)
    list(APPEND avtdatabase_par_DEPENDS avtmir_par)
endif()

#********************************* SERIAL ************************************
visit_add_library(
    NAME       avtdatabase_ser
    SOURCES    ${avtdatabase_ser_SOURCES}
    HEADERS    ${avtdatabase_ser_HEADERS}
    INCLUDES   ${avtdatabase_ser_INCLUDES}
    DEPENDS_ON ${avtdatabase_ser_DEPENDS}
               avtpipeline_ser)

#********************************* PARALLEL **********************************
if(VISIT_PARALLEL)
    visit_add_parallel_library(
        NAME       avtdatabase_par
        SOURCES    ${avtdatabase_par_SOURCES}
        HEADERS    ${avtdatabase_par_HEADERS}
        INCLUDES   ${avtdatabase_par_INCLUDES}
        DEPENDS_ON ${avtdatabase_par_DEPENDS}
                   avtpipeline_par)
endif()

