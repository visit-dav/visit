# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#****************************************************************************

SET(AVTMATH_SOURCES
avtFactorial.C
avtHermitePolynomial.C
avtMatrix.C
avtPlane.C
avtPolynomial.C
avtQuaternion.C
avtTrackball.C
avtVecArray.C
avtVector.C
)

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_COMMON_INCLUDES}
${VTK_INCLUDE_DIRS}
)

if(VTK_VERSION VERSION_EQUAL "8.1.0")
    set(vtklibs vtkCommonCore)
else()
    set(vtklibs VTK::CommonCore)
endif()

# Add link directories
LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY} )

ADD_LIBRARY(avtmath ${AVTMATH_SOURCES})
TARGET_LINK_LIBRARIES(avtmath visitcommon ${vtklibs})

VISIT_INSTALL_TARGETS(avtmath)
