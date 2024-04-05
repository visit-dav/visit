# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Tue Sep 5, 2023 
#   Remove untested, unused EGL logic.
#
#****************************************************************************


include_directories(
${VISIT_BINARY_DIR}/include
${VISIT_SOURCE_DIR}/include
${VTK_INCLUDE_DIRS}
)


add_library(visit_vtk_offscreen vtkOffScreenRenderingFactory.C)

target_link_libraries(visit_vtk_offscreen VTK::CommonCore VTK::RenderingOpenGL2)

if(HAVE_OSMESA)
    target_link_libraries(visit_vtk_offscreen ${OSMESA_LIBRARIES})
endif()

VISIT_INSTALL_TARGETS(visit_vtk_offscreen)
