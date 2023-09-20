# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#****************************************************************************

if(VTK_VERSION VERSION_EQUAL "8.1.0")
    set(vtklibs vtkCommonCore)
else()
    set(vtklibs VTK::CommonCore)
endif()

visit_add_library(
    NAME       avtmath
    SOURCES    avtFactorial.C
               avtHermitePolynomial.C
               avtMatrix.C
               avtPlane.C
               avtPolynomial.C
               avtQuaternion.C
               avtTrackball.C
               avtVecArray.C
               avtVector.C
    INCLUDES   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
               $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/include/avt/Math>
    DEPENDS_ON visitcommon ${vtklibs})

if(VTK_VERSION VERSION_EQUAL "8.1.0")
    target_include_directories(avtmath PUBLIC "$<BUILD_INTERFACE:${VTK_INCLUDE_DIRS}>")
endif()

visit_install_export_targets(avtmath)

