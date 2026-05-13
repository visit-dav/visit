# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Thu Nov  8 10:08:38 PST 2018
#   Added string replacments related to VISIT_MESAGL_DIR.
#
#   Kathleen Biagas, Fri Jul 19 12:12:16 PDT 2019
#   Filter out path from OSPRAY libraries.
#
#   Kathleen Biagas, Mon Jan  4 18:15:45 PST 2021
#   Added logic for retrieving and filtering VTKm includes that come from
#   interface libraries.
#
#   Cyrus Harrison, Thu May 19 11:42:31 PDT 2022
#   Add PYTHON_FOUND guard for python path related logic to avoid
#   error with empty path passed to get_filename_component
#
#   Kathleen Biagas, Wed Jun  8 2022
#   Install VisItIncludeVars.cmake.
#
#   Kathleen Biagas, Tue Jan 31, 2023
#   Change python_include_relative_path (add 'include' at end) for Windows.
#
#   Kathleen Biagas, Fri Mar 10, 2023
#   Replaced VTKh logic with VTKm.
#
#   Kathleen Biagas, Fri Jun 13 2025
#   Remove VisItIncludeVars.cmake.
#
#   Eric Brugger, Mon Jun 16 13:38:54 PDT 2025
#   Replace vtkm_filter with vtkm::filter.
#
#   Kathleen Biagas, Thu July 10, 2025
#   Support OPENEXR libs in 'lib64'.
#
#   Kathleen Biagas, Monday Nov 3, 2025 
#   Remove logic that was associated with PluginVsInstall.cmake.in. 
#
#******************************************************************************


# needed by PluginVsInstall

install(
    FILES       ${VISIT_SOURCE_DIR}/CMake/CheckMinimumCompilerVersion.cmake
                ${VISIT_SOURCE_DIR}/CMake/FindDependencies.cmake
                ${VISIT_SOURCE_DIR}/CMake/PluginMacros.cmake
                ${VISIT_SOURCE_DIR}/CMake/SetupBLT.cmake
                ${VISIT_SOURCE_DIR}/CMake/VisItMacros.cmake
    DESTINATION ${VISIT_INSTALLED_VERSION}/cmake
    PERMISSIONS OWNER_READ OWNER_WRITE
                GROUP_READ GROUP_WRITE
                WORLD_READ)

