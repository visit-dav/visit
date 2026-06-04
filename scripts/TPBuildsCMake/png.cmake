# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

project(PNG VERSION 1.6.55)

# set global variables used by other libraries
set(visit_png_root        ${visit_install_root}/png/${PNG_VERSION})
set(visit_png_include     ${visit_png_root}/include)
set(visit_png_library_dir ${visit_png_root}/lib)
set(visit_png_library     ${visit_png_library_dir}/libpng16.lib)

# has this already been built
if(EXISTS ${visit_png_include}/png.h AND
   EXISTS ${visit_png_library})
    if(BUILD_ALL OR ENABLE_PNG)
        message(STATUS "PNG already installed.")
    endif()
    set(PNG_PREBUILT TRUE)
    return()
endif()

# if not requested, return
if(NOT (BUILD_ALL OR ENABLE_PNG))
    return()
endif()

message(STATUS "PNG being added to the build")

set(PNG_ZIP_NAME "libpng-${PNG_VERSION}.tar.gz")

set(PNG_CMAKE_OPTIONS
    -DPNG_SHARED:BOOL=ON
    -DPNG_STATIC:BOOL=OFF
    -DPNG_TESTS:BOOL=OFF
    -DPNG_TOOLS:BOOL=OFF
    -DZLIB_ROOT:PATH=${visit_zlib_root}
    -DCMAKE_INSTALL_PREFIX:PATH=${visit_png_root})

set(PNG_DEPENDS)

if(NOT ZLIB_PREBUILT)
    list(APPEND PNG_DEPENDS ZLIB)
endif()

ExternalProject_Add(PNG
    PREFIX            PNG
    URL               https://github.com/pnggroup/libpng/archive/refs/tags/v${PNG_VERSION}.tar.gz
    URL_HASH          SHA256=71a2c5b1218f60c4c6d2f1954c7eb20132156cae90bdb90b566c24db002782a6 
    INSTALL_DIR       ${visit_png_root}
    CMAKE_CACHE_ARGS  ${PNG_CMAKE_OPTIONS}
    DEPENDS           ${PNG_DEPENDS} 
    ${LOGGING_OPTIONS})

