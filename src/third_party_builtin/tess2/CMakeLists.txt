# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

set(TESS2_SOURCES
    bucketalloc.C
    dict.C
    geom.C
    mesh.C
    priorityq.C
    sweep.C
    tess.C)

set(TESS2_HEADERS
    bucketalloc.h
    dict.h
    geom.h
    mesh.h
    priorityq.h
    sweep.h
    tess.h
    tess_exports.h
    tesselator.h)


visit_add_library(
    NAME     tess2
    SOURCES  ${TESS2_SOURCES}
    HEADERS  ${TESS2_HEADERS}
    INCLUDES $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
             $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/thirdparty_builtin/tess2>
    FOLDER   thirdparty_builtin/tess2)

