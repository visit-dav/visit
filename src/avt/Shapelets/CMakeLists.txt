# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#****************************************************************************

SET(AVTSHAPELETS_SOURCES
avtShapeletBasis1D.C
avtShapeletBasis2D.C
avtShapeletBasisSet.C
avtShapeletDecompResult.C
avtShapeletDecompose.C
avtShapeletReconstruct.C
)

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_COMMON_INCLUDES}
${VISIT_SOURCE_DIR}/avt/Math
${VTK_INCLUDE_DIRS}
)

set(vtklibs VTK::CommonCore VTK::FiltersCore)

# Add link directories
LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY} )

ADD_LIBRARY(avtshapelets ${AVTSHAPELETS_SOURCES})
TARGET_LINK_LIBRARIES(avtshapelets visitcommon avtmath ${vtklibs})

VISIT_INSTALL_TARGETS(avtshapelets)
