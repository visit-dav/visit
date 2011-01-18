#*****************************************************************************
#
# Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
# Produced at the Lawrence Livermore National Laboratory
# LLNL-CODE-400142
# All rights reserved.
#
# This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
# full copyright notice is contained in the file COPYRIGHT located at the root
# of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
#
# Redistribution  and  use  in  source  and  binary  forms,  with  or  without
# modification, are permitted provided that the following conditions are met:
#
#  - Redistributions of  source code must  retain the above  copyright notice,
#    this list of conditions and the disclaimer below.
#  - Redistributions in binary form must reproduce the above copyright notice,
#    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
#    documentation and/or other materials provided with the distribution.
#  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
#    be used to endorse or promote products derived from this software without
#    specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
# ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
# LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
# DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
# SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
# CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
# LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
# OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
# DAMAGE.
#
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
#****************************************************************************/

# Use the SILO_DIR hint from the config-site .cmake file 
#

INCLUDE(${VISIT_SOURCE_DIR}/CMake/SetUpThirdParty.cmake)

IF (WIN32)
  SET_UP_THIRD_PARTY(SILO lib/${VISIT_MSVC_VERSION} include silohdf5)
  IF(EXISTS ${SILO_DIR}/lib/${VISIT_MSVC_VERSION}/silex.exe)
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy
         ${SILO_DIR}/lib/${VISIT_MSVC_VERSION}/silex.exe
         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ThirdParty)
    INSTALL(FILES ${SILO_DIR}/lib/${VISIT_MSVC_VERSION}/silex.exe
        DESTINATION ${VISIT_INSTALLED_VERSION_BIN}
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        CONFIGURATIONS "";None;Debug;Release;RelWithDebInfo;MinSizeRel
        )
  ENDIF()
ELSE (WIN32)
    SET_UP_THIRD_PARTY(SILO lib include siloh5)
ENDIF (WIN32)

# We use Silo for PDB most of the time so set up additional PDB variables.
IF(SILO_FOUND)

    # Inspect Silo version number to infer whether or not PDB Lite (in Silo)
    # has support for long long type. Failure to either find silo.h header file
    # or find a matching line in that file with SILO_VERS_MAJ/MIN/PAT will result
    # in executing the block where VERSION_LESS is true. That is appropriate as
    # older versions of Silo do not have a PDB Lite with long long support.
    FILE(STRINGS ${SILO_INCLUDE_DIR}/silo.h SILO_VERS_LINE REGEX "#define[ \t]*SILO_VERS_MAJ")
    STRING(REGEX REPLACE "#define[ \t]*SILO_VERS_MAJ[ \t]*(0x0)?([0-9]*)" \\2 SILO_MAJ_NO "${SILO_VERS_LINE}")
    FILE(STRINGS ${SILO_INCLUDE_DIR}/silo.h SILO_VERS_LINE REGEX "#define[ \t]*SILO_VERS_MIN")
    STRING(REGEX REPLACE "#define[ \t]*SILO_VERS_MIN[ \t]*(0x0)?([0-9]*)" \\2 SILO_MIN_NO "${SILO_VERS_LINE}")
    FILE(STRINGS ${SILO_INCLUDE_DIR}/silo.h SILO_VERS_LINE REGEX "#define[ \t]*SILO_VERS_PAT")
    STRING(REGEX REPLACE "#define[ \t]*SILO_VERS_PAT[ \t]*(0x0)?([0-9]*)" \\2 SILO_PAT_NO "${SILO_VERS_LINE}")
    IF("${SILO_MAJ_NO}.${SILO_MIN_NO}.${SILO_PAT_NO}" VERSION_LESS 4.7.2)
        SET(PDB_LITE_HAS_LONG_LONG 0 CACHE INTERNAL "Support for longlong type in PDB Lite")
        MESSAGE(STATUS "    PDB Lite does not have long long support")
    ELSE("${SILO_MAJ_NO}.${SILO_MIN_NO}.${SILO_PAT_NO}" VERSION_LESS 4.7.2)
        SET(PDB_LITE_HAS_LONG_LONG 1 CACHE INTERNAL "Support for longlong type in PDB Lite")
        MESSAGE(STATUS "    PDB Lite has long long support")
    ENDIF("${SILO_MAJ_NO}.${SILO_MIN_NO}.${SILO_PAT_NO}" VERSION_LESS 4.7.2)

    SET(PDB_FOUND 1 CACHE BOOL "PDB library found" FORCE)
    SET(PDB_INCLUDE_DIR ${SILO_INCLUDE_DIR} CACHE PATH "PDB include directory" FORCE)
    SET(PDB_LIBRARY_DIR ${SILO_LIBRARY_DIR} CACHE PATH "PDB library directory" FORCE)
    SET(PDB_LIB ${SILO_LIB} CACHE STRING "PDB library" FORCE)
    MARK_AS_ADVANCED(PDB_INCLUDE_DIR PDB_LIBRARY_DIR PDB_LIB)

ENDIF(SILO_FOUND)
