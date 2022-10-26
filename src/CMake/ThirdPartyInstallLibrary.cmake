# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#   Kathleen Bonnell, Wed Feb  3 17:25:42 PST 2010
#   Add simplifed version for windows.
#
#   Eric Brugger, Mon Mar  1 16:25:36 PST 2010
#   I modified the test to determine if a library was a shared library for
#   AIX since on AIX shared libraries can end in ".a".
#
#   Eric Brugger, Fri Mar 12 16:53:54 PST 2010
#   I corrected a typo I made that prevented archives from being included
#   in a binary distribution when VISIT_INSTALL_THIRD_PARTY was defined.
#
#   Cyrus Harrison, Tue Oct  4 13:26:03 PDT 2011
#   Improved support for installing lib symlinks on OSX & Linux. 
#   The PySide libs have names that include "." in the base file name
#   causing GET_FILENAME_COMPONENT(NAME_WE,EXT) to return invalid names.
#   To resolve this I added a more robust way to construct the list of 
#   libs symlinks to try to install.
#
#   Kathleen Biagas, Thu June 14 13:52:53 MST 2012
#   Use GET_FILENAME_SHORTEXT on Windows, too.
#
#   Kathleen Biagas, Mon May 20 14:37:19 MST 2013
#   On Windows, look for dll's in 'bin' directory if not found in 'lib'.
#
#   Kathleen Biagas, Thu Nov  8 10:11:29 PST 2018
#   Added '*.hxx' pattern for includes.
#
#   Eric Brugger, Wed Jun 26 10:04:14 PDT 2019
#   Removed the CCM reader.
#
#****************************************************************************/

#
# This function finds the portion of the filename after the last '.'
#

FUNCTION(GET_FILENAME_SHORTEXT EXTOUT FNAME)
  # we want the ext after the last "."
  GET_FILENAME_COMPONENT(tmp ${FNAME} EXT)
  STRING(REPLACE "." ";" tmp ${tmp})
  # get last element in the list
  FOREACH(X ${tmp})
    SET(${EXTOUT} ".${X}" PARENT_SCOPE)
  ENDFOREACH(X)
ENDFUNCTION(GET_FILENAME_SHORTEXT)


#
# This function installs a library and any of its needed symlink variants.
#

FUNCTION(THIRD_PARTY_INSTALL_LIBRARY LIBFILE)
  IF(WIN32)
        IF(NOT EXISTS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ThirdParty)
            FILE(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ThirdParty)
        ENDIF(NOT EXISTS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ThirdParty)

        SET(tmpLIBFILE ${LIBFILE})
        GET_FILENAME_SHORTEXT(LIBEXT ${tmpLIBFILE})
        GET_FILENAME_COMPONENT(LIBREALPATH ${tmpLIBFILE} REALPATH)
        GET_FILENAME_COMPONENT(curPATH ${LIBREALPATH} PATH)
        GET_FILENAME_COMPONENT(realNAME ${LIBREALPATH} NAME)
        STRING(REPLACE ${LIBEXT} "" curNAMEWE ${realNAME})
        SET(curNAME "${curPATH}/${curNAMEWE}")
        SET(dllNAME "${curNAME}.dll")
        SET(libNAME "${curNAME}.lib")
        IF(EXISTS ${dllNAME})
            INSTALL(FILES ${dllNAME} 
                DESTINATION ${VISIT_INSTALLED_VERSION_BIN}
                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE 
                            GROUP_READ GROUP_WRITE GROUP_EXECUTE 
                            WORLD_READ WORLD_EXECUTE
                CONFIGURATIONS "" None Debug Release RelWithDebInfo MinSizeRel
                )
            # On Windows, we also need to copy the file to the 
            # binary dir so our out of source builds can run.
            EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy
                            ${dllNAME}
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ThirdParty)
        ELSE() # try 'bin' directory
            SET(dll_path "${curPATH}/../bin/${curNAMEWE}")
            GET_FILENAME_COMPONENT(dll_path ${dll_path} ABSOLUTE)
            
            SET(newdllNAME "${dll_path}.dll")
            IF(EXISTS ${newdllNAME})
                INSTALL(FILES ${newdllNAME} 
                    DESTINATION ${VISIT_INSTALLED_VERSION_BIN}
                    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE 
                                GROUP_READ GROUP_WRITE GROUP_EXECUTE 
                                WORLD_READ WORLD_EXECUTE
                    CONFIGURATIONS "" None Debug Release RelWithDebInfo MinSizeRel
                    )
                # On Windows, we also need to copy the file to the 
                # binary dir so our out of source builds can run.
                EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy
                                ${newdllNAME}
                                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ThirdParty)
            ENDIF(EXISTS ${newdllNAME})
        ENDIF(EXISTS ${dllNAME})

        IF(VISIT_INSTALL_THIRD_PARTY AND EXISTS ${libNAME})
            # also install the import libraries
            INSTALL(FILES ${libNAME}
                DESTINATION ${VISIT_INSTALLED_VERSION_LIB}
                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE 
                            GROUP_READ GROUP_WRITE GROUP_EXECUTE 
                            WORLD_READ WORLD_EXECUTE
                CONFIGURATIONS "" None Debug Release RelWithDebInfo MinSizeRel
                )
        ENDIF(VISIT_INSTALL_THIRD_PARTY AND EXISTS ${libNAME})

  ELSE(WIN32)

    SET(tmpLIBFILE ${LIBFILE})
    # GET_FILENAME_COMPONENT(EXT) will not always give us what we want here,
    # use new helper.
    GET_FILENAME_SHORTEXT(LIBEXT ${tmpLIBFILE})
    IF(NOT ${LIBEXT} STREQUAL ".a")
        SET(isSHAREDLIBRARY "YES")
    ELSE(NOT ${LIBEXT} STREQUAL ".a")
        SET(isSHAREDLIBRARY "NO")
    ENDIF(NOT ${LIBEXT} STREQUAL ".a")
    IF(${CMAKE_SYSTEM_NAME} STREQUAL "AIX")
        GET_FILENAME_COMPONENT(baseNAME ${tmpLIBFILE} NAME_WE)
        # On AIX all ".a" files are archives except the following.
        IF((${baseNAME} STREQUAL "libpython2") OR
           (${baseNAME} STREQUAL "libMesaGL") OR
           (${baseNAME} STREQUAL "libOSMesa") OR
           (${baseNAME} STREQUAL "libsz"))
            SET(isSHAREDLIBRARY "YES")
        ENDIF((${baseNAME} STREQUAL "libpython2") OR
              (${baseNAME} STREQUAL "libMesaGL") OR
              (${baseNAME} STREQUAL "libOSMesa") OR
              (${baseNAME} STREQUAL "libsz"))
    ENDIF(${CMAKE_SYSTEM_NAME} STREQUAL "AIX")

    IF(${isSHAREDLIBRARY} STREQUAL "YES")
        GET_FILENAME_COMPONENT(LIBREALPATH ${tmpLIBFILE} REALPATH)
        ## MESSAGE("***tmpLIBFILE=${tmpLIBFILE}, LIBREALPATH=${LIBREALPATH}")
        IF(NOT ${tmpLIBFILE} STREQUAL ${LIBREALPATH})
            # We need to install a library and its symlinks
            GET_FILENAME_COMPONENT(curPATH ${LIBREALPATH} PATH)
            IF((NOT ${curPATH} STREQUAL "/usr/lib") AND (NOT ${curPATH} MATCHES "^\\/opt\\/local\\/lib.*") AND (NOT ${curPATH} MATCHES "^\\/System\\/Library\\/Frameworks\\/.*") AND (NOT ${curPATH} MATCHES "^\\/Library\\/Frameworks\\/.*"))
                # Extract proper base name by comparing the input lib path w/ the real path.
                GET_FILENAME_COMPONENT(realNAME ${LIBREALPATH} NAME)
                GET_FILENAME_COMPONENT(inptNAME ${tmpLIBFILE}  NAME)
                STRING(REPLACE ${LIBEXT} "" inptNAME ${inptNAME})
                STRING(REPLACE ${inptNAME} "" curEXT ${realNAME})
                # We will have a "." at the end of the string, remove it
                STRING(REGEX REPLACE "\\.$" "" inptNAME ${inptNAME})
                STRING(REPLACE "." ";" extList ${curEXT})
                SET(curNAME "${curPATH}/${inptNAME}")
                # Come up with all of the possible library and symlink names
                SET(allNAMES "${curNAME}${LIBEXT}")
                SET(allNAMES ${allNAMES} "${curNAME}${LIBEXT}.1") # seems to be a standard linux-ism that isn't always covered by the foreach-loop on ${extList}
                SET(allNAMES ${allNAMES} "${curNAME}.a")
                FOREACH(X ${extList})
                    SET(curNAME "${curNAME}.${X}")
                    SET(allNAMES ${allNAMES} "${curNAME}")           # Linux way
                    SET(allNAMES ${allNAMES} "${curNAME}${LIBEXT}")  # Mac way
                ENDFOREACH(X)

                LIST(REMOVE_DUPLICATES allNAMES)

                # Add the names that exist to the install.
                FOREACH(curNAMEWithExt ${allNAMES})
                    ## MESSAGE("** Checking ${curNAMEWithExt}")
                    IF(EXISTS ${curNAMEWithExt})
                        ## MESSAGE("** Need to install ${curNAMEWithExt}")
                        IF(IS_DIRECTORY ${curNAMEWithExt})
                            # It is a framework, install as a directory
                            INSTALL(DIRECTORY ${curNAMEWithExt}
                                DESTINATION ${VISIT_INSTALLED_VERSION_LIB}
                                DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
                                FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
                                CONFIGURATIONS "" None Debug Release RelWithDebInfo MinSizeRel
                            )
                        ELSE(IS_DIRECTORY ${curNAMEWithExt})
                            INSTALL(FILES ${curNAMEWithExt}
                                DESTINATION ${VISIT_INSTALLED_VERSION_LIB}
                                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
                                CONFIGURATIONS "" None Debug Release RelWithDebInfo MinSizeRel
                            )
                        ENDIF(IS_DIRECTORY ${curNAMEWithExt})
                    ENDIF(EXISTS ${curNAMEWithExt})
                ENDFOREACH(curNAMEWithExt)
            ENDIF()
        ELSE(NOT ${tmpLIBFILE} STREQUAL ${LIBREALPATH})
            GET_FILENAME_COMPONENT(curPATH ${LIBREALPATH} PATH)
            IF((NOT ${curPATH} STREQUAL "/usr/lib") AND (NOT ${curPATH} MATCHES "^\\/opt\\/local\\/lib.*") AND (NOT ${curPATH} MATCHES "^\\/System\\/Library\\/Frameworks\\/.*") AND (NOT ${curPATH} MATCHES "^\\/Library\\/Frameworks\\/.*"))
                # We need to install just the library
                IF(IS_DIRECTORY ${tmpLIBFILE})
                    # It is a framework, install as a directory.
                    INSTALL(DIRECTORY ${tmpLIBFILE}
                        DESTINATION ${VISIT_INSTALLED_VERSION_LIB}
                        DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
                        FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
                        CONFIGURATIONS "" None Debug Release RelWithDebInfo MinSizeRel
                        PATTERN "Qt*_debug" EXCLUDE # Exclude Qt*_debug libraries in framework.
                    )
                ELSE(IS_DIRECTORY ${tmpLIBFILE})
                    # Create an install target for just the library file
                    INSTALL(FILES ${tmpLIBFILE}
                        DESTINATION ${VISIT_INSTALLED_VERSION_LIB}
                        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
                        CONFIGURATIONS "" None Debug Release RelWithDebInfo MinSizeRel
                    )
                ENDIF(IS_DIRECTORY ${tmpLIBFILE})
#            MESSAGE("**We need to install lib ${tmpLIBFILE}")
            ENDIF()
        ENDIF(NOT ${tmpLIBFILE} STREQUAL ${LIBREALPATH})
    ELSE(${isSHAREDLIBRARY} STREQUAL "YES")
        # We have a .a that we need to install to archives.
        IF(VISIT_INSTALL_THIRD_PARTY)
#            MESSAGE("***INSTALL ${LIBFILE} to ${VISIT_INSTALLED_VERSION_ARCHIVES}")
            INSTALL(FILES ${tmpLIBFILE}
                DESTINATION ${VISIT_INSTALLED_VERSION_ARCHIVES}
                PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ
                CONFIGURATIONS "" None Debug Release RelWithDebInfo MinSizeRel
            )

            # TODO: We could install windows import libraries here...

        ENDIF(VISIT_INSTALL_THIRD_PARTY)
    ENDIF(${isSHAREDLIBRARY} STREQUAL "YES")
  ENDIF(WIN32)
ENDFUNCTION(THIRD_PARTY_INSTALL_LIBRARY)

#
# This function installs a library's includes.
#

FUNCTION(THIRD_PARTY_INSTALL_INCLUDE pkg incdir)
        IF(VISIT_INSTALL_THIRD_PARTY AND NOT VISIT_HEADERS_SKIP_INSTALL)
            STRING(TOLOWER ${pkg} lcpkg)
#            MESSAGE("***INSTALL ${incdir} -> ${VISIT_INSTALLED_VERSION_INCLUDE}/${lcpkg}")
            INSTALL(DIRECTORY ${incdir}
                DESTINATION ${VISIT_INSTALLED_VERSION_INCLUDE}/${lcpkg}
                DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
                FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
                CONFIGURATIONS "" None Debug Release RelWithDebInfo MinSizeRel
                FILES_MATCHING 
                PATTERN "*.h"
                PATTERN "*.H"
                PATTERN "*.hpp"
                PATTERN "*.hxx"
                PATTERN "*.HPP"
                PATTERN "*.inc"
                PATTERN ".svn" EXCLUDE
            )
        ENDIF(VISIT_INSTALL_THIRD_PARTY AND NOT VISIT_HEADERS_SKIP_INSTALL)
ENDFUNCTION(THIRD_PARTY_INSTALL_INCLUDE)

#
# This function installs a library's executables.
#

FUNCTION(THIRD_PARTY_INSTALL_EXECUTABLE)
    FOREACH(exe ${ARGN})
        IF(EXISTS ${exe})
            INSTALL(PROGRAMS ${exe}
               DESTINATION ${VISIT_INSTALLED_VERSION_BIN}
               PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                           GROUP_READ GROUP_WRITE GROUP_EXECUTE
                           WORLD_READ WORLD_EXECUTE
            )
        ENDIF(EXISTS ${exe})
    ENDFOREACH(exe ${ARGN})
ENDFUNCTION(THIRD_PARTY_INSTALL_EXECUTABLE exes)

# -----------------------------------------------------------------------
# visit_find_imported_location
#   Examines properties of 'target' for its IMPORTED LOCATION
#   First tries to match config with CMAKE_BUILD_TYPE, then will use
#   first available config, or will try Release, RelWithDebInfo, Debug
#   or non-config-specific.
#
# -----------------------------------------------------------------------
macro(visit_find_imported_location target target_location)

    # see if the package populated it's supported configurations
    get_target_property(tconfigs ${target} IMPORTED_CONFIGURATIONS)
    if(tconfigs)
        if(DEFINED CMAKE_BUILD_TYPE)
            string(TOUPPER ${CMAKE_BUILD_TYPE} ucbuildtype)
            list(FIND tconfigs ${ucbuildtype} t_has_config)
            if(${t_has_config} GREATER -1)
                get_target_property(tloc ${target} IMPORTED_LOCATION_${ucbuildtype})
            endif()
        endif()
        if(NOT tloc)
           # use first defined configuration
           list(GET tloc 0 useconfig)
           get_target_property(tloc ${target} IMPORTED_LOCATION_${useconfig})
        endif()
    endif()

    # fallbacks for situation where "target" did not populate configs
    if(NOT tloc)
        get_target_property(tloc ${target} IMPORTED_LOCATION_RELEASE)
    endif()
    if(NOT tloc)
        get_target_property(tloc ${target} IMPORTED_LOCATION_RELWITHDEBINFO)
    endif()
    if(NOT tloc)
        get_target_property(tloc ${target} IMPORTED_LOCATION_DEBUG)
    endif()
    if(NOT tloc)
        get_target_property(tloc ${target} IMPORTED_LOCATION)
    endif()
    if(tloc)
        set(target_location ${tloc})
    endif()
endmacro()

# -----------------------------------------------------------------------
# visit_install_tp_import_lib
#   Function to install a third-party library that was set up as an
#   import target.
#   Utilizes visit_find_imported_location to determine the full path
#   to the imported library target.
# -----------------------------------------------------------------------
function(visit_install_tp_import_lib target)
    visit_find_imported_location(${target} tloc)
    if(NOT tloc)
        message(FATAL_ERROR "Could not locate imported location for third party imported library: ${target}")
    endif()

    THIRD_PARTY_INSTALL_LIBRARY(${tloc})
endfunction()
