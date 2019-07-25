# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Bonnell, Tue Apr 20 19:11:27 MST 2010
#   Change lib name on windows from cgnslib to cgnsdll.
#
#   Kathleen Biagas, Wed Oct 19 09:58:16 MST 2011
#   Remove ${VISIT_MSVC_VERSION} from lib location.
#
#****************************************************************************/

# Use the CGNS_DIR hint from the config-site .cmake file 

INCLUDE(${VISIT_SOURCE_DIR}/CMake/SetUpThirdParty.cmake)

if(WIN32 AND MSVC_VERSION LESS "1910")
  SET_UP_THIRD_PARTY(CGNS lib include cgnsdll)
else()
  SET_UP_THIRD_PARTY(CGNS lib include cgns)
endif()

