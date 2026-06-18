# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

project(ICET)

# version number like this isn't accepted by cmake, so set it separately
# from the 'project' command.
set(ICET_VERSION 77c708f9)

set(visit_icet_root     ${visit_install_root}/icet/${ICET_VERSION})
set(visit_icet_include  ${visit_icet_root}/include)
set(visit_icet_library  ${visit_icet_root}/lib/IceTCore.lib)

# has this already been built
if(EXISTS ${visit_icet_include}/IceT.h AND
   EXISTS ${visit_icet_library})
    if(BUILD_ALL OR ENABLE_ICET)
        message(STATUS "ICET already installed")
    endif()
    set(ICET_PREBUILT TRUE)
    return()
endif()

# if not requested, return
if(NOT (BUILD_ALL OR ENABLE_ICET))
    return()
endif()

message(STATUS "ICET being added to the build")

set(ICET_ZIP_NAME "icet-master-77c708f9090236b576669b74c53e9f105eedbd7e.tar.gz")

set(ICET_CMAKE_OPTIONS
    -DCMAKE_INSTALL_PREFIX:PATH=${visit_install_root}/icet/${ICET_VERSION}
    -DBUILD_SHARED_LIBS:BOOL=${visit_build_shared}
    -DBUILD_TESTING:BOOL=OFF
    -DICET_BUILD_TESTING:BOOL=OFF)

ExternalProject_Add( ICET
    PREFIX           ICET
    URL              ${visit_tp_url}/${ICET_ZIP_NAME}
    URL_HASH         SHA256=38ed9599b4815b376444223435905b66763912cb66749d90d377ef41d430ba77
    INSTALL_DIR      ${visit_install_root}/icet/${ICET_VERSION}
    CMAKE_CACHE_ARGS ${ICET_CMAKE_OPTIONS}
    ${LOGGING_OPTIONS})

