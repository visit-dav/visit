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
#   Cyrus Harrison, Wed Aug 11 16:05:04 PDT 2021
#   Robustify python lib detection logic
#
#   Cyrus Harrison, Wed Jan 19 10:06:17 PST 2022
#   Install extra python front end scripts if they exist
#
#   Kathleen Biagas, Thu Mar 24, 2022
#   Use CMake-style path for installs of python-include on Windows.
#
#   Cyrus Harrison, Fri Nov  4 14:35:16 PDT 2022
#   More status messages.
#
#   Kathleen Biagas, Wed Jan 18, 2023
#   Fix a few uses of paths on Windows, now that VISIT_INSTALLED_VERSION_XXX
#   are relative paths.
#
#   Kathleen Biagas, Thu Sep 28 16:33:45 PDT 2023
#   Set PYLIB to the python library name, used by PluginVsInstall.cmake.in.
#
#   Cyrus Harrison, Fri Feb 16 13:37:57 PST 2024
#   Refactor PYTHON_ADD_DISTUTILS_SETUP to PYTHON_ADD_PIP_SETUP.
#
#   Kathleen Biagas, Wed Feb 28, 2024
#   Removed install(CODE ...) logic for PIP installed modules, in favor of
#   direct install of build/lib/site-packages directory via lib/CMakeLists.txt
#
#   Kathleen Biagas, Tue Mar 5, 2024
#   In PYTHON_ADD_PIP_SETUP, changed setting of abs_dest_path from using
#   CMAKE_BINARY_DIR to VISIT_LIBRARY_DIR which accounts for build
#   configuration being part of the library path on Windows.
#
#****************************************************************************/

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

message(STATUS "Looking for Python")
###
# CODE FROM CONDUIT TO FIND PYTHON INTERP AND LIBS
###
# Find the interpreter first
if(PYTHON_DIR AND NOT PYTHON_EXECUTABLE)
    if(UNIX)
        # look for python 3 first
        set(PYTHON_EXECUTABLE ${PYTHON_DIR}/bin/python3)
        # if this doesn't exist, look for python
        if(NOT EXISTS "${PYTHON_EXECUTABLE}")
            set(PYTHON_EXECUTABLE ${PYTHON_DIR}/bin/python)
        endif()
    elseif(WIN32)
        set(PYTHON_EXECUTABLE ${PYTHON_DIR}/python.exe)
    endif()
    message(STATUS "Using PYTHON_EXECUTABLE from PYTHON_DIR:")
    message(STATUS "PYTHON_DIR: ${PYTHON_DIR}")
    message(STATUS "PYTHON_EXECUTABLE: ${PYTHON_EXECUTABLE}")
endif()


find_package(PythonInterp REQUIRED)
if(PYTHONINTERP_FOUND)
    message(STATUS "PYTHON_EXECUTABLE ${PYTHON_EXECUTABLE}")

    execute_process(COMMAND "${PYTHON_EXECUTABLE}" "-c"
                            "import sys;from sysconfig import get_config_var; sys.stdout.write(get_config_var('VERSION'))"
                    OUTPUT_VARIABLE PYTHON_CONFIG_VERSION
                    ERROR_VARIABLE  ERROR_FINDING_PYTHON_VERSION)
    message(STATUS "PYTHON_CONFIG_VERSION ${PYTHON_CONFIG_VERSION}")

    execute_process(COMMAND "${PYTHON_EXECUTABLE}" "-c"
                            "import sys;from sysconfig import get_path;sys.stdout.write(get_path('include'))"
                    OUTPUT_VARIABLE PYTHON_INCLUDE_DIR
                    ERROR_VARIABLE ERROR_FINDING_INCLUDES)
    message(STATUS "PYTHON_INCLUDE_DIR ${PYTHON_INCLUDE_DIR}")

    if(NOT EXISTS ${PYTHON_INCLUDE_DIR})
        message(FATAL_ERROR "Reported PYTHON_INCLUDE_DIR ${PYTHON_INCLUDE_DIR} does not exist!")
    endif()

    # TODO: replacing distutils.get_python_lib() isn't straight forward
    execute_process(COMMAND "${PYTHON_EXECUTABLE}" "-c"
                            "import sys;from distutils.sysconfig import get_python_lib;sys.stdout.write(get_python_lib())"
                    OUTPUT_VARIABLE PYTHON_SITE_PACKAGES_DIR
                    ERROR_VARIABLE ERROR_FINDING_SITE_PACKAGES_DIR)
    message(STATUS "PYTHON_SITE_PACKAGES_DIR ${PYTHON_SITE_PACKAGES_DIR}")

    if(NOT EXISTS ${PYTHON_SITE_PACKAGES_DIR})
        message(FATAL_ERROR "Reported PYTHON_SITE_PACKAGES_DIR ${PYTHON_SITE_PACKAGES_DIR} does not exist!")
    endif()

    # check if we need "-undefined dynamic_lookup" by inspecting LDSHARED flags
    execute_process(COMMAND "${PYTHON_EXECUTABLE}" "-c"
                            "import sys;import sysconfig;sys.stdout.write(sysconfig.get_config_var('LDSHARED'))"
                    OUTPUT_VARIABLE PYTHON_LDSHARED_FLAGS
                    ERROR_VARIABLE ERROR_FINDING_PYTHON_LDSHARED_FLAGS)

    message(STATUS "PYTHON_LDSHARED_FLAGS ${PYTHON_LDSHARED_FLAGS}")

    if(PYTHON_LDSHARED_FLAGS MATCHES "-undefined dynamic_lookup")
        message(STATUS "PYTHON_USE_UNDEFINED_DYNAMIC_LOOKUP_FLAG is ON")
        set(PYTHON_USE_UNDEFINED_DYNAMIC_LOOKUP_FLAG ON)
    else()
        message(STATUS "PYTHON_USE_UNDEFINED_DYNAMIC_LOOKUP_FLAG is OFF")
        set(PYTHON_USE_UNDEFINED_DYNAMIC_LOOKUP_FLAG OFF)
    endif()

    # our goal is to find the specific python lib, based on info
    # we extract from sysconfig from the python executable
    #
    # check if python libs differs for windows python installs
    if(NOT WIN32)
        # we may build a shared python module against a static python
        # check for both shared and static libs cases

        # combos to try:
        # shared:
        #  LIBDIR + LDLIBRARY
        #  LIBPL + LDLIBRARY
        # static:
        #  LIBDIR + LIBRARY
        #  LIBPL + LIBRARY

        execute_process(COMMAND "${PYTHON_EXECUTABLE}" "-c"
                                "import sys;from sysconfig import get_config_var; sys.stdout.write(get_config_var('LIBDIR'))"
                        OUTPUT_VARIABLE PYTHON_CONFIG_LIBDIR
                        ERROR_VARIABLE  ERROR_FINDING_PYTHON_LIBDIR)

        execute_process(COMMAND "${PYTHON_EXECUTABLE}" "-c"
                                "import sys;from sysconfig import get_config_var; sys.stdout.write(get_config_var('LIBPL'))"
                        OUTPUT_VARIABLE PYTHON_CONFIG_LIBPL
                            ERROR_VARIABLE  ERROR_FINDING_PYTHON_LIBPL)

        execute_process(COMMAND "${PYTHON_EXECUTABLE}" "-c"
                                "import sys;from sysconfig import get_config_var; sys.stdout.write(get_config_var('LDLIBRARY'))"
                        OUTPUT_VARIABLE PYTHON_CONFIG_LDLIBRARY
                        ERROR_VARIABLE  ERROR_FINDING_PYTHON_LDLIBRARY)

        execute_process(COMMAND "${PYTHON_EXECUTABLE}" "-c"
                                "import sys;from sysconfig import get_config_var; sys.stdout.write(get_config_var('LIBRARY'))"
                        OUTPUT_VARIABLE PYTHON_CONFIG_LIBRARY
                        ERROR_VARIABLE  ERROR_FINDING_PYTHON_LIBRARY)

        message(STATUS "PYTHON_CONFIG_LIBDIR:     ${PYTHON_CONFIG_LIBDIR}")
        message(STATUS "PYTHON_CONFIG_LIBPL:      ${PYTHON_CONFIG_LIBPL}")
        message(STATUS "PYTHON_CONFIG_LDLIBRARY:  ${PYTHON_CONFIG_LDLIBRARY}")
        message(STATUS "PYTHON_CONFIG_LIBRARY:    ${PYTHON_CONFIG_LIBRARY}")

        set(PYTHON_LIBRARY "")
        set(PYLIB "") # for PluginVsInstall.cmake.in

        # look for shared libs first
        # shared libdir + ldlibrary
        if(NOT EXISTS ${PYTHON_LIBRARY})
            if(IS_DIRECTORY ${PYTHON_CONFIG_LIBDIR})
                set(_PYTHON_LIBRARY_TEST  "${PYTHON_CONFIG_LIBDIR}/${PYTHON_CONFIG_LDLIBRARY}")
                message(STATUS "Checking for python library at: ${_PYTHON_LIBRARY_TEST}")
                if(EXISTS ${_PYTHON_LIBRARY_TEST})
                    set(PYTHON_LIBRARY ${_PYTHON_LIBRARY_TEST})
                    set(PYLIB ${PYTHON_CONFIG_LDLIBRARY})
                endif()
            endif()
        endif()

        # shared libpl + ldlibrary
        if(NOT EXISTS ${PYTHON_LIBRARY})
            if(IS_DIRECTORY ${PYTHON_CONFIG_LIBPL})
                set(_PYTHON_LIBRARY_TEST  "${PYTHON_CONFIG_LIBPL}/${PYTHON_CONFIG_LDLIBRARY}")
                message(STATUS "Checking for python library at: ${_PYTHON_LIBRARY_TEST}")
                if(EXISTS ${_PYTHON_LIBRARY_TEST})
                    set(PYTHON_LIBRARY ${_PYTHON_LIBRARY_TEST})
                    set(PYLIB ${PYTHON_CONFIG_LDLIBRARY})
                endif()
            endif()
        endif()

        # static: libdir + library
        if(NOT EXISTS ${PYTHON_LIBRARY})
            if(IS_DIRECTORY ${PYTHON_CONFIG_LIBDIR})
                set(_PYTHON_LIBRARY_TEST  "${PYTHON_CONFIG_LIBDIR}/${PYTHON_CONFIG_LIBRARY}")
                message(STATUS "Checking for python library at: ${_PYTHON_LIBRARY_TEST}")
                if(EXISTS ${_PYTHON_LIBRARY_TEST})
                    set(PYTHON_LIBRARY ${_PYTHON_LIBRARY_TEST})
                    set(PYLIB ${PYTHON_CONFIG_LIBRARY})
                endif()
            endif()
        endif()

        # static: libpl + library
        if(NOT EXISTS ${PYTHON_LIBRARY})
            if(IS_DIRECTORY ${PYTHON_CONFIG_LIBPL})
                set(_PYTHON_LIBRARY_TEST  "${PYTHON_CONFIG_LIBPL}/${PYTHON_CONFIG_LIBRARY}")
                message(STATUS "Checking for python library at: ${_PYTHON_LIBRARY_TEST}")
                if(EXISTS ${_PYTHON_LIBRARY_TEST})
                    set(PYTHON_LIBRARY ${_PYTHON_LIBRARY_TEST})
                    set(PYLIB ${PYTHON_CONFIG_LIBRARY})
                endif()
            endif()
        endif()
    else() # windows
        get_filename_component(PYTHON_ROOT_DIR ${PYTHON_EXECUTABLE} DIRECTORY)
        # Note: this assumes that two versions of python are not installed in the same dest dir
        set(_PYTHON_LIBRARY_TEST  "${PYTHON_ROOT_DIR}/libs/python${PYTHON_CONFIG_VERSION}.lib")
        message(STATUS "Checking for python library at: ${_PYTHON_LIBRARY_TEST}")
        if(EXISTS ${_PYTHON_LIBRARY_TEST})
            set(PYTHON_LIBRARY ${_PYTHON_LIBRARY_TEST})
            set(PYLIB python${PYTHON_CONFIG_VERSION}.lib)
        endif()
    endif()

    if(NOT EXISTS ${PYTHON_LIBRARY})
        message(FATAL_ERROR "Failed to find main library using PYTHON_EXECUTABLE=${PYTHON_EXECUTABLE}")
    endif()

    message(STATUS "{PythonLibs from PythonInterp} using: PYTHON_LIBRARY=${PYTHON_LIBRARY}")
    find_package(PythonLibs)

    if(NOT PYTHONLIBS_FOUND)
        message(FATAL_ERROR "Failed to find Python Libraries using PYTHON_EXECUTABLE=${PYTHON_EXECUTABLE}")
    endif()

endif()


find_package_handle_standard_args(Python  DEFAULT_MSG
                                  PYTHON_LIBRARY PYTHON_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)
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

if(PYTHONLIBS_FOUND)
    set(PYTHON_FOUND TRUE)
else()
    set(PYTHON_FOUND FALSE)
endif()

message(STATUS "PYTHON_LIBRARIES = ${PYTHON_LIBRARIES}")

# PYTHON_ADD_MODULE(<name> src1 src2 ... srcN) is used to build modules for python.
# PYTHON_WRITE_MODULES_HEADER(<filename>) writes a header file you can include
# in your sources to initialize the static python modules

get_property(_TARGET_SUPPORTS_SHARED_LIBS
    GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS)

function(PYTHON_ADD_MODULE _NAME )
    option(PYTHON_ENABLE_MODULE_${_NAME} "Add module ${_NAME}" TRUE)
    option(PYTHON_MODULE_${_NAME}_BUILD_SHARED "Add module ${_NAME} shared" ${_TARGET_SUPPORTS_SHARED_LIBS})

    if(PYTHON_ENABLE_MODULE_${_NAME})
        if(PYTHON_MODULE_${_NAME}_BUILD_SHARED)
            set(PY_MODULE_TYPE MODULE)
        else()
          set(PY_MODULE_TYPE STATIC)
          set_property(GLOBAL APPEND PROPERTY PY_STATIC_MODULES_LIST ${_NAME})
        endif()

        set_property(GLOBAL APPEND PROPERTY PY_MODULES_LIST ${_NAME})
        add_library(${_NAME} ${PY_MODULE_TYPE} ${ARGN})
        set_target_properties(${_NAME} PROPERTIES PREFIX "")
        if(WIN32)
            set_target_properties(${_NAME} PROPERTIES SUFFIX ".pyd")
        endif()
        target_link_libraries(${_NAME} ${PYTHON_LIBRARIES})
    endif()
endfunction()


##############################################################################
# Macro to use a pure python pip setup script
##############################################################################
FUNCTION(PYTHON_ADD_PIP_SETUP)
    set(singleValuedArgs NAME DEST_DIR PY_MODULE_DIR PY_SETUP_FILE FOLDER)
    set(multiValuedArgs  PY_SOURCES)

    ## parse the arguments to the macro
    cmake_parse_arguments(args
            "${options}" "${singleValuedArgs}" "${multiValuedArgs}" ${ARGN} )

    # check req'd args
    if(NOT DEFINED args_NAME)
       message(FATAL_ERROR
               "PYTHON_ADD_PIP_SETUP: Missing required argument NAME")
    endif()

    if(NOT DEFINED args_DEST_DIR)
       message(FATAL_ERROR
               "PYTHON_ADD_PIP_SETUP: Missing required argument DEST_DIR")
    endif()

    if(NOT DEFINED args_PY_MODULE_DIR)
       message(FATAL_ERROR
       "PYTHON_ADD_PIP_SETUP: Missing required argument PY_MODULE_DIR")
    endif()

    if(NOT DEFINED args_PY_SETUP_FILE)
       message(FATAL_ERROR
       "PYTHON_ADD_PIP_SETUP: Missing required argument PY_SETUP_FILE")
    endif()

    if(NOT DEFINED args_PY_SOURCES)
       message(FATAL_ERROR
       "PYTHON_ADD_PIP_SETUP: Missing required argument PY_SOURCES")
    endif()

    MESSAGE(STATUS "Configuring python pip setup: ${args_NAME}")

    # dest for build dir
    set(abs_dest_path ${VISIT_LIBRARY_DIR}/${args_DEST_DIR})
    if(WIN32)
        # on windows, python seems to need standard "\" style paths
        string(REGEX REPLACE "/" "\\\\" abs_dest_path  ${abs_dest_path})
    endif()

    # NOTE: With pip, you can't directly control build dir with an arg
    # like we were able to do with distutils, you have to use TMPDIR
    # TODO: we might want to  explore this in the future
    add_custom_command(OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/${args_NAME}_build
            COMMAND ${PYTHON_EXECUTABLE} -m pip install . -V --upgrade
            --disable-pip-version-check --no-warn-script-location
            --target "${abs_dest_path}"
            DEPENDS  ${args_PY_SETUP_FILE} ${args_PY_SOURCES}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

    add_custom_target(${args_NAME} ALL DEPENDS
                      ${CMAKE_CURRENT_BINARY_DIR}/${args_NAME}_build)

    # set folder if passed
    if(DEFINED args_FOLDER)
        blt_set_target_folder(TARGET ${args_NAME} FOLDER ${args_FOLDER})
    endif()
    
    if(WIN32)
        visit_add_to_util_builds(${args_NAME})
    endif()

ENDFUNCTION(PYTHON_ADD_PIP_SETUP)


##############################################################################
# Macro to create a pip script and compiled python module
##############################################################################
FUNCTION(PYTHON_ADD_HYBRID_MODULE)
    set(singleValuedArgs NAME DEST_DIR PY_MODULE_DIR PY_SETUP_FILE FOLDER)
    set(multiValuedArgs  PY_SOURCES SOURCES)

    ## parse the arguments to the macro
    cmake_parse_arguments(args
            "${options}" "${singleValuedArgs}" "${multiValuedArgs}" ${ARGN} )

     # check req'd args
    if(NOT DEFINED args_NAME)
        message(FATAL_ERROR
                "PYTHON_ADD_HYBRID_MODULE: Missing required argument NAME")
    endif()

    if(NOT DEFINED args_DEST_DIR)
        message(FATAL_ERROR
                "PYTHON_ADD_HYBRID_MODULE: Missing required argument DEST_DIR")
    endif()

    if(NOT DEFINED args_PY_MODULE_DIR)
        message(FATAL_ERROR
        "PYTHON_ADD_HYBRID_MODULE: Missing required argument PY_MODULE_DIR")
    endif()

    if(NOT DEFINED args_PY_SETUP_FILE)
        message(FATAL_ERROR
        "PYTHON_ADD_HYBRID_MODULE: Missing required argument PY_SETUP_FILE")
    endif()

    if(NOT DEFINED args_PY_SOURCES)
        message(FATAL_ERROR
        "PYTHON_ADD_HYBRID_MODULE: Missing required argument PY_SOURCES")
    endif()

    if(NOT DEFINED args_SOURCES)
        message(FATAL_ERROR
                "PYTHON_ADD_HYBRID_MODULE: Missing required argument SOURCES")
    endif()

    MESSAGE(STATUS "Configuring hybrid python module: ${args_NAME}")

    PYTHON_ADD_PIP_SETUP(NAME          "${args_NAME}_py_setup"
                         DEST_DIR      ${args_DEST_DIR}
                         PY_MODULE_DIR ${args_PY_MODULE_DIR}
                         PY_SETUP_FILE ${args_PY_SETUP_FILE}
                         PY_SOURCES    ${args_PY_SOURCES}
                         FOLDER        ${args_FOLDER})

    PYTHON_ADD_MODULE(${args_NAME} ${args_SOURCES})


    ##### visit specific code:
    if(NOT WIN32)
         set_target_properties(${args_NAME} PROPERTIES
             LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${args_DEST_DIR}/${args_NAME}/)
    else()
        set_target_properties(${args_NAME} PROPERTIES
             LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/$<CONFIG>/${args_DEST_DIR}/${args_NAME}/")
    endif()
    add_dependencies(${args_NAME} "${args_NAME}_py_setup")
    VISIT_INSTALL_TARGETS_RELATIVE(${args_DEST_DIR}/${args_NAME} ${args_NAME})

ENDFUNCTION(PYTHON_ADD_HYBRID_MODULE)


# Deal with install targets
if(VISIT_PYTHON_SKIP_INSTALL)
    message(STATUS "Python installation will be skipped")
endif()

if(PYTHONLIBS_FOUND AND NOT VISIT_PYTHON_SKIP_INSTALL)
    message(STATUS "We will install Python along with VisIt")
    # Install libpython
    # split the list so that dll's are found correctly on Windows
    foreach(pylib ${PYTHON_LIBRARIES})
        THIRD_PARTY_INSTALL_LIBRARY(${pylib})
    endforeach()

    # Only install Python support files if we are not using the system Python
    if((NOT ${PYTHON_DIR} STREQUAL "/usr"))
        # Install the python executable
        string(SUBSTRING ${PYTHON_VERSION} 0 1 PYX)
        string(SUBSTRING ${PYTHON_VERSION} 0 3 PYX_X)
        THIRD_PARTY_INSTALL_EXECUTABLE(${PYTHON_DIR}/bin/python ${PYTHON_DIR}/bin/python${PYX} ${PYTHON_DIR}/bin/python${PYX_X})
        # note: these can be symlinks to python3.Zm-config
        THIRD_PARTY_INSTALL_EXECUTABLE(${PYTHON_DIR}/bin/python${PYX}-config ${PYTHON_DIR}/bin/python${PYX_X}-config)
        THIRD_PARTY_INSTALL_EXECUTABLE(${PYTHON_DIR}/bin/python${PYX_X}m-config) # python3.Zm-config

        # install extra python front end scripts if they exist
        set(_py_extras ${PYTHON_DIR}/bin/pip${PYX}   # pip3
                       ${PYTHON_DIR}/bin/pip${PYX_X} # pip3.Z
                       ${PYTHON_DIR}/bin/pyvenv
                       ${PYTHON_DIR}/bin/pyvenv-${PYX_X} # pyvenv-3.Z
                       ${PYTHON_DIR}/bin/2to3
                       ${PYTHON_DIR}/bin/2to3-${PYX_X}   #2to3-3.Z
                       )

        foreach(_py_extra ${_py_extras})
            if(EXISTS ${_py_extra})
                THIRD_PARTY_INSTALL_EXECUTABLE(${_py_extra})
            endif()
        endforeach()

        set(filePerms OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ)
        set(dirPerms  OWNER_READ OWNER_WRITE OWNER_EXECUTE
                      GROUP_READ GROUP_WRITE GROUP_EXECUTE
                      WORLD_READ WORLD_EXECUTE)
        # Install the python modules
        # Exclude lib-tk files for now because the permissions are bad on davinci. BJW 12/17/2009
        # Exclude visit module files.
        if(EXISTS ${PYTHON_DIR}/lib/python${PYTHON_VERSION})
            install(DIRECTORY ${PYTHON_DIR}/lib/python${PYTHON_VERSION}
                DESTINATION ${VISIT_INSTALLED_VERSION_LIB}/python/lib
                FILE_PERMISSIONS ${filePerms}
                DIRECTORY_PERMISSIONS ${dirPerms}
                PATTERN "lib-tk" EXCLUDE
                PATTERN "visit.*" EXCLUDE
                PATTERN "visitmodule.*" EXCLUDE
                PATTERN "visit_writer.*" EXCLUDE
                PATTERN "PySide" EXCLUDE
                PATTERN "Python-2.6-py2.6.egg-info" EXCLUDE)
        endif()

        # Install the Python headers
        if(VISIT_HEADERS_SKIP_INSTALL)
            message(STATUS "Skipping python headers installation")
        else()
            # KSB
            #  WIN32  PYTHON_INCLUDE_PATH is
            #    'path-to-python/include'
            #  Non-WIN32 PYTHON_INCLUDE_PATH is:
            #    'path-to-python/include/python<vermaj>.<vermin>m'
            #  So Non-WIN32 needs extra 'include' appended to DESTINATION

            set(pyIncDest ${VISIT_INSTALLED_VERSION_INCLUDE}/python)
            if(NOT WIN32)
                string(APPEND pyIncDest "/include")
            endif()
            install(DIRECTORY ${PYTHON_INCLUDE_PATH}
                DESTINATION ${pyIncDest}
                FILE_PERMISSIONS ${filePerms}
                DIRECTORY_PERMISSIONS ${dirPerms})
        endif()
        #
        # CDH:
        # We also need to install the headers into lib/python dir
        # if we want to be able to install python modules into a visit
        # install.
        #
        set(pyIncDest ${VISIT_INSTALLED_VERSION_LIB}/python)
        if(NOT WIN32)
            string(APPEND pyIncDest "/include")
        endif()
        install(DIRECTORY ${PYTHON_INCLUDE_PATH}
            DESTINATION ${pyIncDest}
            FILE_PERMISSIONS ${filePerms}
            DIRECTORY_PERMISSIONS ${dirPerms})

        if(WIN32)
            # install the import libraries
            install(DIRECTORY ${VISIT_PYTHON_DIR}/lib
                DESTINATION ${VISIT_INSTALLED_VERSION_LIB}/python
                FILE_PERMISSIONS ${filePerms}
                DIRECTORY_PERMISSIONS ${dirPerms})
        endif()
        unset(filePerms)
        unset(dirPerms)
    endif()
endif()

