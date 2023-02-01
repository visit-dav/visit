# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Tues Oct 1 09:33:47 MST 2013
#   Removed VISIT_MSVC_VERSION from windows handling.
#
#****************************************************************************/

# Use the MDSPLUS_DIR hint from the config-site .cmake file

IF (WIN32)
    SET_UP_THIRD_PARTY(MDSPLUS LIBS MdsLib)
ELSE (WIN32)
    IF("${VISIT_CMAKE_PLATFORM}" STREQUAL "Linux")
        # Linux requires librt to resolve "clock_gettime"
        # add this as a general dep:
        SET(MDSPLUS_LIBDEP /usr/lib rt "${MDSPLUS_LIBDEP}")
    ENDIF("${VISIT_CMAKE_PLATFORM}" STREQUAL "Linux")
    SET_UP_THIRD_PARTY(MDSPLUS LIBS MdsLib_client MdsIpShr MdsShr)
ENDIF (WIN32)

