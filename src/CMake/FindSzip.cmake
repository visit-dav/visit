# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Tues Oct 1 09:33:47 MST 2013
#   Removed VISIT_MSVC_VERSION from windows handling.
#
#****************************************************************************/

# Use the SZIP_DIR hint from the config-site .cmake file

IF (WIN32)
  IF(SZIP_LIB_NAME)
      SET_UP_THIRD_PARTY(SZIP LIBS ${SZIP_LIB_NAME})
  ELSE()
      SET_UP_THIRD_PARTY(SZIP LIBS szlibdll)
  ENDIF()
ENDIF (WIN32)

