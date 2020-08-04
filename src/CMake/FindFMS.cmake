# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#
# Use the CONDUIT_DIR hint from the config-site .cmake file 
#

INCLUDE(${VISIT_SOURCE_DIR}/CMake/SetUpThirdParty.cmake)

IF(EXISTS ${VISIT_FMS_DIR}/lib64)
    SET_UP_THIRD_PARTY(FMS lib64 include fms)
ELSE()
    SET_UP_THIRD_PARTY(FMS lib include fms)
ENDIF()

