# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

project(OPENEXR VERSION 3.3.4)

set(visit_openexr_dir      ${visit_install_root}/openexr/${OPENEXR_VERSION})

# has this already been built
if(EXISTS ${visit_openexr_dir}/include/openexr.h AND
   EXISTS ${visit_openexr_dir}/lib/OpenEXR-3_3.lib)
    if(BUILD_ALL OR ENABLE_OPENEXR)
       message(STATUS "OpenEXR already installed.")
    endif()
    set(OPENEXR_PREBUILT TRUE)
    return()
endif()

# if not requested, return
if(NOT (BUILD_ALL OR ENABLE_OPENEXR))
    return()
endif()

message(STATUS "OpenEXR being added to the build")

set(OPENEXR_ZIP_NAME "openexr-${OPENEXR_VERSION}.tar.gz")

set(OPENEXR_URL https://github.com/AcademySoftwareFoundation/openexr/releases/download/v${OPENEXR_VERSION})

set(OPENEXR_CMAKE_OPTIONS
    -DCMAKE_INSTALL_PREFIX:PATH=${visit_install_root}/openexr/${OPENEXR_VERSION}
    -DCMAKE_BUILD_TYPE:STRING=Release
    -DBUILD_SHARED_LIBS:BOOL=ON
    -DBUILD_TESTING:BOOL=OFF
    -DIMATH_CXX_STANDARD:STRING=17
    -DIMATH_OUTPUT_SUBDIR:STRING=
    -DIMATH_INSTALL_PKG_CONFIG:BOOL=OFF
    -DOPENEXR_BUILD_EXAMPLES:BOOL=OFF
    -DOPENEXR_BUILD_TOOLS:BOOL=OFF
    -DOPENEXR_BUILD_TOOLS:BOOL=OFF
    -DOPENEXR_INSTALL_PKG_CONFIG:BOOL=OFF
    -DOPENEXR_INSTALL_TOOLS:BOOL=OFF
    -DOPENEXR_OUTPUT_SUBDIR:STRING=
    -DOPENEXR_TEST_LIBRARIES:BOOL=OFF
    -DOPENEXR_TEST_PYTHON:BOOL=OFF
    -DOPENEXR_TEST_TOOLS:BOOL=OFF
    -DOPENEXR_FORCE_INTERNAL_IMATH:BOOL=ON)


ExternalProject_Add(OPENEXR
    PREFIX   OPENEXR
    URL      ${visit_tp_url}/${OPENEXR_ZIP_NAME}
             ${OPENEXR_URL}/${OPENEXR_ZIP_NAME}
    URL_HASH SHA256=73a6d83edcc68333afb95e133f6e12012073815a854bc41abc1a01c1db5f124c
    INSTALL_DIR ${visit_install_root}/openexr/${OPENEXR_VERSION}
    CMAKE_CACHE_ARGS ${OPENEXR_CMAKE_OPTIONS}
    ${LOGGING_OPTIONS})

