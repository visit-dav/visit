# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

project(JPEG) 

# version number like this isn't accepted by cmake, so set it separately
# from the 'project' command.
set(JPEG_VERSION 9e)

# set global variables used by other libraries
set(visit_jpeg_root        ${visit_install_root}/jpeg/${JPEG_VERSION})
set(visit_jpeg_include     ${visit_jpeg_root}/include)
set(visit_jpeg_library_dir ${visit_jpeg_root}/lib)
set(visit_jpeg_library     ${visit_jpeg_library_dir}/jpeg.lib)

# has this already been built
if(EXISTS ${visit_jpeg_include}/jpeglib.h AND
   EXISTS ${visit_jpeg_library})
    if(BUILD_ALL OR ENABLE_JPEG)
        message(STATUS "JPEG already installed.")
    endif()
    set(JPEG_PREBUILT TRUE)
    return()
endif()

# if not requsted, return
if(NOT (BUILD_ALL OR ENABLE_JPEG))
    return()
endif()

message(STATUS "JPEG being added to the build")

set(JPEG_ZIP_NAME "jpegsrc.v${JPEG_VERSION}.tar.gz")

set(JPEG_CMAKE_OPTIONS
    -DCMAKE_INSTALL_PREFIX:PATH=${visit_jpeg_root})

ExternalProject_Add(JPEG
    PREFIX            JPEG
    URL               http://ijg.org/files/${JPEG_ZIP_NAME}
    URL_HASH          SHA256=4077d6a6a75aeb01884f708919d25934c93305e49f7e3f36db9129320e6f4f3d
    INSTALL_DIR       ${visit_jpeg_root}
    CONFIGURE_COMMAND nmake -f makefile.vs setup-v17
    BUILD_IN_SOURCE   TRUE
    BUILD_COMMAND     msbuild jpeg.sln /p:Configuration=Release /p:Platform=x64
    INSTALL_COMMAND   ${CMAKE_COMMAND} -E make_directory ${visit_jpeg_include}
    COMMAND           ${CMAKE_COMMAND} -E make_directory ${visit_jpeg_library_dir}
    COMMAND           ${CMAKE_COMMAND} -E copy jconfig.h jerror.h jmorecfg.h jpeglib.h  ${visit_jpeg_include}
    COMMAND           ${CMAKE_COMMAND} -E copy Release/x64/jpeg.lib ${visit_jpeg_library_dir}
    ${LOGGING_OPTIONS})

