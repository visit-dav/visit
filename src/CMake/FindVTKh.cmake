# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Wed Oct 31 11:53:01 PDT 2018
#   Added install logic.
#
#   Eric Brugger, Wed Mar  6 16:55:54 PST 2019
#   Adjusted install logic so that VISIT_INSTALL_THIRD_PARTY didn't need
#   to be defined to execute it. Adjusted install logic to only consider
#   libraries that started with vtkh or vtkm.
#
#   Kathleen Biagas, Tue Jan 21 10:46:31 PST 2020
#   Set VTKm_INCLUDE_DIRS.
#
#   Eric Brugger, Thu 21 Jul 2022 06:39:36 PM EDT
#   Modified the logic to install the shared VTK-h libraries in the case
#   where VTK-h was built with RELWITHDEBINFO, which is the case when spack
#   builds VTK-h. Modified the logic to not install static libraries.
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

   set(VTKm_INCLUDE_DIRS "${VTKM_DIR}/include/vtkm-${VTKm_VERSION_MAJOR}.${VTKm_VERSION_MINOR}"
                         "${VTKM_DIR}/include/vtkm-${VTKm_VERSION_MAJOR}.${VTKm_VERSION_MINOR}/vtkm/thirdparty/taotuple"
       CACHE STRING "VTKm include directories")

   # use the vtkh and vtkm CMake properties to find locations and
   # all interface link dependencies
   function(get_lib_loc_and_install _lib)
       get_target_property(ttype ${_lib} TYPE)
       if (ttype STREQUAL "INTERFACE_LIBRARY" OR ttype STREQUAL "STATIC_LIBRARY")
           return()
       endif()
       get_target_property(i_loc ${_lib} IMPORTED_LOCATION)
       if (NOT i_loc)
         get_target_property(i_loc ${_lib} IMPORTED_LOCATION_RELEASE)
       endif()
       if (NOT i_loc)
         get_target_property(i_loc ${_lib} IMPORTED_LOCATION_RELWITHDEBINFO)
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
               string(SUBSTRING "${ll_dep}" 0 4 ll_dep_prefix)
               # only process libraries that start with vtkh or vtkm
               if ("${ll_dep_prefix}" STREQUAL "vtkh" OR
                   "${ll_dep_prefix}" STREQUAL "vtkm")
                   # don't process duplicates
                   if (NOT ${ll_dep} IN_LIST VTKH_INT_LL AND
                       NOT ${ll_dep} IN_LIST addl_ll)
                       get_lib_loc_and_install(${ll_dep})
                       list(APPEND addl_ll ${ll_dep})
                   endif()
               endif()
           endforeach()
       endif()
   endforeach()
   unset(addl_ll)

   if(VISIT_INSTALL_THIRD_PARTY AND NOT VISIT_HEADERS_SKIP_INSTALL)
       THIRD_PARTY_INSTALL_INCLUDE(vtkh ${VISIT_VTKH_DIR}/include)
       THIRD_PARTY_INSTALL_INCLUDE(vtkm ${VTKM_DIR}/include)
   endif()
ENDIF()

