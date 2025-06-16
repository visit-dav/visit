# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Eric Brugger, Tue Mar 16 12:42:01 PDT 2010
#   I modified the script to not install libraries located in /usr/lib.
#
#   Tom Fogal, Tue Mar 16 17:20:59 MDT 2010
#   Set the proper type for some MESSAGE calls which report errors.
#
#   Mark C. Miller, Thu Jul 29 23:39:05 PDT 2010
#   Adjusted to fail fatally when things cannot be found and to override
#   this failure behavior with option IGNORE_THIRD_PARTY_LIB_PROBLEMS.
#   Replaced multiple leves of indentation with inverted IF logic
#   and early returns. Got rid of setup_lib_error variable. Output
#   'not requested' message if pkg was not specified by user. Tried
#   to make status message for failure to find distinguishable in
#   cmake output from other messages.
#
#   Brad Whitlock, Mon Apr 18 15:18:08 PDT 2011
#   Added another check for xxx-NOTFOUND so we don't assume just because we
#   didn't specify a library like TCMALLOC that we have an error situation
#   when it does not exist.
#
#   Allen Sanderson, Kathleen Biagas, Thu Jan 22 08:55:21 MST 2015
#   Added logic for headers-only installs (like boost).  Pass 'NO_LIBS'
#   for 'libs' argument to specify a headers-only build.
#
#   Kathleen Biagas, Fri Mar 17 09:15:37 PDT 2017
#   Set HAVE_LIBXXX when LIBXXX_FOUND.
#
#   Kathleen Biagas, Tue Nov 29, 2022
#   Moved functions from ThirdPartyInstallLibrary.cmake into this file.
#   Moved all FindXXX calls out of root CMakeLists.txt into this file.
#   Use cmake_path functions instead of get_filename_component.
#
#   Kathleen Biagas, Tue Jan 31, 2023
#   Moved FindNSIS into VisItWindowsFunctions.cmake.
#
#   Eric Brugger, Fri Feb 24 14:57:15 PST 2023
#   I replaced vtkh with vtkm.
#
#   Kathleen Biagas, Thu May 2, 2024
#   Add '*.inl' to acceptable patterns when installing headers.
#
#   Eric Brugger, Wed Sep  4 10:31:31 PDT 2024
#   I re-enabled vtkm.
#
#   Kevin Griffin, Mon 24 Feb 2025
#   Added Include for FindANARI.cmake
#
#   Kathleen Biagas, Mon Jun 16, 2025
#   Remove SET_UP_THIRD_PARTY.
#
#****************************************************************************/

# ==============================================
# Functions for checking and installing TP libs
# ==============================================

set(VISIT_TP_PERMS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                   GROUP_READ GROUP_WRITE GROUP_EXECUTE
                   WORLD_READ WORLD_EXECUTE)

# ==============================================
# Installs a library and any of its needed symlink variants.
# ==============================================

function(THIRD_PARTY_INSTALL_LIBRARY LIBFILE)

  if(WIN32)
        if(NOT EXISTS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ThirdParty)
            file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ThirdParty)
        endif()

        cmake_path(SET tmpLIBFILE ${LIBFILE})
        cmake_path(GET tmpLIBFILE EXTENSION LAST_ONLY LIBEXT)
        file(REAL_PATH ${tmpLIBFILE} LIBREALPATH)
        cmake_path(GET LIBREALPATH PARENT_PATH curPATH)
        cmake_path(GET LIBREALPATH FILENAME realNAME)
        string(REPLACE ${LIBEXT} "" curNAMEWE ${realNAME})
        set(curNAME "${curPATH}/${curNAMEWE}")
        set(dllNAME "${curNAME}.dll")
        set(libNAME "${curNAME}.lib")
        if(EXISTS ${dllNAME})
            install(FILES ${dllNAME}
                DESTINATION ${VISIT_INSTALLED_VERSION_BIN}
                PERMISSIONS ${VISIT_TP_PERMS}
                )
            # On Windows, we also need to copy the file to the
            # binary dir so our out of source builds can run.
            execute_process(COMMAND ${CMAKE_COMMAND} -E copy
                            ${dllNAME}
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ThirdParty)
        else() # try 'bin' directory
            file(REAL_PATH "${curPATH}/../bin/${curNAMEWE}" dll_path)
            set(newdllNAME "${dll_path}.dll")
            if(EXISTS ${newdllNAME})
                install(FILES ${newdllNAME}
                    DESTINATION ${VISIT_INSTALLED_VERSION_BIN}
                    PERMISSIONS ${VISIT_TP_PERMS}
                    )
                # On Windows, we also need to copy the file to the
                # binary dir so our out of source builds can run.
                execute_process(COMMAND ${CMAKE_COMMAND} -E copy
                                ${newdllNAME}
                                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ThirdParty)
            endif()
        endif()

        if(VISIT_INSTALL_THIRD_PARTY AND EXISTS ${libNAME})
            # also install the import libraries
            INSTALL(FILES ${libNAME}
                DESTINATION ${VISIT_INSTALLED_VERSION_LIB}
                PERMISSIONS ${VISIT_TP_PERMS}
                )
        endif()

  else(WIN32)

    cmake_path(SET tmpLIBFILE ${LIBFILE})
    cmake_path(GET tmpLIBFILE EXTENSION LAST_ONLY LIBEXT)
    if(NOT ${LIBEXT} STREQUAL ".a")
        set(isSHAREDLIBRARY "YES")
    else()
        set(isSHAREDLIBRARY "NO")
    endif()

    if(${isSHAREDLIBRARY} STREQUAL "YES")
        file(REAL_PATH ${tmpLIBFILE} LIBREALPATH)
        ## message("***tmpLIBFILE=${tmpLIBFILE}, LIBREALPATH=${LIBREALPATH}")
        if(NOT ${tmpLIBFILE} STREQUAL ${LIBREALPATH})
            # We need to install a library and its symlinks
            cmake_path(GET LIBREALPATH PARENT_PATH curPATH)
            if((NOT ${curPATH} STREQUAL "/usr/lib") AND
               (NOT ${curPATH} MATCHES "^\\/opt\\/local\\/lib.*") AND
               (NOT ${curPATH} MATCHES "^\\/System\\/Library\\/Frameworks\\/.*") AND
               (NOT ${curPATH} MATCHES "^\\/Library\\/Frameworks\\/.*"))
                # Extract proper base name by comparing the input lib path w/ the real path.
                cmake_path(GET LIBREALPATH FILENAME realNAME)
                cmake_path(GET tmpLIBFILE FILENAME inptNAME)
                string(REPLACE ${LIBEXT} "" inptNAME ${inptNAME})
                string(REPLACE ${inptNAME} "" curEXT ${realNAME})
                # We will have a "." at the end of the string, remove it
                string(REGEX REPLACE "\\.$" "" inptNAME ${inptNAME})
                string(REPLACE "." ";" extList ${curEXT})
                set(curNAME "${curPATH}/${inptNAME}")
                # Come up with all of the possible library and symlink names
                set(allNAMES "${curNAME}${LIBEXT}")
                set(allNAMES ${allNAMES} "${curNAME}${LIBEXT}.1") # seems to be a standard linux-ism that isn't always covered by the foreach-loop on ${extList}
                set(allNAMES ${allNAMES} "${curNAME}.a")
                foreach(X ${extList})
                    set(curNAME "${curNAME}.${X}")
                    set(allNAMES ${allNAMES} "${curNAME}")           # Linux way
                    set(allNAMES ${allNAMES} "${curNAME}${LIBEXT}")  # Mac way
                endforeach()

                list(REMOVE_DUPLICATES allNAMES)

                # Add the names that exist to the install.
                foreach(curNAMEWithExt ${allNAMES})
                    ## message("** Checking ${curNAMEWithExt}")
                    if(EXISTS ${curNAMEWithExt})
                        ## message("** Need to install ${curNAMEWithExt}")
                        if(IS_DIRECTORY ${curNAMEWithExt})
                            # It is a framework, install as a directory
                            install(DIRECTORY ${curNAMEWithExt}
                                DESTINATION ${VISIT_INSTALLED_VERSION_LIB}
                                DIRECTORY_PERMISSIONS ${VISIT_TP_PERMS}
                                FILE_PERMISSIONS ${VISIT_TP_PERMS}
                            )
                        else()
                            install(FILES ${curNAMEWithExt}
                                DESTINATION ${VISIT_INSTALLED_VERSION_LIB}
                                PERMISSIONS ${VISIT_TP_PERMS}
                            )
                        endif()
                    endif(EXISTS ${curNAMEWithExt})
                endforeach(curNAMEWithExt)
            endif()
        else(NOT ${tmpLIBFILE} STREQUAL ${LIBREALPATH})
            cmake_path(GET LIBREALPATH PARENT_PATH curPATH)
            if((NOT ${curPATH} STREQUAL "/usr/lib") AND
               (NOT ${curPATH} MATCHES "^\\/opt\\/local\\/lib.*") AND
               (NOT ${curPATH} MATCHES "^\\/System\\/Library\\/Frameworks\\/.*") AND
               (NOT ${curPATH} MATCHES "^\\/Library\\/Frameworks\\/.*"))
                # We need to install just the library
                if(IS_DIRECTORY ${tmpLIBFILE})
                    # It is a framework, install as a directory.
                    install(DIRECTORY ${tmpLIBFILE}
                        DESTINATION ${VISIT_INSTALLED_VERSION_LIB}
                        DIRECTORY_PERMISSIONS ${VISIT_TP_PERMS}
                        FILE_PERMISSIONS ${VISIT_TP_PERMS}
                        PATTERN "Qt*_debug" EXCLUDE # Exclude Qt*_debug libraries in framework.
                    )
                else()
                    # Create an install target for just the library file
                    install(FILES ${tmpLIBFILE}
                        DESTINATION ${VISIT_INSTALLED_VERSION_LIB}
                        PERMISSIONS ${VISIT_TP_PERMS})
                endif()
#            message("**We need to install lib ${tmpLIBFILE}")
            endif()
        endif(NOT ${tmpLIBFILE} STREQUAL ${LIBREALPATH})
    else(${isSHAREDLIBRARY} STREQUAL "YES")
        # We have a .a that we need to install to archives.
        if(VISIT_INSTALL_THIRD_PARTY)
#            message("***INSTALL ${LIBFILE} to ${VISIT_INSTALLED_VERSION_ARCHIVES}")
            install(FILES ${tmpLIBFILE}
                DESTINATION ${VISIT_INSTALLED_VERSION_ARCHIVES}
                PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ
            )

            # TODO: We could install windows import libraries here...

        endif(VISIT_INSTALL_THIRD_PARTY)
    endif(${isSHAREDLIBRARY} STREQUAL "YES")
  endif(WIN32)
endfunction()

# ==============================================
#  Installs a library's includes.
# ==============================================

function(THIRD_PARTY_INSTALL_INCLUDE pkg incdir)
        if(VISIT_INSTALL_THIRD_PARTY AND NOT VISIT_HEADERS_SKIP_INSTALL)
            string(TOLOWER ${pkg} lcpkg)
#            message("***INSTALL ${incdir} -> ${VISIT_INSTALLED_VERSION_INCLUDE}/${lcpkg}")
            install(DIRECTORY ${incdir}
                DESTINATION ${VISIT_INSTALLED_VERSION_INCLUDE}/${lcpkg}
                DIRECTORY_PERMISSIONS ${VISIT_TP_PERMS}
                FILE_PERMISSIONS ${VISIT_TP_PERMS}
                FILES_MATCHING
                PATTERN "*.h"
                PATTERN "*.H"
                PATTERN "*.hpp"
                PATTERN "*.hxx"
                PATTERN "*.HPP"
                PATTERN "*.inc"
                PATTERN "*.inl"
                PATTERN ".svn" EXCLUDE
            )
        endif()
endfunction()

# ==============================================
# Function for checking and importing TP libs
# ==============================================

#[=[
  Creates an import library with name ${pkg} (in lower case).
  Cache variables created:
      x_FOUND

  Assumes headers are in ${x_DIR}/include unless INCDIR argument is used
  Assumes libs are in ${x_DIR}/lib64 or ${x_DIR}/lib unless LIBDIR argument is used
  Either LIBS or LIBNAMES must be specified.
  LIBS is used for multiple-library packages.
  LIBNAMES is used for single-library packges if the names may be different depending on platform/version.

  pkg is the name used to specify the x_DIR (generally upper case name of pkg)

  keyword arguments:
      HEADER_ONLY optional specifies HEADER_ONLY package
      LIBS (required if HEADER_ONLY not specified AND LIBNAMES not specified)
             is the list of library names for this package
      LIBNAMES List of possible names for a library
      LIBDIR (optional) are the paths beyond x_DIR where the libs may be found.
      INCDIR (optional) is the path beyond x_DIR where the includes may be found.
      DEFINES (optional) compile definitions
      WIN32DEFINES (optional) compile definitions to be used on Windows.

  Uses path specified by pkg_DIR as base path for the files

#]=]
# ==============================================

function(visit_import_third_party pkg)
    message(STATUS "Looking for ${pkg}")

    # If this package wasn't requested, its ok, just return.
    if(NOT ${pkg}_DIR)
        message(STATUS "${pkg} not requested")
        return()
    endif()

    # If base dir doesn't exist, we can go no further.
    if(NOT EXISTS ${${pkg}_DIR})
        if(IGNORE_THIRD_PARTY_LIB_PROBLEMS)
            message(STATUS "\n** \n** \n** Base Directory for ${pkg} ${${pkg}_DIR} does not exist.\n**\n**")
        else()
            message(FATAL_ERROR "  Base Directory for ${pkg} ${${pkg}_DIR} does not exist.")
        endif()
        return()
    endif()

    cmake_parse_arguments(PARSE_ARGV 1 vitp "HEADER_ONLY" "LIBDIR;INCDIR" "LIBS;LIBNAMES;DEFINES;WIN32DEFINES")

    if(NOT ${vitp_HEADER_ONLY} AND
       NOT DEFINED vitp_LIBNAMES AND
       NOT DEFINED vitp_LIBS)
        if(IGNORE_THIRD_PARTY_LIB_PROBLEMS)
            message(STATUS "  One of LIBS or LIBNAMES for ${pkg} must be provided to visit_import_third_party")
        else()
            message(FATAL_ERROR "  One of LIBS or LIBNAMES for ${pkg} must be provided to visit_import_third_party.")
        endif()
    endif()


    if(DEFINED vitp_LIBNAMES AND DEFINED vitp_LIBS)
        if(IGNORE_THIRD_PARTY_LIB_PROBLEMS)
            message(STATUS "  Both LIBS and LIBNAMES for ${pkg} were provided to visit_import_third_party, use only one.")
        else()
            message(FATAL_ERROR "  Both LIBS and LIBNAMES for ${pkg} were provided to visit_import_third_party, use only one.")
        endif()
    endif()

    if(DEFINED vitp_LIBDIR)
        set(libdirs ${vitp_LIBDIR})
    else()
        set(libdirs lib lib64)
    endif()

    if(DEFINED vitp_INCDIR)
        foreach(inc ${vitp_INCDIR})
            list(APPEND _${pkg}_INCLUDE_DIR ${${pkg}_DIR}/${inc})
        endforeach()
    else()
        set(_${pkg}_INCLUDE_DIR ${${pkg}_DIR}/include)
    endif()

    # lower case package name to be used as imported target name
    string(TOLOWER ${pkg} LNAME)


    if(${vitp_HEADER_ONLY})
        if(EXISTS ${_${pkg}_INCLUDE_DIR})
            blt_import_library(
                NAME        ${LNAME}
                INCLUDES    $<BUILD_INTERFACE:${_${pkg}_INCLUDE_DIR}>
                            $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/${LNAME}/include>
                EXPORTABLE  ON)
            visit_install_export_targets(${LNAME})
            if(${VISIT_${pkg}_SKIP_INSTALL})
                message(STATUS "Skipping installation of ${pkg}")
             else()
                 THIRD_PARTY_INSTALL_INCLUDE(${pkg} ${_${pkg}_INCLUDE_DIR})
             endif()
             set(HAVE_${pkg} TRUE CACHE BOOL "Have ${LNAME} headers")
        else()
            if(IGNORE_THIRD_PARTY_LIB_PROBLEMS)
                message(STATUS "  Include directory for header-only ${pkg} does not exist (${_${pkg}_INCLUDE_DIR})")
            else()
                message(FATAL_ERROR "  Include directory for header-only ${pkg} does not exist (${_${pkg}_INCLUDE_DIR})")
            endif()
        endif()
        return()
    endif()

    set(tplibs)
    if(DEFINED vitp_LIBNAMES)
        find_library(_${pkg}_LIBRARY
                     NAMES ${vitp_LIBNAMES}
                     PATHS ${${pkg}_DIR}
                     PATH_SUFFIXES ${libdirs}
                     NO_DEFAULT_PATH)

        if(_${pkg}_LIBRARY)
            list(APPEND tplibs _${pkg}_LIBRARY)
        endif()
    else()
        set(tplibs)
        foreach (X ${vitp_LIBS})
            find_library(_${pkg}_${X}
                         NAMES ${X}
                         PATHS ${${pkg}_DIR}
                         PATH_SUFFIXES ${libdirs}
                         NO_DEFAULT_PATH)
            if(_${pkg}_${X})
                list(APPEND tplibs _${pkg}_${X})
                message(STATUS "  Found library ${X} in one of ${libdirs}")
            else()
                message(FATAL_ERROR "Library ${X} not found in one of ${libdirs}")
                if(IGNORE_THIRD_PARTY_LIB_PROBLEMS)
                    message(STATUS "\n** \n** \n** Library ${X} not found in ${lib_dir}.\n**\n**")
                else()
                    message(FATAL_ERROR "Library ${X} not found in ${lib_dir}")
                endif()
                return()
            endif()
        endforeach()
     endif()

     find_package_handle_standard_args(${pkg} DEFAULT_MSG
            _${pkg}_INCLUDE_DIR
            ${tplibs})

    if(${pkg}_FOUND)
        set(HAVE_${pkg} TRUE CACHE BOOL "Have ${LNAME} libraries")

        # create a list of libs using BUILD_INTERFACE
        set(buildlibs)
        foreach(lib ${tplibs})
            # lib is a cmake var, so need double-indirection to get at the value
            list(APPEND buildlibs $<BUILD_INTERFACE:${${lib}}>)
        endforeach()

        blt_import_library(
            NAME        ${LNAME}
            INCLUDES    $<BUILD_INTERFACE:${_${pkg}_INCLUDE_DIR}>
                        $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/${LNAME}/include>
            LIBRARIES   ${buildlibs}
            EXPORTABLE  ON)

        if(DEFINED vitp_DEFINES)
            target_compile_definitions(${LNAME} INTERFACE ${vitp_DEFINES})
        endif()

        if(WIN32 AND DEFINED vitp_WIN32DEFINES)
            target_compile_definitions(${LNAME} INTERFACE ${vitp_WIN32DEFINES})
        endif()

        if(${pkg}_LIBDEP)
            message(STATUS "  Looking for dependent libraries for ${pkg}")

            # all dependencies must already have been declared as import targets
            # or full-path to a library
            # they aren't installed by the logic below, only added to the
            # interface of the current library being imported.
            foreach (X ${${pkg}_LIBDEP})
                if(TARGET ${X})
                    message(STATUS "    found target for ${X}")
                    target_link_libraries(${LNAME} INTERFACE
                        $<BUILD_INTERFACE:${X}>
                        $<INSTALL_INTERFACE:${X}>)
                elseif(EXISTS ${X})
                    message(STATUS "    found full path: ${X}")
                    target_link_libraries(${LNAME} INTERFACE
                        $<BUILD_INTERFACE:${X}>)
                else()
                    message(FATAL_ERROR "Dependent lib ${X} is not an imported target, nor is it a full path to the library.")
                endif()
            endforeach()
        endif()


        if(VISIT_INSTALL_THIRD_PARTY)
            # install
            if(${VISIT_${pkg}_SKIP_INSTALL})
                message(STATUS "Skipping installation of ${pkg}")
            else()

                # TODO:  this logic is intended for third-party libraries
                # that were built by build_visit and are installed alongside
                # VisIt, not for system libraries or ThirdParty libraries
                # that aren't in the general VisIt-specified location.
                # 
                # Need a mechanism to bypass the below install commands
                # but still be able to find all necessary dependencies
                # during an 'import visit' or building against an installed
                # version of VisIt

               
                # Install libs and headers
                foreach(lib ${tplibs})
                    cmake_path(SET tmplib ${${lib}})
                    cmake_path(GET tmplib EXTENSION LAST_ONLY LIBEXT)
                    if(${LIBEXT} STREQUAL ".a")
                        set(dest ${VISIT_INSTALLED_VERSION_ARCHIVES})
                    else()
                        set(dest ${VISIT_INSTALLED_VERSION_LIB})
                    endif()
                    cmake_path(GET tmplib FILENAME _lib)
                   # add INSTALL_INTERFACE
                   target_link_libraries(${LNAME} INTERFACE
                       $<INSTALL_INTERFACE:\${_IMPORT_PREFIX}/${dest}/${_lib}>)

                    THIRD_PARTY_INSTALL_LIBRARY(${${lib}})
                endforeach()
                THIRD_PARTY_INSTALL_INCLUDE(${pkg} ${_${pkg}_INCLUDE_DIR})

                # ensure the tp libs are added to visit's export set.
                visit_install_export_targets(${LNAME})

            endif()
        endif()
        set(${pkg}_FOUND true CACHE BOOL "${pkg} library found" FORCE)
    endif()
endfunction()


# ==============================================
# Installs a library's executables.
# ==============================================

function(THIRD_PARTY_INSTALL_EXECUTABLE)
    foreach(exe ${ARGN})
        if(EXISTS ${exe})
            install(PROGRAMS ${exe}
               DESTINATION ${VISIT_INSTALLED_VERSION_BIN}
               PERMISSIONS ${VISIT_TP_PERMS}
            )
        endif(EXISTS ${exe})
    endforeach(exe ${ARGN})
endfunction()

# ==============================================
# Find TP libs
# ==============================================

if(NOT VISIT_DBIO_ONLY)
  include(${VISIT_SOURCE_DIR}/CMake/VisItOpenGL.cmake)
endif()

include(${VISIT_SOURCE_DIR}/CMake/FindNektar++.cmake)

include(${VISIT_SOURCE_DIR}/CMake/FindVisItDamaris.cmake)

include(${VISIT_SOURCE_DIR}/CMake/FindVisItBoost.cmake)

if((VISIT_PYTHON_SCRIPTING OR VISIT_PYTHON_FILTERS) AND NOT VISIT_DBIO_ONLY)
    include(${VISIT_SOURCE_DIR}/CMake/FindVisItPython.cmake)
    if(NOT PYTHON_FOUND)
      message(FATAL_ERROR "Python support requested, but could not find Python "
                          "(PYTHON_FOUND=FALSE) "
                         "(VISIT_PYTHON_SCRIPTING=${VISIT_PYTHON_SCRIPTING} "
                          " VISIT_PYTHON_FILTERS=${VISIT_PYTHON_FILTERS})")
    endif()
endif()

if(VISIT_JAVA)
    include(${CMAKE_ROOT}/Modules/FindJava.cmake)
    enable_language(Java)
endif()

# These need to come before VTK/Qt because they depend on it.

include(${VISIT_SOURCE_DIR}/CMake/FindZlib.cmake)
include(${VISIT_SOURCE_DIR}/CMake/FindOSPRay.cmake)
include(${VISIT_SOURCE_DIR}/CMake/FindANARI.cmake)
include(${VISIT_SOURCE_DIR}/CMake/FindJPEG.cmake)
include(${VISIT_SOURCE_DIR}/CMake/FindSzip.cmake)
include(${VISIT_SOURCE_DIR}/CMake/FindTiff.cmake)


# Configure Qt and Qwt support.
if(NOT VISIT_DBIO_ONLY AND NOT VISIT_ENGINE_ONLY AND NOT VISIT_SERVER_COMPONENTS_ONLY)
    include(${VISIT_SOURCE_DIR}/CMake/FindVisItQt.cmake)
    include(${VISIT_SOURCE_DIR}/CMake/FindQwt.cmake)
endif()

include(${VISIT_SOURCE_DIR}/CMake/FindVisItVTK.cmake)

# Configure PySide Support
# Commented out until #19539 is addressed
#if(VISIT_PYTHON_SCRIPTING AND PYTHONLIBS_FOUND
#                          AND NOT VISIT_DBIO_ONLY
#                          AND NOT VISIT_ENGINE_ONLY
#                          AND NOT VISIT_SERVER_COMPONENTS_ONLY)
#    include(${VISIT_SOURCE_DIR}/CMake/FindPySide.cmake)
#endif()

# Include Ice-T support if we can.
if(VISIT_PARALLEL)
    INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindIceT.cmake)
endif(VISIT_PARALLEL)

if(NOT VISIT_BUILD_MINIMAL_PLUGINS OR VISIT_SELECTED_DATABASE_PLUGINS)
    # HDF5 is found first, since other libraries may depend on it.
    include(${VISIT_SOURCE_DIR}/CMake/FindHDF5.cmake)

    include(${VISIT_SOURCE_DIR}/CMake/FindADIOS.cmake)

    # adios2 needs blosc2
    include(${VISIT_SOURCE_DIR}/CMake/FindBlosc2.cmake)
    include(${VISIT_SOURCE_DIR}/CMake/FindADIOS2.cmake)

    include(${VISIT_SOURCE_DIR}/CMake/FindADVIO.cmake)


    include(${VISIT_SOURCE_DIR}/CMake/FindBoxlib.cmake)

    include(${VISIT_SOURCE_DIR}/CMake/FindCFITSIO.cmake)

    include(${VISIT_SOURCE_DIR}/CMake/FindCGNS.cmake)

    # conduit needs silo
    include(${VISIT_SOURCE_DIR}/CMake/FindSilo.cmake)
    include(${VISIT_SOURCE_DIR}/CMake/FindConduit.cmake)

    include(${VISIT_SOURCE_DIR}/CMake/FindFMS.cmake)

    include(${VISIT_SOURCE_DIR}/CMake/FindGDAL.cmake)

    include(${VISIT_SOURCE_DIR}/CMake/FindH5Part.cmake)

    include(${VISIT_SOURCE_DIR}/CMake/FindMOAB.cmake)

    include(${VISIT_SOURCE_DIR}/CMake/FindMFEM.cmake)

    include(${VISIT_SOURCE_DIR}/CMake/FindMili.cmake)

    include(${VISIT_SOURCE_DIR}/CMake/FindNetcdf.cmake)

    include(${VISIT_SOURCE_DIR}/CMake/FindOpenEXR.cmake)


    include(${VISIT_SOURCE_DIR}/CMake/FindXdmf.cmake)

    include(${VISIT_SOURCE_DIR}/CMake/FindMDSplus.cmake)

    include(${VISIT_SOURCE_DIR}/CMake/FindUintah.cmake)

    include(${VISIT_SOURCE_DIR}/CMake/FindPIDX.cmake)

    include(${VISIT_SOURCE_DIR}/CMake/FindVTKm.cmake)
endif()

unset(VISIT_TP_PERMS)
