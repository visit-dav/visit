# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

project(SILO VERSION 4.12.0)

# set global variables used by other libraries
set(visit_silo_root       ${visit_install_root}/silo/${SILO_VERSION})
set(visit_silo_include    ${visit_silo_root}/include)
set(visit_silo_library    ${visit_silo_root}/lib/siloh5.lib)

# has this already been built
if(EXISTS ${visit_silo_include}/silo.h AND
   EXISTS ${visit_silo_library})
    if(BUILD_ALL OR ENABLE_SILO)
        message(STATUS "SILO already installed.")
    endif()
    set(SILO_PREBUILT TRUE )
    return()
endif()

# if not requested, return
if(NOT (BUILD_ALL OR ENABLE_SILO))
    message(STATUS "SILO already installed.")
endif()

message(STATUS "SILO being added to the build")

set(SILO_ZIP_NAME Silo-${SILO_VERSION}.tar.xz)

set(SILO_CMAKE_OPTIONS
    -DBUILD_SHARED_LIBS:BOOL=${visit_build_shared}
    -DCMAKE_INSTALL_PREFIX:PATH=${visit_silo_root}
    -DSILO_ENABLE_FORTRAN:BOOL=OFF
    -DSILO_ENABLE_INSTALL_LITE_HEADERS:BOOL=ON
    -DSILO_ZLIB_DIR:PATH=${visit_zlib_root}
    -DSILO_ENABLE_HDF5:BOOL=ON
    -DSILO_HDF5_DIR:PATH=${visit_hdf5_root}
    -DSILO_ENABLE_SILEX:BOOL=ON
    -DSILO_QT6_DIR:FILEPATH=${visit_qt_root})

set(SILO_DEPENDS)

if(NOT ZLIB_PREBUILT)
    list(APPEND SILO_DEPENDS ZLIB)
endif()

if(NOT HDF5_PREBUILT)
    list(APPEND SILO_DEPENDS HDF5)
endif()

if(NOT QT_PREBUILT)
    list(APPEND SILO_DEPENDS QT)
endif()

ExternalProject_Add(SILO
    PREFIX           SILO
    URL              ${visit_tp_url}/${SILO_ZIP_NAME}
    URL_HASH         SHA256=bde1685e4547d5dd7416bd6215b41f837efef0e4934d938ba776957afbebdff0
    INSTALL_DIR      ${visit_silo_root}
    CMAKE_CACHE_ARGS ${SILO_CMAKE_OPTIONS}
    DEPENDS          ${SILO_DEPENDS}
    ${LOGGING_OPTIONS}
    PATCH_COMMAND    ${CMAKE_COMMAND} -DSILO_VERSION:STRING=${SILO_VERSION}
                                      -DPATCH_DIR:PATH=${visit_tp_patches}
                                      -DPATCH_CMD:PATH=${visit_patch_command}
                                      -P ${visit_tp_patches}/silo-patch.cmake)

