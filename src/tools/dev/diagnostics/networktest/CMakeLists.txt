# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Wed Oct 22 14:20:27 MST 2014
#   Send 'diagnostics/networktest' type argument to VISIT_TOOLS_ADD_FOLDER.
#
#****************************************************************************

visit_add_executable(
    NAME        networktestclient
    SOURCES     networktestclient.cpp
    DEPENDS_ON  visitcommon
    FOLDER      tools/diagnostics)

visit_add_executable(
    NAME        networktestserver
    SOURCES     networktestserver.cpp
    DEPENDS_ON  visitcommon
    FOLDER      tools/diagnostics)

if(VISIT_PARALLEL)
    visit_add_parallel_executable(
        NAME        networktestserver_par
        SOURCES     networktestserver.cpp
        DEPENDS_ON  visitcommon
        FOLDER      tools/diagnostics)
endif()

