# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

target_sources(avtqtviswindow PRIVATE
    vtkDashedXorGridMapper2D.C
    vtkRubberBandMapper2D.C)

if(NOT APPLE AND X11_FOUND)
    set_source_files_properties(
        vtkDashedXorGridMapper2D.C  vtkRubberBandMapper2D.C
        PROPERTIES COMPILE_FLAGS "-DHAVE_XLIB")
endif()

target_include_directories(avtqtviswindow PUBLIC .)

