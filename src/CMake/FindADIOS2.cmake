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
#****************************************************************************/

# Use the ADIOS_DIR hint from the config-site .cmake file

if(NOT WIN32)
    SET_UP_THIRD_PARTY(ADIOS2 LIBS
        adios2_c adios2_atl adios2_dill adios2_evpath adios2_ffs
        adios2_taustubs adios2_cmenet adios2_cmzplenet adios2_cmepoll
        adios2_core adios2_cmmulticast adios2_enet adios2_cmselect
        adios2_cxx11 adios2_cmsockets adios2_cmudp)
else()
    SET_UP_THIRD_PARTY(ADIOS2 LIBS
        adios2_c adios2_cxx11 adios2_core adios2_taustubs)
endif()

if(VISIT_PARALLEL)
    if(NOT WIN32)
        SET_UP_THIRD_PARTY(ADIOS2_PAR LIBS
            adios2_c_mpi adios2_cxx11_mpi adios2_core_mpi)
    else()
        SET_UP_THIRD_PARTY(ADIOS2_PAR LIBS
            adios2_c adios2_cxx11 adios2_core
            adios2_c_mpi adios2_cxx11_mpi adios2_core_mpi)
    endif()
endif()

