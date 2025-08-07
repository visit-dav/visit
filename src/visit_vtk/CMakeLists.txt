# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Tue Sep 5, 2023 
#   Remove untested, unused EGL logic.
#
#   Kathleen Biagas, Wed June 18, 2025
#   Remove offscreen.
#
#****************************************************************************

project(visit_vtk)

add_subdirectory(lightweight)
add_subdirectory(full)

