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
#   Mark C. Miller, Wed Jan 13 18:30:38 PST 2010
#   Added logic to ensure FORCE_ITAPS_REGEN is never cached in an ON state.
#
#   Mark C. Miller, Thu Jan 14 10:36:49 PST 2010
#   Undid above change as it turns out the value gets cached regardless of
#   an SET() command here. This was not observed in earlier tests because
#   I was ctrl-C'ing after the cmake process had past the point I was testing.
#
#   Mark C. Miller, Thu Jan 14 12:01:48 PST 2010
#   Added -D_LARGEFILE64_SOURCE to definitions for UNIX systems. This could
#   be a bit too global of a context in which to set it. We'll see. I also
#   added logic to set CMAKE_REQUIRED_DEFINITIONS prior to checking size
#   of off64_t. In theory, we might need to set CMAKE_REQUIRED_INCLUDES 
#   too but that was not required on the systems I tested.
#
#   David M. Camp, Th Jan 14 11:50:00 PST 2010
#   Added new function ADD_TARGET_DEFINITIONS to add defines to targets.
#   This was needed for the plots to define ENGINE only for the engine build.
#
#****************************************************************************/

CMAKE_MINIMUM_REQUIRED(VERSION 2.6.4 FATAL_ERROR)

IF (COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
ENDIF (COMMAND cmake_policy)


# Tell the CMake makefile generator to not have rules depend on
# themselves.  This causes extra rebuilds when the include path
# changes from turning a kit on or off.
SET(CMAKE_SKIP_RULE_DEPENDENCY 1)

# for now...
SET(BUILD_SHARED_LIBS 1)


#-----------------------------------------------------------------------------
# Get this computer's hostname using cmake's cross platform helper function.
#----------------------------------------------------------------------------

SITE_NAME(HOSTNAME)

#-----------------------------------------------------------------------------
# Include this computer's config-site if one exists
#-----------------------------------------------------------------------------
FUNCTION(VISIT_OPTION_DEFAULT var)
    SET(${var} ${ARGN} CACHE PATH "${var} default value" FORCE)
ENDFUNCTION(VISIT_OPTION_DEFAULT var)
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/CMake/SetupITAPS.cmake)

IF (NOT WIN32)
  MESSAGE(STATUS "Trying to include config-site/${HOSTNAME}.cmake")
  INCLUDE(config-site/${HOSTNAME}.cmake OPTIONAL)
ENDIF (NOT WIN32)


#-----------------------------------------------------------------------------
# Set compiler from config-site
#-----------------------------------------------------------------------------
SET(CMAKE_C_COMPILER ${VISIT_C_COMPILER})
SET(CMAKE_CXX_COMPILER ${VISIT_CXX_COMPILER})

#-----------------------------------------------------------------------------
# VisIt project. Declare the project after including the input because
#                this lets us set up defaults in the config-site.
#-----------------------------------------------------------------------------

PROJECT(VISIT)

#-----------------------------------------------------------------------------
# Set extended platlform defs.
#-----------------------------------------------------------------------------
INCLUDE(${VISIT_SOURCE_DIR}/CMake/SetUpPlatformDefs.cmake)


#-----------------------------------------------------------------------------
# Setup out of source build indicator.
#-----------------------------------------------------------------------------
IF("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    SET(VISIT_OUT_OF_SOURCE_BUILD 0)
ELSE("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    SET(VISIT_OUT_OF_SOURCE_BUILD 1)
ENDIF("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")

#-----------------------------------------------------------------------------
# Windows needs values that are only set at this point, so now include
# the windows cmake file.
#-----------------------------------------------------------------------------
IF (WIN32)
  MESSAGE(STATUS "Trying to include config-site/windows.cmake")
  INCLUDE(config-site/windows.cmake)
ENDIF (WIN32)

#-----------------------------------------------------------------------------
# Output directories.
#-----------------------------------------------------------------------------

IF(NOT LIBRARY_OUTPUT_PATH)
  SET(LIBRARY_OUTPUT_PATH ${VISIT_BINARY_DIR}/lib CACHE INTERNAL "Single output directory for building all libraries.")
ENDIF(NOT LIBRARY_OUTPUT_PATH)

IF(NOT EXECUTABLE_OUTPUT_PATH)
  SET(EXECUTABLE_OUTPUT_PATH ${VISIT_BINARY_DIR}/exe CACHE INTERNAL "Single output directory for building all executables.")
ENDIF(NOT EXECUTABLE_OUTPUT_PATH)

IF (WIN32 AND NOT RUNTIME_OUTPUT_PATH)
      SET(RUNTIME_OUTPUT_PATH ${VISIT_BINARY_DIR}/exe CACHE INTERNAL "Single output directory for building all executables.")
ENDIF(WIN32 AND NOT RUNTIME_OUTPUT_PATH)

SET(VISIT_LIBRARY_DIR ${LIBRARY_OUTPUT_PATH}/${CMAKE_CFG_INTDIR})
SET(VISIT_EXECUTABLE_DIR ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR})
SET(CXX_TEST_PATH ${EXECUTABLE_OUTPUT_PATH})

#-----------------------------------------------------------------------------
# configure options
#-----------------------------------------------------------------------------
OPTION(VISIT_PARALLEL "Build VisIt's parallel compute engine." OFF)

OPTION(VISIT_TUVOK    "Build VisIt with support for the Tuvok volume rendering library." OFF)
OPTION(VISIT_SLIVR    "Build VisIt with support for the SLIVR volume rendering library." ON)
OPTION(VISIT_STATIC   "Build VisIt statically." OFF)
IF(VISIT_STATIC)
    ADD_DEFINITIONS(-DVISIT_STATIC -DGLEW_STATIC)
ENDIF(VISIT_STATIC)
OPTION(VISIT_PYTHON_SCRIPTING      "Build VisIt with Python scripting support." ON)
OPTION(VISIT_BUILD_ALL_PLUGINS     "Build all of VisIt's plugins." OFF)
OPTION(VISIT_BUILD_MINIMAL_PLUGINS "Build a minimal set of VisIt's plugins." OFF)
OPTION(VISIT_ZLIB "Use VisIt's internal libz" OFF)
OPTION(VISIT_JAVA "Build the VisIt Java client interface" OFF)
OPTION(VISIT_SERVER_COMPONENTS_ONLY "Build only vcl, mdserver, engine and their plugins" OFF)
OPTION(VISIT_ENGINE_ONLY "Build only the compute engine and its plugins" OFF)
OPTION(VISIT_DBIO_ONLY "Build only visitconvert and engine plugins" OFF)
# If we're doing DBIO_ONLY then compile all sources with this flag so we don't 
# ever have to do things like include visit-config.h to get it.
IF(VISIT_DBIO_ONLY)
    ADD_DEFINITIONS(-DDBIO_ONLY)
ENDIF(VISIT_DBIO_ONLY)
OPTION(VISIT_DISABLE_SELECT "Disable use of the select() function" OFF)
OPTION(VISIT_USE_NOSPIN_BCAST "Use VisIt's no-spin Bcast in parallel" ON)
OPTION(VISIT_INSTALL_THIRD_PARTY "Install VisIt's 3rd party I/O libs and includes to permit plugin development" OFF)
OPTION(VISIT_NOLINK_MPI_WITH_LIBRARIES "Do not link MPI with VisIt's parallel shared libraries; just with executables" OFF)
OPTION(VISIT_CREATE_SOCKET_RELAY "Create a seprate executable that forwards VisIt's socket connection between engine and component launcher" OFF)

MACRO(VISIT_3RDPARTY_OPTION vvar comment libvar)
    # Create an option 
    OPTION(${vvar} "${comment}" OFF)
    # If the option is not set to OFF (this means that the user passed it on 
    # the cmake command line or defined it in a config-site) then we define 
    # libvar so our find routines can later use it.
    IF(NOT "${${vvar}}" MATCHES "OFF")
        SET(${libvar} ${${vvar}})
    ENDIF(NOT "${${vvar}}" MATCHES "OFF")
ENDMACRO(VISIT_3RDPARTY_OPTION vvar comment libvar)

# Define options that let us pick our standard libraries: VTK,Mesa,Qt,Python and so on.
VISIT_3RDPARTY_OPTION(VISIT_VTK_DIR    "Path containing the VTK library's bin and lib" VTK_DIR)
VISIT_3RDPARTY_OPTION(VISIT_QT_BIN     "Path to the Qt library's bin (path containing qmake)" QT_BIN)
VISIT_3RDPARTY_OPTION(VISIT_PYTHON_DIR "Path containing the Python library's include and lib" PYTHON_DIR)
VISIT_3RDPARTY_OPTION(VISIT_MESA_DIR   "Path containing the Mesa library's include and lib" MESA_DIR)
VISIT_3RDPARTY_OPTION(VISIT_ICET_DIR   "Path containing the Ice-T library's include and lib" ICET_DIR)
VISIT_3RDPARTY_OPTION(VISIT_TCMALLOC_DIR   "Path containing the tcmalloc library's include and lib" TCMALLOC_DIR)

# Define the options that let us pick I/O library installation locations. When the
# library option is defined then we also define the variable name passed as the
# 3rd argument. That variable is what is used in our various Find routines.
VISIT_3RDPARTY_OPTION(VISIT_ADIOS_DIR     "Path containing the ADIOS library's include and lib" ADIOS_DIR)
VISIT_3RDPARTY_OPTION(VISIT_ADVIO_DIR     "Path containing the AdvIO library's include and lib" ADVIO_DIR)
VISIT_3RDPARTY_OPTION(VISIT_BOXLIB2D_DIR  "Path containing the Boxlib 2D library's include and lib" BOXLIB2D_DIR)
VISIT_3RDPARTY_OPTION(VISIT_BOXLIB3D_DIR  "Path containing the Boxlib 3D library's include and lib" BOXLIB3D_DIR)
VISIT_3RDPARTY_OPTION(VISIT_CCMIO_DIR     "Path containing the CCMIO library's include and lib" CCMIO_DIR)
VISIT_3RDPARTY_OPTION(VISIT_CFITSIO_DIR   "Path containing the CFITSIO library's include and lib" CFITSIO_DIR)
VISIT_3RDPARTY_OPTION(VISIT_CGNS_DIR      "Path containing the CGNS library's include and lib" CGNS_DIR)
VISIT_3RDPARTY_OPTION(VISIT_EXODUSII_DIR  "Path containing the ExodusII library's include and lib" EXODUSII_DIR)
VISIT_3RDPARTY_OPTION(VISIT_FASTBIT_DIR   "Path containing the Fastbit library's include and lib" FASTBIT_DIR)
VISIT_3RDPARTY_OPTION(VISIT_GDAL_DIR      "Path containing the GDAL library's include and lib" GDAL_DIR)
VISIT_3RDPARTY_OPTION(VISIT_HDF4_DIR      "Path containing the HDF4 library's include and lib" HDF4_DIR)
VISIT_3RDPARTY_OPTION(VISIT_HDF4_LIBDEP   "HDF4 library dependencies" HDF4_LIBDEP)
VISIT_3RDPARTY_OPTION(VISIT_HDF5_DIR      "Path containing the HDF5 library's include and lib" HDF5_DIR)
VISIT_3RDPARTY_OPTION(VISIT_HDF5_LIBDEP   "HDF5 library dependencies" HDF5_LIBDEP)
VISIT_3RDPARTY_OPTION(VISIT_H5PART_DIR    "Path containing the H5Part library's include and lib" H5PART_DIR)
VISIT_3RDPARTY_OPTION(VISIT_H5PART_LIBDEP "H5Part library dependencies" H5PART_LIBDEP)
VISIT_3RDPARTY_OPTION(VISIT_MILI_DIR      "Path containing the Mili library's include and lib" MILI_DIR)
VISIT_3RDPARTY_OPTION(VISIT_NETCDF_DIR    "Path containing the NETCDF library's include and lib" NETCDF_DIR)
VISIT_3RDPARTY_OPTION(VISIT_SILO_DIR      "Path containing the Silo library's include and lib" SILO_DIR)
VISIT_3RDPARTY_OPTION(VISIT_SILO_LIBDEP   "Silo library dependencies" SILO_LIBDEP)
VISIT_3RDPARTY_OPTION(VISIT_VISUS_DIR     "Path containing the ViSUS library's include and lib" VISUS_DIR)

#-----------------------------------------------------------------------------
# Read the version
#-----------------------------------------------------------------------------
FILE(STRINGS VERSION VERSION)
IF(NOT VERSION)
    SET(VERSION "2.0.0")
ENDIF(NOT VERSION)
SET(VISIT_VERSION ${VERSION})

#-----------------------------------------------------------------------------
# Set up some installation related value and macros (needs version).
#-----------------------------------------------------------------------------

IF(WIN32)
    IF( NOT APPENDED_VISIT_VERSION_TO_INSTALL_PATH)
        SET(CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX} ${VISIT_VERSION}" 
            CACHE PATH "Install path prefix, prepended onto install directories" FORCE)
        SET(APPENDED_VISIT_VERSION_TO_INSTALL_PATH TRUE CACHE INTERNAL "")
    ENDIF( NOT APPENDED_VISIT_VERSION_TO_INSTALL_PATH)
    SET(VISIT_INSTALLED_VERSION_BIN ${CMAKE_INSTALL_PREFIX} CACHE PATH "VisIt bin install directory" FORCE)
    SET(VISIT_INSTALLED_VERSION_LIB ${CMAKE_INSTALL_PREFIX} CACHE PATH "VisIt lib install directory" FORCE)
    SET(VISIT_INSTALLED_VERSION_INCLUDE ${CMAKE_INSTALL_PREFIX}/include CACHE PATH "VisIt include install directory" FORCE)
    SET(VISIT_INSTALLED_VERSION_HELP ${CMAKE_INSTALL_PREFIX}/help CACHE PATH "VisIt help install directory" FORCE)
    SET(VISIT_INSTALLED_VERSION_PLUGINS ${CMAKE_INSTALL_PREFIX} CACHE PATH "VisIt plugins install directory" FORCE)
ELSE(WIN32)
    INCLUDE(${VISIT_SOURCE_DIR}/CMake/DetermineVisItArchitecture.cmake)
    DETERMINE_VISIT_ARCHITECTURE(VISIT_INSTALL_PLATFORM)
    SET(VISIT_INSTALLED_VERSION ${VISIT_VERSION}/${VISIT_INSTALL_PLATFORM} CACHE PATH "VisIt install directory")
    SET(VISIT_INSTALLED_VERSION_BIN ${VISIT_INSTALLED_VERSION}/bin CACHE PATH "VisIt bin install directory")
    SET(VISIT_INSTALLED_VERSION_LIB ${VISIT_INSTALLED_VERSION}/lib CACHE PATH "VisIt lib install directory")
    SET(VISIT_INSTALLED_VERSION_INCLUDE ${VISIT_INSTALLED_VERSION}/include CACHE PATH "VisIt include install directory")
    SET(VISIT_INSTALLED_VERSION_HELP ${VISIT_INSTALLED_VERSION}/help CACHE PATH "VisIt help install directory")
    SET(VISIT_INSTALLED_VERSION_PLUGINS ${VISIT_INSTALLED_VERSION}/plugins CACHE PATH "VisIt plugins install directory")
    SET(VISIT_INSTALLED_VERSION_ARCHIVES ${VISIT_INSTALLED_VERSION}/archives CACHE PATH "3rd party I/O archives install directory")
    INSTALL(CODE "
      MESSAGE(STATUS \"Symlinking current to \${VISIT_VERSION}\")
      EXECUTE_PROCESS(WORKING_DIRECTORY \${CMAKE_INSTALL_PREFIX}
        COMMAND \${CMAKE_COMMAND} -E create_symlink
        \"${VISIT_VERSION}\"
        \"${CMAKE_INSTALL_PREFIX}/current\")
    ")
ENDIF(WIN32)
MARK_AS_ADVANCED(
VISIT_INSTALLED_VERSION_BIN 
VISIT_INSTALLED_VERSION_LIB 
VISIT_INSTALLED_VERSION_HELP 
VISIT_INSTALLED_VERSION_PLUGINS
VISIT_INSTALLED_VERSION_ARCHIVES
VISIT_INSTALLED_VERSION_INCLUDE
)

MACRO(VISIT_INSTALL_TARGETS)
    IF(VISIT_STATIC)
        # Skip installation of static libraries when we build statically
        FOREACH(T ${ARGN})
            GET_TARGET_PROPERTY(pType ${T} TYPE)
            IF(NOT ${pType} STREQUAL "STATIC_LIBRARY")
                INSTALL(TARGETS ${T}
                    RUNTIME DESTINATION ${VISIT_INSTALLED_VERSION_BIN}
                    BUNDLE  DESTINATION ${VISIT_INSTALLED_VERSION_BIN}
                    LIBRARY DESTINATION ${VISIT_INSTALLED_VERSION_LIB}
                    ARCHIVE DESTINATION ${VISIT_INSTALLED_VERSION_ARCHIVES}
                    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
                    CONFIGURATIONS Debug;Release
                )
            ENDIF(NOT ${pType} STREQUAL "STATIC_LIBRARY")
        ENDFOREACH(T)
    ELSE(VISIT_STATIC)
        INSTALL(TARGETS ${ARGN}
            RUNTIME DESTINATION ${VISIT_INSTALLED_VERSION_BIN}
            BUNDLE  DESTINATION ${VISIT_INSTALLED_VERSION_BIN}
            LIBRARY DESTINATION ${VISIT_INSTALLED_VERSION_LIB}
            ARCHIVE DESTINATION ${VISIT_INSTALLED_VERSION_ARCHIVES}
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
            CONFIGURATIONS Debug;Release
        )
        IF(APPLE)
            FOREACH(target ${ARGN})
                GET_TARGET_PROPERTY(type ${target} TYPE)
                IF(${type} STREQUAL "SHARED_LIBRARY")
                    SET(filename lib${target}.dylib)
                    INSTALL(CODE 
                        "EXECUTE_PROCESS(WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}
                             COMMAND /bin/sh ${VISIT_BINARY_DIR}/CMake/osxfixup -lib ${CMAKE_INSTALL_PREFIX}/${VISIT_INSTALLED_VERSION_LIB}/${filename}
                             OUTPUT_VARIABLE OSXOUT)
                         MESSAGE(STATUS \"\${OSXOUT}\")
                        ")
                ELSEIF(${type} STREQUAL "EXECUTABLE")
                    GET_TARGET_PROPERTY(filename ${target} OUTPUT_NAME)
                    IF(filename STREQUAL "filename-NOTFOUND")
                        SET(filename ${target})
                    ENDIF(filename STREQUAL "filename-NOTFOUND")
                    GET_TARGET_PROPERTY(bundle ${target} MACOSX_BUNDLE)
                    IF(${bundle} STREQUAL "ON")
                        INSTALL(CODE 
                            "EXECUTE_PROCESS(WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}
                                COMMAND /bin/sh ${VISIT_BINARY_DIR}/CMake/osxfixup -bundle ${CMAKE_INSTALL_PREFIX}/${VISIT_INSTALLED_VERSION_BIN} ${filename}
                                OUTPUT_VARIABLE OSXOUT)
                             MESSAGE(STATUS \"\${OSXOUT}\")
                            ")
                    ELSE(${bundle} STREQUAL "ON")
                        INSTALL(CODE 
                            "EXECUTE_PROCESS(WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}
                                COMMAND /bin/sh ${VISIT_BINARY_DIR}/CMake/osxfixup -exe ${CMAKE_INSTALL_PREFIX}/${VISIT_INSTALLED_VERSION_BIN}/${filename}
                                OUTPUT_VARIABLE OSXOUT)
                             MESSAGE(STATUS \"\${OSXOUT}\")
                            ")
                    ENDIF(${bundle} STREQUAL "ON")
                ENDIF(${type} STREQUAL "SHARED_LIBRARY")
            ENDFOREACH(target)
        ENDIF(APPLE)
    ENDIF(VISIT_STATIC)
ENDMACRO(VISIT_INSTALL_TARGETS)

MACRO(VISIT_INSTALL_DATABASE_PLUGINS)
    IF(NOT VISIT_STATIC)
        INSTALL(TARGETS ${ARGN}
            LIBRARY DESTINATION ${VISIT_INSTALLED_VERSION_PLUGINS}/databases
            RUNTIME DESTINATION ${VISIT_INSTALLED_VERSION_PLUGINS}/databases
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
            CONFIGURATIONS Debug;Release
        )
        IF(APPLE)
            FOREACH(target ${ARGN})
                SET(filename lib${target}.dylib)
                INSTALL(CODE 
                    "EXECUTE_PROCESS(WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}
                         COMMAND /bin/sh ${VISIT_BINARY_DIR}/CMake/osxfixup -lib ${CMAKE_INSTALL_PREFIX}/${VISIT_INSTALLED_VERSION_PLUGINS}/databases/${filename}
                         OUTPUT_VARIABLE OSXOUT)
                     MESSAGE(STATUS \"\${OSXOUT}\")
                    ")
            ENDFOREACH(target)
        ENDIF(APPLE)
    ENDIF(NOT VISIT_STATIC)
ENDMACRO(VISIT_INSTALL_DATABASE_PLUGINS)

MACRO(VISIT_INSTALL_OPERATOR_PLUGINS)
    IF(NOT VISIT_STATIC)
        INSTALL(TARGETS ${ARGN}
            LIBRARY DESTINATION ${VISIT_INSTALLED_VERSION_PLUGINS}/operators
            RUNTIME DESTINATION ${VISIT_INSTALLED_VERSION_PLUGINS}/operators
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
            CONFIGURATIONS Debug;Release
        )
        IF(APPLE)
            FOREACH(target ${ARGN})
                SET(filename lib${target}.dylib)
                INSTALL(CODE 
                    "EXECUTE_PROCESS(WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}
                         COMMAND /bin/sh ${VISIT_BINARY_DIR}/CMake/osxfixup -lib ${CMAKE_INSTALL_PREFIX}/${VISIT_INSTALLED_VERSION_PLUGINS}/operators/${filename}
                         OUTPUT_VARIABLE OSXOUT)
                     MESSAGE(STATUS \"\${OSXOUT}\")
                    ")
            ENDFOREACH(target)
        ENDIF(APPLE)
    ENDIF(NOT VISIT_STATIC)
ENDMACRO(VISIT_INSTALL_OPERATOR_PLUGINS)

MACRO(VISIT_INSTALL_PLOT_PLUGINS)
    IF(NOT VISIT_STATIC)
        INSTALL(TARGETS ${ARGN}
            LIBRARY DESTINATION ${VISIT_INSTALLED_VERSION_PLUGINS}/plots
            RUNTIME DESTINATION ${VISIT_INSTALLED_VERSION_PLUGINS}/plots
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
            CONFIGURATIONS Debug;Release
        )
        IF(APPLE)
            FOREACH(target ${ARGN})
                SET(filename lib${target}.dylib)
                INSTALL(CODE 
                    "EXECUTE_PROCESS(WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}
                         COMMAND /bin/sh ${VISIT_BINARY_DIR}/CMake/osxfixup -lib ${CMAKE_INSTALL_PREFIX}/${VISIT_INSTALLED_VERSION_PLUGINS}/plots/${filename}
                         OUTPUT_VARIABLE OSXOUT)
                     MESSAGE(STATUS \"\${OSXOUT}\")
                    ")
            ENDFOREACH(target)
        ENDIF(APPLE)
    ENDIF(NOT VISIT_STATIC)
ENDMACRO(VISIT_INSTALL_PLOT_PLUGINS)

# Install the headers for directories that we need in order to build plugins
INSTALL(DIRECTORY avt common engine gui launcher mdserver third_party_builtin viewer visit_vtk visitpy vtkqt winutil
    DESTINATION ${VISIT_INSTALLED_VERSION_INCLUDE}/visit
    FILE_PERMISSIONS OWNER_WRITE OWNER_READ GROUP_WRITE GROUP_READ WORLD_READ
    DIRECTORY_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_WRITE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    CONFIGURATIONS Debug;Release
    FILES_MATCHING PATTERN "*.h"
    PATTERN ".svn" EXCLUDE
    PATTERN "CMakeFiles" EXCLUDE
)

#-----------------------------------------------------------------------------
# Detect packages here. We could probably write macros that we can include from
# elsewhere for this. 
#-----------------------------------------------------------------------------
INCLUDE(${CMAKE_ROOT}/Modules/CheckIncludeFiles.cmake)
INCLUDE(${CMAKE_ROOT}/Modules/CMakeBackwardCompatibilityC.cmake)
INCLUDE(${CMAKE_ROOT}/Modules/CMakeBackwardCompatibilityCXX.cmake)
INCLUDE(${CMAKE_ROOT}/Modules/CheckTypeSize.cmake)
INCLUDE(${CMAKE_ROOT}/Modules/CheckFunctionExists.cmake)
INCLUDE(${CMAKE_ROOT}/Modules/CheckSymbolExists.cmake)
INCLUDE(${CMAKE_ROOT}/Modules/FindX11.cmake)
INCLUDE(${CMAKE_ROOT}/Modules/TestBigEndian.cmake)
#INCLUDE(${CMAKE_ROOT}/Modules/CheckPrototypeExists.cmake)

IF(VISIT_PYTHON_SCRIPTING)
    INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindVisItPython.cmake)
ENDIF(VISIT_PYTHON_SCRIPTING)

IF(VISIT_JAVA)
    INCLUDE(${CMAKE_ROOT}/Modules/FindJava.cmake)

    ENABLE_LANGUAGE(Java)
    SET(CMAKE_Java_COMPILER ${JAVA_COMPILE})
    SET(CMAKE_Java_FLAGS -source 1.4)
    SET(CMAKE_Java_ARCHIVE ${JAVA_ARCHIVE})
ENDIF(VISIT_JAVA)

CHECK_INCLUDE_FILES (fcntl.h     HAVE_FCNTL_H)
CHECK_INCLUDE_FILES (inttypes.h  HAVE_INTTYPES_H)
CHECK_INCLUDE_FILES (malloc.h    HAVE_MALLOC_H)
CHECK_INCLUDE_FILES (limits.h    HAVE_LIMITS_H)
CHECK_INCLUDE_FILES (memory.h    HAVE_MEMORY_H)
CHECK_INCLUDE_FILES (stdint.h    HAVE_STDINT_H)
CHECK_INCLUDE_FILES (stdlib.h    HAVE_STDLIB_H)
CHECK_INCLUDE_FILES (strings.h   HAVE_STRINGS_H)
CHECK_INCLUDE_FILES (string.h    HAVE_STRING_H)
CHECK_INCLUDE_FILES (sys/time.h  HAVE_SYS_TIME_H)
CHECK_INCLUDE_FILES (sys/types.h HAVE_SYS_TYPES_H)
CHECK_INCLUDE_FILES (sys/stat.h  HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILES (unistd.h    HAVE_UNISTD_H)
CHECK_INCLUDE_FILES (zlib.h      HAVE_ZLIB_H)
CHECK_INCLUDE_FILES (stdbool.h   HAVE_STDBOOL_H)

# Configure OpenGL support.
INCLUDE(${CMAKE_ROOT}/Modules/FindOpenGL.cmake)

# Configure Mesa support.
INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindVisItMesa.cmake)

# Configure VTK support.
INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindVisItVTK.cmake)

# Configure Qt4 support.
INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindVisItQt4.cmake)

# Treat GLEW as a 3rd party builtin for now
SET(GLEW_FOUND 1)
SET(HAVE_LIBGLEW 1)
SET(GLEW_INCLUDE_DIR ${VISIT_SOURCE_DIR}/third_party_builtin/glew/glew/include)
SET(GLEW_LIBRARY_DIR ${VISIT_SOURCE_DIR}/lib)
SET(GLEW_LIB GLEW)

# Set up the tuvok library if it was enabled.
IF(VISIT_TUVOK)
    SET(TUVOK_LIB tuvok ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTOPENGL_LIBRARY} GLEW)
ENDIF(VISIT_TUVOK)

# Set up the SLIVR library if it was enabled.
IF(VISIT_SLIVR)
    SET(SLIVR_INCLUDE_DIR 
        ${VISIT_SOURCE_DIR}/third_party_builtin/slivr/teem-1.9.0-src/src
        ${VISIT_SOURCE_DIR}/third_party_builtin/slivr/teem-1.9.0-src/src/teem
        ${VISIT_SOURCE_DIR}/third_party_builtin/slivr
        ${VISIT_SOURCE_DIR}/third_party_builtin/slivr/slivr
    )
    SET(SLIVR_GUI_IMPL QvisCMap2Display.C QvisCMap2Widget.C)
    SET(SLIVR_GUI_HDR QvisCMap2Display.h QvisCMap2Widget.h)
    SET(SLIVR_LIB slivrG slivrV)
ENDIF(VISIT_SLIVR)

# Include Ice-T support if we can.
IF(VISIT_PARALLEL)
    INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindIceT.cmake)
ENDIF(VISIT_PARALLEL)

# Include TCMALLOC support if we can.
INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindTCMALLOC.cmake)

# Configure adios support.
INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindADIOS.cmake)
IF(ADIOS_FOUND)
    SET(HAVE_LIBADIOS 1)
ENDIF(ADIOS_FOUND)

# Configure advio support.
INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindADVIO.cmake)
IF(ADVIO_FOUND)
    SET(HAVE_LIBADVIO 1)
ENDIF(ADVIO_FOUND)

# Configure Boxlib support.
INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindBoxlib.cmake)
IF(BOXLIB2D_FOUND)
    SET(HAVE_LIBBOXLIB2D 1)
ENDIF(BOXLIB2D_FOUND)
IF(BOXLIB3D_FOUND)
    SET(HAVE_LIBBOXLIB3D 1)
ENDIF(BOXLIB3D_FOUND)

# Configure ccmio support.
INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindCCMIO.cmake)
IF(CCMIO_FOUND)
    SET(HAVE_LIBCCMIO 1)
ENDIF(CCMIO_FOUND)

# Configure cfitsio support.
INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindCFITSIO.cmake)
IF(CFITSIO_FOUND)
    SET(HAVE_LIBCFITSIO 1)
ENDIF(CFITSIO_FOUND)

# Configure CGNS support.
INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindCGNS.cmake)
IF(CGNS_FOUND)
    SET(HAVE_LIBCGNS 1)
ENDIF(CGNS_FOUND)

# Configure Fastbit support.
INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindFastBit.cmake)
IF(FASTBIT_FOUND)
    SET(HAVE_LIBFASTBIT 1)
ENDIF(FASTBIT_FOUND)

# Configure gdal support.
INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindGDAL.cmake)
IF(GDAL_FOUND)
    SET(HAVE_LIBGDAL 1)
ENDIF(GDAL_FOUND)

# Configure HDF4 support.
IF(VISIT_STATIC)
  # There's a problem including HDF4 and Netcdf in the same executable due
  # to some netcdf-like symbols in libmfhdf
  MESSAGE(STATUS "HDF4 disabled for static builds due to collision with NetCDF.")
ELSE(VISIT_STATIC)
  INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindHDF4.cmake)
  IF(HDF4_FOUND)
      SET(HAVE_LIBHDF4 1)
  ENDIF(HDF4_FOUND)
ENDIF(VISIT_STATIC)

# Configure HDF5 support.
INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindHDF5.cmake)
IF(HDF5_FOUND)
    SET(HAVE_LIBHDF5 1)
ENDIF(HDF5_FOUND)

# Configure h5part support.
INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindH5Part.cmake)
IF(H5PART_FOUND)
    SET(HAVE_LIBH5PART 1)
ENDIF(H5PART_FOUND)

# Configure ITAPS support -- it's treated specially.
CONFIGURE_ITAPS()

# Configure Mili support.
INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindMili.cmake)
IF(MILI_FOUND)
    SET(HAVE_LIBMILI 1)
ENDIF(MILI_FOUND)

# Configure netcdf support.
INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindNetcdf.cmake)
IF(NETCDF_FOUND)
    SET(HAVE_LIBNETCDF 1)

    # Configure ExodusII support (Depends on NETCDF)
    INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindExodusII.cmake)
    IF(EXODUSII_FOUND)
        SET(HAVE_LIBEXODUSII 1)
    ENDIF(EXODUSII_FOUND)
ENDIF(NETCDF_FOUND)

# Configure Silo support.
INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindSilo.cmake)
IF(SILO_FOUND)
    SET(HAVE_LIBSILO 1)
ENDIF(SILO_FOUND)

# Configure ViSUS support.
INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindVisus.cmake)
IF(VISUS_FOUND)
    SET(HAVE_LIBVISUS 1)
ENDIF(VISUS_FOUND)


IF(CMAKE_HOST_UNIX)
  SET(ZLIB_LIB z)
  SET(DL_LIB dl)
  ADD_DEFINITIONS(-D_LARGEFILE64_SOURCE)
ELSE(CMAKE_HOST_UNIX)
  IF(WIN32)
      SET(ZLIB_LIBRARY_DIR ${VISIT_WINDOWS_DIR}/ThirdParty/zlib/lib/${VISIT_MSVC_VERSION})
      SET(ZLIB_LIB ${ZLIB_LIBRARY_DIR}/zlib1.lib)
  ELSE(WIN32)
      SET(ZLIB_LIB vtkzlib)
  ENDIF(WIN32)
ENDIF(CMAKE_HOST_UNIX)


IF(VISIT_STATIC)
    SET(BUILD_SHARED_LIBS 0)
ENDIF(VISIT_STATIC)

IF(VISIT_PARALLEL)
    IF(VISIT_MPI_COMPILER)
        MESSAGE(STATUS "Setting up MPI using compiler wrapper")
        SET(MPI_COMPILER ${VISIT_MPI_COMPILER})
         # Invoke cmake's built in module for locating & setting up MPI options.
        INCLUDE(${CMAKE_ROOT}/Modules/FindMPI.cmake)
        IF(NOT MPI_FOUND)
            MESSAGE(FATAL_ERROR  "Failed to setup MPI using compiler wrapper: ${VISIT_MPI_COMPILER}")
        ELSE(NOT MPI_FOUND)
            SET(VISIT_PARALLEL_LIBS          ${MPI_LIBRARIES} CACHE STRING "MPI libraries")
            SET(VISIT_PARALLEL_CFLAGS        "-DPARALLEL -I${MPI_INCLUDE_PATH} ${MPI_COMPILE_FLAGS}" CACHE STRING "Parallel CFLAGS")
            SET(VISIT_PARALLEL_CXXFLAGS      "-DPARALLEL -I${MPI_INCLUDE_PATH} ${MPI_COMPILE_FLAGS}" CACHE STRING "Parallel CXXFLAGS")
            SET(VISIT_PARALLEL_LINKER_FLAGS  "${MPI_LINK_FLAGS}" CACHE STRING "Parallel LDFLAGS")
        ENDIF(NOT MPI_FOUND)
    ELSE(VISIT_MPI_COMPILER)
        MESSAGE(STATUS "Setting up MPI using user defined flags")
        IF(VISIT_MPI_LIBS)
            SET(VISIT_PARALLEL_LIBS          ${VISIT_MPI_LIBS} CACHE STRING "MPI libraries")
            IF(VISIT_MPI_C_FLAGS)
                SET(VISIT_PARALLEL_CFLAGS  "-DPARALLEL ${VISIT_MPI_C_FLAGS}" CACHE STRING "Parallel CFLAGS")
            ELSE(VISIT_MPI_C_FLAGS)
                SET(VISIT_PARALLEL_CFLAGS  "-DPARALLEL" CACHE STRING "Parallel CFLAGS")
            ENDIF(VISIT_MPI_C_FLAGS)
            IF(VISIT_MPI_CXX_FLAGS)
                SET(VISIT_PARALLEL_CXXFLAGS  "-DPARALLEL ${VISIT_MPI_CXX_FLAGS}" CACHE STRING "Parallel CXXFLAGS")
            ELSE(VISIT_MPI_CXX_FLAGS)
                SET(VISIT_PARALLEL_CXXFLAGS  "-DPARALLEL" CACHE STRING "Parallel CXXFLAGS")
            ENDIF(VISIT_MPI_CXX_FLAGS)
            IF(VISIT_MPI_LD_FLAGS)
                SET(VISIT_PARALLEL_LINKER_FLAGS "${VISIT_MPI_LD_FLAGS}" CACHE STRING "Parallel LDFLAGS")
            ELSE(VISIT_MPI_LD_FLAGS)
                SET(VISIT_PARALLEL_LINKER_FLAGS "" CACHE STRING "Parallel LDFLAGS")
            ENDIF(VISIT_MPI_LD_FLAGS)
        ELSE(VISIT_MPI_LIBS)
            MESSAGE(FATAL_ERROR "To build parallel VisIt, you must at a minimum define VISIT_MPI_COMPILER or VISIT_MPI_LIBS.
                    You may also define VISIT_MPI_LD_FLAGS, VISIT_MPI_CXX_FLAGS." )
        ENDIF(VISIT_MPI_LIBS)
    ENDIF(VISIT_MPI_COMPILER)

    MESSAGE(STATUS "Parallel version of VisIt")
    MESSAGE(STATUS "    VISIT_PARALLEL_CFLAGS = ${VISIT_PARALLEL_CFLAGS}")
    MESSAGE(STATUS "    VISIT_PARALLEL_CXXFLAGS = ${VISIT_PARALLEL_CXXFLAGS}")
    MESSAGE(STATUS "    VISIT_PARALLEL_LINKER_FLAGS = ${VISIT_PARALLEL_LINKER_FLAGS}")
    MESSAGE(STATUS "    VISIT_PARALLEL_LIBS = ${VISIT_PARALLEL_LIBS}")
ELSE(VISIT_PARALLEL)
    MESSAGE(STATUS "Serial version of VisIt")
ENDIF(VISIT_PARALLEL)

FUNCTION(ADD_PARALLEL_EXECUTABLE target sources)
    SET(allsources ${sources})
    FOREACH (X ${ARGN})
       SET(allsources ${allsources} ${X})
    ENDFOREACH (X ${ARGN})
    ADD_EXECUTABLE(${target} ${allsources})
    IF(VISIT_PARALLEL_CXXFLAGS)
        SET(PAR_COMPILE_FLAGS "")
        FOREACH (X ${VISIT_PARALLEL_CXXFLAGS})
            SET(PAR_COMPILE_FLAGS "${PAR_COMPILE_FLAGS} ${X}")
        ENDFOREACH (X ${VISIT_PARALLEL_CXXFLAGS})
        SET_TARGET_PROPERTIES(${target} PROPERTIES
            COMPILE_FLAGS ${PAR_COMPILE_FLAGS}
        )
	
        IF(VISIT_PARALLEL_LINKER_FLAGS)
            SET(PAR_LINK_FLAGS "")
            FOREACH (X ${VISIT_PARALLEL_LINKER_FLAGS})
                SET(PAR_LINK_FLAGS "${PAR_LINK_FLAGS} ${X}")
            ENDFOREACH (X ${VISIT_PARALLEL_LINKER_FLAGS})
            SET_TARGET_PROPERTIES(${target} PROPERTIES
                LINK_FLAGS ${PAR_LINK_FLAGS}
            )
        ENDIF(VISIT_PARALLEL_LINKER_FLAGS)
        TARGET_LINK_LIBRARIES(${target} ${VISIT_PARALLEL_LIBS})
    ENDIF(VISIT_PARALLEL_CXXFLAGS)
ENDFUNCTION(ADD_PARALLEL_EXECUTABLE)

FUNCTION(ADD_PARALLEL_LIBRARY target sources)
    SET(allsources ${sources})
    FOREACH (X ${ARGN})
       SET(allsources ${allsources} ${X})
    ENDFOREACH (X ${ARGN})
    ADD_LIBRARY(${target} ${allsources})
    IF(VISIT_PARALLEL_CXXFLAGS)
        SET(PAR_COMPILE_FLAGS "")
        FOREACH (X ${VISIT_PARALLEL_CXXFLAGS})
            SET(PAR_COMPILE_FLAGS "${PAR_COMPILE_FLAGS} ${X}")
        ENDFOREACH (X ${VISIT_PARALLEL_CXXFLAGS})
        SET_TARGET_PROPERTIES(${target} PROPERTIES
            COMPILE_FLAGS ${PAR_COMPILE_FLAGS}
        )
        IF(VISIT_PARALLEL_LINKER_FLAGS)
            SET(PAR_LINK_FLAGS "")
            FOREACH (X ${VISIT_PARALLEL_LINKER_FLAGS})
                SET(PAR_LINK_FLAGS "${PAR_LINK_FLAGS} ${X}")
            ENDFOREACH (X ${VISIT_PARALLEL_LINKER_FLAGS})
            SET_TARGET_PROPERTIES(${target} PROPERTIES
                LINK_FLAGS ${PAR_LINK_FLAGS}
            )
        ENDIF(VISIT_PARALLEL_LINKER_FLAGS)
        IF(NOT VISIT_NOLINK_MPI_WITH_LIBRARIES)
            TARGET_LINK_LIBRARIES(${target} ${VISIT_PARALLEL_LIBS})
        ENDIF(NOT VISIT_NOLINK_MPI_WITH_LIBRARIES)
    ENDIF(VISIT_PARALLEL_CXXFLAGS)
ENDFUNCTION(ADD_PARALLEL_LIBRARY)

# Set up easy to use includes for the common directory
SET(VISIT_COMMON_INCLUDES
    ${VISIT_BINARY_DIR}/include
    ${VISIT_SOURCE_DIR}/include
    ${VISIT_SOURCE_DIR}/common/Exceptions/Database
    ${VISIT_SOURCE_DIR}/common/Exceptions/Pipeline
    ${VISIT_SOURCE_DIR}/common/Exceptions/Plotter
    ${VISIT_SOURCE_DIR}/common/comm
    ${VISIT_SOURCE_DIR}/common/expr
    ${VISIT_SOURCE_DIR}/common/icons
    ${VISIT_SOURCE_DIR}/common/misc
    ${VISIT_SOURCE_DIR}/common/parser
    ${VISIT_SOURCE_DIR}/common/plugin
    ${VISIT_SOURCE_DIR}/common/proxybase
    ${VISIT_SOURCE_DIR}/common/state
    ${VISIT_SOURCE_DIR}/common/utility
    ${VISIT_SOURCE_DIR}/common/common/misc
    ${VISIT_SOURCE_DIR}/common/common/plugin
    ${VISIT_SOURCE_DIR}/common/common/state
    ${VISIT_SOURCE_DIR}/common/common/utility
    )


# Override the OPENGL_glu_LIBRARY returned by CMake on the Mac because the
# AGL framework does NOT contain gluTess functions. We want to instead use
# Mesa's GLU functions.
IF(APPLE)
    SET(OPENGL_glu_LIBRARY ${MESA_LIBRARY_DIR}/libMesaGLU.dylib CACHE INTERNAL "AGL lib for OSX")   
ENDIF(APPLE)

# We use libGLU for its tessellation abilities but it requires libGL sometimes.
IF(UNIX AND NOT APPLE)
    SET(TESSELLATION_LIBRARY ${OPENGL_glu_LIBRARY} ${OPENGL_gl_LIBRARY})
ELSE(UNIX AND NOT APPLE)
    SET(TESSELLATION_LIBRARY ${OPENGL_glu_LIBRARY})
ENDIF(UNIX AND NOT APPLE)

# Windows-specific settings
IF (WIN32)
  ADD_DEFINITIONS(-D_USE_MATH_DEFINES)
  IF ("${VISIT_MSVC_VERSION}" STREQUAL "MSVC80")
      ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)
      ADD_DEFINITIONS(-D_SCL_NO_DEPRECATE -D_SCL_SECURE_NO_DEPRECATE)
      ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
  ENDIF ("${VISIT_MSVC_VERSION}" STREQUAL "MSVC80")

  # cmake automatically uses _DEBUG and MDd for Debug flags, but our
  # third-party libs are NOT debug versions, so we won't be able to
  # link against them if those defines are used.  Use our own instead.
  SET (CMAKE_CXX_FLAGS_DEBUG "/MD /ZI /Ob0 /Od /RTC1" CACHE STRING 
       "Flags used by the compiler during debug builds." FORCE)
  SET (CMAKE_C_FLAGS_DEBUG "/MD /Zi /Ob0 /Od /RTC1" CACHE STRING 
       "Flags used by the compiler during debug builds." FORCE)
  SET (CMAKE_EXE_LINKER_FLAGS_DEBUG "/INCREMENTAL:YES" CACHE STRING 
       "Flags used by the linker during debug builds." FORCE)
  SET (CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "/INCREMENTAL:YES" CACHE STRING 
       "Flags used by the linker during debug builds." FORCE)
  SET (CMAKE_MODULE_LINKER_FLAGS_DEBUG "/INCREMENTAL:YES" CACHE STRING 
       "Flags used by the linker during debug builds." FORCE)
  SET (CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO "/INCREMENTAL:YES" CACHE STRING 
       "Flags used by the linker during debug builds." FORCE)
  SET (CMAKE_SHARED_LINKER_FLAGS_DEBUG "/INCREMENTAL:YES" CACHE STRING 
       "Flags used by the linker during debug builds." FORCE)
  SET (CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO "/INCREMENTAL:YES" CACHE STRING 
       "Flags used by the linker during debug builds." FORCE)
ENDIF (WIN32)

#-----------------------------------------------------------------------------
# Read the SVN revision
#-----------------------------------------------------------------------------
#dummy revision, in case commands fail
SET(VISIT_SVN_REVISION "Unknown") 
# Get the subversion revision number
FIND_PACKAGE(Subversion)
IF(Subversion_FOUND)
    IF(UNIX AND EXISTS ${VISIT_SOURCE_DIR}/.svn)
        # Only try and determine the version number if it looks like we're in
        # a working copy (a .svn directory should be present)
        Subversion_WC_INFO(${PROJECT_SOURCE_DIR} VISIT)
        SET(VISIT_SVN_REVISION "${VISIT_WC_REVISION}")
    ENDIF(UNIX AND EXISTS ${VISIT_SOURCE_DIR}/.svn)
#ELSE(Subversion_FOUND)
# THIS NEEDS TO BE UPDATED: FindTortoiseSVN.cmake needs to use keyword substituion and templates!@
#    IF(WIN32)
#        INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindTortoiseSVN.cmake)
#        IF(TortoiseSVN_FOUND)
#            TortoiseSVN_WC_INFO(${PROJECT_SOURCE_DIR} VISIT)
#            SET(VISIT_SVN_REVISION "${VISIT_WC_REVISION}")
#        ENDIF(TortoiseSVN_FOUND)
#    ENDIF(WIN32)
ENDIF(Subversion_FOUND)

#-----------------------------------------------------------------------------
# Create visit-config.h
#-----------------------------------------------------------------------------

# Set the slash characters based on the platform
IF(WIN32)
    SET(VISIT_SLASH_CHAR   "'\\\\'")
    SET(VISIT_SLASH_STRING "\"\\\\\"")
ELSE(WIN32)
    SET(VISIT_SLASH_CHAR   "'/'")
    SET(VISIT_SLASH_STRING "\"/\"")
ENDIF(WIN32)

# Check for plugin extension
IF(VISIT_STATIC)
    SET(VISIT_PLUGIN_EXTENSION   ".a")
ELSE(VISIT_STATIC)
    IF(WIN32)
        SET(VISIT_PLUGIN_EXTENSION   ".dll")
    ELSE(WIN32)
        IF(APPLE)
            SET(VISIT_PLUGIN_EXTENSION   ".dylib")
        ELSE(APPLE)
            SET(VISIT_PLUGIN_EXTENSION   ".so")
        ENDIF(APPLE)
    ENDIF(WIN32)
ENDIF(VISIT_STATIC)

# Check for type sizes, endian
SET(SIZEOF_BOOLEAN              ${CMAKE_SIZEOF_BOOLEAN})
SET(SIZEOF_CHAR                 ${CMAKE_SIZEOF_CHAR})
SET(SIZEOF_DOUBLE               ${CMAKE_SIZEOF_DOUBLE})
SET(SIZEOF_FLOAT                ${CMAKE_SIZEOF_FLOAT})
SET(SIZEOF_INT                  ${CMAKE_SIZEOF_INT})
SET(SIZEOF_LONG                 ${CMAKE_SIZEOF_LONG})
SET(SIZEOF_LONG_DOUBLE          ${CMAKE_SIZEOF_LONG_DOUBLE})
SET(SIZEOF_LONG_FLOAT           ${CMAKE_SIZEOF_LONG_FLOAT})
SET(SIZEOF_LONG_LONG            ${CMAKE_SIZEOF_LONG_LONG})
SET(SIZEOF_SHORT                ${CMAKE_SIZEOF_SHORT})
SET(SIZEOF_UNSIGNED_CHAR        ${CMAKE_SIZEOF_UNSIGNED_CHAR})
SET(SIZEOF_UNSIGNED_INT         ${CMAKE_SIZEOF_UNSIGNED_INT})
SET(SIZEOF_UNSIGNED_LONG        ${CMAKE_SIZEOF_UNSIGNED_LONG})
SET(SIZEOF_UNSIGNED_LONG_LONG   ${CMAKE_SIZEOF_UNSIGNED_LONG_LONG})
SET(SIZEOF_UNSIGNED_SHORT       ${CMAKE_SIZEOF_UNSIGNED_SHORT})
SET(SIZEOF_VOID_P               ${CMAKE_SIZEOF_VOID_P})
SET(CMAKE_REQUIRED_DEFINITIONS -D_LARGEFILE64_SOURCE)
CHECK_TYPE_SIZE("off64_t" SIZEOF_OFF64_T)
TEST_BIG_ENDIAN(WORDS_BIGENDIAN)

# Check for threads
IF(WIN32)
    SET(HAVE_THREADS  1)
ELSE(WIN32)
    SET(HAVE_THREADS  ${CMAKE_USE_PTHREADS})
ENDIF(WIN32)

# manually check for socklen_t as CHECK_SYMBOL_EXISTS
# doesn't appear to work on linux (at least) 
IF ("HAVE_SOCKLEN_T" MATCHES "^HAVE_SOCKLEN_T$")
    MESSAGE(STATUS "Check for socklen_t")
    TRY_COMPILE(HAVE_SOCKLEN_T
        ${CMAKE_CURRENT_BINARY_DIR}
        ${VISIT_SOURCE_DIR}/CMake/TestSocklenT.c
        OUTPUT_VARIABLE SLT
    )
    IF (HAVE_SOCKLEN_T)
        SET(HAVE_SOCKLEN_T 1 CACHE INTERNAL "support for socklen_t")
        MESSAGE(STATUS "Check for socklen_t - found")
    ELSE(HAVE_SOCKLEN_T)
        SET(HAVE_SOCKLEN_T 0 CACHE INTERNAL "support for socklen_t")
        MESSAGE(STATUS "Check for socklen_t - not found")
    ENDIF (HAVE_SOCKLEN_T)
ENDIF ("HAVE_SOCKLEN_T" MATCHES "^HAVE_SOCKLEN_T$")

# Check for setbuf being a public member in fstream. We could test via
# TRY_COMPILE in CMake 2.6.2. I think this is only false on the Suns, which
# we kind of stopped supporting anyway.
SET(NO_SETBUF 1)

# Check for PTY support
IF(NOT WIN32)
    SET(VISIT_USE_PTY 1)
    INCLUDE(${VISIT_SOURCE_DIR}/CMake/TestPTY.cmake)
ENDIF(NOT WIN32)

FUNCTION(CHECK_ISFINITE_EXISTS VAR)
    MESSAGE(STATUS "Check for isfinite")
    TRY_COMPILE(tmpVar
            ${CMAKE_CURRENT_BINARY_DIR}
            ${VISIT_SOURCE_DIR}/CMake/Testisfinite.cpp
            CMAKE_FLAGS -DLINK_LIBRARIES:STRING=m
            OUTPUT_VARIABLE outvar)
    IF(tmpVar)
        SET(${VAR} 1 CACHE INTERNAL "isfinite found")
        MESSAGE(STATUS "Check for isfinite - found")
    ELSE(tmpVar)
        MESSAGE(STATUS "Check for isfinite - not found")
        IF (NOT WIN32)
            MESSAGE("Check for isfinite - ${outvar}")
        ENDIF (NOT WIN32)
    ENDIF(tmpVar)
ENDFUNCTION(CHECK_ISFINITE_EXISTS)

FUNCTION(CHECK_STRTOF_PROTOTYPE VAR)
    MESSAGE(STATUS "Check for strtof prototype")
    TRY_COMPILE(tmpVar
            ${CMAKE_CURRENT_BINARY_DIR}
            ${VISIT_SOURCE_DIR}/CMake/Teststrtofprototype.cpp
            OUTPUT_VARIABLE outvar)
    IF(tmpVar)
        SET(${VAR} 1 CACHE INTERNAL "strtof prototype")
        MESSAGE(STATUS "Check for strtof prototype - found")
    ELSE(tmpVar)
        MESSAGE(STATUS "Check for strtof prototype - not found")
    ENDIF(tmpVar)
ENDFUNCTION(CHECK_STRTOF_PROTOTYPE)

FUNCTION(CHECK_FTIME_PROTOTYPE VAR)
    MESSAGE(STATUS "Check for ftime prototype")
    TRY_COMPILE(tmpVar
            ${CMAKE_CURRENT_BINARY_DIR}
            ${VISIT_SOURCE_DIR}/CMake/Testftimeprototype.cpp
            OUTPUT_VARIABLE outvar)
    IF(tmpVar)
        SET(${VAR} 1 CACHE INTERNAL "ftime prototype")
        MESSAGE(STATUS "Check for ftime prototype - found")
    ELSE(tmpVar)
        MESSAGE(STATUS "Check for ftime prototype - not found")
    ENDIF(tmpVar)
ENDFUNCTION(CHECK_FTIME_PROTOTYPE)

# Check for some functions
CHECK_FUNCTION_EXISTS(memmove HAVE_MEMMOVE)
CHECK_FUNCTION_EXISTS(strerror HAVE_STRERROR)
CHECK_FUNCTION_EXISTS(div HAVE_DIV)
CHECK_FUNCTION_EXISTS(setenv HAVE_SETENV)
CHECK_FUNCTION_EXISTS(strtof HAVE_STRTOF)
CHECK_STRTOF_PROTOTYPE(HAVE_STRTOF_PROTOTYPE)
CHECK_FTIME_PROTOTYPE(HAVE_FTIME_PROTOTYPE)
CHECK_ISFINITE_EXISTS(HAVE_ISFINITE)

# If we're not disabling the select() call then check for it.
IF(NOT VISIT_DISABLE_SELECT)
    CHECK_FUNCTION_EXISTS(select HAVE_SELECT)
ENDIF(NOT VISIT_DISABLE_SELECT)

IF ("VISIT_RENDERING_SIZE_LIMIT" MATCHES "^VISIT_RENDERING_SIZE_LIMIT$")
    MESSAGE(STATUS "Setting VISIT_RENDERING_SIZE_LIMIT to ${OSMESA_SIZE_LIMIT}")
    SET(VISIT_RENDERING_SIZE_LIMIT ${OSMESA_SIZE_LIMIT} CACHE INTERNAL "rendering size limit")
ENDIF ("VISIT_RENDERING_SIZE_LIMIT" MATCHES "^VISIT_RENDERING_SIZE_LIMIT$")

CONFIGURE_FILE(${VISIT_SOURCE_DIR}/include/visit-cmake.h.in
               ${VISIT_BINARY_DIR}/include/visit-config.h @ONLY IMMEDIATE)

#-----------------------------------------------------------------------------
# Create common/misc/ConfigureInfo.C 
#-----------------------------------------------------------------------------

# Store some configure information into a C file (like version, SVN version
# and platform things that change).
CONFIGURE_FILE(${VISIT_SOURCE_DIR}/common/misc/ConfigureInfo.C.in
               ${VISIT_BINARY_DIR}/common/misc/ConfigureInfo.C @ONLY IMMEDIATE)

#-----------------------------------------------------------------------------
# Create CMake/PluginVsInstall.cmake
#-----------------------------------------------------------------------------

# The normal plugin case. We redefine these when we build against an installed
# version of VisIt
SET(VISIT_INCLUDE_DIR ${VISIT_SOURCE_DIR}  CACHE INTERNAL "Path to VisIt's includes")
SET(VISIT_LIBRARY_DIR ${VISIT_SOURCE_DIR}/lib CACHE INTENAL "Path to VisIt's libraries")
SET(VISIT_PLUGIN_DIR  ${VISIT_BINARY_DIR}/plugins CACHE INTERNAL "Path to VisIt's plugins")

# Store some configure information into a C file (like version, SVN version
# and platform things that change).
CONFIGURE_FILE(${VISIT_SOURCE_DIR}/CMake/PluginVsInstall.cmake.in
               ${VISIT_BINARY_DIR}/CMake/PluginVsInstall.cmake @ONLY IMMEDIATE)
INSTALL(FILES ${VISIT_BINARY_DIR}/CMake/PluginVsInstall.cmake
    DESTINATION ${VISIT_INSTALLED_VERSION_INCLUDE}
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ
)

#-----------------------------------------------------------------------------
# Set flags & options from config-site files after everything is set up since
# setting them beforehand can upset some of CMake's own find routines. Use
# quotes to unite the strings or CMake will create a list.
#-----------------------------------------------------------------------------
SET(CMAKE_VERBOSE_MAKEFILE ${VISIT_VERBOSE_MAKEFILE})
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${VISIT_C_FLAGS}")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${VISIT_CXX_FLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${VISIT_EXE_LINKER_FLAGS}")

#-----------------------------------------------------------------------------
# Top-level subdirectories
#-----------------------------------------------------------------------------

FUNCTION(CREATE_PLUGIN_DEPENDENCIES target comp type)
    FOREACH(P ${ARGN})
        # Like E + Pseudocolor + Plot_ser
        SET(deptarget "${comp}${P}${type}")
        SET(dependencies ${dependencies} ${deptarget})
    ENDFOREACH(P)
    # Construct a variable that contains the names of the dependencies so we 
    # can use that later when we link the target's main exe.
    SET(cachevar "${target}_${comp}${type}")
    SET(${cachevar} ${dependencies} CACHE INTERNAL "dependencies for ${target}")
    #MESSAGE("${cachevar} = ${${cachevar}}")
ENDFUNCTION(CREATE_PLUGIN_DEPENDENCIES)

ADD_SUBDIRECTORY(third_party_builtin)
ADD_SUBDIRECTORY(common)
ADD_SUBDIRECTORY(visit_vtk)
ADD_SUBDIRECTORY(avt)
IF(VISIT_DBIO_ONLY)
    ADD_SUBDIRECTORY(tools)
ELSE(VISIT_DBIO_ONLY)
    ADD_SUBDIRECTORY(engine)
ENDIF(VISIT_DBIO_ONLY)
IF(NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)
    ADD_SUBDIRECTORY(launcher)
    ADD_SUBDIRECTORY(mdserver)
    IF(UNIX)
        ADD_SUBDIRECTORY(sim)
    ENDIF(UNIX)
ENDIF(NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)
IF(NOT VISIT_SERVER_COMPONENTS_ONLY AND NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)
    ADD_SUBDIRECTORY(vtkqt)
    ADD_SUBDIRECTORY(winutil)
    ADD_SUBDIRECTORY(viewer)
    ADD_SUBDIRECTORY(gui)
    IF(VISIT_PYTHON_SCRIPTING AND PYTHONLIBS_FOUND)
        IF(NOT VISIT_STATIC)
            ADD_SUBDIRECTORY(visitpy)
            ADD_SUBDIRECTORY(ultrawrapper)
        ENDIF(NOT VISIT_STATIC)
    ENDIF(VISIT_PYTHON_SCRIPTING AND PYTHONLIBS_FOUND)
    IF(VISIT_JAVA)
        ADD_SUBDIRECTORY(java)
    ENDIF(VISIT_JAVA)
ENDIF(NOT VISIT_SERVER_COMPONENTS_ONLY AND NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)

FUNCTION(ADD_TARGET_DEFINITIONS target newDefs)
    GET_TARGET_PROPERTY(currentDefs ${target} COMPILE_DEFINITIONS)
    IF(${currentDefs} MATCHES NOTFOUND)
        SET(allDefs ${newDefs})
    ELSE(${currentDefs} MATCHES NOTFOUND)
        SET(allDefs ${currentDefs} ${newDefs})
    ENDIF(${currentDefs} MATCHES NOTFOUND)
    SET_TARGET_PROPERTIES(${target} PROPERTIES
            COMPILE_DEFINITIONS ${allDefs}
        )
ENDFUNCTION(ADD_TARGET_DEFINITIONS)

IF(NOT VISIT_DBIO_ONLY)
   ADD_SUBDIRECTORY(plots)
   ADD_SUBDIRECTORY(operators)
ENDIF(NOT VISIT_DBIO_ONLY)
ADD_SUBDIRECTORY(databases)

IF(NOT VISIT_SERVER_COMPONENTS_ONLY AND NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)
    # Must come after plugins in case we're doing a static build
    ADD_SUBDIRECTORY(tools)
ENDIF(NOT VISIT_SERVER_COMPONENTS_ONLY AND NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)

# Now that plugins have been added, do the mains last if we're creating static
# binaries. This is needed to link the plugins into the exe.
IF(VISIT_STATIC)
    ADD_SUBDIRECTORY(engine/main)
    IF(NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)
        ADD_SUBDIRECTORY(mdserver/main)
    ENDIF(NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)
    IF(NOT VISIT_SERVER_COMPONENTS_ONLY AND NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)
        ADD_SUBDIRECTORY(gui/main)
        ADD_SUBDIRECTORY(viewer/main)
        IF(VISIT_PYTHON_SCRIPTING AND PYTHONLIBS_FOUND)
            ADD_SUBDIRECTORY(visitpy)
        ENDIF(VISIT_PYTHON_SCRIPTING AND PYTHONLIBS_FOUND)
    ENDIF(NOT VISIT_SERVER_COMPONENTS_ONLY AND NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)
ENDIF(VISIT_STATIC)

ADD_SUBDIRECTORY(bin)
ADD_SUBDIRECTORY(lib)
ADD_SUBDIRECTORY(include)
ADD_SUBDIRECTORY(help)
ADD_SUBDIRECTORY(svn_bin)

#-----------------------------------------------------------------------------
# Add the "data" directory if it exists
#-----------------------------------------------------------------------------
IF(NOT WIN32 AND IS_DIRECTORY ${VISIT_SOURCE_DIR}/../data)
    IF(VISIT_OUT_OF_SOURCE_BUILD)
        ADD_SUBDIRECTORY(${VISIT_SOURCE_DIR}/../data ${CMAKE_CURRENT_BINARY_DIR}/data )
    ELSE(VISIT_OUT_OF_SOURCE_BUILD)
        ADD_SUBDIRECTORY(${VISIT_SOURCE_DIR}/../data ${VISIT_SOURCE_DIR}/../data)
    ENDIF(VISIT_OUT_OF_SOURCE_BUILD)
ENDIF(NOT WIN32 AND IS_DIRECTORY ${VISIT_SOURCE_DIR}/../data)


#-----------------------------------------------------------------------------
# CPack -- This leverages our install targets to provide a "make package" that
#          packages up a VisIt distribution.
#-----------------------------------------------------------------------------
SET(CPACK_PACKAGE_NAME "visit")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "VisIt ${VISIT_VERSION} is a parallel visualization and data analysis tool")
SET(CPACK_PACKAGE_VENDOR "LLNS")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/help/en_US/copyright.html")

# We should probably parse ${VISIT_VERSION} into its component parts instead.
SET(CPACK_PACKAGE_VERSION_MAJOR "2")
SET(CPACK_PACKAGE_VERSION_MINOR "0")
SET(CPACK_PACKAGE_VERSION_PATCH "0")

IF(WIN32)
    SET(CPACK_PACKAGE_FILE_NAME "visit${VISIT_VERSION}")
    SET(CPACK_PACKAGE_INSTALL_DIRECTORY "VisIt ${VISIT_VERSION}")

    SET(CPACK_GENERATOR "NSIS")
ELSE(WIN32)
    STRING(REPLACE "." "_" UL_VERSION ${VISIT_VERSION})
    SET(CPACK_PACKAGE_FILE_NAME "visit${UL_VERSION}.${VISIT_INSTALL_PLATFORM}")

    SET(CPACK_GENERATOR "TGZ")
ENDIF(WIN32)
INCLUDE(CPack)
