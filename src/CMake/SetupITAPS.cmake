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
#    Kathleen Bonnell, Wed Dec  9 15:11:42 MT 2009
#    Made all messages 'STATUS', otherwise they appear as errors when using 
#    CMake gui.
#
#****************************************************************************/

FUNCTION(ITAPS_ADD_IMPLEMENTATION IMPL)
    IF(ITAPS_IMPLEMENTATIONS)
        SEPARATE_ARGUMENTS(ITAPS_IMPLEMENTATIONS)
        SET(ADD_TO_LIST 1)
        FOREACH(X ${ITAPS_IMPLEMENTATIONS})
            IF(${X} MATCHES ${IMPL})
                SET(ADD_TO_LIST 0)
                BREAK()
            ENDIF(${X} MATCHES ${IMPL})
        ENDFOREACH(X)

        IF(ADD_TO_LIST)
            SET(VAL ${ITAPS_IMPLEMENTATIONS} ${IMPL})
            SET(ITAPS_IMPLEMENTATIONS ${VAL} CACHE INTERNAL "List of ITAPS implementations")
        ENDIF(ADD_TO_LIST)
    ELSE(ITAPS_IMPLEMENTATIONS)
        SET(ITAPS_IMPLEMENTATIONS ${IMPL} CACHE INTERNAL "List of ITAPS implementations")
    ENDIF(ITAPS_IMPLEMENTATIONS)
ENDFUNCTION(ITAPS_ADD_IMPLEMENTATION)

FUNCTION(ADD_UNIQUE_LIST_VALUE VAR DESC VALUE)
    IF(${VAR})
        SEPARATE_ARGUMENTS(${VAR})
        SET(ADD_TO_LIST 1)
        FOREACH(XX ${${VAR}})
            IF("${XX}" STREQUAL "${VALUE}")
                SET(ADD_TO_LIST 0)
                BREAK()
            ENDIF("${XX}" STREQUAL "${VALUE}")
        ENDFOREACH(XX)

        IF(ADD_TO_LIST)
            SET(VAL2 ${${VAR}} ${VALUE})
            SET(${VAR} ${VAL2} CACHE INTERNAL "${DESC}")
        ENDIF(ADD_TO_LIST)
    ELSE(${VAR})
        SET(${VAR} ${VALUE} CACHE INTERNAL "${DESC}")
    ENDIF(${VAR})
ENDFUNCTION(ADD_UNIQUE_LIST_VALUE VAR DESC VALUE)

FUNCTION(ITAPS_INCLUDE_DIRECTORIES IMPL)
    ITAPS_ADD_IMPLEMENTATION(${IMPL})
    FOREACH(X ${ARGN})
        ADD_UNIQUE_LIST_VALUE(ITAPS_${IMPL}_INCLUDE_DIR "ITAPS ${IMPL} include directories" ${X})
    ENDFOREACH(X)
ENDFUNCTION(ITAPS_INCLUDE_DIRECTORIES IMPL)

FUNCTION(ITAPS_FILE_EXTENSIONS IMPL)
    ITAPS_ADD_IMPLEMENTATION(${IMPL})
    FOREACH(X ${ARGN})
        ADD_UNIQUE_LIST_VALUE(ITAPS_${IMPL}_FILE_EXTENSIONS "ITAPS ${IMPL} file extensions" ${X})
    ENDFOREACH(X)
ENDFUNCTION(ITAPS_FILE_EXTENSIONS IMPL)

FUNCTION(ITAPS_LINK_LIBRARIES IMPL)
    ITAPS_ADD_IMPLEMENTATION(${IMPL})
    FOREACH(X ${ARGN})
        ADD_UNIQUE_LIST_VALUE(ITAPS_${IMPL}_LIB "ITAPS ${IMPL} link libraries" ${X})
    ENDFOREACH(X)
ENDFUNCTION(ITAPS_LINK_LIBRARIES IMPL)

FUNCTION(ITAPS_LINK_DIRECTORIES IMPL)
    ITAPS_ADD_IMPLEMENTATION(${IMPL})
    FOREACH(X ${ARGN})
        ADD_UNIQUE_LIST_VALUE(ITAPS_${IMPL}_LIBRARY_DIR "ITAPS ${IMPL} library directories" ${X})
    ENDFOREACH(X)
ENDFUNCTION(ITAPS_LINK_DIRECTORIES IMPL)

#
# Creates new ITAPS plugins for the implementations that have been defined via
# calls to ITAPS_INCLUDE_DIRECTORIES, ITAPS_FILE_EXTENSIONS, ITAPS_LINK_LIBRARIES,
# ITAPS_LINK_DIRECTORIES.
#
# Sets up cache variables:
#    ITAPS_DIRS
#
FUNCTION(CONFIGURE_ITAPS)
    # For each implementation, we must copy the source directory
    SET(ITAPS_DIRS CACHE INTERNAL "ITAPS source directories")
    MESSAGE(STATUS "-- Configuring ITAPS")
    FOREACH(IMPL ${ITAPS_IMPLEMENTATIONS})
        MESSAGE(STATUS "--  ${IMPL}")
        # Just print info for now
        #MESSAGE(STATUS "    includes  =${ITAPS_${IMPL}_INCLUDE_DIR}")
        #MESSAGE(STATUS "    extensions=${ITAPS_${IMPL}_FILE_EXTENSIONS}")
        #MESSAGE(STATUS "    libraries =${ITAPS_${IMPL}_LIB}}")
        #MESSAGE(STATUS "    libdirs   =${ITAPS_${IMPL}_LIBRARY_DIR}}")

        # Create the directories if they do not exist.
        SET(IMPLDIR ${VISIT_SOURCE_DIR}/databases/ITAPS_${IMPL})
        IF(NOT EXISTS ${IMPLDIR})
            FILE(MAKE_DIRECTORY ${IMPLDIR})
        ENDIF(NOT EXISTS ${IMPLDIR})
        SET(ITAPS_DIRS ${ITAPS_DIRS} ITAPS_${IMPL} CACHE INTERNAL "ITAPS source directories")

        # Copy each file to from ITAPS_C to the new location, rename it,
        # and do some string replacements.
        FOREACH(F CMakeLists.txt
            ITAPS_C.xml
            ITAPS_CCommonPluginInfo.C
            ITAPS_CEnginePluginInfo.C
            ITAPS_CMDServerPluginInfo.C
            ITAPS_CPluginInfo.C
            ITAPS_CPluginInfo.h
            avtITAPS_CFileFormat.C
            avtITAPS_CFileFormat.h
            avtITAPS_COptions.C
            avtITAPS_COptions.h
            avtITAPS_CUtility.C
            avtITAPS_CUtility.h
            avtITAPS_CWriter.C
            avtITAPS_CWriter.h)
            # Read the original file
#            MESSAGE(STATUS "Reading ${VISIT_SOURCE_DIR}/databases/ITAPS_C/${F}")
            FILE(READ ${VISIT_SOURCE_DIR}/databases/ITAPS_C/${F} FILECONTENTS)

            # Assemble some extension code.
            SET(defExt "")
            SET(allExts ${ITAPS_${IMPL}_FILE_EXTENSIONS})
            FOREACH(ext ${allExts})
                SET(defExt "${defExt}defaultExtensions.push_back(\"${ext}\");")
            ENDFOREACH(ext)
            STRING(LENGTH "${defExt}" SLEN)

            STRING(REPLACE "ITAPS_C" ITAPS_${IMPL} NEWCONTENTS "${FILECONTENTS}")
            IF(${F} MATCHES "ITAPS_CPluginInfo.C")
                IF(${SLEN} GREATER 0)
                    STRING(REPLACE "defaultExtensions.push_back(\"cub\");" "${defExt}" EXTCONTENTS "${NEWCONTENTS}")
                    SET(NEWCONTENTS "${EXTCONTENTS}")
                    UNSET(EXTCONTENTS)
                ENDIF(${SLEN} GREATER 0)
            ENDIF(${F} MATCHES "ITAPS_CPluginInfo.C")

            # Make a new filename
            STRING(REPLACE "ITAPS_C" ITAPS_${IMPL} NEWNAME ${F})

            # If the file already exists, read it so we can compare the
            # file's contents with what we've created. If they differ then
            # write out the new file.
            SET(WRITE_NEW_FILE 1)
            IF(EXISTS ${IMPLDIR}/${NEWNAME})
                FILE(READ ${IMPLDIR}/${NEWNAME} OLDCONTENTS)
                IF("${NEWCONTENTS}" STREQUAL "${OLDCONTENTS}")
                    SET(WRITE_NEW_FILE 0)
                ENDIF("${NEWCONTENTS}" STREQUAL "${OLDCONTENTS}")
                UNSET(OLDCONTENTS)
            ENDIF(EXISTS ${IMPLDIR}/${NEWNAME})

            # Save out the new file
            IF(WRITE_NEW_FILE)
                MESSAGE(STATUS "NEW FILE: ${IMPLDIR}/${NEWNAME}")
                FILE(WRITE ${IMPLDIR}/${NEWNAME} "${NEWCONTENTS}")
            ENDIF(WRITE_NEW_FILE)
            UNSET(FILECONTENTS)
            UNSET(NEWCONTENTS)
        ENDFOREACH(F)
    ENDFOREACH(IMPL)
#    MESSAGE(STATUS "ITAPS_DIRS=${ITAPS_DIRS}")
ENDFUNCTION(CONFIGURE_ITAPS)
