# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Eric Brugger, Thu Jun 27 09:36:06 PDT 2019
#   Modified to handle the fact that ADIOS2 usually installs the libraries
#   in the directory lib64 instead of lib.
#
#   Eric Brugger, Fri Jun 28 14:39:59 PDT 2019
#   Added additional libraries to the installation.
#
#   Eric Brugger, Mon Jul  1 13:20:00 PDT 2019
#   Renamed the parallel library to adios2_mpi.
#
#   Kathleen Biagas, Tues Oct 1 09:33:47 MST 2013
#   On Windows, only look for adios2 and taustubs.
#
#   Justin Privitera, Wed Apr 27 17:46:52 PDT 2022
#   Updated adios2 to 2.7.1 and added all the libraries it creates to the
#   installation as well as additional logic for parallel building.
#
#   Justin Privitera, Thu Jan 18 09:56:51 PST 2024
#   adios2 libs have changed so I removed many of the old ones.
#
#   Mark C. Miller, Tue Mar 19 12:20:18 PDT 2024
#   Adjust parallel to include both serial and parallel libs.
#
#   Vicente Adolfo Bolea Sanchez, Tue Nov 18 14:00:00 PDT 2025
#   find_package(ADIOS2) to determine ADIOS2 version.
#
#   Kathleen Biagas, Mon May 18, 2026
#   Utilize visit_import_third_party in order to get a single adios2 target
#   that includes all the relevant libraries.
#
#****************************************************************************/

# Uses the ADIOS2_DIR hint from the config-site .cmake file

if(VISIT_ADIOS2_DIR)
   # Get the version
    if(EXISTS ${VISIT_ADIOS2_DIR}/lib/cmake/adios2/adios2-config-version.cmake)
        include(${VISIT_ADIOS2_DIR}/lib/cmake/adios2/adios2-config-version.cmake)
    elseif(EXISTS ${VISIT_ADIOS2_DIR}/lib64/cmake/adios2/adios2-config-version.cmake)
        include(${VISIT_ADIOS2_DIR}/lib64/cmake/adios2/adios2-config-version.cmake)
    else()
        message(FATAL_ERROR "Failed to find ADIOS2 at ADIOS2_DIR=${VISIT_ADIOS2_DIR}/lib*/cmake/adios2")
    endif()

    if(ADIOS2_VERSION VERSION_LESS_EQUAL 2.10)
        set(adios2_cxx_lib adios2_cxx11)
        set(adios2_cxx_mpi_lib adios2_cxx11_mpi)
    else()
        set(adios2_cxx_lib adios2_cxx)
        set(adios2_cxx_mpi_lib adios2_cxx_mpi)
    endif()

    set(adios2_libs
        adios2_atl
        adios2_c
        adios2_core
        adios2_dill
        adios2_ffs
        ${adios2_cxx_lib})

    if(NOT WIN32)
        list(APPEND adios2_libs
             adios2_enet
             adios2_evpath
             adios2_perfstubs)
    endif()

    if(VISIT_PARALLEL)
        list(APPEND adios2_libs
             adios2_c_mpi
             adios2_core_mpi
             ${adios2_cxx_mpi_lib})
    endif()

    visit_import_third_party(
         ADIOS2
         LIBS    ${adios2_libs})
else()
    message(STATUS "ADIOS2 not requested")
endif()

