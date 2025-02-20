# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Bonnell, Wed Jan 05 10:25:13 PST 2010
#   Add FOLDER property.
#
#   Kathleen Biagas, Thu Jun  7 14:45:49 PDT 2018
#   Remove use of VTK_DEFINTIONS in compiler. It initializes some vtk
#   modules, which is now being done in avt/Plotter/vtk/InitVTKRendering.C.
#
#   Kathleen Biagas, Tue Sep 5, 2023 
#   Remove untested, unused EGL logic.
#
#****************************************************************************

visit_add_executable(
    NAME        text2polys
    SOURCES     text2polys.C
                arial_font.C
    HEADERS     arial_font.h
    INCLUDES    ${CMAKE_CURRENT_SOURCE_DIR}
                ${VISIT_SOURCE_DIR}/common/utility
    DEPENDS_ON  VTK::IOLegacy
    FOLDER      tools/fx/annotations)

visit_add_executable(
    NAME        time_annotation
    SOURCES     time_annotation.C
                arial_font.C
    HEADERS     arial_font.h
    INCLUDES    ${CMAKE_CURRENT_SOURCE_DIR}
                ${VISIT_SOURCE_DIR}/common/utility
    DEPENDS_ON  VTK::InteractionStyle
                VTK::IOImage
                VTK::RenderingFreeType
                VTK::RenderingOpenGL2
                VTK::vtksys
    FOLDER      tools/fx/annotations)


if(HAVE_OSMESA)
    visit_patch_target(
        NAME       time_annotation
        DEPENDS_ON ${OSMESA_LIBRARIES})
endif()

