# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

project(H5PART VERSION 1.6.6)

set(visit_h5part_root    ${visit_install_root}/h5part/${H5PART_VERSION})
set(visit_h5part_include ${visit_h5part_root}/include)
set(visit_h5part_library ${visit_h5part_root}/lib/h5part.lib)

# has this already been built
if(EXISTS ${visit_h5part_include}/h5part.h AND
   EXISTS ${visit_h5part_library})
    if(BUILD_ALL OR ENABLE_H5PART)
        message(STATUS "H5PART already installed.")
    endif()
    set(H5PART_PREBUILT TRUE)
    return()
endif()

# if not requested, return
if(NOT (BUILD_ALL OR ENABLE_H5PART))
    return()
endif()

message(STATUS "H5PART being added to the build")

set(H5PART_ZIP_NAME "H5Part-${H5PART_VERSION}.tar.gz")

set(H5PART_CMAKE_OPTIONS
    -DBUILD_SHARED_LIBS:BOOL=${visit_build_shared}
    -DCMAKE_INSTALL_PREFIX:PATH=${visit_h5part_root}
    -DHDF5_INCLUDE_DIR:PATH=${visit_hdf5_include}
    -DHDF5_LIBRARY:FILEPATH=${visit_hdf5_library}
    -DZLIB_INCLUDE_DIR:PATH=${visit_zlib_include}
    -DZLIB_LIBRARY:FILEPATH=${visit_zlib_library})

if(NOT HDF5_PREBUILT)
    list(APPEND H5PART_DEPENDS HDF5)
endif()

if(NOT ZLIB_PREBUILT)
    list(APPEND H5PART_DEPENDS ZLIB)
endif()

ExternalProject_Add(H5PART
    PREFIX           H5PART
    URL              ${visit_tp_url}/${H5PART_ZIP_NAME}
    URL_HASH         SHA256=10347e7535d1afbb08d51be5feb0ae008f73caf889df08e3f7dde717a99c7571
    INSTALL_DIR      ${visit_h5part_root}
    CMAKE_CACHE_ARGS ${H5PART_CMAKE_OPTIONS}
    DEPENDS          ${H5PART_DEPENDS}
    ${LOGGING_OPTIONS}
    PATCH_COMMAND    ${CMAKE_COMMAND} -DH5PART_VERSION:STRING=${H5PART_VERSION}
                                      -DPATCH_DIR:PATH=${visit_tp_patches}
                                      -DPATCH_CMD:PATH=${visit_patch_command}
                                      -P ${visit_tp_patches}/h5part-patch.cmake)

