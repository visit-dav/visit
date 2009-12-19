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
#****************************************************************************/

INCLUDE(${VISIT_SOURCE_DIR}/CMake/ThirdPartyInstallLibrary.cmake)

#
# sets up cache variables for a third_party_package.
#  x_INCLUDE_DIR
#  x_LIBRARY_DIR
#  x_LIB
#  x_FOUND
#
#  assumes headers are in ${x_DIR}/include
#  assumes libs are in ${x_DIR}/${libextensiondir}
#
#
#  NOTE:  this macro does not FIND packages, but merely tranlates the passed
#         parameters into the afore mentioned variables.
#
#  pkg is the name used to specify the x_DIR (generally upper case name 
#  of pkg)
#  libdirextension is the path beyond x_DIR where the libs may be found.
#  incdirextension is the path beyond x_DIR where the includes may be found.
#  libs is the list of library names for this package 
#
#  uses path specified by pkg_DIR as base path for the files
#
FUNCTION(SET_UP_THIRD_PARTY pkg libdirextension incdirextension libs)
  MESSAGE(STATUS "Looking for ${pkg}")
  UNSET(lib_setup_error)
  SET(base_dir "${pkg}_DIR")
  SET(base_dir_val "${${base_dir}}")
  IF ("${base_dir}" AND EXISTS "${base_dir_val}")
    SET(inc_dir_var "${pkg}_INCLUDE_DIR")
    SET(lib_dir_var "${pkg}_LIBRARY_DIR")
    SET(lib_var "${pkg}_LIB")
    SET(tp_found "${pkg}_FOUND")

    #
    # Zero out lib names b/c they may be hanging around from a previous 
    # configure.
    #
    UNSET("${tp_found}")
    SET("${lib_var}" "")

    SET("${inc_dir_var}" "${base_dir_val}/${incdirextension}")
    SET("${lib_dir_var}" "${base_dir_val}/${libdirextension}")

    IF(NOT EXISTS ${${inc_dir_var}})
        MESSAGE(STATUS "Error: Include Directory for ${pkg} (${${inc_dir_var}}) does not exist.")
        SET(lib_setup_error 1)
    ENDIF(NOT EXISTS ${${inc_dir_var}})

    IF(NOT EXISTS ${${lib_dir_var}})
        MESSAGE(STATUS "Error: Library Directory for ${pkg} (${${lib_dir_var}}) does not exist.")
        SET(lib_setup_error 1)
    ENDIF(NOT EXISTS ${${lib_dir_var}})

    # If the inc and lib directories are different then attempt to
    # install the include directory.
    IF(NOT ${${inc_dir_var}} STREQUAL ${${lib_dir_var}})
        THIRD_PARTY_INSTALL_INCLUDE(${pkg} ${${inc_dir_var}})
    ENDIF(NOT ${${inc_dir_var}} STREQUAL ${${lib_dir_var}})

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
            THIRD_PARTY_INSTALL_LIBRARY(${full_lib_path})
            GET_FILENAME_COMPONENT(alib ${full_lib_path} NAME)
            LIST(APPEND "${lib_var}" ${alib})
            MESSAGE(STATUS "  Found library ${X} in ${${lib_dir_var}}")
        ELSE(full_lib_path)
            MESSAGE(STATUS "  Error: Library ${X} not found in ${${lib_dir_var}}")
            SET(lib_setup_error 1)
        ENDIF(full_lib_path)
        UNSET(full_lib_path CACHE)
    ENDFOREACH (X ${all_libs})

    SET(lib_dep "${pkg}_LIBDEP")
    IF(${lib_dep})
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
                    THIRD_PARTY_INSTALL_LIBRARY(${full_lib_path})
                    GET_FILENAME_COMPONENT(alib ${full_lib_path} NAME)
                    LIST(APPEND "${lib_var}" ${alib})
                    MESSAGE(STATUS "  Found library ${X_VALUE} in ${current_lib_dir}")
                ELSE(full_lib_path)
                    MESSAGE(STATUS "  Error: Library ${X_VALUE} not found in ${current_lib_dir}")
                    SET(lib_setup_error 1)
                ENDIF(full_lib_path)
                UNSET(full_lib_path CACHE)
            ENDIF(is_lib)
        ENDFOREACH (X ${${lib_dep}})
    ELSEIF(${lib_dep})
        MESSAGE("${lib_dep} not set")
    ENDIF(${lib_dep})

    IF(lib_setup_error)
        MESSAGE(FATAL_ERROR "Error: Could not setup ${pkg} due to missing libraries or dependancies.")
        UNSET(lib_setup_error CACHE)
    ELSE(lib_setup_error)

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
        MESSAGE(STATUS "${pkg} found")
    ENDIF(lib_setup_error)

  ELSE ("${base_dir}" AND EXISTS "${base_dir_val}")

    MESSAGE(STATUS "${pkg} not found")

  ENDIF ("${base_dir}" AND EXISTS "${base_dir_val}")

ENDFUNCTION(SET_UP_THIRD_PARTY)

