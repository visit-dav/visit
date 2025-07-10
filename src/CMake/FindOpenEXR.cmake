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
SET(SET_UP_THIRD_PARTY_FULL_PATH ON)


if(OPENEXR_VERSION AND OPENEXR_VERSION GREATER_EQUAL(3.3.4))
    SET_UP_THIRD_PARTY(OPENEXR
        LIBS Iex-3_3 IlmThread-3_3 Imath-3_1 OpenEXR-3_3) 
else()

    if(WIN32)
        SET_UP_THIRD_PARTY(OPENEXR
            INCDIR include/OpenEXR
            LIBS IlmImf-2_2 Imath-2_2 Half IlmThread-2_2 Iex-2_2)
    elseif(APPLE)
        # These libraries are being build shared on Apple. We can find them without
        # version suffixes but then they do not install properly during "make install".
        # If we add version suffixes then "make install/make package" do what we want.
        SET_UP_THIRD_PARTY(OPENEXR
            INCDIR include/OpenEXR
            LIBS IlmImf-2_2.22 Imath-2_2.12 Half.12 IlmThread-2_2.12 Iex-2_2.12)
    else()
        SET_UP_THIRD_PARTY(OPENEXR
            INCDIR include/OpenEXR
            LIBS IlmImf Imath Half IlmThread Iex)
    endif()
endif()
