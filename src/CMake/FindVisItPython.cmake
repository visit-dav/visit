#*****************************************************************************
#
# Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
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

# Modified from CMakeFindFrameworks to take firstdir argument, which lets us
# find our own framework path ahead of the system's.
#MACRO(VISIT_FIND_FRAMEWORKS fwk firstdir)
#  SET(${fwk}_FRAMEWORKS)
#  IF(APPLE)
#    FOREACH(dir ${firstdir}/${fwk}.framework)
#      IF(EXISTS ${dir})
#        SET(${fwk}_FRAMEWORKS ${${fwk}_FRAMEWORKS} ${dir})
#      ENDIF(EXISTS ${dir})
#    ENDFOREACH(dir)
#  ENDIF(APPLE)
#ENDMACRO(VISIT_FIND_FRAMEWORKS)

# Search for the python framework on Apple.
#IF(APPLE)
#    VISIT_FIND_FRAMEWORKS(Python ${PYTHON_DIR})
#ENDIF(APPLE)

MESSAGE(STATUS "Looking for Python")

FOREACH(_CURRENT_VERSION 2.7 2.6 2.5 2.4 2.3 2.2 2.1 2.0 1.6 1.5)
  STRING(REPLACE "." "" _CURRENT_VERSION_NO_DOTS ${_CURRENT_VERSION})
  IF(WIN32)
    FIND_LIBRARY(PYTHON_DEBUG_LIBRARY
      NAMES python${_CURRENT_VERSION_NO_DOTS}_d python
      PATHS
      ${PYTHON_DIR}/lib
      ${PYTHON_DIR}/libs
      [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\${_CURRENT_VERSION}\\InstallPath]/libs/Debug
      [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\${_CURRENT_VERSION}\\InstallPath]/libs)
  ENDIF(WIN32)

  FIND_LIBRARY(PYTHON_LIBRARY
    NAMES python${_CURRENT_VERSION_NO_DOTS} python${_CURRENT_VERSION}
    PATHS
      ${PYTHON_DIR}/lib
      ${PYTHON_DIR}/libs
      [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\${_CURRENT_VERSION}\\InstallPath]/libs
    NO_DEFAULT_PATH
    NO_CMAKE_ENVIRONMENT_PATH
    NO_CMAKE_PATH
    NO_SYSTEM_ENVIRONMENT_PATH)

  FIND_PROGRAM(PYTHON_EXECUTABLE
               NAMES python2.7 python2.6 python2.5 python
               PATHS
               ${PYTHON_DIR}/bin
               ${PYTHON_DIR}
               [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\${_CURRENT_VERSION}\\InstallPath]
               NO_DEFAULT_PATH
               NO_CMAKE_ENVIRONMENT_PATH
               NO_CMAKE_PATH
               NO_SYSTEM_ENVIRONMENT_PATH)


  SET(PYTHON_FRAMEWORK_INCLUDES)
  IF(Python_FRAMEWORKS AND NOT PYTHON_INCLUDE_PATH)
    FOREACH(dir ${Python_FRAMEWORKS})
      SET(PYTHON_FRAMEWORK_INCLUDES ${PYTHON_FRAMEWORK_INCLUDES}
        ${dir}/Versions/${_CURRENT_VERSION}/include/python${_CURRENT_VERSION})
    ENDFOREACH(dir)
  ENDIF(Python_FRAMEWORKS AND NOT PYTHON_INCLUDE_PATH)

  FIND_PATH(PYTHON_INCLUDE_PATH
    NAMES Python.h
    PATHS
      ${PYTHON_FRAMEWORK_INCLUDES}
      ${PYTHON_DIR}/include
      [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\${_CURRENT_VERSION}\\InstallPath]/include
    PATH_SUFFIXES
      python${_CURRENT_VERSION}
    NO_DEFAULT_PATH
    NO_CMAKE_ENVIRONMENT_PATH
    NO_CMAKE_PATH
    NO_SYSTEM_ENVIRONMENT_PATH)

  IF (PYTHON_LIBRARY AND PYTHON_INCLUDE_PATH)
      # The python library and include path could be for a path that does
      # not match the version from _CURRENT_VERSION so let's match again
      # against the detected filename.
      GET_FILENAME_COMPONENT(PYLIB ${PYTHON_LIBRARY} NAME)
      FOREACH(CV 2.7 2.6 2.5 2.4 2.3 2.2 2.1 2.0 1.6 1.5)
          IF(WIN32)
              STRING(REPLACE "." "" CV2 ${CV})
	      # Look for the debug verision if it is there.
              if (CMAKE_BUILD_TYPE STREQUAL Debug)
                  SET(curPYLIB "python${CV2}_d.lib")
                  IF(${PYLIB} MATCHES ${curPYLIB})
                      SET(PYTHON_VERSION ${CV})
                      MESSAGE(STATUS "Python version: ${PYTHON_VERSION}")
                      BREAK()
                  ENDIF(${PYLIB} MATCHES ${curPYLIB})
              endif ()

	      # Look for the base version.
              SET(curPYLIB "python${CV2}.lib")
              IF(${PYLIB} MATCHES ${curPYLIB})
                  SET(PYTHON_VERSION ${CV})
                  MESSAGE(STATUS "Python version: ${PYTHON_VERSION}")
                  BREAK()
              ENDIF(${PYLIB} MATCHES ${curPYLIB})	      
          ELSE(WIN32)
	      # Look for the base version.
              SET(curPYLIB "libpython${CV}.")
              IF(${PYLIB} MATCHES ${curPYLIB})
                  SET(PYTHON_VERSION ${CV})
                  MESSAGE(STATUS "Python version: ${PYTHON_VERSION}")
                  BREAK()
              ENDIF(${PYLIB} MATCHES ${curPYLIB})
          ENDIF(WIN32)
      ENDFOREACH(CV)
      BREAK()
  ENDIF (PYTHON_LIBRARY AND PYTHON_INCLUDE_PATH)

ENDFOREACH(_CURRENT_VERSION)

# If we want to be strict about python, add this clause:
#IF(NOT PYTHON_VERSION)
#MESSAGE(FATAL_ERROR "Python is required to build VisIt.")
#ENDIF(NOT PYTHON_VERSION)


MARK_AS_ADVANCED(
  PYTHON_DEBUG_LIBRARY
  PYTHON_LIBRARY
  PYTHON_INCLUDE_PATH
  PYTHON_EXECUTABLE
)

# Python Should be built and installed as a Framework on OSX
IF(Python_FRAMEWORKS)
  # Get the Python framework path from the include directory.
  STRING(REGEX MATCH "[A-za-z/\ -._0-9]*Python.framework" fdir ${PYTHON_INCLUDE_PATH})
  IF(${fdir} MATCHES "")
    SET(fdir ${PYTHON_DIR})
  ELSE(${fdir} MATCHES "")
    STRING(REPLACE "/Python.framework" "" fdir ${fdir})
  ENDIF(${fdir} MATCHES "")

  # If a framework has been selected for the include path,
  # make sure "-framework" is used to link it.
  IF("${PYTHON_INCLUDE_PATH}" MATCHES "Python\\.framework")
    SET(PYTHON_LIBRARY "")
    SET(PYTHON_DEBUG_LIBRARY "")
  ENDIF("${PYTHON_INCLUDE_PATH}" MATCHES "Python\\.framework")
  IF(NOT PYTHON_LIBRARY)
    SET (PYTHON_LIBRARY "-F${fdir} -framework Python" CACHE FILEPATH "Python Framework" FORCE)
  ENDIF(NOT PYTHON_LIBRARY)
  IF(NOT PYTHON_DEBUG_LIBRARY)
    SET (PYTHON_DEBUG_LIBRARY "-F${fdir} -framework Python" CACHE FILEPATH "Python Framework" FORCE)
  ENDIF(NOT PYTHON_DEBUG_LIBRARY)
ENDIF(Python_FRAMEWORKS)

# We use PYTHON_LIBRARY and PYTHON_DEBUG_LIBRARY for the cache entries
# because they are meant to specify the location of a single library.
# We now set the variables listed by the documentation for this
# module.
SET(PYTHON_LIBRARIES "${PYTHON_LIBRARY}")
SET(PYTHON_DEBUG_LIBRARIES "${PYTHON_DEBUG_LIBRARY}")


INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PythonLibs DEFAULT_MSG PYTHON_LIBRARIES PYTHON_INCLUDE_PATH)

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

#    TARGET_LINK_LIBRARIES(${_NAME} ${PYTHON_LIBRARIES})

  ENDIF(PYTHON_ENABLE_MODULE_${_NAME})
ENDFUNCTION(PYTHON_ADD_MODULE)

FUNCTION(PYTHON_WRITE_MODULES_HEADER _filename)

  GET_PROPERTY(PY_STATIC_MODULES_LIST  GLOBAL  PROPERTY PY_STATIC_MODULES_LIST)

  GET_FILENAME_COMPONENT(_name "${_filename}" NAME)
  STRING(REPLACE "." "_" _name "${_name}")
  STRING(TOUPPER ${_name} _name)

  SET(_filenameTmp "${_filename}.in")
  FILE(WRITE ${_filenameTmp} "/*Created by cmake, do not edit, changes will be lost*/\n")
  FILE(APPEND ${_filenameTmp} 
"#ifndef ${_name}
#define ${_name}

#include <Python.h>

#ifdef __cplusplus
extern \"C\" {
#endif /* __cplusplus */

")

  FOREACH(_currentModule ${PY_STATIC_MODULES_LIST})
    FILE(APPEND ${_filenameTmp} "extern void init${PYTHON_MODULE_PREFIX}${_currentModule}(void);\n\n")
  ENDFOREACH(_currentModule ${PY_STATIC_MODULES_LIST})

  FILE(APPEND ${_filenameTmp} 
"#ifdef __cplusplus
}
#endif /* __cplusplus */

")


  FOREACH(_currentModule ${PY_STATIC_MODULES_LIST})
    FILE(APPEND ${_filenameTmp} "int CMakeLoadPythonModule_${_currentModule}(void) \n{\n  static char name[]=\"${PYTHON_MODULE_PREFIX}${_currentModule}\"; return PyImport_AppendInittab(name, init${PYTHON_MODULE_PREFIX}${_currentModule});\n}\n\n")
  ENDFOREACH(_currentModule ${PY_STATIC_MODULES_LIST})

  FILE(APPEND ${_filenameTmp} "#ifndef EXCLUDE_LOAD_ALL_FUNCTION\nvoid CMakeLoadAllPythonModules(void)\n{\n")
  FOREACH(_currentModule ${PY_STATIC_MODULES_LIST})
    FILE(APPEND ${_filenameTmp} "  CMakeLoadPythonModule_${_currentModule}();\n")
  ENDFOREACH(_currentModule ${PY_STATIC_MODULES_LIST})
  FILE(APPEND ${_filenameTmp} "}\n#endif\n\n#endif\n")
  
# with CONFIGURE_FILE() cmake complains that you may not use a file created using FILE(WRITE) as input file for CONFIGURE_FILE()
  EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_filenameTmp}" "${_filename}" OUTPUT_QUIET ERROR_QUIET)

ENDFUNCTION(PYTHON_WRITE_MODULES_HEADER)

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
