# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#*****************************************************************************

visit_append_list(
    NAME  avtexpressions_ser_SOURCES
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtApplyDataBinningExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtApplyEnumerationExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtApplyMapExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtArrayComponentwiseDivisionExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtArrayComponentwiseProductExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtArrayComposeExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtArrayComposeWithBinsExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtArrayDecompose2DExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtArrayDecomposeExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtArraySumExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtBinExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtColorComposeExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtConnComponentsExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtConstantFunctionExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCoordinateExtremaExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCrackWidthExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCurlExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCurveDomainExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCurveExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCurveIntegrateExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCurveSwapXYExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDataIdExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDegreeExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDistanceToBestFitLineExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDivergenceExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDominantMaterialExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtEdgeNormalExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtExpressionComponentMacro.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtFindExternalExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtGeodesicVectorQuantizeExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtGhostZoneIdExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtGradientExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtHSVColorComposeExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtIsNaNExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtKeyAggregatorExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLambda2Expression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLaplacianExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLocalThresholdExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMagnitudeExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMatErrorExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMatvfExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMergeTreeExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMeshCoordinateExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMinMaxExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMIRvfExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtNeighborEvaluatorExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtNMatsExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtNormalizeExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPerformColorTableLookupExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPerMaterialValueExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtProcessorIdExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtQCriterionExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtRandomExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtRecenterExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtRectilinearLaplacianExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtRelativeDifferenceExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtResampleExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtResradExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSpecMFExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSurfaceNormalExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTensorContractionExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtThreadIdExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTransposeExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVariableSkewExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVectorComposeExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVectorDecomposeExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtViscousStressExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtZoneCentersExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtZoneTypeLabelExpression.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtZoneTypeRankExpression.C)

visit_append_list(
    NAME  avtexpressions_ser_HEADERS
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtApplyDataBinningExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtApplyEnumerationExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtApplyMapExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtArrayComponentwiseDivisionExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtArrayComponentwiseProductExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtArrayComposeExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtArrayComposeWithBinsExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtArrayDecompose2DExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtArrayDecomposeExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtArraySumExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtBinExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtColorComposeExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtConnComponentsExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtConstantFunctionExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCoordinateExtremaExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCrackWidthExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCurlExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCurveDomainExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCurveExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCurveIntegrateExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCurveSwapXYExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDataIdExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDegreeExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDistanceToBestFitLineExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDivergenceExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDominantMaterialExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtEdgeNormalExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtExpressionComponentMacro.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtFindExternalExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtGeodesicVectorQuantizeExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtGhostZoneIdExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtGradientExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtHSVColorComposeExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtIsNaNExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtKeyAggregatorExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLambda2Expression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLaplacianExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLocalThresholdExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMagnitudeExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMatErrorExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMatvfExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMergeTreeExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMeshCoordinateExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMinMaxExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMIRvfExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtNeighborEvaluatorExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtNMatsExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtNormalizeExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPerformColorTableLookupExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtPerMaterialValueExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtProcessorIdExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtQCriterionExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtRandomExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtRecenterExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtRectilinearLaplacianExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtRelativeDifferenceExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtResampleExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtResradExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSpecMFExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSurfaceNormalExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTensorContractionExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtThreadIdExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTransposeExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVariableSkewExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVectorComposeExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVectorDecomposeExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtViscousStressExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtZoneCentersExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtZoneTypeLabelExpression.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtZoneTypeRankExpression.h)

if(VISIT_PYTHON_FILTERS)
    visit_append_list(
        NAME  avtexpressions_ser_SOURCES
        ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtPythonExpression.C)
    visit_append_list(
        NAME  avtexpressions_ser_HEADERS
        ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtPythonExpression.h)
endif()

visit_append_list(
    NAME  avtexpressions_ser_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/Expressions/General>)


