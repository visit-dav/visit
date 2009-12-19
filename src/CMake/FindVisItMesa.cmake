#*****************************************************************************
#
# Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
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
#   Kathleen Bonnell, Thu Dec  3 10:55:03 PST 2009
#   Wrap CMAKE_X_LIBS so that it won't parse on windows. Change ${MESA_FOUND}
#   to MESA_FOUND to remove cmake error.
#
#   Kathleen Bonnell, Wed Dec  9 15:13:27 MT 2009
#   Copy Mesa dlls to execution directory for OSMesa test on windows.
#
#****************************************************************************/

# Use the VTK_DIR hint from the config-site .cmake file 
INCLUDE(${VISIT_SOURCE_DIR}/CMake/SetUpThirdParty.cmake)

IF (WIN32)
    SET_UP_THIRD_PARTY(MESA lib/${VISIT_MSVC_VERSION} include MesaGL32 osmesa32)
ELSE (WIN32)
    SET_UP_THIRD_PARTY(MESA lib include MesaGL OSMesa)

    # Install Mesa headers
    INSTALL(DIRECTORY ${MESA_INCLUDE_DIR}
        DESTINATION ${VISIT_INSTALLED_VERSION_INCLUDE}/mesa
        FILE_PERMISSIONS OWNER_WRITE OWNER_READ GROUP_WRITE GROUP_READ WORLD_READ
        DIRECTORY_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_WRITE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    )
ENDIF (WIN32)

IF(NOT MESA_FOUND)
    MESSAGE(FATAL_ERROR "MESA is required to build VisIt")
ENDIF(NOT MESA_FOUND)

IF ("HAVE_OSMESA_SIZE" MATCHES "^HAVE_OSMESA_SIZE$")
    SET(MSG "Check for osmesa size limit")
    MESSAGE(STATUS ${MSG})
    # Need to have the mesa libs.
    SET(MY_LIBS ${MESA_LIB})
    # Unix needs X_LIBS and THREAD_LIBS.
    IF (NOT WIN32)
      IF (CMAKE_X_LIBS)
        SET(MY_LIBS ${MY_LIBS} ${CMAKE_X_LIBS})
      ENDIF (CMAKE_X_LIBS)
    ENDIF (NOT WIN32)
    IF (CMAKE_THREAD_LIBS)
        SET(MY_LIBS ${MY_LIBS} ${CMAKE_THREAD_LIBS})
    ENDIF (CMAKE_THREAD_LIBS)

    SET(TRY_RUN_DIR ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/CMakeTmp)
   
    IF (WIN32) 
        # Need these dlls to run the program
        FILE(COPY ${MESA_LIBRARY_DIR}/MesaGL32.dll DESTINATION ${TRY_RUN_DIR}/CMakeFiles/CMakeTmp/debug/)
        FILE(COPY ${MESA_LIBRARY_DIR}/osmesa32.dll DESTINATION ${TRY_RUN_DIR}/CMakeFiles/CMakeTmp/debug/)
    ENDIF (WIN32) 

    TRY_RUN(TRY_RUN_RESULT HAVE_OSMESA_SIZE
        ${TRY_RUN_DIR}
        ${VISIT_SOURCE_DIR}/CMake/FindOSMesaSize.C
        CMAKE_FLAGS "-DINCLUDE_DIRECTORIES:STRING=${MESA_INCLUDE_DIR}"
                    "-DLINK_DIRECTORIES:STRING=${MESA_LIBRARY_DIR}"
                    "-DLINK_LIBRARIES:STRING=${MY_LIBS}"
        OUTPUT_VARIABLE OUTPUT
    )
    IF (HAVE_OSMESA_SIZE)
        IF ("${TRY_RUN_RESULT}" MATCHES "FAILED_TO_RUN")
            MESSAGE(STATUS "${MSG} - failed to run, defaulting to 4096")
            SET(OSMESA_SIZE_LIMIT 4096)
        ELSE ("${TRY_RUN_RESULT}" MATCHES "FAILED_TO_RUN")
            IF (WIN32)
                SET(OSMESA_SIZE_LIMIT ${TRY_RUN_RESULT})
                MESSAGE(STATUS "${MSG} - found (${OSMESA_SIZE_LIMIT})")
                SET(HAVE_OSMESA_SIZE 1 CACHE INTERNAL "support for osmesa_size")
            ELSE (WIN32)
                IF (EXISTS ${CMAKE_BINARY_DIR}/junk.txt)
                    FILE(STRINGS "${CMAKE_BINARY_DIR}/junk.txt" OSMESA_SIZE_LIMIT)
                    FILE(REMOVE "${CMAKE_BINARY_DIR}/junk.txt")
                    MESSAGE(STATUS "${MSG} - found (${OSMESA_SIZE_LIMIT})")
                    SET(HAVE_OSMESA_SIZE 1 CACHE INTERNAL "support for osmesa_size")
                ELSE (EXISTS ${CMAKE_BINARY_DIR}/junk.txt)
                    MESSAGE(STATUS "${MSG} - could not find junk.txt")
                ENDIF (EXISTS ${CMAKE_BINARY_DIR}/junk.txt)
            ENDIF (WIN32)
        ENDIF ("${TRY_RUN_RESULT}" MATCHES "FAILED_TO_RUN")
    ELSE(HAVE_OSMESA_SIZE)
        MESSAGE(STATUS "${MSG} - OUTPUT_VARIABLE: ${OUTPUT}")
        MESSAGE(STATUS "${MSG} - not found, defaulting to 4096")
        SET(HAVE_OSMESA_SIZE 0 CACHE INTERNAL "support for osmesa_size")
        SET(OSMESA_SIZE_LIMIT 4096)
    ENDIF (HAVE_OSMESA_SIZE)
ENDIF ("HAVE_OSMESA_SIZE" MATCHES "^HAVE_OSMESA_SIZE$")



