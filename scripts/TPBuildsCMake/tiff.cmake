# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

project(TIFF VERSION 4.5.0)

# set global variables used by other libraries 
set(visit_tiff_root    ${visit_install_root}/tiff/${TIFF_VERSION})
set(visit_tiff_include ${visit_tiff_root}/include)
set(visit_tiff_library ${visit_tiff_root}/lib/tiff.lib)

# has this already been built
if(EXISTS ${visit_tiff_include}/tiff.h AND
   EXISTS ${visit_tiff_library})
    if(BUILD_ALL OR ENABLE_TIFF)
        message(STATUS "TIFF already installed.")
    endif()
    set(TIFF_PREBUILT TRUE)
    return()
endif()

# if not requested, return
if(NOT (BUILD_ALL OR ENABLE_TIFF))
    return()
endif()

message(STATUS "TIFF being added to the build")

set(TIFF_ZIP_NAME "tiff-${TIFF_VERSION}.tar.gz")

set(TIFF_CMAKE_OPTIONS
    -DCMAKE_INSTALL_PREFIX:PATH=${visit_tiff_root}
    -Dtiff-tests:BOOL=OFF
    -Dtiff-tools:BOOL=OFF
    -Djpeg:BOOL=ON
    -DJPEG_INCLUDE_DIR:PATH=${visit_jpeg_include}
    -DJPEG_LIBRARY_RELEASE:PATH=${visit_jpeg_library}
    -Dzlib:BOOL=ON
    -DZLIB_INCLUDE_DIR:PATH=${visit_zlib_include}
    -DZLIB_LIBRARY_RELEASE:PATH=${visit_zlib_library})

set(TIFF_DEPENDS)

if(NOT JPEG_PREBUILT)
    list(APPEND TIFF_DEPENDS JPEG)
endif()

if(NOT ZLIB_PREBUILT)
    list(APPEND TIFF_DEPENDS ZLIB)
endif()

ExternalProject_Add(TIFF
    PREFIX           TIFF
    URL              https://download.osgeo.org/libtiff/${TIFF_ZIP_NAME}
    URL_HASH         SHA256=c7a1d9296649233979fa3eacffef3fa024d73d05d589cb622727b5b08c423464 
    INSTALL_DIR      ${visit_tiff_root}
    CMAKE_CACHE_ARGS ${TIFF_CMAKE_OPTIONS}
    DEPENDS          ${TIFF_DEPENDS}
    ${LOGGING_OPTIONS})

