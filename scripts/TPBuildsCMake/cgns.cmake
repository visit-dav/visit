# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

project(CGNS VERSION 4.1.0)

# set global variables used by other libraries
set(visit_cgns_root     ${visit_install_root}/cgns/${CGNS_VERSION})
set(visit_cgns_include  ${visit_cgns_root}/include)
set(visit_cgns_library  ${visit_cgns_root}/lib/cgnsdll.lib)

# has this already been built
if(EXISTS ${visit_cgns_include}/cgnslib.h AND
   EXISTS ${visit_cgns_library})
    if(BUILD_ALL OR  ENABLE_CGNS)
        message(STATUS "CGNS already installed")
    endif()
    set(CGNS_PREBUILT TRUE)
    return()
endif()

# if not requested, return
if(NOT (BUILD_ALL OR  ENABLE_CGNS))
    return() 
endif()

message(STATUS "CGNS being added to the build")

set(CGNS_ZIP_NAME "CGNS-${CGNS_VERSION}.tar.gz")

set(CGNS_CMAKE_OPTIONS
    -DBUILD_SHARED_LIBS:BOOL=${visit_build_shared}
    -DCMAKE_INSTALL_PREFIX:PATH=${visit_install_root}/cgns/${CGNS_VERSION}
    -DCGNS_ENABLE_64BIT:BOOL=ON
    -DENABLE_EXAMPLES:BOOL=FALSE
    -DENABLE_TESTS:BOOL=FALSE
    -DENABLE_PARALLEL:BOOL=FALSE)

set(CGNS_DEPENDS)

list(APPEND CGNS_CMAKE_OPTIONS
     -DCGNS_ENABLE_HDF5:BOOL=ON
     -DHDF5_DIR:PATH=${visit_hdf5_root}/cmake
     -DHDF5_INCLUDE_PATH:PATH=${visit_hdf5_include}
     -DHDF5_LIBRARY:FILEPATH=${visit_hdf5_library}
     -DHDF5_NEED_ZLIB:BOOL=ON
     -DZLIB_INCLUDE_DIR:PATH=${visit_zlib_include}
     -DZLIB_LIBRARY:FILEPATH=${visit_zlib_library})

if(NOT ZLIB_PREBUILT)
    list(APPEND CGNS_DEPENDS ZLIB)
endif()

if(NOT HDF5_PREBUILT)
    list(APPEND CGNS_DEPENDS HDF5)
endif()

ExternalProject_Add( CGNS
    PREFIX           CGNS
    URL              ${visit_tp_url}/${CGNS_ZIP_NAME}
    URL_HASH         SHA256=b4584e4d0fa52c737a0fb4738157a88581df251c8c5886175ee287e1777e99fd
    INSTALL_DIR      ${visit_install_root}/cgns/${CGNS_VERSION}
    CMAKE_CACHE_ARGS ${CGNS_CMAKE_OPTIONS}
    DEPENDS          ${CGNS_DEPENDS}
    ${LOGGING_OPTIONS}
    PATCH_COMMAND    ${CMAKE_COMMAND} -DCGNS_VERSION:STRING=${CGNS_VERSION}
                                      -DPATCH_DIR:PATH=${visit_tp_patches}
                                      -DPATCH_CMD:PATH=${visit_patch_command}
                                      -P ${visit_tp_patches}/cgns-patch.cmake)


