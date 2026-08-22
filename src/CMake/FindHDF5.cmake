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
#   Kathleen Biagas, Mon Aug 17, 2026
#   Use new visit_install_thirdparty_targets function to properly install
#   hdf5 targets without guessing configuration type.
#
#   Kathleen Biagas, Thur Aug 20, 2026
#   Add call to generate_lib_setup_cmake.
#
#****************************************************************************/

# Uses the HDF5_DIR hint from the config-site .cmake file


if(EXISTS ${VISIT_HDF5_DIR}/cmake)
  set(HDF5_DIR ${VISIT_HDF5_DIR}/cmake)
endif()

find_package(HDF5 CONFIG PATHS ${HDF5_DIR} NO_DEFAULT_PATH)

if(TARGET hdf5-shared)
    # We use CMake vars for HDF5 pointing to the actual target.
    # Should ease maintenance if actual library name changes.
    # For example: hdf5-shared vs hdf5-static.
    set(HDF5_LIB hdf5-shared)
    set(HAVE_HDF5 TRUE CACHE BOOL "Have HDF5 libraries")
    set(hdf5_targets hdf5-shared)

    if(TARGET hdf5_hl-shared)
        set(HDF5_HL_LIB hdf5_hl-shared)
        set(HAVE_HDF5_HL TRUE CACHE BOOL "Have HDF5 HL libraries")
        list(APPEND hdf5_targets hdf5_hl-shared)
    endif()

    visit_install_thirdparty_targets(NAME hdf5 TARGETS ${hdf5_targets})
    THIRD_PARTY_INSTALL_INCLUDE(hdf5 ${HDF5_INCLUDE_DIR})

    # for plugin vs install
    # write SetupHDF5.cmake for our export sets.
    include(${VISIT_SOURCE_DIR}/CMake/WriteThirdPartySetup.cmake)
    create_lib_setup_cmake(NAME "HDF5"
                           NAMESPACE "hdf5"
                           INCBASE "hdf5"
                           ITEMS ${HDF5_LIB} ${HDF5_HL_LIB}
                           SIMPLE_INCLUDE true)

    # Add the needed CMake vars to the setup file.
    get_lib_setup_cmake_input_file(fname "HDF5")
    file(APPEND ${fname} "\nset(HDF5_LIB ${HDF5_LIB})\n")
    file(APPEND ${fname} "\nset(HDF5_HL_LIB ${HDF5_HL_LIB})\n")
    generate_lib_setup_cmake(NAME "HDF5")
endif()

