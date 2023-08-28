# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

set(PICK_SOURCES
    avtActualCoordsQuery.C
    avtActualNodeCoordsQuery.C
    avtActualZoneCoordsQuery.C
    avtCurvePickQuery.C
    avtLocateCellQuery.C
    avtLocateNodeQuery.C
    avtLocateQuery.C
    avtNodePickQuery.C
    avtPickByNodeQuery.C
    avtPickByZoneQuery.C
    avtPickQuery.C
    avtZonePickQuery.C)

target_sources(avtquery_ser PRIVATE ${PICK_SOURCES})
target_include_directories(avtquery_ser PUBLIC .)

if(VISIT_PARALLEL)
    target_sources(avtquery_par PRIVATE ${PICK_SOURCES})
    target_include_directories(avtquery_par PUBLIC .)
endif()

unset(PICK_SOURCES)

