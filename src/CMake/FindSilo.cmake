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
#****************************************************************************/

# Use the SILO_DIR hint from the config-site .cmake file
#

IF (WIN32)
  if(EXISTS ${SILO_DIR}/lib/silohdf5.lib)
      SET_UP_THIRD_PARTY(SILO LIBS silohdf5)
  else() 
      SET_UP_THIRD_PARTY(SILO LIBS siloh5)
  endif()
  IF(EXISTS ${SILO_DIR}/lib/silex.exe)
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy
         ${SILO_DIR}/lib/silex.exe
         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ThirdParty)
    INSTALL(FILES ${SILO_DIR}/lib/silex.exe
        DESTINATION ${VISIT_INSTALLED_VERSION_BIN}
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ GROUP_WRITE GROUP_EXECUTE
                    WORLD_READ WORLD_EXECUTE
        CONFIGURATIONS "" None Debug Release RelWithDebInfo MinSizeRel
        )
  ENDIF()
  IF(EXISTS ${SILO_DIR}/lib/browser.exe)
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy
         ${SILO_DIR}/lib/browser.exe
         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ThirdParty)
    INSTALL(FILES ${SILO_DIR}/lib/browser.exe
        DESTINATION ${VISIT_INSTALLED_VERSION_BIN}
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ GROUP_WRITE GROUP_EXECUTE
                    WORLD_READ WORLD_EXECUTE
        CONFIGURATIONS "" None Debug Release RelWithDebInfo MinSizeRel
        )
  ENDIF()
ELSE (WIN32)
    FIND_LIBRARY(SILOH5_LIBRARY_EXISTS
      NAME siloh5
      PATHS ${SILO_DIR}/lib
      NO_DEFAULT_PATH
      NO_CMAKE_ENVIRONMENT_PATH
      NO_CMAKE_PATH
      NO_SYSTEM_ENVIRONMENT_PATH)
    IF(SILOH5_LIBRARY_EXISTS)
        SET_UP_THIRD_PARTY(SILO LIBS siloh5)
    ELSE()
        SET_UP_THIRD_PARTY(SILO LIBS silo)
    ENDIF()
ENDIF (WIN32)

# We use Silo for PDB most of the time so set up additional PDB variables.
IF(SILO_FOUND)

    MESSAGE(STATUS "    Using PDB Lite built into Silo")
    SET(PDB_FOUND 1 CACHE BOOL "PDB library found" FORCE)
    SET(PDB_INCLUDE_DIR ${SILO_INCLUDE_DIR} CACHE PATH "PDB include directory" FORCE)
    SET(PDB_LIBRARY_DIR ${SILO_LIBRARY_DIR} CACHE PATH "PDB library directory" FORCE)
    SET(PDB_LIB ${SILO_LIB} CACHE STRING "PDB library" FORCE)
    MARK_AS_ADVANCED(PDB_INCLUDE_DIR PDB_LIBRARY_DIR PDB_LIB)

ENDIF(SILO_FOUND)

