# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Bonnell, Fri Dec 10 14:36:52 PST 2010
#   Set ZLIB_LIB to full path.
#
#   Brad Whitlock, Thu Apr 18 14:38:02 PDT 2013
#   Look for zlib on other platforms if VISIT_ZLIB_DIR is set.
#
#   Kathleen Biagas, Wed Jun  5 16:47:21 PDT 2013
#   Always set VTKZLIB_LIB on windows if the vtkzlib target exists.
#
#   Kathleen Biagas, Fri Aug  9 11:01:29 PDT 2013
#   IF this file finds zlib, set HAVE_ZLIB_H. CHECK_INCLUDE_FILES might not
#   find it, especially on windows.
#
#   Kathleen Biagas, Tues Oct 1 09:33:47 MST 2013
#   Removed VISIT_MSVC_VERSION from windows handling.
#
#   Kathleen Biagas, Fri Mar 17 10:29:19 PDT 2017
#   Set HAVE_LIBZ when ZLIB_FOUND.
#
#   Kathleen Biagas, Tue May 28 09:08:56 PDT 2019
#   Since we require zlib from build_visit, no longer need HAVE_ZLIB_H
#
#   Kathleen Biagas, Fri May 31, 2024
#   Modified to create an INTERFACE target via blt_import_library.
#   This allows zlib to be a native CMake target, and will also be accounted
#   for in the generated export set.
#
#****************************************************************************/

# Use the ZLIB_DIR hint from the config-site .cmake file

if(ZLIB_DIR)

    find_path(_zlib_INCLUDE_DIR zlib.h
              PATHS ${ZLIB_DIR}
              PATH_SUFFIXES include
              NO_DEFAULT_PATH)

    find_library(_zlib_LIBRARY
             NAMES zlib zlib1 z
             PATHS ${ZLIB_DIR}
             PATH_SUFFIXES lib lib64
             NO_DEFAULT_PATH)

    find_package_handle_standard_args(ZLIB DEFAULT_MSG
        _zlib_INCLUDE_DIR
        _zlib_LIBRARY)

    if(ZLIB_FOUND)
        get_filename_component(libz ${_zlib_LIBRARY} NAME)
        set(_zlib_DLL)

        ####
        # until everything is updated to use the new target
        get_filename_component(ZLIB_LIBRARY_DIR ${_zlib_LIBRARY} PATH)
        set(ZLIB_LIBRARY ${_zlib_LIBRARY})
        set(ZLIB_LIB ${libz})
        set(ZLIB_INCLUDE_DIR ${_zlib_INCLUDE_DIR})
        ####


        blt_import_library(
            NAME        zlib
            INCLUDES    $<BUILD_INTERFACE:${_zlib_INCLUDE_DIR}>
                        $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/zlib/include>
            LIBRARIES   $<BUILD_INTERFACE:${_zlib_LIBRARY}>
            EXPORTABLE  ON)


        # CMake doesn't prepend ${_IMPORT_PREFIX} in the generated export
        # set for INTERFACE libs, so "${IMPORT_PREFIX}" needs to be
        # explicitly added the the INSTALL_INTERFACE, and escaped so it
        # doesn't get evaluated.
        #
        # Also, it seems if the INSTALL_INTERFACE is used in the
        # blt_import_library,  the ${_IMPORT_PREFIX} is stripped, even
        # if it is escaped, so it appears to be getting evaluated.
        # That's why it is added separately here instead of being added
        # to the LIBRARIES above.
        target_link_libraries(zlib INTERFACE
            $<INSTALL_INTERFACE:\${_IMPORT_PREFIX}/${VISIT_INSTALLED_VERSION_LIB}/${libz}>)

        # install and export
        if(VISIT_INSTALL_THIRD_PARTY)
            visit_install_export_targets(zlib)

            # headers aren't being installed. Perhaps because it is INTERFACE lib?
            # There is a PUBLIC_HEADER property for INTERFACE libraries
            # that will allow the installation of the headers in a normal
            # install(TARGETS) command (like the one being used by
            # visit_install_export_targets).
            # However, you cannot specifiy a directory as PUBLIC_HEADER
            # property, but must instead list them all, so using this
            # property would require a file(glob). Do we want to do that?
            # I think it is easier to install the directory the way we do
            # for TP libs in the following function:

            THIRD_PARTY_INSTALL_INCLUDE(zlib ${_zlib_INCLUDE_DIR})
        endif()
    else()
        message(FATAL_ERROR "VisIt requires lib z and it could not be found. Tried ZLIB_DIR: ${ZLIB_DIR}")
    endif()
else()
    message(FATAL_ERROR "VisIt requires lib z and it could not be found. Please set ZLIB_DIR")
endif()

