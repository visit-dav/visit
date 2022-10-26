# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
#  Global Functions
#*****************************************************************************

#-----------------------------------------------------------------------------
# Helper function for installing VisIt targets with export
#-----------------------------------------------------------------------------

function(visit_install_targets_export)

  # currently not using DESTINATION vars in the call to 'install', so it will use CMake defaults:
  # RUNTIME -> bin  (this is change for Windows)
  # LIBRARY -> lib
  # ARCHIVE -> lib
  # PRIVATE_HEADER -> inlude
  # PUBLIC_HEADER -> include

  # if it is decided to change and not use CMake defaults, remember that for the EXPORT part of
  # install to work in RELOCATABLE sense, the DESTINATION vars must be RELATIVE:
  # VISIT_INSTALLED_VERSION_XXX cannot be used!

    if(VISIT_STATIC)
        # Skip installation of static libraries when we build statically
        foreach(T ${ARGN})
            get_target_property(pType ${T} TYPE)
            if(NOT ${pType} STREQUAL "STATIC_LIBRARY")
                install(TARGETS ${T} EXPORT visitTargets
                    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                                GROUP_READ GROUP_WRITE GROUP_EXECUTE
                                WORLD_READ             WORLD_EXECUTE
                )
            endif()
        endforeach()
    else() # not static
        install(TARGETS ${ARGN} EXPORT visitTargets
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                        GROUP_READ GROUP_WRITE GROUP_EXECUTE
                        WORLD_READ             WORLD_EXECUTE
        )
    endif()
endfunction()

