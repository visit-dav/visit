# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Tue Sep 5, 2023
#   Remove untested, unused EGL logic.
#
#   Kathleen Biagas, Thu Oct 3, 2024
#   Utilize visit_add_library.
#
#****************************************************************************


visit_add_library(
    NAME        visit_vtk_offscreen
    SOURCES     vtkOffScreenRenderingFactory.C
    HEADERS     vtkOffScreenRenderingFactory.h
                visit_vtk_offscreen_exports.h
    INCLUDES    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                $<BUILD_INTERFACE:${VISIT_BINARY_DIR}/include>
                $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/visit_vtk/offscreen>
    DEPENDS_ON  VTK::CommonCore
                VTK::RenderingOpenGL2)

if(HAVE_OSMESA)
    target_link_libraries(visit_vtk_offscreen PUBLIC ${OSMESA_LIBRARIES})
endif()

