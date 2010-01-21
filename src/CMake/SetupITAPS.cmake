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
#    Jeremy Meredith, Mon Dec 28 16:05:49 EST 2009
#    File extensions no longer exist; switched to file patterns.
#
#    Mark C. Miller, Wed Jan 13 16:15:39 PST 2010
#    Replaced explicit listing of ITAPS_C source files with more general
#    logic that uses globs on *.[Ch]. Added more logic to handle cases where
#    it appears the generated files have been modified. It that case, we
#    error fataly unless the option FORCE_ITAPS_REGEN is on.
#
#    Mark C. Miller, Thu Jan 14 10:52:19 PST 2010
#    Removed FORCE_ITAPS_REGEN option as there was really no way for it
#    to work in such a way that it would NOT get cached. Previous tests
#    indicating that it was NOT getting cached were the result of having
#    ctrl-C'd the cmake process prior to its completion. I also made the 
#    logic smarter about knowing when generated files have changed. It
#    now makes a copy of the generated files in a separate dir in the 
#    databases directory and compares contents to this prestine copies to
#    identify cases where an ITAPS_XXX plugin source file has been
#    changed.
#
#    Mark C. Miller, Wed Jan 20 16:40:04 PST 2010
#    Added logic to handle 'special' case in which the ITAPS_C plugin
#    itself is being built against a single iMesh implementation. This is
#    to facilitate development (as opposed to installation) of ITAPS plugin.
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

FUNCTION(ITAPS_FILE_PATTERNS IMPL)
    ITAPS_ADD_IMPLEMENTATION(${IMPL})
    FOREACH(X ${ARGN})
        ADD_UNIQUE_LIST_VALUE(ITAPS_${IMPL}_FILE_PATTERNS "ITAPS ${IMPL} file patterns" ${X})
    ENDFOREACH(X)
ENDFUNCTION(ITAPS_FILE_PATTERNS IMPL)

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
# calls to ITAPS_INCLUDE_DIRECTORIES, ITAPS_FILE_PATTERNS, ITAPS_LINK_LIBRARIES,
# ITAPS_LINK_DIRECTORIES.
#
# Sets up cache variables:
#    ITAPS_DIRS
#
FUNCTION(CONFIGURE_ITAPS)
    # For each implementation, we must copy the source directory
    SET(ITAPS_DIRS CACHE INTERNAL "ITAPS source directories")
    MESSAGE(STATUS "Configuring ITAPS")

    FOREACH(IMPL ${ITAPS_IMPLEMENTATIONS})
        IF("${IMPL}" STREQUAL "C")
            LIST(LENGTH ITAPS_IMPLEMENTATIONS NUM_IMPLS)
            IF(NUM_IMPLS GREATER 1)
                MESSAGE(FATAL_ERROR "To configure cmake for DEVELOPMENT of the ITAPS plugin "
                    "you must specify ONLY the ONE ITAPS_C plugin.")
            ELSE(NUM_IMPLS GREATER 1)
                SET(ITAPS_DIRS "ITAPS_C" CACHE INTERNAL "ITAPS source directories")
                MESSAGE(STATUS "  ITAPS_C (development)")
                RETURN()
            ENDIF(NUM_IMPLS GREATER 1)
        ENDIF("${IMPL}" STREQUAL "C")
    ENDFOREACH(IMPL ${ITAPS_IMPLEMENTATIONS})

    FOREACH(IMPL ${ITAPS_IMPLEMENTATIONS})
        MESSAGE(STATUS "  ${IMPL}")
        # Just print info for now
        #MESSAGE(STATUS "    includes  =${ITAPS_${IMPL}_INCLUDE_DIR}")
        #MESSAGE(STATUS "    patterns  =${ITAPS_${IMPL}_FILE_PATTERNS}")
        #MESSAGE(STATUS "    libraries =${ITAPS_${IMPL}_LIB}}")
        #MESSAGE(STATUS "    libdirs   =${ITAPS_${IMPL}_LIBRARY_DIR}}")

        # Create the directories if they do not exist.
        SET(ITAPS_DIRS ${ITAPS_DIRS} ITAPS_${IMPL} CACHE INTERNAL "ITAPS source directories")
        SET(IMPLDIR ${VISIT_SOURCE_DIR}/databases/ITAPS_${IMPL})
        SET(IMPLDIR_COPY ${VISIT_SOURCE_DIR}/databases/.ITAPS_${IMPL}_copy)
        IF(NOT EXISTS ${IMPLDIR})
            FILE(MAKE_DIRECTORY ${IMPLDIR})
        ENDIF(NOT EXISTS ${IMPLDIR})
        IF(NOT EXISTS ${IMPLDIR_COPY})
            FILE(MAKE_DIRECTORY ${IMPLDIR_COPY})
        ENDIF(NOT EXISTS ${IMPLDIR_COPY})

        # Build a list of the ITAPS_C source files
        FILE(GLOB ITAPS_ABS_FILES ${VISIT_SOURCE_DIR}/databases/ITAPS_C/*.[Ch]
                              ${VISIT_SOURCE_DIR}/databases/ITAPS_C/ITAPS_C.xml
                              ${VISIT_SOURCE_DIR}/databases/ITAPS_C/CMakeLists.txt)

        UNSET(ITAPS_FILES)
        FOREACH(F ${ITAPS_ABS_FILES})
            GET_FILENAME_COMPONENT(TMP ${F} NAME)
            SET(ITAPS_FILES ${ITAPS_FILES};${TMP})
        ENDFOREACH(F)

        # Copy each file from ITAPS_C to the new location, rename it,
        # and do some string replacements.
        FOREACH(F ${ITAPS_FILES})
            # Read the original file
#            MESSAGE(STATUS "Reading ${VISIT_SOURCE_DIR}/databases/ITAPS_C/${F}")
            FILE(READ ${VISIT_SOURCE_DIR}/databases/ITAPS_C/${F} FILECONTENTS)

            # Assemble some extension code.
            SET(defExt "")
            SET(allExts ${ITAPS_${IMPL}_FILE_PATTERNS})
            FOREACH(ext ${allExts})
                SET(defExt "${defExt}defaultPatterns.push_back(\"${ext}\");")
            ENDFOREACH(ext)
            STRING(LENGTH "${defExt}" SLEN)

            STRING(REPLACE "ITAPS_C" ITAPS_${IMPL} NEWCONTENTS "${FILECONTENTS}")
            IF(${F} MATCHES "ITAPS_CPluginInfo.C")
                IF(${SLEN} GREATER 0)
                    STRING(REPLACE "defaultPatterns.push_back(\"cub\");" "${defExt}" EXTCONTENTS "${NEWCONTENTS}")
                    SET(NEWCONTENTS "${EXTCONTENTS}")
                    UNSET(EXTCONTENTS)
                ENDIF(${SLEN} GREATER 0)
            ENDIF(${F} MATCHES "ITAPS_CPluginInfo.C")

            # Make a new filename
            STRING(REPLACE "ITAPS_C" ITAPS_${IMPL} NEWNAME ${F})

            # If the file already exists, read it so we can compare the
            # file's contents with what we've created. If they differ then
            # write out the new file.
            IF(EXISTS ${IMPLDIR}/${NEWNAME})
                FILE(READ ${IMPLDIR}/${NEWNAME} OLDCONTENTS)
                IF(EXISTS ${IMPLDIR_COPY}/${NEWNAME})
                    FILE(READ ${IMPLDIR_COPY}/${NEWNAME} OLDCONTENTS_COPY)
                    IF(NOT "${OLDCONTENTS_COPY}" STREQUAL "${OLDCONTENTS}")
                        MESSAGE(FATAL_ERROR "${IMPLDIR}/${NEWNAME} "
                            "needs to get re-generated. However, that file appears to have been changed "
                            "from what was originally generated and re-generating it will destroy those "
                            "changes. If you have edited this file and/or other ITAPS source files in "
                            "directories OTHER THAN the ITAPS_C directory, be aware that the files in "
                            "the ITAPS_C directory are the TRUE SOURCE and all others are essentially "
                            "copies of that directory. Please ensure whatever changes you need get "
                            "propogated back to the corresponding (TRUE) source files in the ITAPS_C "
                            "directory and then remove either the one file ${IMPLDIR}/${NEWNAME} "
                            "or if there are several files that have been changed, then you can remove "
                            "the entire directory ${IMPLDIR} before re-cmaking/re-configuring.")
                    ENDIF(NOT "${OLDCONTENTS_COPY}" STREQUAL "${OLDCONTENTS}")
                ENDIF(EXISTS ${IMPLDIR_COPY}/${NEWNAME})
                IF(NOT "${NEWCONTENTS}" STREQUAL "${OLDCONTENTS}")
                    # Save out the new file
                    MESSAGE(STATUS "    NEW FILE: ${IMPLDIR}/${NEWNAME}")
                    FILE(WRITE ${IMPLDIR}/${NEWNAME} "${NEWCONTENTS}")
                    FILE(WRITE ${IMPLDIR_COPY}/${NEWNAME} "${NEWCONTENTS}")
                ENDIF(NOT "${NEWCONTENTS}" STREQUAL "${OLDCONTENTS}")
                UNSET(OLDCONTENTS)
                UNSET(OLDCONTENTS_COPY)
            ELSE(EXISTS ${IMPLDIR}/${NEWNAME})
                # Save out the new file
                MESSAGE(STATUS "    NEW FILE: ${IMPLDIR}/${NEWNAME}")
                FILE(WRITE ${IMPLDIR}/${NEWNAME} "${NEWCONTENTS}")
                FILE(WRITE ${IMPLDIR_COPY}/${NEWNAME} "${NEWCONTENTS}")
            ENDIF(EXISTS ${IMPLDIR}/${NEWNAME})
            UNSET(FILECONTENTS)
            UNSET(NEWCONTENTS)
        ENDFOREACH(F)
    ENDFOREACH(IMPL)
#    MESSAGE(STATUS "ITAPS_DIRS=${ITAPS_DIRS}")
ENDFUNCTION(CONFIGURE_ITAPS)
