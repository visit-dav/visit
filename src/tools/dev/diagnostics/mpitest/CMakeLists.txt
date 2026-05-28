# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Wed Oct 22 14:20:27 MST 2014
#   Add call to VISIT_TOOLS_ADD_FOLDER.
#
#   Kathleen Biagas, Mon June 16, 2025
#   Utilize visit_add_parallel_executable. 
#
#****************************************************************************

if(VISIT_PARALLEL)
    visit_add_parallel_executable(
        NAME        mpitest
        SOURCES     mpitest.c
        FOLDER      tools/diagnostics)
endif()

