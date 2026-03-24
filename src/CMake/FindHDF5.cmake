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
#   Kathleen Biagas, Mon Mar 3, 2026
#   On *nix, use the library specified by IMPORTED_SONAME_RELEASE for
#   installation so that all the proper symlinks will be installed
#   alongside VisIt.
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
    if(WIN32)
        get_target_property(hdf5_locr hdf5-shared IMPORTED_IMPLIB_RELEASE)
        THIRD_PARTY_INSTALL_LIBRARY(${hdf5_locr})
    else()
        get_target_property(hdf5_locr hdf5-shared IMPORTED_LOCATION_RELEASE)
        get_target_property(hdf5_soname hdf5-shared IMPORTED_SONAME_RELEASE)
        cmake_path(GET hdf5_locr PARENT_PATH hdf5_libdir)
        THIRD_PARTY_INSTALL_LIBRARY(${hdf5_libdir}/${hdf5_soname})
    endif()

    if(TARGET hdf5_hl-shared)
        set(HDF5_HL_LIB hdf5_hl-shared)
        set(HAVE_LIBHDF5_HL TRUE CACHE BOOL "Have HDF5 HL libraries")
        if(WIN32)
            get_target_property(hdf5_hl_locr hdf5_hl-shared IMPORTED_IMPLIB_RELEASE)
            THIRD_PARTY_INSTALL_LIBRARY(${hdf5_hl_locr})
        else()
            get_target_property(hdf5_hl_locr hdf5_hl-shared IMPORTED_LOCATION_RELEASE)
            get_target_property(hdf5_hl_soname hdf5_hl-shared IMPORTED_SONAME_RELEASE)
            cmake_path(GET hdf5_hl_locr PARENT_PATH hdf5_hl_libdir)
            THIRD_PARTY_INSTALL_LIBRARY(${hdf5_hl_libdir}/${hdf5_hl_soname})
        endif()
    endif()

    THIRD_PARTY_INSTALL_INCLUDE(hdf5 ${HDF5_INCLUDE_DIR})

    # for plugin vs install
    # write SetupHDF5.cmake for our export sets.
    include(${VISIT_SOURCE_DIR}/CMake/WriteThirdPartySetup.cmake)
    create_lib_setup_cmake(KIT "HDF5"
                           NAMESPACE "hdf5"
                           INCBASE "hdf5/include"
                           ITEMS ${HDF5_LIB} ${HDF5_HL_LIB}
                           SIMPLE_INCLUDE true)

    # need a few extras in the Setup file.
    set(fname ${VISIT_BINARY_DIR}/SetupHDF5.cmake)
    file(APPEND ${fname} "\nset(HDF5_LIB ${HDF5_LIB})\n")
    file(APPEND ${fname} "\nset(HDF5_HL_LIB ${HDF5_HL_LIB})\n")
endif()

