# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#*****************************************************************************

visit_append_list(
    NAME  avtexpressions_ser_SOURCES
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtAbsValExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtArccosExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtArcsinExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtArctanExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtArctan2Expression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtBase10LogExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtBase10LogWithMinExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtBinaryAddExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtBinaryAndExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtBinaryDivideExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtBinaryMultiplyExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtBinaryPowerExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtBinarySubtractExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCeilingExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCosExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCoshExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCylindricalCoordinatesExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCylindricalRadiusExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDegreeToRadianExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDeterminantExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtEigenvalueExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtEigenvectorExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtExpExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtFloorExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtIdentityExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtInverseExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtModuloExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtNaturalLogExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPolarCoordinatesExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtRadianToDegreeExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtRoundExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSinExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSinhExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSmartDivideExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSquareExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSquareRootExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTanExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTanhExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTraceExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtUnaryMinusExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVectorCrossProductExpression.C)

visit_append_list(
    NAME  avtexpressions_ser_HEADERS
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtAbsValExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtArccosExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtArcsinExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtArctanExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtArctan2Expression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtBase10LogExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtBase10LogWithMinExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtBinaryAddExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtBinaryAndExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtBinaryDivideExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtBinaryMultiplyExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtBinaryPowerExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtBinarySubtractExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCeilingExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCosExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCoshExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCylindricalCoordinatesExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCylindricalRadiusExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDegreeToRadianExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDeterminantExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtEigenvalueExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtEigenvectorExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtExpExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtFloorExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtIdentityExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtInverseExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtModuloExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtNaturalLogExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPolarCoordinatesExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtRadianToDegreeExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtRoundExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSinExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSinhExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSmartDivideExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSquareExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSquareRootExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTanExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTanhExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTraceExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtUnaryMinusExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVectorCrossProductExpression.h)

visit_append_list(
    NAME  avtexpressions_ser_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/Expressions/Math>)

