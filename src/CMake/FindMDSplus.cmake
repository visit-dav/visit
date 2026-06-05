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

if(WIN32)
    visit_import_third_party(MDSPLUS LIBS MdsLib)
else()
    if("${VISIT_CMAKE_PLATFORM}" STREQUAL "Linux")
        # Linux requires librt to resolve "clock_gettime"
        # add this as a general dep:
        set(MDSPLUS_LIBDEP /usr/lib/rt "${MDSPLUS_LIBDEP}")
    endif()
    visit_import_third_party(MDSPLUS LIBS MdsLib_client MdsIpShr MdsShr)
endif()

