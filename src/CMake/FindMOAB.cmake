# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#    Iulian R. Grindeanu & Vijay S. Mahadevan via Mark C Miller
#    Wed Aug 10 14:54:05 PDT 2016
#    Added support for ANL's Mesh Object datABase (MOAB)
#****************************************************************************/

# Use the H5PART_DIR hint from the config-site .cmake file 

INCLUDE(${VISIT_SOURCE_DIR}/CMake/SetUpThirdParty.cmake)

SET_UP_THIRD_PARTY(MOAB lib include MOAB)
IF(VISIT_PARALLEL)
    SET_UP_THIRD_PARTY(MOAB_MPI lib include MOAB_mpi)
ENDIF(VISIT_PARALLEL)
