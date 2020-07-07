# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Brad Whitlock, Wed Nov 25 13:39:52 PST 2009
#   I changed the code to support looking for frameworks in ${PYTHON_DIR}
#   before looking in the system places. I also made sure that the proper
#   -F is added for the framework location.
#
#   Kathleen Bonnell, Thu Dec  3 10:33:54 PST 2009
#   Append MSVC_VERSION to searchpath (for windows).
#
#   Kathleen Bonnell, Thu Dec 10 17:50:01 MT 2009
#   Removed MSVC_VERSION from searchpath in favor of 'libs' for windows.
#
#   Gunther H. Weber, Fri Jan 29 12:00:39 PST 2010
#   Only install Python if we are not using the system Python.
#
#   Kathleen Bonnell, Wed Mar 17 10:01:22 MT 2010
#   Exclude '.svn' from being included when installing directories.
#
#   Kathleen Bonnell, Wed Mar 24 16:26:32 MST 2010
#   Change install on windows due to different directory structure.
#
#   Cyrus Harrison, Mon Aug 16 11:41:22 PDT 2010
#   Create a complete python install in ${VISIT_INSTALLED_VERSION_LIB}/python
#   This allows us to install python modules directly to VisIt's python.
#
#   Kathleen Bonnell, Wed Apr 20 11:03:05 MST 2011
#   Change PYTHON_ADD_MODULE to use extension '.pyd' and PY_MODULE_TYPE
#   SHARED (instead of MODULE) on Windows.
#
#   Kathleen Bonnell, Tue May 3 15:13:27 MST 2011
#   Revert PY_MODULE_TYPE used in PYTHON_ADD_MODULE to MODULE.
#
#   Cyrus Harrison, Mon Aug  1 16:24:33 PDT 2011
#   Include pyc & pyo files in install.
#   Don't use 'visit*' pattern to exclude from install,
#   this actually excluded parts of the std python lib.
#   Instead use visit.*, visitmodule.* and visit_writer.*
#
#   Cyrus Harrison, Mon Oct  3 15:55:53 PDT 2011
#   Exclude install of PySide module (FindPySide.cmake handles this)
#
#   Cyrus Harrison, Mon Apr 16 14:20:20 PDT 2012
#   Add detection the PYTHON_EXECUTABLE, and ADD_PYTHON_DISTUTILS_SETUP
#   command.
#
#   Cyrus Harrison, Mon Apr 16 14:20:20 PDT 2012
#   Fix problem with PYTHON_EXECUTABLE detection.
#
#   Kathleen Biagas, Thu May 10 10:24:15 MST 2012
#   Add windows-specific cases for PYTHON_ADD_DISTUTILS_SETUP, to correctly
#   handle windows path and path-with-spaces issues. Don't change
#   library output directory on widnows for PYTHON_ADD_HYBRID_MODULE.
#
#   Kathleen Biagas, Tue Jun 5 14:49:52 PDT 2012
#   Fix problem with setting of PYTHON_VERSION on windows.
#
#   Cyrus Harrison, Fri Apr 10 13:47:15 PDT 2020
#   Python 3 Support.
#
#****************************************************************************/

INCLUDE(${VISIT_SOURCE_DIR}/CMake/ThirdPartyInstallLibrary.cmake)

# - Find python libraries
# This module finds if Python is installed and determines where the
# include files and libraries are. It also determines what the name of
# the library is. This code sets the following variables:
#
#  PYTHONLIBS_FOUND     = have the Python libs been found
#  PYTHON_LIBRARIES     = path to the python library
#  PYTHON_INCLUDE_PATH  = path to where Python.h is found
#  PYTHON_DEBUG_LIBRARIES = path to the debug library
#  PYTHON_VERSION       = version number of found python
#

MESSAGE(STATUS "Looking for Python")

###
# CODE FROM CONDUIT TO FIND PYTHON INTERP AND LIBS
###
# Find the interpreter first
if(PYTHON_DIR AND NOT PYTHON_EXECUTABLE)
    set(PYTHON_EXECUTABLE ${PYTHON_DIR}/bin/python)
    # if this doesn't exist, we may be using python3, which
    # in many variants only creates "python3" exe, not "python"
    if(NOT EXISTS "${PYTHON_EXECUTABLE}")
        set(PYTHON_EXECUTABLE ${PYTHON_DIR}/bin/python3)
    endif()
endif()

find_package(PythonInterp REQUIRED)
if(PYTHONINTERP_FOUND)
        
        MESSAGE(STATUS "PYTHON_EXECUTABLE ${PYTHON_EXECUTABLE}")
        
        execute_process(COMMAND "${PYTHON_EXECUTABLE}" "-c" 
                                "import sys;from distutils.sysconfig import get_python_inc;sys.stdout.write(get_python_inc())"
                        OUTPUT_VARIABLE PYTHON_INCLUDE_DIR
                        ERROR_VARIABLE ERROR_FINDING_INCLUDES)
        MESSAGE(STATUS "PYTHON_INCLUDE_DIR ${PYTHON_INCLUDE_DIR}")
        
        if(NOT EXISTS ${PYTHON_INCLUDE_DIR})
            MESSAGE(FATAL_ERROR "Reported PYTHON_INCLUDE_DIR ${PYTHON_INCLUDE_DIR} does not exist!")
        endif()

        execute_process(COMMAND "${PYTHON_EXECUTABLE}" "-c" 
                                "import sys;from distutils.sysconfig import get_python_lib;sys.stdout.write(get_python_lib())"
                        OUTPUT_VARIABLE PYTHON_SITE_PACKAGES_DIR
                        ERROR_VARIABLE ERROR_FINDING_SITE_PACKAGES_DIR)
        MESSAGE(STATUS "PYTHON_SITE_PACKAGES_DIR ${PYTHON_SITE_PACKAGES_DIR}")

        if(NOT EXISTS ${PYTHON_SITE_PACKAGES_DIR})
            MESSAGE(FATAL_ERROR "Reported PYTHON_SITE_PACKAGES_DIR ${PYTHON_SITE_PACKAGES_DIR} does not exist!")
        endif()

        execute_process(COMMAND "${PYTHON_EXECUTABLE}" "-c" 
                                "import sys;from distutils.sysconfig import get_config_var; sys.stdout.write(get_config_var('LIBDIR'))"
                        OUTPUT_VARIABLE PYTHON_LIB_DIR
                        ERROR_VARIABLE ERROR_FINDING_LIB_DIR)
        MESSAGE(STATUS "PYTHON_LIB_DIR ${PYTHON_LIB_DIR}")

        # if we are on macOS or linux, expect PYTHON_LIB_DIR to exist
        # windows logic does not need PYTHON_LIB_DIR
        if(NOT WIN32 AND NOT EXISTS ${PYTHON_LIB_DIR})
            MESSAGE(FATAL_ERROR "Reported PYTHON_LIB_DIR ${PYTHON_LIB_DIR} does not exist!")
        endif()

        # check if we need "-undefined dynamic_lookup" by inspecting LDSHARED flags
        execute_process(COMMAND "${PYTHON_EXECUTABLE}" "-c" 
                                "import sys;import sysconfig;sys.stdout.write(sysconfig.get_config_var('LDSHARED'))"
                        OUTPUT_VARIABLE PYTHON_LDSHARED_FLAGS
                        ERROR_VARIABLE ERROR_FINDING_PYTHON_LDSHARED_FLAGS)

        MESSAGE(STATUS "PYTHON_LDSHARED_FLAGS ${PYTHON_LDSHARED_FLAGS}")

        if(PYTHON_LDSHARED_FLAGS MATCHES "-undefined dynamic_lookup")
             MESSAGE(STATUS "PYTHON_USE_UNDEFINED_DYNAMIC_LOOKUP_FLAG is ON")
            set(PYTHON_USE_UNDEFINED_DYNAMIC_LOOKUP_FLAG ON)
        else()
             MESSAGE(STATUS "PYTHON_USE_UNDEFINED_DYNAMIC_LOOKUP_FLAG is OFF")
            set(PYTHON_USE_UNDEFINED_DYNAMIC_LOOKUP_FLAG OFF)
        endif()

        # check for python libs differs for windows python installs
        if(NOT WIN32)
            # we may build a shared python module against a static python
            # check for both shared and static libs cases

            # check for shared first
            set(PYTHON_GLOB_TEST "${PYTHON_LIB_DIR}/libpython*${CMAKE_SHARED_LIBRARY_SUFFIX}")
            FILE(GLOB PYTHON_GLOB_RESULT ${PYTHON_GLOB_TEST})
            # then for static if shared is not found
            if(NOT PYTHON_GLOB_RESULT)
                set(PYTHON_GLOB_TEST "${PYTHON_LIB_DIR}/libpython*${CMAKE_STATIC_LIBRARY_SUFFIX}")
            endif()
        else()
            if(PYTHON_LIB_DIR)
                set(PYTHON_GLOB_TEST "${PYTHON_LIB_DIR}/python*.lib")
            else()
                get_filename_component(PYTHON_ROOT_DIR ${PYTHON_EXECUTABLE} DIRECTORY)
                set(PYTHON_GLOB_TEST "${PYTHON_ROOT_DIR}/libs/python*.lib")
            endif()
        endif()

        FILE(GLOB PYTHON_GLOB_RESULT ${PYTHON_GLOB_TEST})

        # make sure we found something
        if(NOT PYTHON_GLOB_RESULT)
            message(FATAL_ERROR "Failed to find main python library using pattern: ${PYTHON_GLOB_TEST}")
        endif()

        if(NOT WIN32)
            # life is ok on windows, but elsewhere
            # the glob result might be a list due to symlinks, etc
            # if it is a list, select the first entry as py lib
            list(LENGTH PYTHON_GLOB_RESULT PYTHON_GLOB_RESULT_LEN)
            if(${PYTHON_GLOB_RESULT_LEN} GREATER 1)
                list(GET PYTHON_GLOB_RESULT 0 PYTHON_GLOB_RESULT)
            endif()
        endif()

        get_filename_component(PYTHON_LIBRARY "${PYTHON_GLOB_RESULT}" ABSOLUTE)

        MESSAGE(STATUS "{PythonLibs from PythonInterp} using: PYTHON_LIBRARY=${PYTHON_LIBRARY}")
        find_package(PythonLibs)

        if(NOT PYTHONLIBS_FOUND)
            MESSAGE(FATAL_ERROR "Failed to find Python Libraries using PYTHON_EXECUTABLE=${PYTHON_EXECUTABLE}")
        endif()
        
endif()


find_package_handle_standard_args(Python  DEFAULT_MSG
                                  PYTHON_LIBRARIES PYTHON_INCLUDE_DIR)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PYTHONLIBS DEFAULT_MSG PYTHON_LIBRARIES PYTHON_INCLUDE_PATH)

message(STATUS "PYTHONLIBS_FOUND = ${PYTHONLIBS_FOUND}")

# the rest of VisIt's cmake logic expects PYTHON_VERSION to be the compatibility version
# (3.7, not 3.7.6 or 2.7, not 2.7.14)

set(PYTHON_VERSION_FULL "${PYTHONLIBS_VERSION_STRING}")

string(REPLACE "." ";" PYTHON_VERSION_LIST ${PYTHON_VERSION_FULL})
list(GET PYTHON_VERSION_LIST 0 PYTHON_VERSION_MAJOR)
list(GET PYTHON_VERSION_LIST 1 PYTHON_VERSION_MINOR)
list(GET PYTHON_VERSION_LIST 2 PYTHON_VERSION_PATCH)

set(PYTHON_VERSION "${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}")

message(STATUS "PYTHON_VERSION = ${PYTHON_VERSION}")

IF(PYTHONLIBS_FOUND)
    SET(PYTHON_FOUND TRUE)
ELSE()
    SET(PYTHON_FOUND FALSE)
ENDIF()

# PYTHON_ADD_MODULE(<name> src1 src2 ... srcN) is used to build modules for python.
# PYTHON_WRITE_MODULES_HEADER(<filename>) writes a header file you can include
# in your sources to initialize the static python modules

GET_PROPERTY(_TARGET_SUPPORTS_SHARED_LIBS
  GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS)

FUNCTION(PYTHON_ADD_MODULE _NAME )
  OPTION(PYTHON_ENABLE_MODULE_${_NAME} "Add module ${_NAME}" TRUE)
  OPTION(PYTHON_MODULE_${_NAME}_BUILD_SHARED "Add module ${_NAME} shared" ${_TARGET_SUPPORTS_SHARED_LIBS})

  IF(PYTHON_ENABLE_MODULE_${_NAME})
    IF(PYTHON_MODULE_${_NAME}_BUILD_SHARED)
      SET(PY_MODULE_TYPE MODULE)
    ELSE(PYTHON_MODULE_${_NAME}_BUILD_SHARED)
      SET(PY_MODULE_TYPE STATIC)
      SET_PROPERTY(GLOBAL  APPEND  PROPERTY  PY_STATIC_MODULES_LIST ${_NAME})
    ENDIF(PYTHON_MODULE_${_NAME}_BUILD_SHARED)

    SET_PROPERTY(GLOBAL  APPEND  PROPERTY  PY_MODULES_LIST ${_NAME})
    ADD_LIBRARY(${_NAME} ${PY_MODULE_TYPE} ${ARGN})
    SET_TARGET_PROPERTIES(${_NAME} PROPERTIES PREFIX "")
    IF (WIN32)
        SET_TARGET_PROPERTIES(${_NAME} PROPERTIES SUFFIX ".pyd")
    ENDIF(WIN32)

    TARGET_LINK_LIBRARIES(${_NAME} ${PYTHON_LIBRARIES})

  ENDIF(PYTHON_ENABLE_MODULE_${_NAME})
ENDFUNCTION(PYTHON_ADD_MODULE)


#
# Function that calls a distutils based setup python script
# and installs a python module.
#

FUNCTION(PYTHON_ADD_DISTUTILS_SETUP target_name dest_dir setup_file)
MESSAGE(STATUS "Configuring python distutils setup: ${target_name}")
IF(NOT WIN32)
    add_custom_command(OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/build
            COMMAND ${PYTHON_EXECUTABLE} ${setup_file} -v
            build
            --build-base=${CMAKE_CURRENT_BINARY_DIR}/build
            install
            --install-purelib=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${dest_dir}
            DEPENDS  ${setup_file} ${ARGN}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

    add_custom_target(${target_name} ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/build)
    # Also use distutils for the install ...
    #
    # The following if uses the CMAKE_INSTALL_PREFIX directly if it
    # is an absolute path, otherwise it prepends the VISIT_BINARY_DIR
    # to it. Also note that adding STATUS to the message call causes
    # the message to not get output.
    INSTALL(CODE
        "
        if(\"\${CMAKE_INSTALL_PREFIX}\" MATCHES \"^/.*\")
           set(PREFIX_DIR_ABSOLUTE \"\${CMAKE_INSTALL_PREFIX}\")
        else()
           get_filename_component(PREFIX_DIR_ABSOLUTE \"${CMAKE_INSTALL_PREFIX}\" ABSOLUTE BASE_DIR \"${VISIT_BINARY_DIR}\")
        endif()
        execute_process(WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMAND ${PYTHON_EXECUTABLE} ${setup_file} -v
                build   --build-base=${CMAKE_CURRENT_BINARY_DIR}/build_install
                install --install-purelib=\${PREFIX_DIR_ABSOLUTE}/${VISIT_INSTALLED_VERSION_LIB}/${dest_dir}
            OUTPUT_VARIABLE PY_DIST_UTILS_INSTALL_OUT)
        message(\"\${PY_DIST_UTILS_INSTALL_OUT}\")
        ")
ELSE(NOT WIN32)

    FILE(TO_NATIVE_PATH ${VISIT_LIBRARY_DIR} VLD_NATIVE)
    STRING(REPLACE "\\" "\\\\" VLD_ESC_PATH "${VLD_NATIVE}")
    FILE(TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR} CCBD_NATIVE)
    STRING(REPLACE "\\" "\\\\" CCBD_ESC_PATH "${CCBD_NATIVE}")

    add_custom_target(${target_name} ALL
            COMMAND ${PYTHON_EXECUTABLE} ${setup_file} -v
            build
            --build-base=${CMAKE_CURRENT_BINARY_DIR}/build
            install
            --install-purelib=${VLD_NATIVE}/${dest_dir}
            DEPENDS  ${setup_file} ${ARGN}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    set_target_properties(${target_name} PROPERTIES FOLDER visit_py_setup)
    if(VISIT_NO_ALLBUILD)
        add_dependencies(visit_special_builds ${target_name})
    endif()

    # also use distutils for the install ...
    FILE(TO_NATIVE_PATH ${VISIT_INSTALLED_VERSION_LIB} VIVL_NATIVE)
    STRING(REPLACE "\\" "\\\\" VIVL_ESC_PATH "${VIVL_NATIVE}")
    INSTALL(CODE
       "
       EXECUTE_PROCESS(WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
           COMMAND ${PYTHON_EXECUTABLE} ${setup_file} -v
           build   \"--build-base=${CCBD_ESC_PATH}\\\\build_install\"
           install \"--install-purelib=${VIVL_ESC_PATH}\\\\${dest_dir}\")
       MESSAGE(STATUS \"\${PY_DIST_UTILS_INSTALL_OUT}\")
       ")
ENDIF(NOT WIN32)


ENDFUNCTION(PYTHON_ADD_DISTUTILS_SETUP)

FUNCTION(PYTHON_ADD_HYBRID_MODULE target_name dest_dir setup_file py_sources)
    MESSAGE(STATUS "Configuring hybrid python module: ${target_name}")
    PYTHON_ADD_DISTUTILS_SETUP("${target_name}_py_setup"
                               ${dest_dir}
                               ${setup_file}
                               ${py_sources})
    PYTHON_ADD_MODULE(${target_name} ${ARGN})
    if(NOT WIN32)
        SET_TARGET_PROPERTIES(${target_name} PROPERTIES
                                             LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${dest_dir}/${target_name}/)
    else()
        foreach(cfg ${CMAKE_CONFIGURATION_TYPES})
            string(TOUPPER ${cfg} UCFG)
            set_target_properties(${target_name} PROPERTIES
                 LIBRARY_OUTPUT_DIRECTORY_${UCFG} "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${cfg}/${dest_dir}/${target_name}/")
        endforeach()
    endif()
    ADD_DEPENDENCIES(${target_name} "${target_name}_py_setup")
    VISIT_INSTALL_TARGETS_RELATIVE(${dest_dir}/${target_name} ${target_name})

ENDFUNCTION(PYTHON_ADD_HYBRID_MODULE)


# Deal with install targets
IF(VISIT_PYTHON_SKIP_INSTALL)
    MESSAGE(STATUS "Python installation will be skipped")
ENDIF(VISIT_PYTHON_SKIP_INSTALL)

IF(PYTHONLIBS_FOUND AND NOT VISIT_PYTHON_SKIP_INSTALL)
    IF(Python_FRAMEWORKS)
        MESSAGE("We need to install the Python framework.")
    ELSE(Python_FRAMEWORKS)
        MESSAGE(STATUS "We're gonna install Python")

        # Install libpython
        THIRD_PARTY_INSTALL_LIBRARY(${PYTHON_LIBRARIES})

        # Only install Python support files if we are not using the system Python
        IF((NOT ${PYTHON_DIR} STREQUAL "/usr"))
            # Install the python executable
            STRING(SUBSTRING ${PYTHON_VERSION} 0 1 PYX)
            STRING(SUBSTRING ${PYTHON_VERSION} 0 3 PYX_X)
            THIRD_PARTY_INSTALL_EXECUTABLE(${PYTHON_DIR}/bin/python ${PYTHON_DIR}/bin/python${PYX} ${PYTHON_DIR}/bin/python${PYX_X})

            # Install the python modules
            # Exclude lib-tk files for now because the permissions are bad on davinci. BJW 12/17/2009
            # Exclude visit module files.
            IF(EXISTS ${PYTHON_DIR}/lib/python${PYTHON_VERSION})
                INSTALL(DIRECTORY ${PYTHON_DIR}/lib/python${PYTHON_VERSION}
                    DESTINATION ${VISIT_INSTALLED_VERSION_LIB}/python/lib
                    FILE_PERMISSIONS OWNER_READ OWNER_WRITE
                                     GROUP_READ GROUP_WRITE
                                     WORLD_READ
                    DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                                          GROUP_READ GROUP_WRITE GROUP_EXECUTE
                                          WORLD_READ WORLD_EXECUTE
                    PATTERN "lib-tk" EXCLUDE
                    PATTERN "visit.*" EXCLUDE
                    PATTERN "visitmodule.*" EXCLUDE
                    PATTERN "visit_writer.*" EXCLUDE
                    PATTERN "PySide" EXCLUDE
                    PATTERN "Python-2.6-py2.6.egg-info" EXCLUDE
                )
            ENDIF(EXISTS ${PYTHON_DIR}/lib/python${PYTHON_VERSION})

            # Install the Python headers
            IF (NOT WIN32)
                IF(VISIT_HEADERS_SKIP_INSTALL)
                    MESSAGE(STATUS "Skipping python headers installation")
                ELSE(VISIT_HEADERS_SKIP_INSTALL)
                    INSTALL(DIRECTORY ${PYTHON_INCLUDE_PATH}
                        DESTINATION ${VISIT_INSTALLED_VERSION_INCLUDE}/python/include
                        FILE_PERMISSIONS OWNER_READ OWNER_WRITE
                                         GROUP_READ GROUP_WRITE
                                         WORLD_READ
                        DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                                              GROUP_READ GROUP_WRITE GROUP_EXECUTE
                                              WORLD_READ             WORLD_EXECUTE
                        PATTERN ".svn" EXCLUDE
                        )
                ENDIF(VISIT_HEADERS_SKIP_INSTALL)
                #
                # CDH:
                # We also need to install the headers into lib/python dir
                # if we want to be able to install python modules into a visit
                # install.
                #
                INSTALL(DIRECTORY ${PYTHON_INCLUDE_PATH}
                  DESTINATION ${VISIT_INSTALLED_VERSION_LIB}/python/include
                  FILE_PERMISSIONS OWNER_READ OWNER_WRITE
                                   GROUP_READ GROUP_WRITE
                                   WORLD_READ
                  DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                                        GROUP_READ GROUP_WRITE GROUP_EXECUTE
                                        WORLD_READ             WORLD_EXECUTE
                  PATTERN ".svn" EXCLUDE
                )
            ELSE (NOT WIN32)
                # CDH
                # The WIN32 & NOT WIN32 cases seem almost the same here?
                # The only diff I can see is the "*.h" glob is used?
                #
                IF(VISIT_HEADERS_SKIP_INSTALL)
                    MESSAGE(STATUS "Skipping python headers installation")
                ELSE(VISIT_HEADERS_SKIP_INSTALL)
                INSTALL(DIRECTORY ${PYTHON_INCLUDE_PATH}/
                    DESTINATION ${VISIT_INSTALLED_VERSION_INCLUDE}/python
                    FILE_PERMISSIONS OWNER_READ OWNER_WRITE
                                     GROUP_READ GROUP_WRITE
                                     WORLD_READ
                    DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                                          GROUP_READ GROUP_WRITE GROUP_EXECUTE
                                          WORLD_READ             WORLD_EXECUTE
                    FILES_MATCHING PATTERN "*.h"
                    PATTERN ".svn" EXCLUDE
                )
                ENDIF(VISIT_HEADERS_SKIP_INSTALL)
                #
                # CDH:
                # We also need to install the headers into lib/python dir
                # if we want to be able to install python modules into a visit
                # install.
                #
                INSTALL(DIRECTORY ${PYTHON_INCLUDE_PATH}/
                    DESTINATION ${VISIT_INSTALLED_VERSION_LIB}/python/include
                    FILE_PERMISSIONS OWNER_READ OWNER_WRITE
                                     GROUP_READ GROUP_WRITE
                                     WORLD_READ
                    DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                                          GROUP_READ GROUP_WRITE GROUP_EXECUTE
                                          WORLD_READ             WORLD_EXECUTE
                    FILES_MATCHING PATTERN "*.h"
                    PATTERN ".svn" EXCLUDE
                )
                INSTALL(DIRECTORY ${VISIT_PYTHON_DIR}/lib
                    DESTINATION ${VISIT_INSTALLED_VERSION_LIB}/python
                    FILE_PERMISSIONS OWNER_READ OWNER_WRITE
                                     GROUP_READ GROUP_WRITE
                                     WORLD_READ
                    DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                                          GROUP_READ GROUP_WRITE GROUP_EXECUTE
                                          WORLD_READ             WORLD_EXECUTE
                    PATTERN ".svn"   EXCLUDE
                )
            ENDIF (NOT WIN32)
        ENDIF((NOT ${PYTHON_DIR} STREQUAL "/usr"))
    ENDIF(Python_FRAMEWORKS)
ENDIF(PYTHONLIBS_FOUND AND NOT VISIT_PYTHON_SKIP_INSTALL)
