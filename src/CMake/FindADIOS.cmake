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
#****************************************************************************/

# Use the ADIOS_DIR hint from the config-site .cmake file

if(VISIT_PARALLEL)
    SET_UP_THIRD_PARTY(ADIOS LIBS adiosread adiosread_nompi)
else()
    SET_UP_THIRD_PARTY(ADIOS LIBS adiosread_nompi)
endif()

