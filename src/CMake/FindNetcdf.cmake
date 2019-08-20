# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Tues Oct 1 09:33:47 MST 2013
#   Removed logic handling windows differently than other platforms.
#
#****************************************************************************/

# Use the H5PART_DIR hint from the config-site .cmake file 

INCLUDE(${VISIT_SOURCE_DIR}/CMake/SetUpThirdParty.cmake)

SET_UP_THIRD_PARTY(NETCDF lib include netcdf)

if (WIN32)
    if (MSVC_VERSION LESS "1910")
      set(NETCDF_CXX_DIR ${NETCDF_DIR})
      SET_UP_THIRD_PARTY(NETCDF_CXX lib include netcdf_c++)
    endif()
else()
    set(NETCDF_CXX_DIR ${NETCDF_DIR})
    SET_UP_THIRD_PARTY(NETCDF_CXX lib include netcdf_c++)
endif()

