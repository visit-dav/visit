# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Wed Oct 22 14:20:27 MST 2014
#   Send 'diagnostics/exceptiontest' type argument to VISIT_TOOLS_ADD_FOLDER.
#
#****************************************************************************


visit_add_executable(
    NAME        exceptiontest
    SOURCES     exceptiontest.cpp
    DEPENDS_ON  visitcommon
    FOLDER      tools/diagnostics)

if(VISIT_PARALLEL)
    visit_add_parallel_executable(
        NAME        exceptiontest_par
        SOURCES     exceptiontest.cpp
        DEPENDS_ON  visitcommon
        FOLDER      tools/diagnostics)
endif()

