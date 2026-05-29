# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

project(CFITSIO VERSION 4.6.3)

# set global variables used by other libraries
set(visit_cfitsio_root     ${visit_install_root}/cfitsio/${CFITSIO_VERSION})
set(visit_cfitsio_include  ${visit_cfitsio_root}/include)
set(visit_cfitsio_library  ${visit_cfitsio_root}/lib/cfitsio.lib)

# has this already been built
if(EXISTS ${visit_cfitsio_include}/fitsio.h AND
   EXISTS ${visit_cfitsio_library})
    if(BUILD_ALL OR ENABLE_CFITSIO)
        message(STATUS "CFITSIO already installed")
    endif()
    set(CFITSIO_PREBUILT TRUE)
    return()
endif()

# if not requested return
if(NOT (BUILD_ALL OR ENABLE_CFITSIO))
    message(STATUS "CFITSIO already installed")
endif()

message(STATUS "CFITSIO being added to the build")

set(CFITSIO_ZIP_NAME "cfitsio-${CFITSIO_VERSION}.tar.gz")


set(CFITSIO_CMAKE_OPTIONS
    -DBUILD_SHARED_LIBS:BOOL=${visit_build_shared}
    -DCMAKE_INSTALL_PREFIX:PATH=${visit_install_root}/cfitsio/${CFITSIO_VERSION}
    -DTESTS:BOOL=OFF
    -DUSE_CURL:BOOL=OFF
    -DUTILS:BOOL=OFF)

set(CFITSIO_DEPENDS)

list(APPEND CFITSIO_CMAKE_OPTIONS
    -DZLIB_INCLUDE_DIR:FILEPATH=${visit_zlib_include}
    -DZLIB_LIBRARY_RELEASE:FILEPATH=${visit_zlib_library})

if(NOT ZLIB_PREBUILT)
    list(APPEND CFITSIO_DEPENDS ZLIB)
endif()


ExternalProject_Add( CFITSIO
    PREFIX           CFITSIO
    URL              ${visit_tp_url}/${CFITSIO_ZIP_NAME}
    URL_HASH         SHA256=fad44fff274fdda5ffcc0c0fff3bc3c596362722b9292fc8944db91187813600 
    INSTALL_DIR      ${visit_install_root}/cfitsio/${CFITSIO_VERSION}
    CMAKE_CACHE_ARGS ${CFITSIO_CMAKE_OPTIONS}
    DEPENDS          ${CFITSIO_DEPENDS}
    ${LOGGING_OPTIONS})


