# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#   Kathleen Biagas, Mon Mar 24, 2025
#   Utilize visit_import_third_party.
#
#*****************************************************************************

# Uses the OPENEXR_DIR hint from the config-site .cmake file

if(WIN32)
  visit_import_third_party(OPENEXR
     INCDIR include/OpenEXR
     LIBS IlmImf-2_2 Imath-2_2 Half IlmThread-2_2 Iex-2_2)
elseif(APPLE)
    # These libraries are being build shared on Apple. We can find them without
    # version suffixes but then they do not install properly during "make install".
    # If we add version suffixes then "make install/make package" do what we want.
    visit_import_third_party(OPENEXR
      INCDIR include/OpenEXR
      LIBS IlmImf-2_2.22 Imath-2_2.12 Half.12 IlmThread-2_2.12 Iex-2_2.12)
else()
    visit_import_third_party(OPENEXR
      INCDIR include/OpenEXR
      LIBS IlmImf Imath Half IlmThread Iex)
endif()

