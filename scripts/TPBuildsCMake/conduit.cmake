# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

project(CONDUIT VERSION 0.9.4)

set(visit_conduit_root ${visit_install_root}/conduit/${CONDUIT_VERSION})
set(visit_conduit_include_dir ${visit_conduit_root}/include/conduit)
set(visit_conduit_library_dir ${visit_conduit_root}/lib)

# has this already been built
if(EXISTS ${visit_conduit_include_dir}/conduit.h AND
   EXISTS ${visit_conduit_library_dir}/conduit.lib)
    if(BUILD_ALL OR ENABLE_CONDUIT)
        message(STATUS "CONDUIT already installed.")
    endif()
    set(CONDUIT_PREBUILT TRUE)
    return()
endif()

# if not requested, return
if(NOT (BUILD_ALL OR ENABLE_CONDUIT))
    return()
endif()

message(STATUS "CONDUIT being added to the build")

set(CONDUIT_ZIP_NAME "conduit-v${CONDUIT_VERSION}-src-with-blt.tar.gz")


set(CONDUIT_CMAKE_OPTIONS
    -DBUILD_SHARED_LIBS:BOOL=${visit_build_shared}
    -DBUILD_GTEST:BOOL=OFF
    -DBUILD_TESTING:BOOL=OFF
    -DCONDUIT_ENABLE_TESTS:BOOL=OFF
    -DCONDUIT_ENABLE_EXAMPLES:BOOL=OFF
    -DCONDUIT_INSTALL_PREFIX:PATH=${visit_conduit_root}
    -DCMAKE_INSTALL_PREFIX:PATH=${visit_conduit_root}
    -DENABLE_MPI:BOOL=ON
    -DENABLE_FIND_MPI:BOOL=ON
    -DENABLE_ASTYLE:BOOL=OFF
    -DENABLE_CLANGFORMAT:BOOL=OFF
    -DENABLE_CLANGQUERY:BOOL=OFF
    -DENABLE_CLANGTIDY:BOOL=OFF
    -DENABLE_CMAKEFORMAT:BOOL=OFF
    -DENABLE_CPPCHECK:BOOL=OFF
    -DENABLE_DOCS:BOOL=OFF
    -DENABLE_DOXYGEN:BOOL=OFF
    -DENABLE_EXAMPLES:BOOL=OFF
    -DENABLE_FRUIT:BOOL=OFF
    -DENABLE_GIT:BOOL=OFF
    -DENABLE_GTEST:BOOL=OFF
    -DENABLE_RELAY_WEBSERVER:BOOL=OFF
    -DENABLE_SPHINX:BOOL=OFF
    -DENABLE_TESTS:BOOL=OFF
    -DENABLE_UNCRUSTIFY:BOOL=OFF
    -DENABLE_UTILS:BOOL=OFF
    -DENABLE_VALGRIND:BOOL=OFF
    -DENABLE_YAPF:BOOL=OFF
    -DZLIB_DIR:PATH=${visit_zlib_dir}
    -DHDF5_DIR:PATH=${visit_hdf5_root}
    -DSILO_DIR:PATH=${visit_silo_root}
    -DENABLE_PYTHON:BOOL=ON
    -DPYTHON_EXECUTABLE:FILEPATH=${visit_python_executable})

set(CONDUIT_DEPENDS)

if(NOT ZLIB_PREBUILT)
    list(APPEND CONDUIT_DEPENDS ZLIB)
endif()

if(NOT HDF5_PREBUILT)
    list(APPEND CONDUIT_DEPENDS HDF5)
endif()

if(NOT SILO_PREBUILT)
    list(APPEND CONDUIT_DEPENDS SILO)
endif()

if(NOT PYTHON_PREBUILT)
    list(APPEND CONDUIT_DEPENDS PYTHON)
endif()

ExternalProject_Add(CONDUIT
    PREFIX           CONDUIT
    URL              ${visit_tp_url}/${CONDUIT_ZIP_NAME}
    URL_HASH         SHA256=c9edfb2ff09890084313ad9c2d83bfb7c10e70b696980762d1ae1488f9f08e6c
    INSTALL_DIR      ${visit_conduit_root}
    CMAKE_CACHE_ARGS ${CONDUIT_CMAKE_OPTIONS}
    SOURCE_SUBDIR    src
    DEPENDS          ${CONDUIT_DEPENDS}
    ${LOGGING_OPTIONS})

