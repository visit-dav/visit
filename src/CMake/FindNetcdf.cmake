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
#   Kathleen Biagas, Mon Mar 31, 2025
#   Utilize visit_import_third_party.
#
#****************************************************************************/

# Uses the NETCDF_DIR hint from the config-site .cmake file

visit_import_third_party(NETCDF LIBS netcdf WIN32DEFINES DLL_NETCDF)

if(NOT WIN32)
    set(NETCDF_CXX_DIR ${NETCDF_DIR})
    visit_import_third_party(NETCDF_CXX LIBS netcdf_c++)
endif()

