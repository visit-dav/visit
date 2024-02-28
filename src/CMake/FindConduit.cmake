# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#-----------------------------------------------------------------------------
# Modifications:
#   Kathleen Biagas, Wed Feb 28, 2024
#   Removed install logic for conduit python.  Now handled in
#   lib/CMakeLists.txt with build/lib/site-packages/ directory install.
#
#-----------------------------------------------------------------------------

#
# Use the CONDUIT_DIR hint from the config-site .cmake file 
#

SET_UP_THIRD_PARTY(CONDUIT
    INCDIR include/conduit
    LIBS conduit conduit_relay conduit_blueprint)

# par engine needs conduit relay's mpi lib, find it using setup_third_party
# so it will be installed properly
if(VISIT_PARALLEL)
    set(CONDUIT_MPI_DIR ${CONDUIT_DIR})
    SET_UP_THIRD_PARTY(CONDUIT_MPI
        INCDIR include/conduit
        LIBS conduit_relay_mpi conduit_blueprint_mpi)
endif()

if(CONDUIT_FOUND)
    set(HAVE_CONDUIT TRUE CACHE BOOL "Have Conduit libraries")
endif()

