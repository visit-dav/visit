#*****************************************************************************
#
# Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
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
#   Kathleen Bonnell, Thu Dec 10 17:53:36 MT 2009
#   Use the same find routines whether on Windows or not.
#
#   Cyrus Harrison, Fri Feb 19 15:41:04 PST 2010
#   Added install of vtk python wrappers (if they exist).
#
#   Cyrus Harrison, Tue Mar  9 07:51:00 PST 2010
#   Added install_name_tool patch of of vtk python wrappers (if they exist)
#
#   Kathleen Bonnell,  Wed Mar 17 10:03:52 MST 2010
#   Prevent '.svn' from being included when installing directories. 
#   Change how python wrappers are handled on windows, due to different
#   VTK directory structure.
#
#****************************************************************************/

INCLUDE(${VISIT_SOURCE_DIR}/CMake/ThirdPartyInstallLibrary.cmake)

# Use the VTK_DIR hint from the config-site .cmake file 

# Declare VTK_USE_MANGLED_MESA as a cache variable so we can access it later.
# We rely on FindVTK to set it to the right value.
SET(VTK_USE_MANGLED_MESA OFF CACHE INTERNAL "Set a cache variable that FindVTK can override")

INCLUDE(${CMAKE_ROOT}/Modules/FindVTK.cmake)

# Add path to the directory that includes MangledMesa to the include 
# directories.
GET_FILENAME_COMPONENT(MANGLEMESADIR ${VTK_DIR}/../../include ABSOLUTE)
SET(VTK_INCLUDE_DIRS ${VTK_INCLUDE_DIRS} ${MANGLEMESADIR})

# Set the VisIt mangled mesa off of the VTK mangled mesa variable.
IF("${VTK_USE_MANGLED_MESA}" STREQUAL "ON")
   MESSAGE(STATUS "VTK uses mangled mesa")
   SET(VISIT_USE_MANGLED_MESA "ON" CACHE BOOL "Use mangled mesa in VisIt")
ENDIF("${VTK_USE_MANGLED_MESA}" STREQUAL "ON")

# Add install commands for all of the VTK libraries. Is there a better way?
IF(APPLE)
    SET(SO_EXT "dylib")
ELSE(APPLE)
    IF(WIN32)
        SET(SO_EXT "dll")
    ELSE(WIN32)
        SET(SO_EXT "so")
    ENDIF(WIN32)
ENDIF(APPLE)

FOREACH(VTKLIB vtkCommon
    vtkCommonPythonD
    vtkDICOMParser
    vtkFiltering
    vtkFilteringPythonD
    vtkGenericFiltering
    vtkGenericFilteringPythonD
    vtkGraphics
    vtkGraphicsPythonD
    vtkHybrid
    vtkHybridPythonD
    vtkIO
    vtkIOPythonD
    vtkImaging
    vtkImagingPythonD
    vtkMPEG2Encode
    vtkRendering
    vtkRenderingPythonD
    vtkVolumeRendering
    vtkVolumeRenderingPythonD
    vtkexpat
    vtkfreetype
    vtkftgl
    vtkjpeg
    vtkpng
    vtksys
    vtktiff
    vtkzlib
)
    IF(WIN32)
        SET(LIBNAME ${VTK_RUNTIME_DIRS}/${VTKLIB}.${SO_EXT})
    ELSE(WIN32)
        SET(LIBNAME ${VTK_LIBRARY_DIRS}/lib${VTKLIB}.${SO_EXT})
    ENDIF(WIN32)
    IF(EXISTS ${LIBNAME})
        THIRD_PARTY_INSTALL_LIBRARY(${LIBNAME})
    ENDIF(EXISTS ${LIBNAME})
ENDFOREACH(VTKLIB)  

# Add install targets for VTK headers too -- but just the vtk-5.0 dir.
# The VTK_INCLUDE_DIRS may contain stuff like /usr/include or the
# Python directory and we just want VTK here.
FOREACH(X ${VTK_INCLUDE_DIRS})
    IF(EXISTS ${X}/vtkActor.h)
        #MESSAGE("Install ${X} to ${VISIT_INSTALLED_VERSION_INCLUDE}/vtk/include")
        INSTALL(DIRECTORY ${X}
            DESTINATION ${VISIT_INSTALLED_VERSION_INCLUDE}/vtk/include
            FILE_PERMISSIONS OWNER_WRITE OWNER_READ GROUP_WRITE GROUP_READ WORLD_READ
            DIRECTORY_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_WRITE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
            PATTERN ".svn" EXCLUDE
        )
        IF(MESA_FOUND AND EXISTS ${X}/../MangleMesaInclude)
          INSTALL(DIRECTORY ${X}/../MangleMesaInclude
            DESTINATION ${VISIT_INSTALLED_VERSION_INCLUDE}/vtk/include
            FILE_PERMISSIONS OWNER_WRITE OWNER_READ GROUP_WRITE GROUP_READ WORLD_READ
            DIRECTORY_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_WRITE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
            PATTERN ".svn" EXCLUDE
          )
        ENDIF()
    ENDIF(EXISTS ${X}/vtkActor.h)
ENDFOREACH(X)

# check for python wrappers
IF (NOT WIN32)
    FILE(GLOB VTK_PY_WRAPPERS_DIR ${VTK_LIBRARY_DIRS}/python*/)
ELSE (NOT WIN32)
    FILE(GLOB VTK_PY_WRAPPERS_DIR ${VTK_LIBRARY_DIRS})
ENDIF (NOT WIN32)

IF(EXISTS ${VTK_PY_WRAPPERS_DIR}/site-packages/vtk)
    MESSAGE(STATUS "Found VTK Python Wrappers - ${VTK_PY_WRAPPERS_DIR}")
    FILE(GLOB VTK_PY_EGG ${VTK_PY_WRAPPERS_DIR}/site-packages/*.egg*)
    FILE(GLOB VTK_PY_MODULE ${VTK_PY_WRAPPERS_DIR}/site-packages/vtk)
    INSTALL(FILES ${VTK_PY_EGG}
            DESTINATION ${VISIT_INSTALLED_VERSION_LIB}/site-packages/
            PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ
           )

    INSTALL(DIRECTORY ${VTK_PY_MODULE}
            DESTINATION ${VISIT_INSTALLED_VERSION_LIB}/site-packages/
            FILE_PERMISSIONS OWNER_WRITE OWNER_READ GROUP_WRITE GROUP_READ WORLD_READ
            DIRECTORY_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_WRITE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
            PATTERN ".svn" EXCLUDE
           )
#
# On OSX we need to patch the lib names in the vtk python wrappers.
#
# Obtain a list of all '.so' libs from the module source directory and
# use these names to create an install rule that executes 'osxfixup'.
# Yes - VTK generates '.so's here instead of 'dylib's ...
#
    IF(APPLE)
        FILE(GLOB vtkpylibs ${VTK_PY_MODULE}/*so)
        FOREACH(vtkpylib ${vtkpylibs})
            GET_FILENAME_COMPONENT(libname ${vtkpylib} NAME)
            INSTALL(CODE
                    "EXECUTE_PROCESS(WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}
                     COMMAND /bin/sh ${VISIT_SOURCE_DIR}/CMake/osxfixup -lib 
                     \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${VISIT_INSTALLED_VERSION_LIB}/site-packages/vtk/${libname}\"
                     OUTPUT_VARIABLE OSXOUT)
                     MESSAGE(STATUS \"\${OSXOUT}\")
                     ")
        ENDFOREACH(vtkpylib ${vtkpylibs})
    ENDIF(APPLE)

    SET(VTK_PYTHON_WRAPPERS_FOUND TRUE)
ELSE(EXISTS ${VTK_PY_WRAPPERS_DIR}/site-packages/vtk)
    SET(VTK_PYTHON_WRAPPERS_FOUND FALSE)
ENDIF(EXISTS ${VTK_PY_WRAPPERS_DIR}/site-packages/vtk)

MARK_AS_ADVANCED(VTK_PYTHON_WRAPPERS_FOUND)

#INSTALL(DIRECTORY vtk
#            DESTINATION ${VISIT_INSTALLED_VERSION_INCLUDE}/vtk/include
#            #FILE_PERMISSIONS OWNER_WRITE OWNER_READ GROUP_WRITE GROUP_READ WORLD_READ
#            DIRE

IF(NOT ${VTK_FOUND})
    MESSAGE(FATAL_ERROR "VTK is required to build VisIt.")
ENDIF(NOT ${VTK_FOUND})
