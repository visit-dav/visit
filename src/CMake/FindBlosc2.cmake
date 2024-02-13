# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

# Use the BLOSC_DIR hint from the config-site .cmake file

if(WIN32)
    SET_UP_THIRD_PARTY(BLOSC2 LIBS libblosc2)
else()
    SET_UP_THIRD_PARTY(BLOSC2 LIBS blosc2)
endif()

