# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#-----------------------------------------------------------------------------
# Modifications:
#   Kathleen Biagas, Wed Feb 28, 2024
#   Removed install logic for conduit python.  Now handled in
#   lib/CMakeLists.txt with build/lib/site-packages/ directory install.
#
#   Justin Privitera, Thu Aug 15 12:02:44 PDT 2024
#   Added conduit_relay_mpi_io to parallel deps.
#
#   Kathleen Biagas, Tue June 25, 2024
#   Create exportable library via blt_import_library.
#
#   Kathleen Biagas, Mon Mar 31 2025
#   Utilize visit_import_third_party.
#
#-----------------------------------------------------------------------------

#
# Uses the CONDUIT_DIR hint from the config-site .cmake file
#

visit_import_third_party(CONDUIT
    INCDIR include/conduit
    LIBS conduit conduit_relay conduit_blueprint)

# par engine needs conduit relay's mpi lib, find it using setup_third_party
# so it will be installed properly
if(VISIT_PARALLEL)
    set(CONDUIT_MPI_DIR ${CONDUIT_DIR})
    visit_import_third_party(CONDUIT_MPI
        INCDIR include/conduit
        LIBS conduit_relay_mpi conduit_relay_mpi_io conduit_blueprint_mpi)
endif()

