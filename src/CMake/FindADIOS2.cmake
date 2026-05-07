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
#   Kathleen Biagas, Mon Mar 24, 2025
#   Utilize visit_import_third_party in order to get a single adios2 target
#   that includes all the relevant libraries.
#
#****************************************************************************/

# Uses the ADIOS2_DIR hint from the config-site .cmake file

set(adios2_libs
    adios2_atl
    adios2_c
    adios2_core
    adios2_cxx11
    adios2_dill
    adios2_ffs)

if(NOT WIN32)
    list(APPEND adios2_libs
        adios2_enet
        adios2_evpath
        adios2_perfstubs)
endif()

set(adios2_defines)
if(VISIT_PARALLEL)
    list(APPEND adios2_libs
        adios2_c_mpi
        adios2_cxx11_mpi
        adios2_core_mpi)
endif()

visit_import_third_party(ADIOS2
     LIBS    ${adios2_libs}
     DEFINES ${adios2_defines})

