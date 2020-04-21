# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************


include_directories(
${VISIT_BINARY_DIR}/include
${VISIT_SOURCE_DIR}/include
)


add_library(visit_vtk_offscreen vtkOffScreenRenderingFactory.C)

if(HAVE_OSMESA)
    target_link_libraries(visit_vtk_offscreen ${OSMESA_LIBRARIES})
elseif(HAVE_EGL)
    target_link_libraries(visit_vtk_offscreen ${EGL_LIBRARIES})
endif()

VISIT_INSTALL_TARGETS(visit_vtk_offscreen)
target_link_libraries(visit_vtk_offscreen VTK::CommonCore)
