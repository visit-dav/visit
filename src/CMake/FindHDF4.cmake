# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Wed Oct 19 09:58:16 MST 2011
#   Add xdrdll.  Remove 64-bit specific include.
#
#   Kathleen Biagas, Thu Jan 23 15:20:43 MST 2014
#   Allow for lib names from newer HDF4 versions (no 'dll' in name).
#
#   Kathleen Biagas, Tue Mar 11 10:44:54 MST 2014
#   Change how newer versions are handled.
#
#****************************************************************************/

# Use the HDF4_DIR hint from the config-site .cmake file 

include(${VISIT_SOURCE_DIR}/CMake/SetUpThirdParty.cmake)

if(WIN32)
  if(HDF4_LIBNAMES_AFFIX_DLL)
      SET_UP_THIRD_PARTY(HDF4 lib include hdfdll mfhdfdll xdrdll)
  else()
      SET_UP_THIRD_PARTY(HDF4 lib include hdf mfhdf xdr)
  endif()
else()
  SET_UP_THIRD_PARTY(HDF4 lib include mfhdf hdf)
endif()
