# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Bonnell, Mon Dec 27, 17:52:39 MST 2010
#   Added high-level hdf5 lib to search on Windows. (hdf5_hldll).
#
#   Kathleen Biagas, Wed Oct 19 09:58:16 MST 2011
#   Remove ${VISIT_MSVC_VERSION} from lib location.
#
#   Kathleen Biagas, Thu Jan 9 18:47:21 PDT 2014
#   Add patch from John Cary for hdf5 without 'dll' suffix on name.
#
#   Kathleen Biagas, Wed July 31, 2024
#   Add hdf5_hl to search on Linux.
#
#****************************************************************************/

# Use the HDF5_DIR hint from the config-site .cmake file


if(EXISTS ${VISIT_HDF5_DIR}/cmake)
  set(HDF5_DIR ${VISIT_HDF5_DIR}/cmake)
endif()

find_package(HDF5 CONFIG PATHS ${HDF5_DIR} NO_DEFAULT_PATH)

get_cmake_property(_vars VARIABLES)
list(SORT _vars)
foreach(v IN LISTS _vars)
  if(v MATCHES "^(HDF5|hdf5)_")
    # Avoid noisy internal variables unless you want them
    message(STATUS "${v}='${${v}}'")
  endif()
endforeach()

if(TARGET hdf5-shared)
   set(HDF5_LIB hdf5-shared)
   get_target_property(hdf5_locr hdf5-shared IMPORTED_LOCATION_RELEASE)
   message(STATUS "hdf5_locr is ${hdf5_locr}")
   THIRD_PARTY_INSTALL_LIBRARY(${hdf5_locr})
   THIRD_PARTY_INSTALL_INCLUDE(hdf5 ${HDF5_INCLUDE_DIR})
   set(HDF5_LIBRARY_DIR ${hdf5_locr})
   message(STATUS "HDF5_LIBRARY_DIR is ${HDF5_LIBRARY_DIR}")
endif()

