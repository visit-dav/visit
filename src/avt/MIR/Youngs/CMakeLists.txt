# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#****************************************************************************

set(YOUNGS_SOURCES
    CellInterface.C
    CellInterface2D.C
    CellInterface3D.C
    vtkCQS.C
    vtkDataSetGradient.C
    vtkYoungsMaterialInterface.C
    YoungsMIR.C)

target_sources(avtmir_ser PRIVATE ${YOUNGS_SOURCES})
target_include_directories(avtmir_ser PUBLIC .)

if(VISIT_PARALLEL)
    target_sources(avtmir_par PRIVATE ${YOUNGS_SOURCES})
    target_include_directories(avtmir_par PUBLIC .)
endif()

unset(YOUNGS_SOURCES)

