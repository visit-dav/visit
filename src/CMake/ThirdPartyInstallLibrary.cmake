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
#
#   Kathleen Bonnell, Wed Feb  3 17:25:42 PST 2010
#   Add simplifed version for windows.
#
#****************************************************************************/

#
# This function installs a library and any of its needed symlink variants.
#

FUNCTION(THIRD_PARTY_INSTALL_LIBRARY LIBFILE)
  IF(WIN32)
        IF(NOT EXISTS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ThirdParty)
            FILE(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ThirdParty)
        ENDIF(NOT EXISTS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ThirdParty)

        SET(tmpLIBFILE ${LIBFILE})
        GET_FILENAME_COMPONENT(LIBREALPATH ${tmpLIBFILE} REALPATH)
        GET_FILENAME_COMPONENT(curPATH ${LIBREALPATH} PATH)
        GET_FILENAME_COMPONENT(curNAMEWE ${LIBREALPATH} NAME_WE)
        SET(curNAME "${curPATH}/${curNAMEWE}")
        SET(dllNAME "${curNAME}.dll")
        SET(libNAME "${curNAME}.lib")
        IF(EXISTS ${dllNAME})
            INSTALL(FILES ${dllNAME} 
                DESTINATION ${VISIT_INSTALLED_VERSION_BIN}
                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
                CONFIGURATIONS "";None;Debug;Release;RelWithDebInfo;MinSizeRel
                )
            # On Windows, we also need to copy the file to the 
            # binary dir so our out of source builds can run.
            EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy
                            ${dllNAME}
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ThirdParty)
        ENDIF(EXISTS ${dllNAME})

        IF(VISIT_INSTALL_THIRD_PARTY AND EXISTS ${libNAME})
            # also install the import libraries
            INSTALL(FILES ${libNAME}
                DESTINATION ${VISIT_INSTALLED_VERSION_LIB}
                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
                CONFIGURATIONS "";None;Debug;Release;RelWithDebInfo;MinSizeRel
                )
        ENDIF(VISIT_INSTALL_THIRD_PARTY AND EXISTS ${libNAME})

  ELSE(WIN32)

    SET(tmpLIBFILE ${LIBFILE})
    GET_FILENAME_COMPONENT(LIBEXT ${tmpLIBFILE} EXT)
    IF(NOT ${LIBEXT} STREQUAL ".a")
        GET_FILENAME_COMPONENT(LIBREALPATH ${tmpLIBFILE} REALPATH)
#        MESSAGE("***tmpLIBFILE=${tmpLIBFILE}, LIBPATH=${LIBPATH}, LIBREALPATH=${LIBREALPATH}")
        IF(NOT ${tmpLIBFILE} STREQUAL ${LIBREALPATH})
            # We need to install a library and its symlinks
            GET_FILENAME_COMPONENT(curPATH ${LIBREALPATH} PATH)
            IF((NOT ${curPATH} STREQUAL "/usr/lib") AND (NOT ${curPATH} MATCHES "^\\/System\\/Library\\/Frameworks\\/.*"))
                GET_FILENAME_COMPONENT(curNAMEWE ${LIBREALPATH} NAME_WE)
                GET_FILENAME_COMPONENT(curEXT ${LIBREALPATH} EXT)
                STRING(REPLACE "." ";" extList ${curEXT})
                SET(curNAME "${curPATH}/${curNAMEWE}")
                # Come up with all of the possible library and symlink names
                SET(allNAMES "${curNAME}${LIBEXT}")
                FOREACH(X ${extList})
                    SET(curNAME "${curNAME}.${X}")
                    SET(allNAMES ${allNAMES} "${curNAME}")           # Linux way
                    SET(allNAMES ${allNAMES} "${curNAME}${LIBEXT}")  # Mac way
                ENDFOREACH(X)

                LIST(REMOVE_DUPLICATES allNAMES)

                # Add the names that exist to the install.
                FOREACH(curNAMEWithExt ${allNAMES})
                    IF(EXISTS ${curNAMEWithExt})
                        #MESSAGE("** Need to install ${curNAMEWithExt}")
                        IF(IS_DIRECTORY ${curNAMEWithExt})
                            # It is a framework, install as a directory
                            INSTALL(DIRECTORY ${curNAMEWithExt}
                                DESTINATION ${VISIT_INSTALLED_VERSION_LIB}
                                DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
                                FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
                                CONFIGURATIONS "";None;Debug;Release;RelWithDebInfo;MinSizeRel
                            )

                            # On Apple, we need to make the framework be executable relative
                            GET_FILENAME_COMPONENT(frameworkNameWE ${curNAMEWithExt} NAME_WE)
                            GET_FILENAME_COMPONENT(realFramework ${curNAMEWithExt}/${frameworkNameWE} REALPATH)
                            STRING(REGEX MATCH "${frameworkNameWE}[A-Za-z0-9._/-]*" frameworkMatch ${realFramework})
                            INSTALL(CODE 
                                "EXECUTE_PROCESS(WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}
                                    COMMAND /bin/sh ${VISIT_SOURCE_DIR}/CMake/osxfixup -lib ${CMAKE_INSTALL_PREFIX}/${VISIT_INSTALLED_VERSION_LIB}/${frameworkMatch}
                                    OUTPUT_VARIABLE OSXOUT)
                                 MESSAGE(STATUS \"\${OSXOUT}\")
                                ")
                        ELSE(IS_DIRECTORY ${curNAMEWithExt})
                            INSTALL(FILES ${curNAMEWithExt}
                                DESTINATION ${VISIT_INSTALLED_VERSION_LIB}
                                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
                                CONFIGURATIONS "";None;Debug;Release;RelWithDebInfo;MinSizeRel
                            )

                            # On Apple, we need to make the library be executable relative.
                            IF(APPLE)
                                GET_FILENAME_COMPONENT(libName ${curNAMEWithExt} NAME)
                                INSTALL(CODE 
                                    "EXECUTE_PROCESS(WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}
                                        COMMAND /bin/sh ${VISIT_SOURCE_DIR}/CMake/osxfixup -lib ${CMAKE_INSTALL_PREFIX}/${VISIT_INSTALLED_VERSION_LIB}/${libName}
                                        OUTPUT_VARIABLE OSXOUT)
                                     MESSAGE(STATUS \"\${OSXOUT}\")
                                    ")
                            ENDIF(APPLE)
                        ENDIF(IS_DIRECTORY ${curNAMEWithExt})
                    ENDIF(EXISTS ${curNAMEWithExt})
                ENDFOREACH(curNAMEWithExt)
            ENDIF((NOT ${curPATH} STREQUAL "/usr/lib") AND (NOT ${curPATH} MATCHES "^\\/System\\/Library\\/Frameworks\\/.*"))
        ELSE(NOT ${tmpLIBFILE} STREQUAL ${LIBREALPATH})
            # We need to install just the library
            IF(IS_DIRECTORY ${tmpLIBFILE})
                # It is a framework, install as a directory.
                INSTALL(DIRECTORY ${tmpLIBFILE}
                    DESTINATION ${VISIT_INSTALLED_VERSION_LIB}
                    DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
                    FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
                    CONFIGURATIONS "";None;Debug;Release;RelWithDebInfo;MinSizeRel
                    PATTERN "Qt*_debug" EXCLUDE # Exclude Qt*_debug libraries in framework.
                )

                # On Apple, we need to make the framework be executable relative
                GET_FILENAME_COMPONENT(frameworkNameWE ${tmpLIBFILE} NAME_WE)
                GET_FILENAME_COMPONENT(realFramework ${tmpLIBFILE}/${frameworkNameWE} REALPATH)
                STRING(REGEX MATCH "${frameworkNameWE}[A-Za-z0-9._/-]*" frameworkMatch ${realFramework})
                INSTALL(CODE 
                    "EXECUTE_PROCESS(WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}
                        COMMAND /bin/sh ${VISIT_SOURCE_DIR}/CMake/osxfixup -lib ${CMAKE_INSTALL_PREFIX}/${VISIT_INSTALLED_VERSION_LIB}/${frameworkMatch}
                        OUTPUT_VARIABLE OSXOUT)
                     MESSAGE(STATUS \"\${OSXOUT}\")
                    ")
            ELSE(IS_DIRECTORY ${tmpLIBFILE})
                # Create an install target for just the library file
                INSTALL(FILES ${tmpLIBFILE}
                    DESTINATION ${VISIT_INSTALLED_VERSION_LIB}
                    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
                    CONFIGURATIONS "";None;Debug;Release;RelWithDebInfo;MinSizeRel
                )

                # On Apple, we need to make the library be executable relative.
                IF(APPLE)
                    GET_FILENAME_COMPONENT(libName ${tmpLIBFILE} NAME)
                    INSTALL(CODE 
                        "EXECUTE_PROCESS(WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}
                            COMMAND /bin/sh ${VISIT_SOURCE_DIR}/CMake/osxfixup -lib ${CMAKE_INSTALL_PREFIX}/${VISIT_INSTALLED_VERSION_LIB}/${libName}
                            OUTPUT_VARIABLE OSXOUT)
                         MESSAGE(STATUS \"\${OSXOUT}\")
                        ")
                ENDIF(APPLE)
            ENDIF(IS_DIRECTORY ${tmpLIBFILE})
#            MESSAGE("**We need to install lib ${tmpLIBFILE}")
        ENDIF(NOT ${tmpLIBFILE} STREQUAL ${LIBREALPATH})
    ELSE(NOT ${LIBEXT} STREQUAL ".a")
        # We have a .a that we need to install to archives.
        IF(VISIT_INSTALL_THIRD_PARTY)
#            MESSAGE("***INSTALL ${LIBFILE} to ${VISIT_INSTALLED_VERSION_ARCHIVES}")
MESSAGE("INSTALL files Path 6")
            INSTALL(FILES ${tmpLIBFILE}
                DESTINATION ${VISIT_INSTALLED_VERSION_ARCHIVES}
                PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ
                CONFIGURATIONS "";None;Debug;Release;RelWithDebInfo;MinSizeRel
            )

            # TODO: We could install windows import libraries here...

        ENDIF(VISIT_INSTALL_THIRD_PARTY)
    ENDIF(NOT ${LIBEXT} STREQUAL ".a")
  ENDIF(WIN32)
ENDFUNCTION(THIRD_PARTY_INSTALL_LIBRARY)

#
# This function installs a library's includes.
#

FUNCTION(THIRD_PARTY_INSTALL_INCLUDE pkg incdir)
        IF(VISIT_INSTALL_THIRD_PARTY)
            STRING(TOLOWER ${pkg} lcpkg)
#            MESSAGE("***INSTALL ${incdir} -> ${VISIT_INSTALLED_VERSION_INCLUDE}/${lcpkg}")
            INSTALL(DIRECTORY ${incdir}
                DESTINATION ${VISIT_INSTALLED_VERSION_INCLUDE}/${lcpkg}
                DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
                FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
                CONFIGURATIONS "";None;Debug;Release;RelWithDebInfo;MinSizeRel
                FILES_MATCHING 
                PATTERN "*.h"
                PATTERN "*.H"
                PATTERN "*.hpp"
                PATTERN "*.HPP"
                PATTERN "*.inc"
                PATTERN "libccmio" EXCLUDE
                PATTERN ".svn" EXCLUDE
            )
        ENDIF(VISIT_INSTALL_THIRD_PARTY)
ENDFUNCTION(THIRD_PARTY_INSTALL_INCLUDE)
