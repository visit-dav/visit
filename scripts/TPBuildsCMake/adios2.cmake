# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

project(ADIOS2)

# version number like this isn't accepted by cmake, so set it separately
# from the 'project' command.
set(ADIOS2_VERSION 2.10.0-rc1)

# set global variables used by other librariess
set(visit_adios2_root     ${visit_install_root}/adios2/${ADIOS2_VERSION})
set(visit_adios2_include  ${visit_adios2_root}/include)
set(visit_adios2_library  ${visit_adios2_root}/lib/adios2_core.lib)
set(visit_adios2_library_par  ${visit_adios2_root}/lib/adios2_core_mpi.lib)

# has this already been built
if(EXISTS ${visit_adios2_include}/adios2.h AND
   EXISTS ${visit_adios2_library} AND
   EXISTS ${visit_adios2_library_par})
    if(BUILD_ALL OR ENABLE_ADIOS2)
        message(STATUS "ADIOS2 already installed")
    endif()
    set(ADIOS2_PREBUILT TRUE)
    return()
endif()

# if not requested, return
if(NOT (BUILD_ALL OR ENABLE_ADIOS2))
    return()
endif()

message(STATUS "ADIOS2 being added to the build")

set(ADIOS2_ZIP_NAME "adios2-${ADIOS2_VERSION}.tar.gz")


set(ADIOS2_CMAKE_OPTIONS
    -DBUILD_SHARED_LIBS:BOOL=${visit_build_shared}
    -DADIOS2_BUILD_EXAMPLES:BOOL=OFF
    -DADIOS2_BUILD_EXAMPLES_EXPERIMENTAL:BOOL=OFF
    -DADIOS2_USE_ZeroMQ:STRING=OFF
    -DADIOS2_USE_Fortran:STRING=OFF
    -DADIOS2_USE_Python:STRING=OFF
    -DADIOS2_USE_SST:STRING=ON
    -DADIOS2_RUN_INSTALL_TEST:BOOL=OFF
    -DADIOS2_INSTALL_GENERATE_CONFIG:BOOL=OFF
    -DADIOS2_USE_Catalyst:STRING=OFF
    -DADIOS2_USE_DAOS:STRING=OFF
    -DADIOS2_USE_HDF5_VOL:STRING=OFF
    -DCMAKE_INSTALL_PREFIX:STRING=${visit_adios2_root})

set(ADIOS2_DEPENDS)

list(APPEND ADIOS2_CMAKE_OPTIONS
     -DADIOS2_USE_Blosc2:STRING=ON
     -DBlosc2_DIR:PATH=${visit_blosc2_root}
     -DBLOSC2_INCLUDE_DIR:PATH=${visit_blosc2_include}
     -DBLOSC2_LIBRARY:FILEPATH=${visit_blosc2_library})

if(NOT BLOSC2_PREBUILT)
    list(APPEND ADIOS2_DEPENDS BLOSC2)
endif()

list(APPEND ADIOS2_CMAKE_OPTIONS
     -DZLIB_LIBRARY_RELEASE:FILEPATH=${visit_zlib_library}
     -DZLIB_INCLUDE_DIR:PATH=${visit_zlib_include})
if(NOT ZLIB_PREBUILT)
    list(APPEND ADIOS2_DEPENDS ZLIB)
endif()

list(APPEND ADIOS2_CMAKE_OPTIONS
      -DADIOS2_USE_HDF5:STRING=ON
      -DHDF5_DIR:PATH=${visit_install_root}/hdf5/${HDF5_VERSION}/cmake)
if(NOT HDF5_PREBUILT)
    list(APPEND ADIOS2_DEPENDS HDF5)
endif()

set(ADIOS2_URL ${visit_tp_url}/${ADIOS2_ZIP_NAME})
set(ADIOS2_URL_HASH SHA256=8b72142bd5aabfb80c7963f524df11b8721c09ef20caea6df5fb00c31a7747c0)

ExternalProject_Add(ADIOS2
    PREFIX           ADIOS2
    URL              ${ADIOS2_URL}
    URL_HASH         ${ADIOS2_URLHASH}
    INSTALL_DIR      ${visit_adios2_root}
    CMAKE_CACHE_ARGS ${ADIOS2_CMAKE_OPTIONS} -DADIOS2_USE_MPI:STRING=ON
    DEPENDS          ${ADIOS2_DEPENDS}
    ${LOGGING_OPTIONS}
    PATCH_COMMAND    ${CMAKE_COMMAND} -DADIOS2_VERSION:STRING=${ADIOS2_VERSION}
                                      -DPATCH_CMD:PATH=${visit_patch_command}
                                      -DPATCH_DIR:PATH=${visit_tp_patches}
                                      -P ${visit_tp_patches}/adios2-patch.cmake)


