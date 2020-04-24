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
#****************************************************************************/

# Use the JPEG_DIR hint from the config-site .cmake file

IF (WIN32)
  INCLUDE(${VISIT_SOURCE_DIR}/CMake/SetUpThirdParty.cmake)

  if (JPEG_LIBNAME_PREFIX_LIB)
      SET_UP_THIRD_PARTY(JPEG lib include libjpeg)
  else() 
      SET_UP_THIRD_PARTY(JPEG lib include jpeg)
  endif()
  # VTK's Find needs this var set.
  set(JPEG_LIBRARY ${JPEG_LIBRARY_DIR}/${JPEG_LIB})
ENDIF (WIN32)

