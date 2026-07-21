# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#   Kathleen Biagas, Tues Oct 1 09:33:47 MST 2013
#   Removed logic handling windows differently than other platforms.
#
#   Kathleen Biagas, Thu Sep 27 11:28:34 PDT 2018
#   Added adiosread_nompi so it will get installed when necessary.
#
#   Justin Privitera, Fri Apr 22 13:07:34 PDT 2022
#   Added logic so adios can build without MPI being enabled.
#
#   Kathleen Biagas, Mon Mar 24, 2025
#   Utilize visit_import_third_party.
#
#****************************************************************************/

# Uses the ADIOS_DIR hint from the config-site .cmake file

visit_import_third_party(ADIOS LIBS adiosread_nompi)

if(VISIT_PARALLEL)
    set(ADIOS_MPI_DIR ${VISIT_ADIOS_DIR})
    visit_import_third_party(ADIOS_MPI LIBS adiosread)
endif()

