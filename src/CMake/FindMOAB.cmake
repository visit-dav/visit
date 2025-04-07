# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#    Iulian R. Grindeanu & Vijay S. Mahadevan via Mark C Miller
#    Wed Aug 10 14:54:05 PDT 2016
#    Added support for ANL's Mesh Object datABase (MOAB)
#
#    Kathleen Biagas, Mon Mar 31, 2025
#    Utilize visit_import_third_party.
#
#****************************************************************************/

# Uses the MOAB_DIR hint from the config-site .cmake file

visit_import_third_party(MOAB LIBS MOAB)

if(VISIT_PARALLEL)
    visit_import_third_party(MOAB_MPI LIBS MOAB_mpi)
endif()

