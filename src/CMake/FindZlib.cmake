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
#****************************************************************************/

# Use the ZLIB_DIR hint from the config-site .cmake file

if (WIN32)
    if(ZLIB_LIB_NAME)
        SET_UP_THIRD_PARTY(ZLIB LIBS ${ZLIB_LIB_NAME})
    else()
        SET_UP_THIRD_PARTY(ZLIB LIBS zlib1)
    endif()
    if (ZLIB_FOUND)
        # use full path here, instead of just lib file.
        #set(ZLIB_LIBRARY "${ZLIB_LIB}" CACHE STRING "zlib library" FORCE)
        set(ZLIB_LIBRARY "${ZLIB_LIBRARY_DIR}/${ZLIB_LIB}" CACHE STRING "full path to zlib library" FORCE)
        message(STATUS "ZLIB_LIBRARY = ${ZLIB_LIBRARY}.")
    else ()
        message(WARNING "ZLIB_LIBRARY not found.  VISIT_ZLIB_DIR = ${VISIT_ZLIB_DIR}, ZLIB_LIB_NAME = ${ZLIB_LIB_NAME}.")
    endif ()
else (WIN32)
    # Have we told VisIt where to look for zlib?
    if (VISIT_ZLIB_DIR)
        SET_UP_THIRD_PARTY(ZLIB LIBS z)
        if (ZLIB_FOUND)
        # use full path here, instead of just lib file.
            set(ZLIB_LIBRARY "${ZLIB_LIBRARY_DIR}/${ZLIB_LIB}" CACHE STRING "zlib library" FORCE)
        endif ()
    else ()
      message(WARNING "VISIT_ZLIB_DIR not set.")
    endif ()
endif (WIN32)

if(ZLIB_FOUND)
    set(HAVE_LIBZ true CACHE BOOL "Have lib z")
else()
    message(FATAL_ERROR "VisIt requires lib z and it could not be found.")
endif()

