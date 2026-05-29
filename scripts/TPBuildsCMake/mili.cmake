# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

project(MILI VERSION 24.1.1)

set(visit_mili_root ${visit_install_root}/mili/${MILI_VERSION})

# has this already been built
if(EXISTS ${visit_mili_root}/include/mili.h AND
   EXISTS ${visit_mili_root}/lib/mili.lib)
    if(BUILD_ALL OR ENABLE_MILI)
        message(STATUS "MILI already installed")
    endif()
    set(MILI_PREBUILT TRUE)
    return()
endif()

# if not requested, return
if(NOT (BUILD_ALL OR ENABLE_MILI))
    message(STATUS "MILI already installed")
endif()

message(STATUS "MILI being added to the build")

set(MILI_ZIP_NAME "mili-${MILI_VERSION}.tar.gz")

set(MILI_CMAKE_OPTIONS
    -DCMAKE_INSTALL_PREFIX:PATH=${visit_mili_root})

ExternalProject_Add(MILI
    PREFIX           MILI
    URL              https://github.com/mdg-graphics/mili/archive/refs/tags/24.1.1.tar.gz 
                     #${visit_tp_url}/${MILI_ZIP_NAME}
    INSTALL_DIR      ${visit_mili_root}
    CMAKE_CACHE_ARGS ${MILI_CMAKE_OPTIONS} 
    ${LOGGING_OPTIONS}
    PATCH_COMMAND    ${CMAKE_COMMAND} -DMILI_VERSION:STRING=${MILI_VERSION}
                                      -DPATCH_CMD:PATH=${visit_patch_command}
                                      -DPATCH_DIR:PATH=${visit_tp_patches}
                                      -P ${visit_tp_patches}/mili-patch.cmake)

