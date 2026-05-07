# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#    Kathleen Bonnell, Wed Dec  9 15:09:03 MT 2009
#    Make PDB_LIB a cached STRING, and PDB_FOUND a cached BOOL.
#
#    Kathleen Bonnell, Thu Apr  8 17:17:22 MST 2010
#    Add install and copy of silex for windows.
#
#    Mark C. Miller, Mon Jun 21 16:55:51 PDT 2010
#    Replaced logic to TRY_RUN a tiny PDB Lite test to simply query the
#    Silo version number.
#
#    Mark C. Miller, Tue Jul 20 10:09:44 PDT 2010
#    Fixed query for Silo version number to use MAJ/MIN/PAT symbols in
#    silo.h header file instead of Silo_version_... thingy. This allows
#    it to correctly interpret '4.8-pre3' for example.
#
#    Brad Whitlock, Wed Sep 22 14:15:23 PST 2010
#    I added FORCE to the lines that set the PDB variables.
#
#    Eric Brugger, Wed Feb  9 16:29:57 PST 2011
#    I modified the script so that it works with silo configured with and
#    without hdf5 support. It looks for silo configured with hdf5 support
#    first, then without it.
#
#    Mark C. Miller, Thu Jan 31 10:47:11 PST 2013
#    Removed logic to detect version of Silo and determine if PDB Lite has
#    long long support. VisIt no requires Silo 4.9 or newer and all newer
#    versions have long long support in PDB Lite.
#
#    Kathleen Biagas, Tues Oct 1 09:33:47 MST 2013
#    Removed VISIT_MSVC_VERSION from windows handling.
#
#    Kathleen Bonnell, Fri May 2 09:21:12 MST 2014
#    Install a copy of browser for windows.
#
#    Kathleen Biagas, Mon May 22 12:44:32 PDT 2023
#    Support different library names on Windows.
#
#    Kathleen Biagas, Tue Nov 25, 2025
#    silex/browser now stored in bin dir.
#
#    Kathleen Biagas, Mon Mar 3, 2026
#    On *nix, use the library specified by IMPORTED_SONAME_RELEASE for
#    installation so that all the proper symlinks will be installed
#    alongside VisIt.
#
#****************************************************************************/

# Use the SILO_DIR hint from the config-site .cmake file
#

if(EXISTS ${VISIT_SILO_DIR}/lib64/cmake/Silo)
    set(SILO_DIR ${VISIT_SILO_DIR}/lib64/cmake/Silo)
elseif(EXISTS ${VISIT_SILO_DIR}/lib/cmake/Silo)
    set(SILO_DIR ${VISIT_SILO_DIR}/lib/cmake/Silo)
endif()


find_package(Silo PATHS ${SILO_DIR} NO_DEFAULT_PATH)

if(TARGET silo)
    set(SILO_FOUND true)
    set(SILO_LIB silo)
    if(WIN32)
        get_target_property(silo_loc silo IMPORTED_IMPLIB_RELEASE)
    else()
        get_target_property(silo_loc silo IMPORTED_LOCATION_RELEASE)

    endif()

    # include dirs aren't attached to the library in the export set
    target_include_directories(silo INTERFACE ${SILO_INCLUDE_DIR})
    THIRD_PARTY_INSTALL_LIBRARY(${silo_loc})
    THIRD_PARTY_INSTALL_INCLUDE(silo ${SILO_INCLUDE_DIR})
endif()


# We use Silo for PDB most of the time so set up additional PDB variables.
if(SILO_FOUND)
    message(STATUS "    Using PDB Lite built into Silo")
    set(PDB_FOUND 1 CACHE BOOL "PDB library found" FORCE)
    set(PDB_LIB silo CACHE STRING "PDB library" FORCE)
    mark_as_advanced(PDB_LIB)

   # for plugin vs install:
   cmake_path(GET silo_loc FILENAME SILO_IMPORT_LIB)
endif()

