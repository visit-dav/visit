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
        get_filename_component(libjpeg ${_jpeg_LIBRARY} NAME)

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


        # CMake doesn't prepend ${_IMPORT_PREFIX} in the generated export
        # set for INTERFACE libs, so "${IMPORT_PREFIX}" needs to be
        # explicitly added to the INSTALL_INTERFACE, and escaped so it
        # doesn't get evaluated.
        #
        # Also, it seems if the INSTALL_INTERFACE is used in the
        # blt_import_library,  the ${_IMPORT_PREFIX} is stripped, even
        # if it is escaped, so it appears to be getting evaluated.
        # That's why it is added separately here instead of being added
        # to the LIBRARIES above.
        target_link_libraries(jpeg INTERFACE
            $<INSTALL_INTERFACE:\${_IMPORT_PREFIX}/${VISIT_INSTALLED_VERSION_LIB}/${libjpeg}>)

        # install and export
        if(VISIT_INSTALL_THIRD_PARTY)
            visit_install_export_targets(jpeg)
            THIRD_PARTY_INSTALL_INCLUDE(jpeg ${_jpeg_INCLUDE_DIR})
        endif()

        if(WIN32)
            # need to copy the dll to the build dir
            cmake_path(REPLACE_EXTENSION _libjpeg dll OUTPUT_VARIABLE _jpeg_DLL)
            if(EXISTS ${_jpeg_DLL})
                cmake_path(SET _jpeg_DLL ${JPEG_LIBRARY_DIR}/${_jpeg_DLL})
            else()    
                cmake_path(SET _jpeg_DLL NORMALIZE ${JPEG_LIBRARY_DIR}/../bin/${_jpeg_DLL})
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

