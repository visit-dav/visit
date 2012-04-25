#*****************************************************************************
#
# Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
# Produced at the Lawrence Livermore National Laboratory
# LLNL-CODE-442911
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
#   Eric Brugger, Tue Mar 16 12:42:01 PDT 2010
#   I modified the script to not install libraries located in /usr/lib.
#
#   Tom Fogal, Tue Mar 16 17:20:59 MDT 2010
#   Set the proper type for some MESSAGE calls which report errors.
#
#   Brad Whitlock, Mon Apr 18 15:18:08 PDT 2011
#   Added another check for xxx-NOTFOUND so we don't assume just because we
#   didn't specify a library like TCMALLOC that we have an error situation
#   when it does not exist.
#
#****************************************************************************/

INCLUDE(${VISIT_SOURCE_DIR}/CMake/ThirdPartyInstallLibrary.cmake)

#
# sets up cache variables for a third_party_package.
#  x_INCLUDE_DIR
#  x_LIBRARY_DIR
#  x_LIB
#  x_FOUND
#
#  assumes headers are in ${x_DIR}/${incextensiondir}
#  assumes libs are in ${x_DIR}/${libextensiondir}
#
#
#  pkg is the name used to specify the x_DIR (generally upper case name 
#  of pkg)
#  libdirextensions are the paths beyond x_DIR where the libs may be found.
#  incdirextension is the path beyond x_DIR where the includes may be found.
#  libs is the list of library names for this package 
#
#  uses path specified by pkg_DIR as base path for the files
#
#  Modfications:
#    Mark C. Miller, Thu Jul 29 23:39:05 PDT 2010
#    Adjusted to fail fatally when things cannot be found and to override
#    this failure behavior with option IGNORE_THIRD_PARTY_LIB_PROBLEMS.
#    Replaced multiple leves of indentation with inverted IF logic
#    and early returns. Got rid of setup_lib_error variable. Output
#    'not requested' message if pkg was not specified by user. Tried
#    to make status message for failure to find distinguishable in
#    cmake output from other messages.
#
FUNCTION(SET_UP_THIRD_PARTY pkg libdirextensions incdirextension libs)
  MESSAGE(STATUS "Looking for ${pkg}")
  SET(base_dir "${pkg}_DIR")
  SET(base_dir_val "${${base_dir}}")
  SET(base_dir_NF "VISIT_${base_dir}-NOTFOUND")

  # If this packaged wasn't requested, thats ok so just return.
  IF ("${base_dir_val}" STREQUAL "" OR "${base_dir_val}" STREQUAL "${base_dir_NF}")
    MESSAGE(STATUS "  ${pkg} not requested")
    RETURN()
  ENDIF ("${base_dir_val}" STREQUAL "" OR "${base_dir_val}" STREQUAL "${base_dir_NF}")

  # If base dir doesn't exist, we can't go further.
  IF (NOT (EXISTS "${base_dir_val}"))
    IF(IGNORE_THIRD_PARTY_LIB_PROBLEMS)
      MESSAGE(STATUS "\n** \n** \n** Base Directory for ${pkg} ${base_dir_val} does not exist.\n**\n**")
    ELSE(IGNORE_THIRD_PARTY_LIB_PROBLEMS)
      MESSAGE(FATAL_ERROR "  Base Directory for ${pkg} ${base_dir_val} does not exist.")
    ENDIF(IGNORE_THIRD_PARTY_LIB_PROBLEMS)
    RETURN()
  ENDIF (NOT (EXISTS "${base_dir_val}"))

  SET(inc_dir_var "${pkg}_INCLUDE_DIR")
  SET(lib_dir_var "${pkg}_LIBRARY_DIR")
  SET(lib_var "${pkg}_LIB")
  SET(tp_found "${pkg}_FOUND")
  SET(lib_skip_install "VISIT_${pkg}_SKIP_INSTALL")

  #
  # Zero out lib names b/c they may be hanging around from a previous 
  # configure.
  #
  UNSET("${tp_found}")
  SET("${lib_var}" "")

  SET("${inc_dir_var}" "${base_dir_val}/${incdirextension}")

  IF(NOT EXISTS ${${inc_dir_var}})
      IF(IGNORE_THIRD_PARTY_LIB_PROBLEMS)
          MESSAGE(STATUS "\n** \n** \n** Include Directory for ${pkg} (${${inc_dir_var}}) does not exist.\n**\n**")
      ELSE(IGNORE_THIRD_PARTY_LIB_PROBLEMS)
          MESSAGE(FATAL_ERROR "  Include Directory for ${pkg} (${${inc_dir_var}}) does not exist.")
      ENDIF(IGNORE_THIRD_PARTY_LIB_PROBLEMS)
      RETURN()
  ENDIF(NOT EXISTS ${${inc_dir_var}})
##
# Set lib dir to fitst existing of base_dir/extensions
# Minimal intrusion on existing logic, but one might wnat
# to allow libs to exist in multiple directories, as
# they do for Qt.
##
  SET(${lib_dir_var} "")
  FOREACH(X ${libdirextensions})
    IF(EXISTS ${base_dir_val}/${X} AND "${${lib_dir_var}}" STREQUAL "")
      SET(${lib_dir_var} ${base_dir_val}/${X})
    ENDIF()
  ENDFOREACH(X)

#
# If non empty string, lib_dir was found
#
  IF("${${lib_dir_var}}" STREQUAL "")
      IF(IGNORE_THIRD_PARTY_LIB_PROBLEMS)
          MESSAGE(STATUS "\n** \n** \n** None of library directories for ${pkg} (${base_dir_val}/${libdirextensions}) exist.\n**\n**")
      ELSE(IGNORE_THIRD_PARTY_LIB_PROBLEMS)
          MESSAGE(FATAL_ERROR "   None of library directories for ${pkg} (${base_dir_val}/${libdirextensions}) exist.\n**\n**")
      ENDIF(IGNORE_THIRD_PARTY_LIB_PROBLEMS)
      RETURN()
  ENDIF()

  # If the inc and lib directories are different then attempt to
  # install the include directory.
  IF(${${lib_skip_install}})
    MESSAGE(STATUS "Skipping installation of ${pkg}")
  ELSE(${${lib_skip_install}})
    IF(NOT ${${inc_dir_var}} STREQUAL ${${lib_dir_var}})
        THIRD_PARTY_INSTALL_INCLUDE(${pkg} ${${inc_dir_var}})
    ENDIF(NOT ${${inc_dir_var}} STREQUAL ${${lib_dir_var}})
  ENDIF(${${lib_skip_install}})

  SET(all_libs ${libs})
  FOREACH (X ${ARGN})
      SET(all_libs ${all_libs} ${X})
  ENDFOREACH (X ${ARGN})

  FOREACH (X ${all_libs})
      FIND_LIBRARY(full_lib_path ${X}
                   PATHS ${${lib_dir_var}}
                   NO_DEFAULT_PATH
                   NO_CMAKE_ENVIRONMENT_PATH
                   NO_CMAKE_PATH
                   NO_SYSTEM_ENVIRONMENT_PATH
                   NO_CMAKE_SYSTEM_PATH)
      IF(full_lib_path)
          IF(${${lib_skip_install}})
            MESSAGE(STATUS "Skipping installation of ${full_lib_path}")
          ELSE(${${lib_skip_install}})
            IF(NOT "${lib_dir_var}" STREQUAL "/usr/lib")
                THIRD_PARTY_INSTALL_LIBRARY(${full_lib_path})
            ENDIF(NOT "${lib_dir_var}" STREQUAL "/usr/lib")
          ENDIF(${${lib_skip_install}})
          GET_FILENAME_COMPONENT(alib ${full_lib_path} NAME)
          LIST(APPEND "${lib_var}" ${alib})
          MESSAGE(STATUS "  Found library ${X} in ${${lib_dir_var}}")
      ELSE(full_lib_path)
          IF(IGNORE_THIRD_PARTY_LIB_PROBLEMS)
              MESSAGE(STATUS "\n** \n** \n** Library ${X} not found in ${${lib_dir_var}}.\n**\n**")
          ELSE(IGNORE_THIRD_PARTY_LIB_PROBLEMS)
              MESSAGE(FATAL_ERROR "Library ${X} not found in ${${lib_dir_var}}")
          ENDIF(IGNORE_THIRD_PARTY_LIB_PROBLEMS)
          RETURN()
      ENDIF(full_lib_path)
      UNSET(full_lib_path CACHE)
  ENDFOREACH (X ${all_libs})

  SET(lib_dep "${pkg}_LIBDEP")
  IF(NOT "${${lib_dep}}" STREQUAL "")
    MESSAGE(STATUS "  Looking for dependent libraries for ${pkg}")
  ENDIF(NOT "${${lib_dep}}" STREQUAL "")
  
  #
  # This alternates between a reading path & a lib from ${pkg}_LIBDEP
  #
  SET(is_lib 1)
  FOREACH (X ${${lib_dep}})
      IF(${X})
          SET(X_VALUE ${${X}})
      ELSE(${X})
          SET(X_VALUE ${X})
      ENDIF(${X})
      IF(is_lib)
          SET(is_lib 0)
          LIST(APPEND "${lib_dir_var}" ${X_VALUE})
          SET(current_lib_dir ${X_VALUE})
      ELSE(is_lib)
          SET(is_lib 1)
          FIND_LIBRARY(full_lib_path
                       NAMES ${X_VALUE}
                       PATHS ${current_lib_dir}
                       NO_DEFAULT_PATH
                       NO_CMAKE_ENVIRONMENT_PATH
                       NO_CMAKE_PATH
                       NO_SYSTEM_ENVIRONMENT_PATH
                       NO_CMAKE_SYSTEM_PATH)
          IF(full_lib_path)
            IF(${${lib_skip_install}})
                MESSAGE(STATUS "Skipping installation of ${full_lib_path}") 
            ELSE(${${lib_skip_install}})
              IF(NOT "${current_lib_dir}" STREQUAL "/usr/lib")
                  THIRD_PARTY_INSTALL_LIBRARY(${full_lib_path})
              ENDIF(NOT "${current_lib_dir}" STREQUAL "/usr/lib")
            ENDIF(${${lib_skip_install}})
              GET_FILENAME_COMPONENT(alib ${full_lib_path} NAME)
              LIST(APPEND "${lib_var}" ${alib})
              MESSAGE(STATUS "     Found library ${X_VALUE} in ${current_lib_dir}")
          ELSE(full_lib_path)
              IF(IGNORE_THIRD_PARTY_LIB_PROBLEMS)
                  MESSAGE(STATUS "\n** \n** \n**     Library ${X_VALUE} not found in ${current_lib_dir}.\n**\n**")
              ELSE(IGNORE_THIRD_PARTY_LIB_PROBLEMS)
                  MESSAGE(FATAL_ERROR "     Library ${X_VALUE} not found in ${current_lib_dir}")
              ENDIF(IGNORE_THIRD_PARTY_LIB_PROBLEMS)
              RETURN()
          ENDIF(full_lib_path)
          UNSET(full_lib_path CACHE)
      ENDIF(is_lib)
  ENDFOREACH (X ${${lib_dep}})

  # Cache final results
  SET("${tp_found}"    1                 CACHE BOOL   "${pkg} library found" FORCE)
  SET("${base_dir}"    ${${base_dir}}    CACHE PATH   "${pkg} base directory" FORCE)
  SET("${inc_dir_var}" ${${inc_dir_var}} CACHE PATH   "${pkg} include directory" FORCE)
  SET("${lib_dir_var}" ${${lib_dir_var}} CACHE PATH   "${pkg} library directory" FORCE)
  SET("${lib_var}"     ${${lib_var}}     CACHE STRING "${pkg} library" FORCE)

  MARK_AS_ADVANCED("${tp_found}"
                   "${base_dir}"
                   "${inc_dir_var}"
                   "${lib_dir_var}"
                   "${lib_var}")
  MESSAGE(STATUS "  ${pkg} found")

ENDFUNCTION(SET_UP_THIRD_PARTY)

