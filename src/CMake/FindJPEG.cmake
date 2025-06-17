# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Tues Oct 1 09:33:47 MST 2013
#   Removed VISIT_MSVC_VERSION.
#
#   Kathleen Biagas, Thu Jan 23 15:21:59 MST 2014
#   Allow for newer versions (no 'lib' in name).
#
#   Kathleen Biagas, Tue Mar 11 10:44:54 MST 2014
#   Change how newer versions are handled.
#
#   Kathleen Biagas, Thu Apr 10, 2025
#   Modified to create an INTERFACE target via blt_import_library.
#   This allows jpeg to be a native CMake target, and will also be accounted
#   for in the generated export set.
#
#****************************************************************************/

# Uses the JPEG_DIR hint from the config-site .cmake file

if(JPEG_DIR)

    find_path(_jpeg_INCLUDE_DIR jpeglib.h
              PATHS ${JPEG_DIR}
              PATH_SUFFIXES include
              NO_DEFAULT_PATH)

    find_library(_jpeg_LIBRARY
             NAMES jpeg libjpeg
             PATHS ${JPEG_DIR}
             PATH_SUFFIXES lib lib64
             NO_DEFAULT_PATH)

    find_package_handle_standard_args(JPEG DEFAULT_MSG
        _jpeg_INCLUDE_DIR
        _jpeg_LIBRARY)

    if(JPEG_FOUND)
        ####
        # VTK needs this var set in order to find our version of jpeg
        set(JPEG_LIBRARY ${_jpeg_LIBRARY})
        ####

        blt_import_library(
            NAME        jpeg
            INCLUDES    $<BUILD_INTERFACE:${_jpeg_INCLUDE_DIR}>
                        $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/jpeg/include>
            LIBRARIES   $<BUILD_INTERFACE:${_jpeg_LIBRARY}>
            EXPORTABLE  ON)

       # need just the library name for  INSTALL_INTERFACE
        get_filename_component(libjpeg ${_jpeg_LIBRARY} NAME)
        target_link_libraries(jpeg INTERFACE
            $<INSTALL_INTERFACE:\${_IMPORT_PREFIX}/${VISIT_INSTALLED_VERSION_LIB}/${libjpeg}>)

        # install and export
        if(VISIT_INSTALL_THIRD_PARTY)
            visit_install_export_targets(jpeg)
            THIRD_PARTY_INSTALL_LIBRARY(${_jpeg_LIBRARY})
            THIRD_PARTY_INSTALL_INCLUDE(jpeg ${_jpeg_INCLUDE_DIR})
        endif()

        if(WIN32)
            # need to copy the dll to the build dir
            cmake_path(SET libjpeg_path ${_jpeg_LIBRARY})
            cmake_path(REPLACE_EXTENSION libjpeg_path dll OUTPUT_VARIABLE _jpeg_DLL)
            if(NOT EXISTS ${_jpeg_DLL})
                cmake_path(GET _jpeg_DLL PARENT_PATH _JPEG_LIBRARY_DIR)
                cmake_path(GET _jpeg_DLL FILENAME _JPEG_DLL_NAME)
                cmake_path(SET _jpeg_DLL NORMALIZE ${_JPEG_LIBRARY_DIR}/../bin/${_JPEG_DLL_NAME})
            endif()

            if(EXISTS ${_jpeg_DLL})
                execute_process(COMMAND ${CMAKE_COMMAND} -E copy
                                ${_jpeg_DLL}
                                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ThirdParty)
            endif()
        endif()
    else()
        message(STATUS "Jpeg was requested and it could not be found. Tried JPEG_DIR: ${JPEG_DIR}")
    endif()
endif()

