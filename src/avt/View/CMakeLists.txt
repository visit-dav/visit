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

SET(AVTVIEW_SOURCES
avtView2D.C
avtView3D.C
avtViewCurve.C
avtViewInfo.C
avtViewAxisArray.C
)

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_COMMON_INCLUDES}
${VTK_INCLUDE_DIRS}
)

set(vtklibs VTK::RenderingOpenGL2 VTK::RenderingFreeType VTK::InteractionStyle)

# Add link directories
LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY} )

ADD_LIBRARY(avtview ${AVTVIEW_SOURCES})
TARGET_LINK_LIBRARIES(avtview visitcommon ${vtklibs})

VISIT_INSTALL_TARGETS(avtview)
