# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Tue Sep 5, 2023 
#   Remove untested, unused EGL logic.
#
#****************************************************************************

project(visit_vtk)

add_subdirectory(lightweight)
add_subdirectory(full)

# if osmesa is enabled, add our offscreen rendering factory
if(HAVE_OSMESA)
    add_subdirectory(offscreen)
endif()

