# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

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


# check if conduit was built with python support, if so we want
# to install conduit's python modules
if(EXISTS ${CONDUIT_DIR}/python-modules/conduit)
    message(STATUS "Found Conduit Python Wrappers - ${CONDUIT_DIR}/python-modules/conduit")
    install(DIRECTORY ${CONDUIT_DIR}/python-modules/conduit
            DESTINATION ${VISIT_INSTALLED_VERSION_LIB}/site-packages/
            FILE_PERMISSIONS
                    OWNER_WRITE
                    OWNER_READ
                    GROUP_WRITE
                    GROUP_READ
                    WORLD_READ
            DIRECTORY_PERMISSIONS
                    OWNER_WRITE
                    OWNER_READ
                    OWNER_EXECUTE
                    GROUP_WRITE
                    GROUP_READ
                    GROUP_EXECUTE
                    WORLD_READ WORLD_EXECUTE
                )
endif()

if(CONDUIT_FOUND)
    set(HAVE_CONDUIT TRUE CACHE BOOL "Have Conduit libraries")
endif()

