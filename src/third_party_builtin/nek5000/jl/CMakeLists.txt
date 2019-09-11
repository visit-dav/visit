# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#   Kathleen Biagas, Wed Nov  6 18:01:08 PST 2013
#   Changed location of generated 'nek_config.h' so that it won't overwrite
#   repo object on Windows.
#
#****************************************************************************

set(CMAKE_VERBOSE_MAKEFILE ON)

set(FUN_PREFIX "nek_" CACHE STRING "prefix for exported symbols")

option(LOCAL_LONG "use long for local index type (instead of int)" ON)
option(LOCAL_LONG_LONG "use long long for local index type (instead of int)" OFF)

ADD_DEFINITIONS(-DPREFIX=${FUN_PREFIX} -DUSE_NAIVE_BLAS -DNO_NEK_EXITT -DUNDERSCORE)

if(LOCAL_LONG)
  add_definitions(-DUSE_LONG)
  set(NEK_UINT "unsigned long")
elseif(LOCAL_LONG_LONG)
  add_definitions(-DUSE_LONG_LONG)
  set(NEK_UINT "unsigned long long")
else()
  set(NEK_UINT "unsigned")
endif()

configure_file (
  "${CMAKE_CURRENT_SOURCE_DIR}/ext/nek_config.h.in"
  "${VISIT_BINARY_DIR}/include/nek_config.h"
  )


SET(NEK5000_INTERP_SOURCES
sort.c
sarray_sort.c
obbox.c
tensor.c
poly.c
lob_bnd.c
findpts_el_3.c
findpts_el_2.c
fail.c
comm.c
gs_local.c
findpts_local.c
mem.c
ext/findpts_local_ext.c
)

INCLUDE_DIRECTORIES(
${VISIT_SOURCE_DIR}/third_party_builtin/nek5000
${VISIT_SOURCE_DIR}/visit_vtk/full
${CMAKE_CURRENT_SOURCE_DIR}
${CMAKE_CURRENT_BINARY_DIR}
)

#Add link directories needed.
LINK_DIRECTORIES()

ADD_LIBRARY(nek5000_interp ${NEK5000_INTERP_SOURCES})
TARGET_LINK_LIBRARIES(nek5000_interp)

VISIT_INSTALL_TARGETS(nek5000_interp)
SET_TARGET_PROPERTIES(nek5000_interp PROPERTIES FOLDER thirdparty_builtin/nek5000_interp)
