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
#****************************************************************************/

# Use the ADIOS_DIR hint from the config-site .cmake file

INCLUDE(${VISIT_SOURCE_DIR}/CMake/SetUpThirdParty.cmake)

IF(EXISTS ${VISIT_ADIOS2_DIR}/lib64)
    SET(LIB lib64)
ELSE()
    SET(LIB lib)
ENDIF()

if(NOT WIN32)
    SET_UP_THIRD_PARTY(ADIOS2 ${LIB} include adios2 adios2_atl adios2_dill adios2_evpath adios2_ffs adios2_sst taustubs)
else()
    SET_UP_THIRD_PARTY(ADIOS2 ${LIB} include adios2 taustubs)
endif()

IF(VISIT_PARALLEL)
    SET_UP_THIRD_PARTY(ADIOS2_PAR ${LIB} include adios2_mpi)
ENDIF(VISIT_PARALLEL)
