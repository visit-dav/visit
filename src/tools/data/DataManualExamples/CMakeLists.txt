# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Wed Jul 16 10:27:14 MST 2014
#   Change how Simulations directory is handled on Windows.
# 
#****************************************************************************

IF(NOT WIN32)
    ADD_SUBDIRECTORY(CreatingCompatible)
    ADD_SUBDIRECTORY(CreatingCompatibleAdvanced)
    ADD_SUBDIRECTORY(Simulations)
ELSE()
    IF(VISIT_DATA_MANUAL_EXAMPLES)
        ADD_SUBDIRECTORY(Simulations)
    ELSE()
        # The examples haven't been expressly requested, so exclude them
        # from the VisIt solution.  A 'project' command in the Simulations
        # CMakeLists.txt file will create a separate solution file and allow
        # them to be built separately without requiring CMake to be re-run.
        ADD_SUBDIRECTORY(Simulations EXCLUDE_FROM_ALL)
    ENDIF()
ENDIF(NOT WIN32)


