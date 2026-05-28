# Copyright (c) 2011, Allinea
# All rights reserved.
#
# This file has been contributed to the VisIt project, which is
# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

set(DDTSIM_SOURCES
    visitsim.cpp
    visitdata.cpp
    ddtsim.cpp
    visitmesh.cpp
    visitvariable.cpp)

set(DDTSIM_HEADERS
    visitsim.h
    visitdata.h
    ddtsim.h
    visitmesh.h
    visitvariable.h)

# Sources needed only when using dynamic linking
set(DDTSIM_DYNAMIC_SOURCES
    mpicompat/openmpi.c
    mpicompat/mpich.c
    dllibrary.cpp
    libsimlibrary.cpp)

set(DDTSIM_DYNAMIC_HEADERS
    mpicompat/openmpi.h
    mpicompat/mpich.h
    dllibrary.h
    libsimlibrary.h)

if(NOT VISIT_STATIC)
    visit_add_parallel_library(
        NAME        ddtsim
        SOURCES     ${DDTSIM_SOURCES}
                    ${DDTSIM_DYNAMIC_SOURCES}
        HEADERS     ${DDTSIM_HEADERS}
                    ${DDTSIM_DYNAMIC_HEADERS}
        INCLUDES    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/mpicompat>
                    $<BUILD_INTERFACE:${VISIT_SOURCE_DIR}/sim/V2/lib>
        SKIP_INSTALL)
        set_target_properties(ddtsim PROPERTIES COMPILE_OPTIONS "-g")

    install(TARGETS ddtsim
        ARCHIVE DESTINATION ${VISIT_INSTALLED_VERSION}/libsim/V2/lib
        LIBRARY DESTINATION ${VISIT_INSTALLED_VERSION}/libsim/V2/lib
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ GROUP_WRITE GROUP_EXECUTE
                    WORLD_READ             WORLD_EXECUTE)

else()
    visit_add_parallel_library(
        NAME        ddtsim_static
        SOURCES     ${DDTSIM_SOURCES}
        HEADERS     ${DDTSIM_HEADERS}
        INCLUDES    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/mpicompat>
                    $<BUILD_INTERFACE:${VISIT_SOURCE_DIR}/sim/V2/lib>
        DEFINES     VISIT_STATIC
        DEPENDS_ON  simv2_static_par
        SKIP_INSTALL)
        set_target_properties(ddtsim_static PROPERTIES COMPILE_OPTIONS "-g")

    if(NOT APPLE)
        set_target_properties(ddtsim_static PROPERTIES INSTALL_RPATH "$ORIGIN")
    endif()

    install(TARGETS ddtsim_static
        ARCHIVE DESTINATION ${VISIT_INSTALLED_VERSION}/libsim/V2/lib
        LIBRARY DESTINATION ${VISIT_INSTALLED_VERSION}/libsim/V2/lib
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ GROUP_WRITE GROUP_EXECUTE
                    WORLD_READ             WORLD_EXECUTE)
endif()

