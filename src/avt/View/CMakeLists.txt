# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Thu Jun  7 14:45:49 PDT 2018
#   Remove use of VTK_DEFINTIONS in compiler. It initializes some vtk
#   modules, which is now being done in avt/Plotter/vtk/InitVTKRendering.C.
#
#****************************************************************************

visit_add_library(
    NAME        avtview
    SOURCES     avtView2D.C
                avtView3D.C
                avtViewAxisArray.C
                avtViewCurve.C
                avtViewInfo.C
    HEADERS     avtView2D.h
                avtView3D.h
                avtViewAxisArray.h
                avtViewCurve.h
                avtViewInfo.h
                view_exports.h
    INCLUDES    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/View>
    DEPENDS_ON  visitcommon VTK::RenderingOpenGL2 VTK::RenderingFreeType VTK::InteractionStyle)

