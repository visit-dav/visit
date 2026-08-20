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
#   Eric Brugger, Fri Feb 24 14:57:15 PST 2023
#   Renamed to FindVTKm.cmake and modified to find vtkm.
#
#   Kathleen Biagas, Wed Mar 15, 2023
#   Changed 'if(DEFINED VISIT_VTKM_DIR)' to 'if(VISIT_VTKM_DIR)' since
#   the var is always defined, but may be empty. The new test will return
#   return true only if the var is not empty. This fixes the case where
#   VISIT_VTKM_DIR hasn't been set by config-site file.
#
#   Eric Brugger, Wed Sep  4 10:31:31 PT 2024
#   Modified the logic to also install static libraries. I also fixed a
#   bug that prevented installing dependent libraries.
#
#   Eric Brugger, Mon Jun 16 13:38:54 PDT 2025
#   Replace vtkm_filter with vtkm::filter.
#
#   Kathleen Biagas, Mon Aug 17, 2026
#   Use new visit_install_thirdparty_targets function to propery install
#   vtkm targets without guessing configuration type.
#
#   Kathleen Biagas, Thur Aug 20, 2026
#   Add call to generate_lib_setup_cmake.
#
#****************************************************************************/

if (VISIT_VTKM_DIR)
   file(GLOB VTKm_DIR "${VISIT_VTKM_DIR}/lib/cmake/vtkm-*")
   if(NOT VTKm_DIR)
      message(FATAL_ERROR "Failed to find VTKm at VTKM_DIR=${VTKM_DIR}/lib/cmake/vtk-*")
   endif()
   include(${VTKm_DIR}/VTKmConfig.cmake)
   find_package(VTKm REQUIRED QUIET)
   set(VTKM_FOUND true CACHE BOOL "VTKM library found" FORCE)
   set("HAVE_VTKM" true CACHE BOOL "Have VTKM library" FORCE)

   message(STATUS "  VTKM_DIR = ${VTKM_DIR}")
   message(STATUS "  VTKM_FOUND = ${VTKM_FOUND}")
   message(STATUS "  VTKm_VERSION_MAJOR = ${VTKm_VERSION_MAJOR}")
   message(STATUS "  VTKm_VERSION_MINOR = ${VTKm_VERSION_MINOR}")
   message(STATUS "  VTKm_VERSION_PATCH = ${VTKm_VERSION_PATCH}")
   message(STATUS "  VTKm_VERSION_FULL = ${VTKm_VERSION_FULL}")
   message(STATUS "  VTKm_VERSION = ${VTKm_VERSION}")

   if(VISIT_INSTALL_THIRD_PARTY AND NOT VISIT_VTKM_SKIP_INSTALL)
       set(vtkm_majmin ${VTKm_VERSION_MAJOR}.${VTKm_VERSION_MINOR})

       # using a couple of main vtkm targets, find all of the link library
       # targets for installation
       list(APPEND vtkm_targets vtkm::filter vtkm::diy)
       get_target_property(VTKM_INT_LL vtkm::filter INTERFACE_LINK_LIBRARIES)
       # pluginVsInstall test for Slice on Windows revealed need for this module
       # and its link dependencies to be installed, too.
       get_target_property(VTKM_DIY_LL vtkm::diy INTERFACE_LINK_LIBRARIES)
       set(addl_ll)
       foreach(vtkmll ${VTKM_INT_LL} ${VTKM_DIY_LL})
           # only process libraries that start with vtkm
	   string(SUBSTRING ${vtkmll} 0 4 ll_dep_prefix)
           if (TARGET ${vtkmll} AND "${ll_dep_prefix}" STREQUAL "vtkm")
               list(APPEND vtkm_targets ${vtkmll})
           endif()

           get_target_property(VTKM_LL_DEP ${vtkmll} INTERFACE_LINK_LIBRARIES)
           if(VTKM_LL_DEP)
               foreach(ll_dep ${VTKM_LL_DEP})
                   # only process libraries that start with vtkm
	           string(SUBSTRING ${ll_dep} 0 4 ll_dep_prefix)
                   if (TARGET ${ll_dep} AND "${ll_dep_prefix}" STREQUAL "vtkm")
                       list(APPEND vtkm_targets ${ll_dep})
                   endif()
               endforeach()
           endif()
       endforeach()
       list(REMOVE_DUPLICATES vtkm_targets)
       unset(addl_ll)

       # install all library targets
       visit_install_thirdparty_targets(NAME vtkm TARGETS ${vtkm_targets})

       if(NOT VISIT_HEADERS_SKIP_INSTALL)
           install(DIRECTORY ${VTKM_DIR}/include/vtkm-${vtkm_majmin}
                DESTINATION ${VISIT_INSTALLED_VERSION_INCLUDE}
                FILE_PERMISSIONS OWNER_WRITE OWNER_READ
                                 GROUP_WRITE GROUP_READ
                                 WORLD_READ
                DIRECTORY_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE
                                      GROUP_WRITE GROUP_READ GROUP_EXECUTE
                                      WORLD_READ WORLD_EXECUTE)
       endif()

        # write SetupVTKM.cmake
        include(${VISIT_SOURCE_DIR}/CMake/WriteThirdPartySetup.cmake)
        set(target_list vtkm::filter)
        create_lib_setup_cmake(NAME "VTKM"
                               NAMESPACE "vtkm::"
                               INCBASE "vtkm-${vtkm_majmin}"
                               ITEMS ${target_list}
                               NESTED_INCLUDES true)
        unset(target_list)

        get_lib_setup_cmake_input_file(fname "VTKM")
        file(APPEND ${fname} "\nadd_library(vtkm::loguru INTERFACE IMPORTED)\n")
        file(APPEND ${fname} "set_target_properties(vtkm::loguru PROPERTIES INTERFACE_LINK_LIBRARIES \"Threads::Threads\")\n")
        file(APPEND ${fname} "\nadd_library(vtkm::diy_developer_flags INTERFACE IMPORTED)\n")
        generate_lib_setup_cmake(NAME "VTKM")
    endif()
endif()

