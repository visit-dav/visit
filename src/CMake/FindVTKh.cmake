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
#   Kathleen Biagas, Wed Oct 31 11:53:01 PDT 2018
#   Added install logic.
#
#****************************************************************************/

IF (DEFINED VISIT_VTKH_DIR)
   INCLUDE(${VISIT_VTKH_DIR}/lib/VTKhConfig.cmake)

   SET("VTKH_FOUND" true CACHE BOOL "VTKH library found" FORCE)
   SET("HAVE_LIBVTKH" true CACHE BOOL "Have VTKH library" FORCE)

   MESSAGE(STATUS "  VTKh_INCLUDE_DIRS = ${VTKh_INCLUDE_DIRS}")
   MESSAGE(STATUS "  VTKh_VERSION_MAJOR = ${VTKh_VERSION_MAJOR}")
   MESSAGE(STATUS "  VTKh_VERSION_MINOR = ${VTKh_VERSION_MINOR}")
   MESSAGE(STATUS "  VTKh_VERSION_PATCH = ${VTKh_VERSION_PATCH}")
   MESSAGE(STATUS "  VTKh_VERSION_FULL = ${VTKh_VERSION_FULL}")
   MESSAGE(STATUS "  VTKh_VERSION = ${VTKh_VERSION}")

   MESSAGE(STATUS "  VTKM_DIR = ${VTKM_DIR}")
   MESSAGE(STATUS "  VTKM_FOUND = ${VTKM_FOUND}")
   MESSAGE(STATUS "  VTKm_VERSION_MAJOR = ${VTKm_VERSION_MAJOR}")
   MESSAGE(STATUS "  VTKm_VERSION_MINOR = ${VTKm_VERSION_MINOR}")
   MESSAGE(STATUS "  VTKm_VERSION_PATCH = ${VTKm_VERSION_PATCH}")
   MESSAGE(STATUS "  VTKm_VERSION_FULL = ${VTKm_VERSION_FULL}")
   MESSAGE(STATUS "  VTKm_VERSION = ${VTKm_VERSION}")

   if(VISIT_INSTALL_THIRD_PARTY)
       include(${VISIT_SOURCE_DIR}/CMake/ThirdPartyInstallLibrary.cmake)
       # use the vtkh and vtkm CMake properties to find locations and
       # all interface link dependencies
       function(get_lib_loc_and_install _lib)
           get_target_property(ttype ${_lib} TYPE)
           if (ttype STREQUAL "INTERFACE_LIBRARY")
               return()
           endif()
           get_target_property(i_loc ${_lib} IMPORTED_LOCATION)
           if (NOT i_loc)
             get_target_property(i_loc ${_lib} IMPORTED_LOCATION_RELEASE)
           endif()
           if(i_loc)
               THIRD_PARTY_INSTALL_LIBRARY(${i_loc})
           endif()
       endfunction()

       get_target_property(VTKH_INT_LL vtkh INTERFACE_LINK_LIBRARIES)
       set(addl_ll)
       foreach(vtkhll ${VTKH_INT_LL})
           get_lib_loc_and_install(${vtkhll})
           get_target_property(VTKH_LL_DEP ${vtkhll} INTERFACE_LINK_LIBRARIES)
           if(VTKH_LL_DEP)
               foreach(ll_dep ${VTKH_LL_DEP})
                   # don't process duplicates
                   if (NOT ${ll_dep} IN_LIST VTKH_INT_LL AND
                       NOT ${ll_dep} IN_LIST addl_ll)
                       get_lib_loc_and_install(${ll_dep})
                       list(APPEND addl_ll ${ll_dep})
                   endif()
               endforeach()
           endif()
       endforeach()
       unset(addl_ll)

       if(NOT VISIT_HEADERS_SKIP_INSTALL)
           THIRD_PARTY_INSTALL_INCLUDE(vtkh ${VISIT_VTKH_DIR}/include)
           THIRD_PARTY_INSTALL_INCLUDE(vtkm ${VTKM_DIR}/include)
       endif()
   endif()
ENDIF()
