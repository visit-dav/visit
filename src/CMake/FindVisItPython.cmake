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
#   Also added cleanup of build artifacts left in source by `pip install`.
#
#   Kathleen Biagas, Mon Nov 24, 2025
#   Remove Python 2 logic.
#
#   Kathleen Biagas, Thu Dec 4, 2025
#   Use CMake's find_package to find Python.
#
#   Cyrus Harrison, Wed Jan 21 11:40:15 PST 2026
#   Change python install logic, use explicit include list for modules in
#   site-packages to limit modules installed.
#
#   Kathleen Biagas, Tue Mar 24, 2026
#   Fix DESTINATION for install of python's site-packages.
#
#   Kathleen Biagas, Wed Mar 25, 2026
#   For install of python's site-packages, use PATTERN EXCLUDE to eliminate
#   what we don't want instead of PATTERN to include what we do want, since
#   the latter always seems to include everything, not just the PATTERNs.
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

if(VISIT_PYTHON_DIR)
    message(STATUS "Looking for Python using VISIT_PYTHON_DIR: ${VISIT_PYTHON_DIR}")
    set(Python3_ROOT_DIR ${VISIT_PYTHON_DIR})
    find_package(Python3 3.13 REQUIRED COMPONENTS Interpreter Development)
elseif(PYTHON_DIR)
    message(STATUS "Looking for Python using PYTHON_DIR: ${PYTHON_DIR}")
    set(Python3_ROOT_DIR ${PYTHON_DIR})
    find_package(Python3 3.13 REQUIRED COMPONENTS Interpreter Development)
else()
    message(STATUS "Looking for Python in system locations, if this fails try setting Python3_ROOT_DIR before calling CMake.")
    find_package (Python3 REQUIRED COMPONENTS Interpreter Development)
endif()

if(Python3_FOUND)
    message(STATUS "Python3 version      ${Python3_VERSION}")
    message(STATUS "Python3 executable   ${Python3_EXECUTABLE}")
    message(STATUS "Python3 include dirs ${Python3_INCLUDE_DIRS}")
    message(STATUS "Python3 libraries:   ${Python3_LIBRARIES}")
    if(TARGET Python3::Python)
        set(PYTHONLIBS_FOUND true)
    endif()

    if(NOT PYTHON_DIR)
        if(Python3_ROOT_DIR)
            set(PYTHON_DIR ${Python3_ROOT_DIR})
        else()
           cmake_path(GET Python3_EXECUTABLE PARENT_PATH PYTHON_DIR)
           if(NOT WIN32)
               # parent path of executable is the root dir on Windows
               # but not elsewhere, so move up 1 more dir
               cmake_path(SET PYTHON_DIR NORMALIZE ${PYTHON_DIR}/../)
            endif()
        endif()
    endif()
    message(STATUS "PYTHON_DIR: ${PYTHON_DIR}")

    # set some vars so that the rest of VisIt doesn't need to change
    set(PYTHON_LIBRARY Python3::Python)
    set(PYTHON_EXECUTABLE ${Python3_EXECUTABLE})
    set(PYTHON_INCLUDE_PATH ${Python3_INCLUDE_DIRS})
    set(PYTHON_FOUND true)
    set(PYTHON_COMPAT_VERSION  ${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR})
    set(PYTHON_VERSION  ${PYTHON_COMPAT_VERSION})

    # for pluginVsInstall
    cmake_path(GET Python3_LIBRARIES FILENAME PYLIB)

    execute_process(COMMAND "${PYTHON_EXECUTABLE}" "-c" "import sys; import pip; sys.stdout.write(pip.__version__)"
                    OUTPUT_VARIABLE PYTHON_PIP_VERSION
                    ERROR_VARIABLE  ERROR_FINDING_PYTHON_PIP_MODULE
                    COMMAND_ERROR_IS_FATAL ANY)
    message(STATUS "PYTHON_PIP_VERSION:       ${PYTHON_PIP_VERSION}")

    include(${VISIT_SOURCE_DIR}/CMake/WriteThirdPartySetup.cmake)
    create_lib_setup_cmake(NAME "PYTHON"
                           NAMESPACE "Python3::"
                           INCBASE "python"
                           ITEMS Python3::Python Python3::Interpreter
                           SIMPLE_INCLUDE true)

    set(fname ${VISIT_BINARY_DIR}/SetupPYTHON.cmake)
    file(APPEND ${fname} "\nset(PYTHON_LIBRARY Python3::Python)\n")
    file(APPEND ${fname} "set(PYTHON_EXECUTABE Python3::Interpreter)\n\n")
else()
    message("Python3 not found")
endif()


set(PYTHON_FOUND ${Python3_FOUND})


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
        target_link_libraries(${_NAME} PRIVATE ${PYTHON_LIBRARY})
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

    # Use a stamp file to track when the pip comand was last executed
    # wrt its dependencies
    set(stamp ${CMAKE_CURRENT_BINARY_DIR}/${args_NAME}.stamp)

    # add some cleanup (rm -rf) for the build artifacts left in source
    # by pip-install
    # since the egg-info dir for flow_vpe doesn't match its dirname
    # (flow.egg-info instead of visit_flow_vpe.egg-info)
    # there isn't a reliable way to only delete it when visit_flow_vpe
    # calls this function. So, go ahead and add it to all the pip installs
    # the 'rm -rf' will not error out if the dir doesn't exist
    add_custom_command(OUTPUT ${stamp}
            COMMAND ${PYTHON_EXECUTABLE} -m pip install . -V --upgrade
            --disable-pip-version-check --no-warn-script-location
            --target "${abs_dest_path}"
            COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_CURRENT_SOURCE_DIR}/build
            COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_CURRENT_SOURCE_DIR}/${args_PY_MODULE_DIR}.egg-info
            COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_CURRENT_SOURCE_DIR}/flow.egg-info
            COMMAND ${CMAKE_COMMAND} -E touch ${stamp}
            DEPENDS  ${args_PY_SETUP_FILE} ${args_PY_SOURCES}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

    # The pip command wipes the --target directory, so all dependent modules need
    # to be linked afterwards. Propagate this dependency as a usage requirement.
    add_library(${args_NAME} INTERFACE ${stamp})
    set_property(TARGET ${args_NAME} APPEND PROPERTY INTERFACE_LINK_DEPENDS ${stamp})

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
    target_link_libraries(${args_NAME} PRIVATE "${args_NAME}_py_setup")
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
    foreach(pylib ${Python3_LIBRARIES})
        THIRD_PARTY_INSTALL_LIBRARY(${pylib})
    endforeach()

    # Only install Python support files if we are not using the system Python
    if((NOT ${PYTHON_DIR} STREQUAL "/usr"))
        # Install the python executable
        set(PYX ${Python3_VERSION_MAJOR})
        set(PYX_X ${PYTHON_COMPAT_VERSION})
        THIRD_PARTY_INSTALL_EXECUTABLE(${PYTHON_DIR}/bin/python ${PYTHON_DIR}/bin/python${PYX} ${PYTHON_DIR}/bin/python${PYX_X})
        # note: these can be symlinks to python3.Zm-config
        THIRD_PARTY_INSTALL_EXECUTABLE(${PYTHON_DIR}/bin/python${PYX}-config ${PYTHON_DIR}/bin/python${PYX_X}-config)
        THIRD_PARTY_INSTALL_EXECUTABLE(${PYTHON_DIR}/bin/python${PYX_X}m-config) # python3.Zm-config

        # install extra python front end scripts if they exist
        set(_py_extras ${PYTHON_DIR}/bin/pip${PYX}   # pip3
                       ${PYTHON_DIR}/bin/pip${PYX_X} # pip3.Z
                       ${PYTHON_DIR}/bin/pyvenv
                       ${PYTHON_DIR}/bin/pyvenv-${PYX_X} # pyvenv-3.Z
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
        if(EXISTS ${PYTHON_DIR}/lib/python${PYTHON_VERSION})
            install(DIRECTORY ${PYTHON_DIR}/lib/python${PYTHON_VERSION}
                    DESTINATION ${VISIT_INSTALLED_VERSION_LIB}/python/lib
                    FILE_PERMISSIONS ${filePerms}
                    DIRECTORY_PERMISSIONS ${dirPerms}
                    PATTERN "site-packages" EXCLUDE)

            # Use a separate install for general site packages, with an
            # include list for modules thats users of visit will want.
            # This filters out several packages related to building and
            # creating our docs that are not needed by users -- some of
            # which may be flagged by security scans over time, causing
            # visit to get blocked.
            # Using 'PATTERN' for the items we want to include does not seem
            # to work, everything is always included.
            # 'PATTERN EXCLUDE' is effective, if onerous due to the excessive
            # numer of packages
            
            install(DIRECTORY ${PYTHON_DIR}/lib/python${PYTHON_VERSION}/site-packages
                    DESTINATION ${VISIT_INSTALLED_VERSION_LIB}/python/lib/python${PYTHON_VERSION}/
                    FILE_PERMISSIONS ${filePerms}
                    DIRECTORY_PERMISSIONS ${dirPerms}
                    ## basic build tools we allow
                    #PATTERN "distutils"
                    #PATTERN "pip"
                    #PATTERN "setuptools"
                    #PATTERN "wheel"
                    ## modules users may want
                    #PATTERN "pillow"
                    #PATTERN "PIL"
                    #PATTERN "numpy"
                    #PATTERN "Cython"
                    #PATTERN "cython"
                    #PATTERN "mpi4py"
                    # what we don't want
                    PATTERN "alabaster*" EXCLUDE
                    PATTERN "babel*" EXCLUDE
                    PATTERN "calver*" EXCLUDE
                    PATTERN "certifi*" EXCLUDE
                    PATTERN "charset*" EXCLUDE
                    PATTERN "colorama*" EXCLUDE
                    PATTERN "docutils*" EXCLUDE
                    PATTERN "editables*" EXCLUDE
                    PATTERN "flit*" EXCLUDE
                    PATTERN "hatch*" EXCLUDE
                    PATTERN "idna*" EXCLUDE
                    PATTERN "image*" EXCLUDE
                    PATTERN "importlib*" EXCLUDE
                    PATTERN "jinja*" EXCLUDE
                    PATTERN "markup*" EXCLUDE
                    PATTERN "meson*" EXCLUDE
                    PATTERN "packaging*" EXCLUDE
                    PATTERN "pathspec*" EXCLUDE
                    PATTERN "pluggy*" EXCLUDE
                    PATTERN "pybind11*" EXCLUDE
                    PATTERN "pygments*" EXCLUDE
                    PATTERN "pyproject*" EXCLUDE
                    PATTERN "requests*" EXCLUDE
                    PATTERN "roman*" EXCLUDE
                    PATTERN "scikit*" EXCLUDE
                    PATTERN "snowball*" EXCLUDE
                    PATTERN "sphinx*" EXCLUDE
                    PATTERN "trove*" EXCLUDE
                    PATTERN "urllib*" EXCLUDE
                    PATTERN "zipp*" EXCLUDE
            )

        endif()

        # Install the Python headers
        if(VISIT_HEADERS_SKIP_INSTALL)
            message(STATUS "Skipping python headers installation")
        else()
            install(DIRECTORY ${PYTHON_INCLUDE_PATH}/
                DESTINATION ${VISIT_INSTALLED_VERSION_INCLUDE}/python
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

