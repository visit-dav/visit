# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

SET(TESS2_SOURCES
    bucketalloc.C
    dict.C
    geom.C
    mesh.C
    priorityq.C
    sweep.C
    tess.C
)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

ADD_LIBRARY(tess2 ${TESS2_SOURCES})

VISIT_INSTALL_TARGETS(tess2)
SET_TARGET_PROPERTIES(tess2 PROPERTIES FOLDER thirdparty_builtin/tess2)
