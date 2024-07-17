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


set(NEK5000_FUN_PREFIX "nek_" CACHE STRING "prefix for nek5000 exported symbols")

option(NEK5000_LOCAL_LONG "use long for nek5000 local index type (instead of int)" ON)
option(NEK5000_LOCAL_LONG_LONG "use long long for nek5000 local index type (instead of int)" OFF)

set(nek5000_interp_DEFINES
    PREFIX=${NEK5000_FUN_PREFIX}
    USE_NAIVE_BLAS
    NO_NEK_EXITT
    UNDERSCORE)

if(NEK5000_LOCAL_LONG)
  list(APPEND nek5000_interp_DEFINES NEK_USE_LONG)
  set(NEK_UINT "unsigned long")
elseif(NEK5000_LOCAL_LONG_LONG)
  list(APPEND nek5000_interp_DEFINES NEK_USE_LONG_LONG)
  set(NEK_UINT "unsigned long long")
else()
  set(NEK_UINT "unsigned")
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ext/nek_config.h.in
               ${VISIT_BINARY_DIR}/include/nek_config.h)


set(nek5000_interp_SOURCES
    comm.c
    fail.c
    findpts_el_2.c
    findpts_el_3.c
    findpts_local.c
    gs_local.c
    lob_bnd.c
    mem.c
    obbox.c
    poly.c
    sarray_sort.c
    sort.c
    tensor.c
    ext/findpts_local_ext.c)

set(nek5000_interp_HEADERS
    c99.h
    comm.h
    fail.h
    findpts_el.h
    findpts_local.h
    findpts_local_imp.h
    gs.h
    gs_defs.h
    gs_local.h
    lob_bnd.h
    mem.h
    name.h
    obbox.h
    poly.h
    poly_imp.h
    sarray_sort.h
    sort.h
    sort_imp.h
    tensor.h
    types.h
    ext/findpts_local.h
    ${VISIT_BINARY_DIR}/include/nek_config.h)

set(nek5000_interp_INCLUDES
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${VISIT_BINARY_DIR}/include>)


visit_add_library(
    NAME     nek5000_interp
    SOURCES  ${nek5000_interp_SOURCES}
    HEADERS  ${nek5000_interp_HEADERS}
    DEFINES  ${nek5000_interp_DEFINES}
    INCLUDES ${nek5000_interp_INCLUDES}
    FOLDER   thirdparty_builtin/nek5000_interp)

