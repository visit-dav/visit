#*****************************************************************************
#
# Copyright (c) 2000 - 2018, Lawrence Livermore National Security, LLC
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
#
#****************************************************************************/

# Use the VTKM_DIR hint from the config-site .cmake file 

INCLUDE(${VISIT_SOURCE_DIR}/CMake/SetUpThirdParty.cmake)

SET_UP_THIRD_PARTY(VTKM lib include NO_LIBS)

IF ("${VTKM_FOUND}")
   set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${VISIT_VTKM_DIR}/lib/cmake/vtkm-1.1)
   #set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${VTKM_INCLUDE_DIR})
   set(BOOST_INCLUDEDIR ${VISIT_BOOST_DIR}/include)
   find_package(VTKm REQUIRED COMPONENTS Serial OPTIONAL_COMPONENTS CUDA TBB)

   SET(VTKM_INCLUDE_DIR ${VTKm_INCLUDE_DIRS})
   SET(VTKM_LIBRARY_DIR ${VISIT_VTKM_DIR}/lib)
   SET(VTKM_LIB ${VTKm_LIBRARIES})

   # Add cuda.
   IF (VTKm_CUDA_FOUND)
     SET(VTKM_INCLUDE_DIR "${VTKM_INCLUDE_DIR};${CUDA_INCLUDE_DIRS}"     CACHE STRING "VTKm include" FORCE)
     SET(VTKM_LIB "${VTKm_LIBRARIES};${CUDA_CUDART_LIBRARY}"             CACHE STRING "VTKm library" FORCE)
   ENDIF()

   # Make sure we install the libraries. This is a bit of a hack.
   FOREACH(X ${VTKM_LIB})
       IF(NOT ${X} STREQUAL "vtkm")
           IF(EXISTS ${X})
               MESSAGE(STATUS "Installing ${X}")
               THIRD_PARTY_INSTALL_LIBRARY(${X})
           ELSE()
               FOREACH(EXT .a;.so;.dylib;.dll;-1.1.a;-1.1.so;-1.1.dylib;-1.1.dll)
                   SET(curlib ${VISIT_VTKM_DIR}/lib/lib${X}${EXT})
                   IF(EXISTS ${curlib})
                       MESSAGE(STATUS "Installing ${curlib}")
                       THIRD_PARTY_INSTALL_LIBRARY(${curlib})
                  ENDIF()
               ENDFOREACH(EXT)
           ENDIF()
       ENDIF()
   ENDFOREACH(X)

   MESSAGE(STATUS "  VTKm_LIBRARIES = ${VTKm_LIBRARIES}")
   MESSAGE(STATUS "  VTKm_INCLUDE_DIRS = ${VTKm_INCLUDE_DIRS}")
   MESSAGE(STATUS "  VTKm_LIBRARIES = ${VTKm_LIBRARIES}")
   MESSAGE(STATUS "  VTKM_FOUND = ${VTKM_FOUND}")
   MESSAGE(STATUS "  VTKM_INCLUDE_DIR = ${VTKM_INCLUDE_DIR}")
   MESSAGE(STATUS "  VTKM_LIBRARY_DIR = ${VTKM_LIBRARY_DIR}")
   MESSAGE(STATUS "  VTKM_LIB = ${VTKM_LIB}")
   MESSAGE(STATUS "  VTKm_CUDA_FOUND = ${VTKm_CUDA_FOUND}")
   MESSAGE(STATUS "  CUDA_INCLUDE_DIRS = ${CUDA_INCLUDE_DIRS}")
   MESSAGE(STATUS "  CUDA_CUDART_LIBRARY = ${CUDA_CUDART_LIBRARY}")
ENDIF()

