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
#   Kathleen Biagas, Mon Mar 9, 2026
#   Use find_package.
#
#****************************************************************************/

# Use the ZLIB_DIR hint from the config-site .cmake file

if(VISIT_ZLIB_DIR)
    set(ZLIB_ROOT ${VISIT_ZLIB_DIR})
endif()

find_package(ZLIB)

if(ZLIB_FOUND)
    set(HAVE_LIBZ true CACHE BOOL "Have lib z")

    if(TARGET ZLIB::ZLIB)
        # for VisIt libraries' needs
        set(ZLIB_LIBRARY ZLIB::ZLIB)

        # Install 
        get_target_property(zlib_loc ZLIB::ZLIB IMPORTED_LOCATION_RELEASE)
        get_target_property(zlib_inc ZLIB::ZLIB INTERFACE_INCLUDE_DIRECTORIES)
        THIRD_PARTY_INSTALL_LIBRARY(${zlib_loc})
        THIRD_PARTY_INSTALL_INCLUDE(zlib ${zlib_inc})
    endif()

else()
    message(FATAL_ERROR "VisIt requires lib z and it could not be found.")
endif()

