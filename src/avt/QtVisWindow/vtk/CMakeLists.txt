# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#  Kathleen Biagas, Thu Oct 24, 2024
#  Utilize visit_patch_target.
#
#****************************************************************************

visit_patch_target(
    NAME      avtqtviswindow
    SOURCES   vtkDashedXorGridMapper2D.C
              vtkRubberBandMapper2D.C
    HEADERS   vtkDashedXorGridMapper2D.h
              vtkRubberBandMapper2D.h
    INCLUDES  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

if(NOT APPLE AND X11_FOUND)
    set_source_files_properties(
        vtkDashedXorGridMapper2D.C  vtkRubberBandMapper2D.C
        PROPERTIES COMPILE_FLAGS "-DHAVE_XLIB")
endif()


