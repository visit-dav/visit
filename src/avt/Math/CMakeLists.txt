# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#****************************************************************************

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
    HEADERS    avtBezierSegment.h
               avtFactorial.h
               avtHermitePolynomial.h
               avtMatrix.h
               avtPlane.h
               avtPolynomial.h
               avtQuaternion.h
               avtTrackball.h
               avtVec.h
               avtVecArray.h
               avtVector.h
               math_exports.h
    INCLUDES   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
               $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/Math>
    DEPENDS_ON visitcommon VTK::CommonCore)

