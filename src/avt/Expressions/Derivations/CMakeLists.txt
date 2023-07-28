# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#*****************************************************************************

set(DERIVATIONS_SOURCES
    avtCurvatureExpression.C
    avtDisplacementExpression.C
    avtEffectiveTensorExpression.C
    avtLocalizedCompactnessExpression.C
    avtPrincipalDeviatoricTensorExpression.C
    avtPrincipalTensorExpression.C
    avtRelativeVolumeExpression.C
    avtStrainAlmansiExpression.C
    avtStrainExpression.C
    avtStrainGreenLagrangeExpression.C
    avtStrainInfinitesimalExpression.C
    avtStrainRateExpression.C
    avtStrainVolumetricExpression.C
    avtTensorMaximumShearExpression.C)

target_sources(avtexpressions_ser PRIVATE ${DERIVATIONS_SOURCES})
target_include_directories(avtexpressions_ser PUBLIC .)

if(VISIT_PARALLEL)
    target_sources(avtexpressions_par PRIVATE ${DERIVATIONS_SOURCES})
    target_include_directories(avtexpressions_par PUBLIC .)
endif()

unset(DERIVATIONS_SOURCES)

