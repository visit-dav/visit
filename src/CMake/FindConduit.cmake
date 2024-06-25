# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#-----------------------------------------------------------------------------
# Modifications:
#   Kathleen Biagas, Wed Feb 28, 2024
#   Removed install logic for conduit python.  Now handled in
#   lib/CMakeLists.txt with build/lib/site-packages/ directory install.
#
#   Kathleen Biagas, Tue June 25, 2024
#   Create exportable library via blt_import_library.
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
    list(REMOVE_DUPLICATES CONDUIT_LIBRARY_DIR)
    list(REMOVE_DUPLICATES CONDUIT_LIB)
    # create a list of full-path libraries to be used when linking
    foreach(libdir ${CONDUIT_LIBRARY_DIR})
        foreach(lib ${CONDUIT_LIB})
            if(EXISTS ${libdir}/${lib})
                list(APPEND CONDUIT_LIBRARIES
                    $<BUILD_INTERFACE:${libdir}/${lib}>)
            endif()
        endforeach()
    endforeach()

    # create the import library
    blt_import_library(
        NAME       conduit
        INCLUDES   $<BUILD_INTERFACE:${CONDUIT_INCLUDE_DIR}>
                   $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/conduit/conduit>
        LIBRARIES  ${CONDUIT_LIBRARIES}
        EXPORTABLE ON)

    unset(CONDUIT_LIBRARIES)
    # Add install interface for link libraries
    foreach(lib ${CONDUIT_LIB})
        target_link_libraries(conduit INTERFACE
            $<INSTALL_INTERFACE:\${_IMPORT_PREFIX}/${VISIT_INSTALLED_VERSION_LIB}/${lib}>)
    endforeach()

    if(VISIT_PARALLEL)
        list(REMOVE_DUPLICATES CONDUIT_MPI_LIBRARY_DIR)
        list(REMOVE_DUPLICATES CONDUIT_MPI_LIB)
        # create a list of full-path libraries to be used when linking
        foreach(libdir ${CONDUIT_MPI_LIBRARY_DIR})
            foreach(lib ${CONDUIT_MPI_LIB})
                if(EXISTS ${libdir}/${lib})
                    list(APPEND CONDUIT_MPI_LIBRARIES
                        $<BUILD_INTERFACE:${libdir}/${lib}>)
                endif()
            endforeach()
        endforeach()

        # Create import library
        blt_import_library(
            NAME       conduit_mpi
            INCLUDES   $<BUILD_INTERFACE:${CONDUIT_INCLUDE_DIR}>
                       $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/conduit/conduit>
            LIBRARIES  ${CONDUIT_MPI_LIBRARIES}
            EXPORTABLE ON)

        # Add install interface for link libraries
        foreach(lib ${CONDUIT_MPI_LIB})
            if(WIN32 AND ${lib} MATCHES "msmpi")
                continue()
            endif()
            target_link_libraries(conduit_mpi INTERFACE
                $<INSTALL_INTERFACE:\${_IMPORT_PREFIX}/${VISIT_INSTALLED_VERSION_LIB}/${lib}>)
        endforeach()
        unset(CONDUIT_MPI_LIBRARIES)
    endif()

   # install the export targets
   if(VISIT_INSTALL_THIRD_PARTY)
       visit_install_export_targets(conduit)
       if(VISIT_PARALLEL)
           visit_install_export_targets(conduit_mpi)
       endif()
   endif()
endif()

