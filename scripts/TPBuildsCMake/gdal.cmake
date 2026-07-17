# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

project(GDAL VERSION 2.2.4)

set(visit_gdal_root    ${visit_install_root}/gdal/${GDAL_VERSION})
set(visit_gdal_include ${visit_gdal_root}/include)
set(visit_gdal_library ${visit_gdal_root}/lib/gdal_i.lib)

# has this already been built
if(EXISTS ${visit_gdal_include}/gdal.h AND
   EXISTS ${visit_gdal_library})
    if(BUILD_ALL OR ENABLE_GDAL)
       message(STATUS "GDAL already installed.")
    endif()
    set(GDAL_PREBUILT TRUE)
    return()
endif()

# if not requested, return
if(NOT (BUILD_ALL OR ENABLE_GDAL))
    return()
endif()

message(STATUS "GDAL being added to the build")

set(GDAL_ZIP_NAME "gdal-${GDAL_VERSION}.tar.gz")

set(GDAL_CMAKE_OPTIONS
    -DCMAKE_INSTALL_PREFIX:PATH=${visit_gdal_root})

ExternalProject_Add(GDAL
    PREFIX           GDAL
    URL              ${visit_tp_url}/${GDAL_ZIP_NAME}
    URL_HASH         SHA256=b9d5a723787f3006a82cb276db171c721187b048b866c0e20e6df464d671a1a4
    BUILD_IN_SOURCE  TRUE
    CMAKE_CACHE_ARGS ${GDAL_CMAKE_OPTIONS}
    ${LOGGING_OPTIONS}
    PATCH_COMMAND    ${CMAKE_COMMAND} -DGDAL_VERSION:STRING=${GDAL_VERSION}
                                      -DPATCH_DIR:PATH=${visit_tp_patches}
                                      -DPATCH_CMD:PATH=${visit_patch_command}
                                      -P ${visit_tp_patches}/gdal-patch.cmake)

