# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Bonnell, Wed Jan 05 10:25:13 PST 2010
#   Add FOLDER property.
#
#   Kathleen Biagas, Tue Jan 13 10:57:44 PST 2015
#   Now have a vtk dependency. since ImageObject now writes non-ppm formats
#   using vtk.
#
#****************************************************************************

visit_add_executable(
    NAME        visit_composite
    SOURCES     ImageObject.C
                visit_composite.C
    HEADERS     ImageObject.h
    INCLUDES    ${CMAKE_CURRENT_SOURCE_DIR}
                ${VISIT_SOURCE_DIR}/common/utility
    DEPENDS_ON  visit_vtk
                VTK::IOImage
    FOLDER      tools/fx/imagetools)

visit_add_executable(
    NAME        visit_transition
    SOURCES     ImageObject.C
                visit_transition.C
    HEADERS     ImageObject.h
    INCLUDES    ${CMAKE_CURRENT_SOURCE_DIR}
                ${VISIT_SOURCE_DIR}/common/utility
    DEPENDS_ON  visit_vtk
                VTK::IOImage
    FOLDER      tools/fx/imagetools)

