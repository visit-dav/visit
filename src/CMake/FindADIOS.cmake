# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#   Kathleen Biagas, Tues Oct 1 09:33:47 MST 2013
#   Removed logic handling windows differently than other platforms.
#
#   Kathleen Biagas, Thu Sep 27 11:28:34 PDT 2018
#   Added adiosread_nompi so it will get installed when necessary.
#
#****************************************************************************/

# Use the ADIOS_DIR hint from the config-site .cmake file 

INCLUDE(${VISIT_SOURCE_DIR}/CMake/SetUpThirdParty.cmake)

SET_UP_THIRD_PARTY(ADIOS lib include adiosread adiosread_nompi)


