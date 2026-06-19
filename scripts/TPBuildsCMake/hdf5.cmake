# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

project(HDF5 VERSION 2.0.0)

# set global variables used by other libraries
set(visit_hdf5_root    ${visit_install_root}/hdf5/${HDF5_VERSION})
set(visit_hdf5_include ${visit_hdf5_root}/include)
set(visit_hdf5_library ${visit_hdf5_root}/lib/hdf5.lib)

# has this already been built 
if(EXISTS ${visit_hdf5_include}/hdf5.h AND
   EXISTS ${visit_hdf5_library})
    if(BUILD_ALL OR ENABLE_HDF5)
        message(STATUS "HDF5 already installed.")
    endif()
    set(HDF5_PREBUILT TRUE)
    return()
endif()

# if not requested, return
if(NOT (BUILD_ALL OR ENABLE_HDF5))
    return()
endif()

message(STATUS "HDF5 being added to the build")

set(HDF5_ZIP_NAME "hdf5-${HDF5_VERSION}.tar.xz")

set(HDF5_CMAKE_OPTIONS
    -DEXAMPLES_EXTERNALLY_CONFIGURED:BOOL=OFF
    -DBUILD_LZ4_LIBRARY_SOURCE:BOOL=OFF
    -DBUILD_TESTING:BOOL=OFF
    -DHDF5_BUILD_CPP_LIB:BOOL=OFF
    -DHDF5_BUILD_EXAMPLES:BOOL=OFF
    -DHDF5_BUILD_HL_LIB:BOOL=ON
    -DHDF5_BUILD_JAVA:BOOL=OFF
    -DHDF5_BUILD_TOOLS:BOOL=OFF
    -DHDF5_BUILD_UTILS:BOOL=OFF
    -DHDF5_ENABLE_ALL_WARNINGS:BOOL=OFF
    -DHDF5_ENABLE_NONSTANDARD_FEATURES:BOOL=OFF
    -DHDF5_ENABLE_PARALLEL:BOOL=OFF
    -DHDF5_ENABLE_SUBFILING_VFD:BOOL=OFF
    -DHDF5_TEST_API:BOOL=OFF
    -DHDF5_TEST_EXAMPLES:BOOL=OFF
    -DHDF5_TEST_SERIAL:BOOL=OFF
    -DHDF5_TEST_SHELL_SCRIPTS:BOOL=OFF
    -DHDF5_TEST_SWMR:BOOL=OFF
    -DHDF5_TEST_TOOLS:BOOL=OFF
    -DHDF5_ENABLE_ZLIB_SUPPORT:BOOL=ON
    -DZLIB_DIR:PATH=${visit_zlib_root}
    -DZLIB_INCLUDE_DIR:PATH=${visit_zlib_include}
    -DZLIB_LIBRARY_RELEASE:FILEPATH=${visit_zlib_library}
    -DCMAKE_INSTALL_PREFIX:PATH=${visit_hdf5_root})

if(${visit_build_shared})
    list(APPEND HDF5_CMAKE_OPTIONS
        -DHDF5_ONLY_SHARED_LIBS:BOOL=ON)
else()
    list(APPEND HDF5_CMAKE_OPTIONS
        -DBUILD_SHARED_LIBS:BOOL=OFF
        -DBUILD_STATIC_LIBS:BOOL=ON)
endif()


set(HDF5_DEPENDS)

if(NOT ZLIB_PREBUILT)
    list(APPEND HDF5_DEPENDS ZLIB)
endif()

ExternalProject_Add(HDF5
    PREFIX           HDF5
    URL              ${visit_tp_url}/${HDF5_ZIP_NAME}
    URL_HASH         SHA256=7649f45b4b3cd3350e0b13b790204d377c89e4ce3b346d9ecffb29552737dc71
    INSTALL_DIR      ${visit_hdf5_root}
    CMAKE_CACHE_ARGS ${HDF5_CMAKE_OPTIONS}
    DEPENDS          ${HDF5_DEPENDS}
    ${LOGGING_OPTIONS}
    PATCH_COMMAND    ${CMAKE_COMMAND} -DHDF5_VERSION:STRING=${HDF5_VERSION}
                                      -DPATCH_CMD:PATH=${visit_patch_command}
                                      -DPATCH_DIR:PATH=${visit_tp_patches}
                                      -P ${visit_tp_patches}/hdf5-patch.cmake)

