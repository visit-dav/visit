# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

project(OSPRAY VERSION 3.0.0)

# set global variables used by other libraries
set(visit_ospray_root     ${visit_install_root}/ospray/${OSPRAY_VERSION})
set(visit_ospray_config_dir ${visit_ospray_root}/lib/cmake/ospray-${OSPRAY_VERSION})
set(visit_ospray_include ${visit_ospray_root}/include/ospray)
set(visit_ospray_library ${visit_ospray_root}/lib/ospray.lib)

# has this already been built
if(EXISTS ${visit_ospray_include}/ospray.h AND
   EXISTS ${visit_ospray_library})
    if(BUILD_ALL OR ENABLE_OSPRAY)
        message(STATUS "OSPRAY already installed.")
    endif()
    set(OSPRAY_PREBUILT TRUE)
    return()
endif()

# if not requested, return
if(NOT (BUILD_ALL OR ENABLE_OSPRAY))
    return()
endif()

message(STATUS "OSPRAY being added to the build")

set(OSPRAY_CMAKE_OPTIONS
    -DBUILD_SHARED_LIBS:BOOL=${visit_build_shared}
    -DCMAKE_BUILD_TYPE:STRING=Release
    -DCMAKE_INSTALL_PREFIX:STRING=${visit_ospray_root}
    -DBUILD_OIDN:BOOL=OFF
    -DBUILD_OSPRAY_APPS:BOOL=OFF
    -DOSPRAY_ENABLE_APPS:BOOL=OFF
    -DOSPRAY_ENABLE_APPS_BENCHMARK:BOOL=OFF
    -DOSPRAY_ENABLE_APPS_EXAMPLES:BOOL=OFF
    -DOSPRAY_ENABLE_APPS_TESTING:BOOL=OFF
    -DOSPRAY_ENABLE_APPS_TUTORIALS:BOOL=OFF
    -DOSPRAY_ENABLE_TARGET_CLANGFORMAT:BOOL=OFF)

set(OSPRAY_ZIP_NAME "ospray-${OSPRAY_VERSION}.tar.gz")

ExternalProject_Add(OSPRAY
    PREFIX           OSPRAY
    URL              ${visit_tp_url}/${OSPRAY_ZIP_NAME}
    URL_HASH         SHA256=d8d8e632d77171c810c0f38f8d5c8387470ca19b75f5b80ad4d3d12007280288
    INSTALL_DIR      ${visit_ospray_root}
    SOURCE_SUBDIR    scripts/superbuild
    CMAKE_CACHE_ARGS ${OSPRAY_CMAKE_OPTIONS}
    ${LOGGING_OPTIONS}
    PATCH_COMMAND    ${CMAKE_COMMAND} -DOSPRAY_VERSION:STRING=${OSPRAY_VERSION}
                                      -DPATCH_CMD:PATH=${visit_patch_command}
                                      -DPATCH_DIR:PATH=${visit_tp_patches}
                                      -P ${visit_tp_patches}/ospray-patch.cmake)

