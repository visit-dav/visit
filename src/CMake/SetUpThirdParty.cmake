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
#****************************************************************************/

# ==============================================
# Functions for checking and installing TP libs
# ==============================================

# ==============================================
#[=[
  Sets up cache variables for a third_party_package.
      x_INCLUDE_DIR
      x_LIBRARY_DIR
      x_LIB
      x_FOUND

  Assumes headers are in ${x_DIR}/include unless INCDIR argument is used
  Assumes libs are in ${x_DIR}/lib64 or ${x_DIR}/lib unless LIBDIR argument is used

  pkg is the name used to specify the x_DIR (generally upper case name of pkg)

  keyword arguments:
      LIBS (required) is the list of library names for this package
      LIBDIR (optional) are the paths beyond x_DIR where the libs may be found.
      INCDIR (optional) is the path beyond x_DIR where the includes may be found.

  Uses path specified by pkg_DIR as base path for the files

#]=]
# ==============================================

function(SET_UP_THIRD_PARTY pkg)
    message(STATUS "Looking for ${pkg}")
    set(base_dir "${pkg}_DIR")
    set(base_dir_val "${${base_dir}}")
    set(base_dir_NF "VISIT_${base_dir}-NOTFOUND")

    # If this package wasn't requested, thats ok so just return.
    if ("${base_dir_val}" STREQUAL "" OR "${base_dir_val}" STREQUAL "${base_dir_NF}")
        message(STATUS "  ${pkg} not requested")
        RETURN()
    endif ("${base_dir_val}" STREQUAL "" OR "${base_dir_val}" STREQUAL "${base_dir_NF}")

    # If base dir doesn't exist, we can't go further.
    if (NOT (EXISTS "${base_dir_val}"))
        if(IGNORE_THIRD_PARTY_LIB_PROBLEMS)
            message(STATUS "\n** \n** \n** Base Directory for ${pkg} ${base_dir_val} does not exist.\n**\n**")
        else(IGNORE_THIRD_PARTY_LIB_PROBLEMS)
            message(FATAL_ERROR "  Base Directory for ${pkg} ${base_dir_val} does not exist.")
        endif(IGNORE_THIRD_PARTY_LIB_PROBLEMS)
        RETURN()
    endif (NOT (EXISTS "${base_dir_val}"))


    cmake_parse_arguments(PARSE_ARGV 1 sutp "" "LIBDIR;INCDIR" "LIBS")
    if(NOT DEFINED sutp_LIBS)
        if(IGNORE_THIRD_PARTY_LIB_PROBLEMS)
            message(STATUS "  LIBS for ${pkg} have not been defined.")
        else()
            message(FATAL_ERROR "  LIBS for ${pkg} have not been defined.")
        endif()
    else()
        set(libs ${sutp_LIBS})
    endif()


    if(DEFINED sutp_LIBDIR)
        set(libdir ${sutp_LIBDIR})
    else()
        if(EXISTS ${base_dir_val}/lib64)
            set(libdir "lib64")
        else()
            set(libdir "lib")
        endif()
    endif()

    if(DEFINED sutp_INCDIR)
        set(incdir ${sutp_INCDIR})
    else()
        set(incdir "include")
    endif()

    set(inc_dir_var "${pkg}_INCLUDE_DIR")
    set(lib_dir_var "${pkg}_LIBRARY_DIR")
    set(lib_var "${pkg}_LIB")
    set(tp_found "${pkg}_FOUND")
    set(have_tp "HAVE_LIB${pkg}")
    set(lib_skip_install "VISIT_${pkg}_SKIP_INSTALL")

    #
    # Zero out lib names b/c they may be hanging around from a previous
    # configure.
    #
    unset("${tp_found}")
    unset("${have_tp}")
    set("${lib_var}" "")
    set("${inc_dir_var}" "${base_dir_val}/${incdir}")

    if(NOT EXISTS ${${inc_dir_var}})
        if(IGNORE_THIRD_PARTY_LIB_PROBLEMS)
            message(STATUS "\n** \n** \n** Include Directory for ${pkg} (${${inc_dir_var}}) does not exist.\n**\n**")
        else(IGNORE_THIRD_PARTY_LIB_PROBLEMS)
            message(FATAL_ERROR "  Include Directory for ${pkg} (${${inc_dir_var}}) does not exist.")
        endif(IGNORE_THIRD_PARTY_LIB_PROBLEMS)
        return()
    endif(NOT EXISTS ${${inc_dir_var}})
    ##
    # Set lib dir to first existing of base_dir/libdir
    # Minimal intrusion on existing logic, but one might wnat
    # to allow libs to exist in multiple directories, as
    # they do for Qt.
    ##
    set(${lib_dir_var} "")
    if(NOT "${libs}" STREQUAL "NO_LIBS")
        foreach(X ${libdir})
            if(EXISTS ${base_dir_val}/${X} AND "${${lib_dir_var}}" STREQUAL "")
                set(${lib_dir_var} ${base_dir_val}/${X})
            endif()
        endforeach(X)
    endif()

    #
    # If the library is NO_LIBS then header only installation
    #
    if("${libs}" STREQUAL "NO_LIBS")
        if(${${lib_skip_install}})
            message(STATUS "Skipping installation of ${pkg}")
        else(${${lib_skip_install}})
            THIRD_PARTY_INSTALL_INCLUDE(${pkg} ${${inc_dir_var}})
        endif(${${lib_skip_install}})
    else("${libs}" STREQUAL "NO_LIBS")
        #
        # If non empty string, lib_dir was found
        #
        if("${${lib_dir_var}}" STREQUAL "")
            if(IGNORE_THIRD_PARTY_LIB_PROBLEMS)
                message(STATUS "\n** \n** \n** None of library directories for ${pkg} (${base_dir_val}/${libdir}) exist.\n**\n**")
            else(IGNORE_THIRD_PARTY_LIB_PROBLEMS)
                message(FATAL_ERROR "   None of library directories for ${pkg} (${base_dir_val}/${libdir}) exist.\n**\n**")
            endif(IGNORE_THIRD_PARTY_LIB_PROBLEMS)
            return()
        endif()

        # If the inc and lib directories are different then attempt to
        # install the include directory.
        if(${${lib_skip_install}})
            message(STATUS "Skipping installation of ${pkg}")
        else(${${lib_skip_install}})
            if(NOT ${${inc_dir_var}} STREQUAL ${${lib_dir_var}})
                THIRD_PARTY_INSTALL_INCLUDE(${pkg} ${${inc_dir_var}})
            endif(NOT ${${inc_dir_var}} STREQUAL ${${lib_dir_var}})
        endif(${${lib_skip_install}})

        foreach (X ${libs})
            find_library(full_lib_path ${X}
                         PATHS ${${lib_dir_var}}
                         NO_DEFAULT_PATH
                         NO_CMAKE_ENVIRONMENT_PATH
                         NO_CMAKE_PATH
                         NO_SYSTEM_ENVIRONMENT_PATH
                         NO_CMAKE_SYSTEM_PATH)
            if(full_lib_path)
                if(${${lib_skip_install}})
                    message(STATUS "Skipping installation of ${full_lib_path}")
                else(${${lib_skip_install}})
                    if(NOT ${lib_dir_var} STREQUAL "/usr/lib")
                        THIRD_PARTY_INSTALL_LIBRARY(${full_lib_path})
                    endif()
                endif(${${lib_skip_install}})
                get_filename_component(alib ${full_lib_path} NAME)
                if(SET_UP_THIRD_PARTY_FULL_PATH)
                    list(APPEND "${lib_var}" ${full_lib_path})
                else(SET_UP_THIRD_PARTY_FULL_PATH)
                    list(APPEND "${lib_var}" ${alib})
                endif(SET_UP_THIRD_PARTY_FULL_PATH)
                message(STATUS "  Found library ${X} in ${${lib_dir_var}}")
            else(full_lib_path)
                if(IGNORE_THIRD_PARTY_LIB_PROBLEMS)
                    message(STATUS "\n** \n** \n** Library ${X} not found in ${${lib_dir_var}}.\n**\n**")
                else(IGNORE_THIRD_PARTY_LIB_PROBLEMS)
                    message(FATAL_ERROR "Library ${X} not found in ${${lib_dir_var}}")
                endif(IGNORE_THIRD_PARTY_LIB_PROBLEMS)
                RETURN()
            endif(full_lib_path)
            unset(full_lib_path CACHE)
        endforeach (X ${all_libs})

        set(lib_dep "${pkg}_LIBDEP")
        if(NOT "${${lib_dep}}" STREQUAL "")
            message(STATUS "  Looking for dependent libraries for ${pkg}")
        endif(NOT "${${lib_dep}}" STREQUAL "")

        #
        # This alternates between a reading path & a lib from ${pkg}_LIBDEP
        #
        set(is_lib_path 1)
        foreach (X ${${lib_dep}})
            if(${X})
                set(X_VALUE ${${X}})
            else()
                set(X_VALUE ${X})
            endif()
            if(is_lib_path)
                set(is_lib_path 0)
                list(APPEND "${lib_dir_var}" ${X_VALUE})
                set(current_lib_dir ${X_VALUE})
            else(is_lib_path)
                set(is_lib_path 1)
                find_library(full_lib_path
                             NAMES ${X_VALUE}
                             PATHS ${current_lib_dir}
                             NO_DEFAULT_PATH
                             NO_CMAKE_ENVIRONMENT_PATH
                             NO_CMAKE_PATH
                             NO_SYSTEM_ENVIRONMENT_PATH
                             NO_CMAKE_SYSTEM_PATH)
                if(full_lib_path)
                    if(${${lib_skip_install}})
                        message(STATUS "Skipping installation of ${full_lib_path}")
                    else(${${lib_skip_install}})
                        if(NOT "${current_lib_dir}" STREQUAL "/usr/lib")
                            THIRD_PARTY_INSTALL_LIBRARY(${full_lib_path})
                        endif(NOT "${current_lib_dir}" STREQUAL "/usr/lib")
                    endif(${${lib_skip_install}})
                    get_filename_component(alib ${full_lib_path} NAME)
                    list(APPEND "${lib_var}" ${alib})
                    message(STATUS "     Found library ${X_VALUE} in ${current_lib_dir}")
                else(full_lib_path)
                    if(IGNORE_THIRD_PARTY_LIB_PROBLEMS)
                        message(STATUS "\n** \n** \n**     Library ${X_VALUE} not found in ${current_lib_dir}.\n**\n**")
                    else()
                        message(FATAL_ERROR "     Library ${X_VALUE} not found in ${current_lib_dir}")
                    endif()
                    return()
                endif(full_lib_path)
                unset(full_lib_path CACHE)
            endif(is_lib_path)
        endforeach(X ${${lib_dep}})

        #
        # Look for include dependencies, if any
        #
        set(inc_dep "${pkg}_INCDEP")
        if(NOT "${${inc_dep}}" STREQUAL "")
            message(STATUS "  Looking for dependent includes for ${pkg}")
        endif()

        foreach(X ${${inc_dep}})
            message(STATUS "looking for include dependency: ${X}")
            if(EXISTS ${${X}})
                list(APPEND ${inc_dir_var} "${${X}}")
            endif()
        endforeach()
    endif("${libs}" STREQUAL "NO_LIBS")

    # Cache final results
    set("${tp_found}" true CACHE BOOL "${pkg} library found" FORCE)
    set("${have_tp}"  true CACHE BOOL "Have ${pkg} library" FORCE)
    set("${base_dir}"    ${${base_dir}}    CACHE PATH   "${pkg} base directory" FORCE)
    set("${inc_dir_var}" ${${inc_dir_var}} CACHE PATH   "${pkg} include directory" FORCE)

    if(NOT "${libs}" STREQUAL "NO_LIBS")
        set("${lib_dir_var}" ${${lib_dir_var}} CACHE PATH   "${pkg} library directory" FORCE)
        set("${lib_var}"     ${${lib_var}}     CACHE STRING "${pkg} library" FORCE)
    endif()

    mark_as_advanced("${tp_found}"
                     "${base_dir}"
                     "${inc_dir_var}"
                     "${lib_dir_var}"
                     "${lib_var}")

    if(NOT "${libs}" STREQUAL "NO_LIBS")
      message(STATUS "  ${pkg} found")
    else()
      message(STATUS "  ${pkg} found - headers only - no libs")
    endif()

endfunction()


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
endfunction(THIRD_PARTY_INSTALL_LIBRARY)

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
endfunction(THIRD_PARTY_INSTALL_INCLUDE)

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
include(${VISIT_SOURCE_DIR}/CMake/FindJPEG.cmake)
include(${VISIT_SOURCE_DIR}/CMake/FindSzip.cmake)
if(VTK_VERSION VERSION_GREATER_EQUAL "9.1.0")
    include(${VISIT_SOURCE_DIR}/CMake/FindTiff.cmake)
endif()


# Configure Qt and Qwt support.
if(NOT VISIT_DBIO_ONLY AND NOT VISIT_ENGINE_ONLY AND NOT VISIT_SERVER_COMPONENTS_ONLY)
    include(${VISIT_SOURCE_DIR}/CMake/FindVisItQt.cmake)
    include(${VISIT_SOURCE_DIR}/CMake/FindQwt.cmake)
endif()

include(${VISIT_SOURCE_DIR}/CMake/FindVisItVTK.cmake)

# Configure PySide Support
if(VTK_VERSION VERSION_EQUAL "8.1.0")
  if(VISIT_PYTHON_SCRIPTING AND PYTHONLIBS_FOUND
                          AND NOT VISIT_DBIO_ONLY
                          AND NOT VISIT_ENGINE_ONLY
                          AND NOT VISIT_SERVER_COMPONENTS_ONLY)
    include(${VISIT_SOURCE_DIR}/CMake/FindPySide.cmake)
  endif()
endif()

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

    include(${VISIT_SOURCE_DIR}/CMake/FindConduit.cmake)

    include(${VISIT_SOURCE_DIR}/CMake/FindFMS.cmake)

    include(${VISIT_SOURCE_DIR}/CMake/FindGDAL.cmake)

    include(${VISIT_SOURCE_DIR}/CMake/FindH5Part.cmake)

    include(${VISIT_SOURCE_DIR}/CMake/FindMOAB.cmake)

    include(${VISIT_SOURCE_DIR}/CMake/FindMFEM.cmake)

    include(${VISIT_SOURCE_DIR}/CMake/FindMili.cmake)

    include(${VISIT_SOURCE_DIR}/CMake/FindNetcdf.cmake)

    include(${VISIT_SOURCE_DIR}/CMake/FindOpenEXR.cmake)

    include(${VISIT_SOURCE_DIR}/CMake/FindSilo.cmake)

    include(${VISIT_SOURCE_DIR}/CMake/FindXdmf.cmake)

    include(${VISIT_SOURCE_DIR}/CMake/FindMDSplus.cmake)

    include(${VISIT_SOURCE_DIR}/CMake/FindUintah.cmake)

    include(${VISIT_SOURCE_DIR}/CMake/FindPIDX.cmake)

    if(VTK_VERSION VERSION_EQUAL "8.1.0")
        include(${VISIT_SOURCE_DIR}/CMake/FindVTKm.cmake)
    endif()

    include(${VISIT_SOURCE_DIR}/CMake/FindGFortran.cmake)
endif()

unset(VISIT_TP_PERMS)

