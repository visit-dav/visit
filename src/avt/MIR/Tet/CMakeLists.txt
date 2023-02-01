# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#****************************************************************************

set(TET_SOURCES
    Edge.C
    Face.C
    mat3d_tet.C
    MaterialTetrahedron.C
    MaterialTriangle.C
    QuadraticHash.C
    TetMIR.C
    Tetrahedralizer.C
    Triangulator.C)

target_sources(avtmir_ser PRIVATE ${TET_SOURCES})
target_include_directories(avtmir_ser PUBLIC .)

if(VISIT_PARALLEL)
    target_sources(avtmir_par PRIVATE ${TET_SOURCES})
    target_include_directories(avtmir_par PUBLIC .)
endif()

unset(TET_SOURCES)

