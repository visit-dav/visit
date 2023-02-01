# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Tues Oct 1 09:33:47 MST 2013
#   Removed logic handling windows differently than other platforms.
#
#   Kathleen Biagas, Thu July 15, 2021
#   Add NETCDF_WIN32_DEFINES (used by xml2cmake to add preprocessor defines).
#
#****************************************************************************/

# Use the H5PART_DIR hint from the config-site .cmake file

SET_UP_THIRD_PARTY(NETCDF LIBS netcdf)

if (WIN32)
    if (MSVC_VERSION LESS "1910")
      set(NETCDF_CXX_DIR ${NETCDF_DIR})
      SET_UP_THIRD_PARTY(NETCDF_CXX LIBS netcdf_c++)
    endif()
    set(NETCDF_WIN32_DEFINES "DLL_NETCDF")
else()
    set(NETCDF_CXX_DIR ${NETCDF_DIR})
    SET_UP_THIRD_PARTY(NETCDF_CXX LIBS netcdf_c++)
endif()

