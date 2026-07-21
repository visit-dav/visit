# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

project(PYTHON VERSION 3.13.9)

# set global variables used by other libraries 
set(visit_python_dir        ${visit_install_root}/python/${PYTHON_VERSION})
set(visit_python_executable ${visit_python_dir}/python.exe)
set(visit_python_include    ${visit_python_dir}/include)
set(visit_python_library    ${visit_python_dir}/libs/python${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR}.lib)

# has this already been built
if(EXISTS ${visit_python_include}/python.h AND
   EXISTS ${visit_python_library})
    if(BUILD_ALL OR ENABLE_PYTHON)
        message(STATUS "PYTHON already installed.")
    endif()
    set(PYTHON_PREBUILT TRUE )
    return()
endif()

# if not requested, return
if(NOT (BUILD_ALL OR ENABLE_PYTHON))
    return()
endif()

message(STATUS "PYTHON being added to the build")

set(PYTHON_ZIP_NAME "Python-${PYTHON_VERSION}.tgz")

set(PYTHON_CMAKE_OPTIONS
    -DCMAKE_INSTALL_PREFIX:PATH=${visit_python_dir})

set(PYTHON_DEPENDS)
# for pillow: jpeg, zlib and tiff
if(NOT JPEG_PREBUILT)
    list(APPEND PYTHON_DEPENDS JPEG)
endif()

if(NOT ZLIB_PREBUILT)
    list(APPEND PYTHON_DEPENDS ZLIB)
endif()

if(NOT TIFF_PREBUILT)
    list(APPEND PYTHON_DEPENDS TIFF)
endif()

ExternalProject_Add(PYTHON
    PREFIX           PYTHON
    URL              ${visit_tp_url}/${PYTHON_ZIP_NAME}
    URL_HASH         SHA256=c4c066af19c98fb7835d473bebd7e23be84f6e9874d47db9e39a68ee5d0ce35c
    BUILD_IN_SOURCE  TRUE
    DEPENDS          ${PYTHON_DEPENDS}
    CMAKE_CACHE_ARGS ${PYTHON_CMAKE_OPTIONS}
    INSTALL_COMMAND  ${CMAKE_COMMAND} --install <SOURCE_DIR>
    COMMAND          ${CMAKE_COMMAND} -DPYCMD:FILEPATH=${visit_python_executable}
                                      -DURL:STRING=${visit_tp_url}
                                      -DJPEG_DIR:PATH=${visit_jpeg_root}
                                      -DZLIB_DIR:PATH=${visit_zlib_root}
                                      -DTIFF_DIR:PATH=${visit_tiff_root}
                                      -P <SOURCE_DIR>/install_python_packages.cmake
    ${LOGGING_OPTIONS}
    PATCH_COMMAND    ${CMAKE_COMMAND} -DPYTHON_VERSION:STRING=${PYTHON_VERSION}
                                      -DPATCH_DIR:PATH=${visit_tp_patches}
                                      -DPATCH_CMD:PATH=${visit_patch_command}
                                      -P ${visit_tp_patches}/python-patch.cmake)

