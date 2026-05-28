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
    visit_import_third_party(JPEG LIBNAMES jpeg libjpeg ADD_GLOBAL_INCLUDE ADD_GLOBAL_LIBRARY)

    if(TARGET jpeg)
       add_library(JPEG::JPEG ALIAS jpeg)
    else()
        message(STATUS "Jpeg was requested and it could not be found. Tried JPEG_DIR: ${JPEG_DIR}")
    endif()
endif()

