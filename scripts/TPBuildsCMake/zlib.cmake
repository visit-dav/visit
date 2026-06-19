# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

project(ZLIB VERSION 1.3.1)

# set global variables used by other libraries
set(visit_zlib_root    ${visit_install_root}/zlib/${ZLIB_VERSION})
set(visit_zlib_include ${visit_zlib_root}/include)
set(visit_zlib_library ${visit_zlib_root}/lib/zlib.lib)

# has this already been built
if(EXISTS ${visit_zlib_include}/zlib.h AND
   EXISTS ${visit_zlib_library})
    if(BUILD_ALL OR ENABLE_ZLIB)
        message(STATUS "ZLIB already installed.")
    endif()
    set(ZLIB_PREBUILT TRUE)
    return()
endif()

# if not requested, return
if(NOT (BUILD_ALL OR ENABLE_ZLIB))
   return()
endif()

message(STATUS "ZLIB being added to the build")

set(ZLIB_ZIP_NAME "zlib-${ZLIB_VERSION}.tar.xz")

set(ZLIB_CMAKE_OPTIONS
    -DBUILD_SHARED_LIBS:BOOL=${visit_build_shared}
    -DZLIB_BUILD_EXAMPLES:BOOL=OFF
    -DCMAKE_INSTALL_PREFIX:PATH=${visit_zlib_root})

ExternalProject_Add(ZLIB
    PREFIX           ZLIB
    URL              ${visit_tp_url}/${ZLIB_ZIP_NAME}
    URL_HASH         SHA256=38ef96b8dfe510d42707d9c781877914792541133e1870841463bfa73f883e32
    INSTALL_DIR      ${visit_zlib_root}
    CMAKE_CACHE_ARGS ${ZLIB_CMAKE_OPTIONS}
    ${LOGGING_OPTIONS}
    PATCH_COMMAND    ${CMAKE_COMMAND} -DZLIB_VERSION:STRING=${ZLIB_VERSION} -DPATCH_DIR:PATH=${visit_tp_patches} -P ${visit_tp_patches}/zlib-patch.cmake )

