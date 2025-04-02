# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Bonnell, Wed Dec  9 15:12:38 MT 2009
#   Made Message of type 'status', otherwise it appears as an error when 
#   using the CMake gui.
#
#   Kathleen Biagas, Thu Mar 21, 2025
#   Remove AIX.
#
#****************************************************************************/

#
# sets up extended platform defines: LINUX
#

SET(VISIT_CMAKE_PLATFORM ${CMAKE_HOST_SYSTEM_NAME})
MESSAGE(STATUS "CMake Platform: ${VISIT_CMAKE_PLATFORM}")

IF (${VISIT_CMAKE_PLATFORM} MATCHES "Linux")
   SET(LINUX 1)
ENDIF (${VISIT_CMAKE_PLATFORM} MATCHES "Linux")





