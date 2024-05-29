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

SET(COMMON_SOURCES ImageObject.C)

SET(VISIT_COMPOSITE_SOURCES
visit_composite.C ${COMMON_SOURCES}
)

SET(VISIT_TRANSITION_SOURCES
visit_transition.C ${COMMON_SOURCES}
)

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_SOURCE_DIR}/common/utility
${VTK_INCLUDE_DIRS}
${VISIT_SOURCE_DIR}/visit_vtk/full
)

set(vtkimglib VTK::IOImage)

ADD_EXECUTABLE(visit_composite  ${VISIT_COMPOSITE_SOURCES})
target_link_libraries(visit_composite ${vtkimglib} visit_vtk)

ADD_EXECUTABLE(visit_transition ${VISIT_TRANSITION_SOURCES})
target_link_libraries(visit_transition ${vtkimglib} visit_vtk)

# Installation targets
VISIT_INSTALL_TARGETS(visit_composite visit_transition)
VISIT_FX_ADD_FOLDER(imagetools visit_composite visit_transition)
