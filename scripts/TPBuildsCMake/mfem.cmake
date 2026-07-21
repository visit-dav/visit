# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

project(MFEM VERSION 4.8)

set(visit_mfem_root ${visit_install_root}/mfem/${MFEM_VERSION})
set(visit_mfem_include ${visit_mfem_root}/include)
set(visit_mfem_library ${visit_mfem_root}/lib/mfem.lib)

# has this already been built
if(EXISTS ${visit_mfem_include}/mfem.hpp AND
   EXISTS ${visit_mfem_library})
    if(BUILD_ALL OR ENABLE_MFEM)
        message(STATUS "MFEM already installed.")
    endif()
    set(MFEM_PREBUILT TRUE)
    return()
endif()

# if not requested, return
if(NOT (BUILD_ALL OR ENABLE_MFEM))
    return()
endif()

message(STATUS "MFEM being added to the build")

set(MFEM_ZIP_NAME "mfem-${MFEM_VERSION}.tar.gz")

set(MFEM_CMAKE_OPTIONS
    -DBUILD_SHARED_LIBS:BOOL=${visit_build_shared}
    -DCMAKE_INSTALL_PREFIX:PATH=${visit_mfem_root}
    -DMFEM_ENABLE_TESTING:BOOL=OFF
    -DMFEM_USE_EXCEPTIONS:BOOL=ON
    -DMFEM_USE_FMS:BOOL=OFF
    -DMFEM_USE_ZLIB:BOOL=ON
    -DZLIB_LIBRARY:FILEPATH=${visit_zlib_library}
    -DZLIB_INCLUDE_DIR:FILEPATH=${visit_zlib_include}
    -DHDF5_DIR:PATH=${visit_hdf5_root}
    -DMFEM_USE_CONDUIT:BOOL=ON
    -DCONDUIT_DIR:PATH=${visit_conduit_root})

set(MFEM_DEPENDS)

if(NOT ZLIB_PREBUILT)
    list(APPEND MFEM_DEPENDS ZLIB)
endif()

if(NOT HDF5_PREBUILT)
    list(APPEND MFEM_DEPENDS HDF5)
endif()

if(NOT CONDUIT_PREBUILT)
    list(APPEND MFEM_DEPENDS CONDUIT)
endif()

ExternalProject_Add(MFEM
    PREFIX           MFEM
    URL              ${visit_tp_url}/${MFEM_ZIP_NAME}
    URL_HASH         SHA256=65472f732d273832c64b2c39460649dd862df674222c71bfa82cf2da76705052
    INSTALL_DIR      ${visit_mfem_root}
    CMAKE_CACHE_ARGS ${MFEM_CMAKE_OPTIONS}
    DEPENDS          ${MFEM_DEPENDS}
    ${LOGGING_OPTIONS})

