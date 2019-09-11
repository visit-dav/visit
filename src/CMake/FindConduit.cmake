# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#
# Use the CONDUIT_DIR hint from the config-site .cmake file 
#

INCLUDE(${VISIT_SOURCE_DIR}/CMake/SetUpThirdParty.cmake)

SET_UP_THIRD_PARTY(CONDUIT lib include/conduit conduit conduit_relay conduit_blueprint)

# par engine needs conduit relay's mpi lib, find it using setup_third_party
# so it will be installed properly
if(VISIT_PARALLEL)
    set(CONDUIT_MPI_DIR ${CONDUIT_DIR})
    SET_UP_THIRD_PARTY(CONDUIT_MPI lib include/conduit conduit_relay_mpi)
endif()


