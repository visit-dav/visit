#/*****************************************************
# **  PIDX Parallel I/O Library                      **
# **  Copyright (c) 2010-2014 University of Utah     **
# **  Scientific Computing and Imaging Institute     **
# **  72 S Central Campus Drive, Room 3750           **
# **  Salt Lake City, UT 84112                       **
# **                                                 **
# **  PIDX is licensed under the Creative Commons    **
# **  Attribution-NonCommercial-NoDerivatives 4.0    **
# **  International License. See LICENSE.md.         **
# **                                                 **
# **  For information about this project see:        **
# **  http://www.cedmav.com/pidx                     **
# **  or contact: pascucci@sci.utah.edu              **
# **  For support: PIDX-support@visus.net            **
# **                                                 **
# *****************************************************/


# This module finds if PIDX is installed, and sets the following variables
# indicating where it is:
#
# PIDX_FOUND               - system has PIDX
# PIDX_INCLUDE_DIR         - path to PIDX include directory
# PIDX_LIBRARIES           - all PIDX libraries
#
# Execute cmake with "-DPIDX_DIR=/path/to/pidx" to help find the library.
#

FIND_PATH(PIDX_INCLUDE_DIR NAMES PIDX.h PATHS ${PIDX_DIR}/include NO_DEFAULT_PATH)
FIND_PATH(HDF5_INCLUDE_DIR NAMES PIDX.h)

IF (PIDX_INCLUDE_DIR)
    
  SET(PIDX_INCLUDE_DIRS "${PIDX_INCLUDE_DIR}")

  FIND_LIBRARY(PIDX_LIBRARY     pidx    PATHS ${PIDX_DIR}/lib NO_DEFAULT_PATH)
  
  SET(PIDX_LIBRARIES 
    ${PIDX_LIBRARY}
  )

  SET(PIDX_FOUND   true CACHE BOOL "PIDX library found" FORCE)
  SET(HAVE_LIBPIDX true CACHE BOOL "Have PIDX library" FORCE)

  IF (CMAKE_VERBOSE_MAKEFILE)
    MESSAGE("Using PIDX_INCLUDE_DIR  = ") 
    FOREACH(inc ${PIDX_INCLUDE_DIR})
      MESSAGE("  " ${inc})
    ENDFOREACH()
    MESSAGE("Found PIDX_LIBRARIES    = ")
    FOREACH(lib ${PIDX_LIBRARIES})
      MESSAGE("  " ${lib})
    ENDFOREACH()
  ENDIF (CMAKE_VERBOSE_MAKEFILE)

ELSE ()
  IF (PIDX_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "PIDX library not found. Try setting PIDX_DIR")
  ELSE()
    MESSAGE(STATUS "PIDX library not found. Try setting PIDX_DIR")
  ENDIF()
ENDIF ()
                         
