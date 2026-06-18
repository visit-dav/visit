# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

project(NETCDF VERSION 4.9.3)

set(visit_netcdf_root    ${visit_install_root}/netcdf/${NETCDF_VERSION})
set(visit_netcdf_include ${visit_netcdf_root}/include)
set(visit_netcdf_library ${visit_netcdf_root}/lib/netcdf.lib)

# has this already been built
if(EXISTS ${visit_netcdf_include}/netcdf.h AND
   EXISTS ${visit_netcdf_library})
    if(BUILD_ALL OR ENABLE_NETCDF)
        message(STATUS "NETCDF already installed.")
    endif()
    set(NETCDF_PREBUILT TRUE)
    return()
endif()

# if not requested, return
if(NOT (BUILD_ALL OR ENABLE_NETCDF))
    return() 
endif()

message(STATUS "NETCDF being added to the build")

set(NETCDF_ZIP_NAME "netcdf-c-${NETCDF_VERSION}.tar.gz")

set(NETCDF_CMAKE_OPTIONS
    -DBUILD_SHARED_LIBS:BOOL=${visit_build_shared}
    -DBUILD_TESTING:BOOL=OFF
    -DBUILD_TESTSETS:BOOL=OFF
    -DNETCDF_BUILD_UTILITIES:BOOL=OFF
    -DNETCDF_ENABLE_DAP:BOOL=OFF
    -DNETCDF_ENABLE_DAP2:BOOL=OFF
    -DNETCDF_ENABLE_DAP4:BOOL=OFF
    -DNETCDF_ENABLE_DAP_REMOTE_TESTS:BOOL=OFF
    -DNETCDF_ENABLE_EXAMPLES:BOOL=OFF
    -DNETCDF_ENABLE_FILTER_BLOSC:BOOL=OFF
    -DNETCDF_ENABLE_FILTER_BZ2:BOOL=OFF
    -DNETCDF_ENABLE_FILTER_SZIP:BOOL=OFF
    -DNETCDF_ENABLE_FILTER_TESTING:BOOL=OFF
    -DNETCDF_ENABLE_FILTER_ZSTD:BOOL=OFF
    -DNETCDF_ENABLE_HDF5:BOOL=ON
    -DNETCDF_ENABLE_HDF5:BOOL=ON
    -DNETCDF_ENABLE_NCZARR:BOOL=OFF
    -DNETCDF_ENABLE_NCZARR_FILTERS:BOOL=OFF
    -DNETCDF_ENABLE_NCZARR_S3:BOOL=OFF
    -DNETCDF_ENABLE_NCZARR_ZIP:BOOL=OFF
    -DNETCDF_ENABLE_PLUGINS:BOOL=OFF
    -DNETCDF_ENABLE_REMOTE_FUNCTIONALITY:BOOL=OFF
    -DNETCDF_ENABLE_TESTS:BOOL=OFF
    -DNETCDF_ENABLE_UNIT_TESTS:BOOL=OFF
    -DNETCDF_PACKAGE:BOOL=OFF
    -DENABLE_V2_API:BOOL=ON
    -DNETCDF_ENABLE_MMAP:BOOL=OFF
    -DNETCDF_ENABLE_BASH_SCRIPT_TESTING:BOOL=OFF
    -DUSE_DAP:BOOL=OFF
    -DHDF5_DIR:PATH=${visit_hdf5_root}/cmake
    -DZLIB_INCLUDE_DIR:PATH=${visit_zlib_include}
    -DZLIB_LIBRARY:FILEPATH=${visit_zlib_library}
    -DCMAKE_BUILD_TYPE:STRING=Release
    -DCMAKE_INSTALL_PREFIX:PATH=${visit_netcdf_root})

if(NOT HDF5_PREBUILT)
    list(APPEND NETCDF_DEPENDS HDF5)
endif()

if(NOT ZLIB_PREBUILT)
    list(APPEND NETCDF_DEPENDS ZLIB)
endif()

ExternalProject_Add(NETCDF
    PREFIX           NETCDF
    URL              ${visit_tp_url}/${NETCDF_ZIP_NAME}
    URL_HASH         SHA256=a474149844e6144566673facf097fea253dc843c37bc0a7d3de047dc8adda5dd
    INSTALL_DIR      ${visit_netcdf_root}
    CMAKE_CACHE_ARGS ${NETCDF_CMAKE_OPTIONS}
    DEPENDS          ${NETCDF_DEPENDS}
    ${LOGGING_OPTIONS}
    PATCH_COMMAND    ${CMAKE_COMMAND} -DNETCDF_VERSION:STRING=${NETCDF_VERSION}
                                      -DPATCH_DIR:PATH=${visit_tp_patches}
                                      -DPATCH_CMD:PATH=${visit_patch_command}
                                      -P ${visit_tp_patches}/netcdf-patch.cmake)

