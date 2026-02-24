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

if(TARGET hdf5-shared)
   set(HDF5_LIB hdf5-shared)
   set(HAVE_LIBHDF5 TRUE CACHE BOOL "Have HDF5 libraries")
   get_target_property(hdf5_locr hdf5-shared IMPORTED_LOCATION_RELEASE)
   THIRD_PARTY_INSTALL_INCLUDE(hdf5 ${HDF5_INCLUDE_DIR})
   THIRD_PARTY_INSTALL_LIBRARY(${hdf5_locr})

   if(TARGET hdf5_hl-shared)
       set(HDF5_HL_LIB hdf5_hl-shared)
       set(HAVE_LIBHDF5_HL TRUE CACHE BOOL "Have HDF5 HL libraries")
       get_target_property(hdf5_hl_locr hdf5_hl-shared IMPORTED_LOCATION_RELEASE)
       THIRD_PARTY_INSTALL_LIBRARY(${hdf5_hl_locr})
   endif()

   if(HDF5_PROVIDES_ZLIB_SUPPORT)
      # hdf5 targets don't have 'zlib' listed as an interface-link-library
      # but it should be.
      set_property(TARGET hdf5-shared APPEND PROPERTY LINK_LIBRARIES "${ZLIB_LIBRARY}")
   endif()

endif()
