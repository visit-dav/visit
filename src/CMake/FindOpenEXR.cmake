# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

# Use the OPENEXR_DIR hint from the config-site .cmake file

# Force SET_UP_THIRD_PARTY to make the OPENEXR_LIB variable contain full paths
# to the libraries so we don't have to add the OPENEXR_LIBRARY_DIR directory
# all over the place in the build.
SET(SET_UP_THIRD_PARTY_FULL_PATH ON)

IF (WIN32)
  SET_UP_THIRD_PARTY(OPENEXR
     INCDIR include/OpenEXR
     LIBS IlmImf-2_2 Imath-2_2 Half IlmThread-2_2 Iex-2_2)
ELSE (WIN32)
  IF(APPLE)
    # These libraries are being build shared on Apple. We can find them without
    # version suffixes but then they do not install properly during "make install".
    # If we add version suffixes then "make install/make package" do what we want.
    SET_UP_THIRD_PARTY(OPENEXR
      INCDIR include/OpenEXR
      LIBS IlmImf-2_2.22 Imath-2_2.12 Half.12 IlmThread-2_2.12 Iex-2_2.12)
  ELSE(APPLE)
    SET_UP_THIRD_PARTY(OPENEXR
      INCDIR include/OpenEXR
      LIBS IlmImf Imath Half IlmThread Iex)
  ENDIF(APPLE)
ENDIF (WIN32)

