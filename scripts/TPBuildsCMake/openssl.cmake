# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#[[ NOTES:
   Ensure perl.exe and nasm.exe are in your PATH env var.

   Currently only used for the build of Qt, so it will be built
   as a static library.
#]]

project(OPENSSL VERSION 3.5.5)

# set global variables used by other libraries
set(visit_openssl_root       ${visit_install_root}/openssl/${OPENSSL_VERSION})
set(visit_openssl_include    ${visit_openssl_root}/include/openssl)
set(visit_openssl_library    ${visit_openssl_root}/lib/libssl.lib)

# has this already been built
if(EXISTS ${visit_openssl_include}/ssl.h AND
   EXISTS ${visit_openssl_library})
    if(BUILD_ALL OR ENABLE_OPENSSL)
       message(STATUS "OPENSSL already installed.")
    endif()
    set(OPENSSL_PREBUILT TRUE )
    return()
endif()

# if not requested, return
if(NOT (BUILD_ALL OR ENABLE_OPENSSL))
    return()
endif()

message(STATUS "OPENSSL being added to the build")

set(OPENSSL_ZIP_NAME openssl-${OPENSSL_VERSION}.tar.gz)

ExternalProject_Add(OPENSSL
    PREFIX           OPENSSL
    URL              https://github.com/openssl/openssl/releases/download/openssl-${OPENSSL_VERSION}/${OPENSSL_ZIP_NAME}
    URL_HASH         SHA256=b28c91532a8b65a1f983b4c28b7488174e4a01008e29ce8e69bd789f28bc2a89
    INSTALL_DIR      ${visit_openssl_root}
    BUILD_IN_SOURCE   TRUE
    CONFIGURE_COMMAND perl.exe Configure no-makedepend no-shared
                      --prefix=${visit_openssl_root}
                      --openssldir=${visit_openssl_root} VC-WIN64A 
    BUILD_COMMAND      nmake
    INSTALL_COMMAND    nmake install
    ${LOGGING_OPTIONS})

