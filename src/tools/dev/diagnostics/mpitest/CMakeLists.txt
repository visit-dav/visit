# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Wed Oct 22 14:20:27 MST 2014
#   Add call to VISIT_TOOLS_ADD_FOLDER.
#
#****************************************************************************

IF(VISIT_PARALLEL)
    ADD_PARALLEL_EXECUTABLE(mpitest mpitest.c)
    VISIT_INSTALL_TARGETS(mpitest)
    VISIT_TOOLS_ADD_FOLDER(diagnostics/mpitest mpitest)
ENDIF(VISIT_PARALLEL)
