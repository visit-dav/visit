# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#    Justin Privitera, Mon Oct 28 15:26:44 PDT 2024
#    Added avtGlobalAvgExpression.C,     avtGlobalMaxExpression.C, 
#          avtGlobalMinExpression.C,     avtGlobalRMSExpression.C, 
#          avtGlobalStdDevExpression.C,  avtGlobalSumExpression.C, 
#      and avtGlobalVarianceExpression.C
#
#*****************************************************************************

set(MATH_SOURCES
    avtAbsValExpression.C
    avtArccosExpression.C
    avtArcsinExpression.C
    avtArctanExpression.C
    avtArctan2Expression.C
    avtBase10LogExpression.C
    avtBase10LogWithMinExpression.C
    avtBinaryAddExpression.C
    avtBinaryAndExpression.C
    avtBinaryDivideExpression.C
    avtBinaryModuloExpression.C
    avtBinaryMultiplyExpression.C
    avtBinaryPowerExpression.C
    avtBinarySubtractExpression.C
    avtCeilingExpression.C
    avtCosExpression.C
    avtCoshExpression.C
    avtCylindricalCoordinatesExpression.C
    avtCylindricalRadiusExpression.C
    avtDegreeToRadianExpression.C
    avtDeterminantExpression.C
    avtEigenvalueExpression.C
    avtEigenvectorExpression.C
    avtExpExpression.C
    avtFloorExpression.C
    avtIdentityExpression.C
    avtInverseExpression.C
    avtGlobalAvgExpression.C
    avtGlobalMaxExpression.C
    avtGlobalMinExpression.C
    avtGlobalRMSExpression.C
    avtGlobalStdDevExpression.C
    avtGlobalSumExpression.C
    avtGlobalVarianceExpression.C
    avtNaturalLogExpression.C
    avtPolarCoordinatesExpression.C
    avtRadianToDegreeExpression.C
    avtRoundExpression.C
    avtSinExpression.C
    avtSinhExpression.C
    avtSmartDivideExpression.C
    avtSquareExpression.C
    avtSquareRootExpression.C
    avtTanExpression.C
    avtTanhExpression.C
    avtTraceExpression.C
    avtUnaryMinusExpression.C
    avtVectorCrossProductExpression.C)

target_sources(avtexpressions_ser PRIVATE ${MATH_SOURCES})
target_include_directories(avtexpressions_ser PUBLIC .)

if(VISIT_PARALLEL)
    target_sources(avtexpressions_par PRIVATE ${MATH_SOURCES})
    target_include_directories(avtexpressions_par PUBLIC .)
endif()

unset(MATH_SOURCES)

