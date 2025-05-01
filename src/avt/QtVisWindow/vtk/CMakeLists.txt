# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Tue April 15, 2025
#   Removed setting of HAVE_XLIB definition. No longer needed.
#
#****************************************************************************

target_sources(avtqtviswindow PRIVATE
    vtkDashedXorGridMapper2D.C
    vtkRubberBandMapper2D.C)

target_include_directories(avtqtviswindow PUBLIC .)

