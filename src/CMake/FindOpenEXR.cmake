# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

# Modifications:
#   Kathleen Biagas, Thu July 10, 2025
#   Add support for version 3.3.4.

# Use the OPENEXR_DIR hint from the config-site .cmake file

# Force SET_UP_THIRD_PARTY to make the OPENEXR_LIB variable contain full paths
# to the libraries so we don't have to add the OPENEXR_LIBRARY_DIR directory
# all over the place in the build.


# cannot use older OpenEXR with VTK-9.5 which is now the default and minimum vtk version.
if(OPENEXR_VERSION AND OPENEXR_VERSION GREATER_EQUAL(3.3.4))

    set(SET_UP_THIRD_PARTY_FULL_PATH ON)

    SET_UP_THIRD_PARTY(OPENEXR
        LIBS Iex-3_3 IlmThread-3_3 Imath-3_1 OpenEXR-3_3 OpenEXRCore-3_3) 

    set(SET_UP_THIRD_PARTY_FULL_PATH OFF)
endif()

