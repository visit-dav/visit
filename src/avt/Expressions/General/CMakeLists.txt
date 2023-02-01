# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#*****************************************************************************

set(GENERAL_SOURCES
    avtApplyDataBinningExpression.C
    avtApplyEnumerationExpression.C
    avtApplyMapExpression.C
    avtArrayComponentwiseDivisionExpression.C
    avtArrayComponentwiseProductExpression.C
    avtArrayComposeExpression.C
    avtArrayComposeWithBinsExpression.C
    avtArrayDecompose2DExpression.C
    avtArrayDecomposeExpression.C
    avtArraySumExpression.C
    avtBinExpression.C
    avtColorComposeExpression.C
    avtConnComponentsExpression.C
    avtConstantFunctionExpression.C
    avtCoordinateExtremaExpression.C
    avtCrackWidthExpression.C
    avtCurlExpression.C
    avtCurveDomainExpression.C
    avtCurveExpression.C
    avtCurveIntegrateExpression.C
    avtCurveSwapXYExpression.C
    avtDataIdExpression.C
    avtDegreeExpression.C
    avtDistanceToBestFitLineExpression.C
    avtDivergenceExpression.C
    avtDominantMaterialExpression.C
    avtEdgeNormalExpression.C
    avtExpressionComponentMacro.C
    avtFindExternalExpression.C
    avtGeodesicVectorQuantizeExpression.C
    avtGhostZoneIdExpression.C
    avtGradientExpression.C
    avtHSVColorComposeExpression.C
    avtIsNaNExpression.C
    avtKeyAggregatorExpression.C
    avtLambda2Expression.C
    avtLaplacianExpression.C
    avtLocalThresholdExpression.C
    avtMagnitudeExpression.C
    avtMatErrorExpression.C
    avtMatvfExpression.C
    avtMergeTreeExpression.C
    avtMeshCoordinateExpression.C
    avtMinMaxExpression.C
    avtMIRvfExpression.C
    avtNeighborEvaluatorExpression.C
    avtNMatsExpression.C
    avtNormalizeExpression.C
    avtPerformColorTableLookupExpression.C
    avtPerMaterialValueExpression.C
    avtProcessorIdExpression.C
    avtQCriterionExpression.C
    avtRandomExpression.C
    avtRecenterExpression.C
    avtRectilinearLaplacianExpression.C
    avtRelativeDifferenceExpression.C
    avtResampleExpression.C
    avtResradExpression.C
    avtSpecMFExpression.C
    avtSurfaceNormalExpression.C
    avtTensorContractionExpression.C
    avtThreadIdExpression.C
    avtTransposeExpression.C
    avtVariableSkewExpression.C
    avtVectorComposeExpression.C
    avtVectorDecomposeExpression.C
    avtViscousStressExpression.C
    avtZoneCentersExpression.C
    avtZoneTypeLabelExpression.C
    avtZoneTypeRankExpression.C)

if(VISIT_PYTHON_FILTERS)
    list(APPEND GENERAL_SOURCES avtPythonExpression.C)
endif()

target_sources(avtexpressions_ser PRIVATE ${GENERAL_SOURCES})
target_include_directories(avtexpressions_ser PUBLIC .)

if(VISIT_PARALLEL)
    target_sources(avtexpressions_par PRIVATE ${GENERAL_SOURCES})
    target_include_directories(avtexpressions_par PUBLIC .)
endif()

unset(GENERAL_SOURCES)

