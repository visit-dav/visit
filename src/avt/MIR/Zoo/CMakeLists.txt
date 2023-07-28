# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#****************************************************************************

set(ZOO_SOURCES
    BitUtils.C
    CellReconstructor.C
    IsovolumeCellReconstructor.C
    RecursiveCellReconstructor.C
    ResampledMat.C
    ZooMIR.C)

target_sources(avtmir_ser PRIVATE ${ZOO_SOURCES})
target_include_directories(avtmir_ser PUBLIC .)

if(VISIT_PARALLEL)
    target_sources(avtmir_par PRIVATE ${ZOO_SOURCES})
    target_include_directories(avtmir_par PUBLIC .)
endif()

unset(ZOO_SOURCES)

