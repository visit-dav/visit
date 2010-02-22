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
#   Kathleen Bonnell, Thu Dec 10 17:53:36 MT 2009
#   Use the same find routines whether on Windows or not.
#
#   Cyrus Harrison, Fri Feb 19 15:41:04 PST 2010
#   Added install of vtk python wrappers (if they exist).
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
        )
        INSTALL(DIRECTORY ${X}/../MangleMesaInclude
            DESTINATION ${VISIT_INSTALLED_VERSION_INCLUDE}/vtk/include
            FILE_PERMISSIONS OWNER_WRITE OWNER_READ GROUP_WRITE GROUP_READ WORLD_READ
            DIRECTORY_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_WRITE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        )
    ENDIF(EXISTS ${X}/vtkActor.h)
ENDFOREACH(X)

# check for python wrappers
FILE(GLOB VTK_PY_WRAPPERS_DIR ${VTK_LIBRARY_DIRS}/python*/)

IF(EXISTS ${VTK_PY_WRAPPERS_DIR})
    MESSAGE(STATUS ${VTK_PY_WRAPPERS_DIR})
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
           )
    SET(VTK_PYTHON_WRAPPERS_FOUND TRUE)
ELSE(EXISTS ${VTK_PY_WRAPPERS_DIR})
    SET(VTK_PYTHON_WRAPPERS_FOUND FALSE)
ENDIF(EXISTS ${VTK_PY_WRAPPERS_DIR})

MARK_AS_ADVANCED(VTK_PYTHON_WRAPPERS_FOUND)

#INSTALL(DIRECTORY vtk
#            DESTINATION ${VISIT_INSTALLED_VERSION_INCLUDE}/vtk/include
#            #FILE_PERMISSIONS OWNER_WRITE OWNER_READ GROUP_WRITE GROUP_READ WORLD_READ
#            DIRE

IF(NOT ${VTK_FOUND})
    MESSAGE(FATAL_ERROR "VTK is required to build VisIt.")
ENDIF(NOT ${VTK_FOUND})
