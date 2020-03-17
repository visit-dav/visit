# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Wed Jul 16 10:27:14 MST 2014
#   Change how Simulations directory is handled on Windows.
# 
#   Kathleen Biagas, Thu Feb 27 2020
#   Only add Simulations if VISIT_ENABLE_LIBSIM is true. 
# 
#****************************************************************************

if(NOT WIN32)
    add_subdirectory(CreatingCompatible)
    add_subdirectory(CreatingCompatibleAdvanced)
endif()

if(VISIT_ENABLE_LIBSIM)
    add_subdirectory(Simulations)
endif()


