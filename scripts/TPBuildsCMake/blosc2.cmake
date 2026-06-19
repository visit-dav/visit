# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

project(BLOSC2 VERSION 2.11.3)

# set global variables used by other libraries
set(visit_blosc2_root     ${visit_install_root}/blosc2/${BLOSC2_VERSION})
set(visit_blosc2_include  ${visit_blosc2_root}/include)
set(visit_blosc2_library  ${visit_blosc2_root}/lib/libblosc2.lib)

# has this already been built
if(EXISTS ${visit_blosc2_include}/blosc2.h AND
   EXISTS ${visit_blosc2_library})
    if(BUILD_ALL OR ENABLE_BLOSC2)
        message(STATUS "BLOSC2 already installed")
    endif()
    set(BLOSC2_PREBUILT TRUE)
    return()
endif()

# if not requested,return
if(NOT (BUILD_ALL OR ENABLE_BLOSC2))
    return()
endif()

message(STATUS "BLOSC2 being added to the build")

set(BLOSC2_ZIP_NAME "c-blosc2-${BLOSC2_VERSION}.tar.gz")

set(BLOSC2_CMAKE_OPTIONS
    -DCMAKE_INSTALL_PREFIX:PATH=${visit_install_root}/blosc2/${BLOSC2_VERSION}
    -DBUILD_SHARED:BOOL=FALSE
    -DBUILD_STATIC:BOOL=TRUE
    -DBUILD_BENCHMARKS:BOOL=FALSE
    -DBUILD_EXAMPLES:BOOL=FALSE
    -DBUILD_TESTS:BOOL=FALSE
    -DBUILD_FUZZERS:BOOL=FALSE)

set(BLOSC2_DEPENDS)

ExternalProject_Add(BLOSC2
    PREFIX           BLOSC2
    URL              ${visit_tp_url}/${BLOSC2_ZIP_NAME}
    URL_HASH         SHA256=7273ec3ab42adc247425ab34b0601db86a6e2a6aa1a97a11e29df02e078f5037
    INSTALL_DIR      ${visit_install_root}/blosc2/${BLOSC2_VERSION}
    CMAKE_CACHE_ARGS ${BLOSC2_CMAKE_OPTIONS}
    DEPENDS          ${BLOSC2_DEPENDS}
    ${LOGGING_OPTIONS}
    PATCH_COMMAND    ${CMAKE_COMMAND} -DBLOSC2_VERSION:STRING=${BLOSC2_VERSION}
                                      -DPATCH_CMD:PATH=${visit_patch_command}
                                      -DPATCH_DIR:PATH=${visit_tp_patches}
                                      -P ${visit_tp_patches}/blosc2-patch.cmake)


