# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

# Add XML Code Gen Targets

# Metadata Objects
foreach(obj_name MinMaxInfo)
    ADD_CPP_GEN_TARGET(${obj_name}
                       ${CMAKE_CURRENT_SOURCE_DIR}
                       ${CMAKE_CURRENT_SOURCE_DIR})
endforeach()

visit_append_list(
    NAME  avtquery_ser_SOURCES
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtActualDataMinMaxQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtActualDataNumNodesQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtActualDataNumZonesQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtAggregateChordLengthDistributionQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtAggregateRayLengthDistributionQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtAreaBetweenCurvesQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtAverageMeanCurvatureQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtAverageValueQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtBestFitLineQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCentroidQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCompactnessQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtConnComponentsAreaQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtConnComponentsCentroidQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtConnComponentsLengthQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtConnComponentsQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtConnComponentsSummaryQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtConnComponentsVariableQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtConnComponentsVolumeQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtConnComponentsWeightedVariableQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCycleQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDistanceFromBoundaryQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtEllipticalCompactnessFactorQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtEulerianQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtExpectedValueQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtFlattenQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtGridInformationQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtGyRadiusQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtHohlraumFluxQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtIndividualChordLengthDistributionQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtIndividualRayLengthDistributionQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtIntegralCurveInfoQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtIntegrateQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtKurtosisQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtL2NormBetweenCurvesQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtL2NormQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLineSamplerInfoQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLineScanQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLineScanTransformQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLocalizedCompactnessFactorQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLocateAndPickNodeQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLocateAndPickZoneQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMassDistributionQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMemoryUsageQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMinMaxQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMomentOfInertiaQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtNodeCoordsQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtNumNodesQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtNumZonesQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtOriginalDataMinMaxQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtOriginalDataNumNodesQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtOriginalDataNumZonesQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtOriginalDataSpatialExtentsQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSampleStatisticsQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtShapeletDecompositionQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSkewnessQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSphericalCompactnessFactorQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSummationQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTimeQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTotalLengthQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTotalRevolvedSurfaceAreaQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTotalRevolvedVolumeQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTotalSurfaceAreaQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTotalVolumeQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTrajectoryByNode.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTrajectoryByZone.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVariableByNodeQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVariableByZoneQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVariableSummationQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtWatertightQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtWeightedVariableSummationQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtXRayImageQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/avtZoneCenterQuery.C
          ${CMAKE_CURRENT_SOURCE_DIR}/MinMaxInfo.C)

visit_append_list(
    NAME  avtquery_ser_HEADERS
    ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtActualDataMinMaxQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtActualDataNumNodesQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtActualDataNumZonesQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtAggregateChordLengthDistributionQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtAggregateRayLengthDistributionQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtAreaBetweenCurvesQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtAverageMeanCurvatureQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtAverageValueQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtBestFitLineQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCentroidQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCompactnessQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtConnComponentsAreaQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtConnComponentsCentroidQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtConnComponentsLengthQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtConnComponentsQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtConnComponentsSummaryQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtConnComponentsVariableQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtConnComponentsVolumeQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtConnComponentsWeightedVariableQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtCycleQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtDistanceFromBoundaryQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtEllipticalCompactnessFactorQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtEulerianQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtExpectedValueQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtFlattenQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtGridInformationQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtGyRadiusQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtHohlraumFluxQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtIndividualChordLengthDistributionQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtIndividualRayLengthDistributionQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtIntegralCurveInfoQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtIntegrateQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtKurtosisQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtL2NormBetweenCurvesQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtL2NormQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLineSamplerInfoQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLineScanQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLineScanTransformQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLocalizedCompactnessFactorQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLocateAndPickNodeQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtLocateAndPickZoneQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMassDistributionQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMemoryUsageQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMinMaxQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtMomentOfInertiaQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtNodeCoordsQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtNumNodesQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtNumZonesQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtOriginalDataMinMaxQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtOriginalDataNumNodesQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtOriginalDataNumZonesQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtOriginalDataSpatialExtentsQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSampleStatisticsQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtShapeletDecompositionQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSkewnessQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSphericalCompactnessFactorQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtSummationQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTimeQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTotalLengthQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTotalRevolvedSurfaceAreaQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTotalRevolvedVolumeQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTotalSurfaceAreaQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTotalVolumeQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTrajectoryByNode.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtTrajectoryByZone.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVariableByNodeQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVariableByZoneQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtVariableSummationQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtWatertightQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtWeightedVariableSummationQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtXRayImageQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/avtZoneCenterQuery.h
          ${CMAKE_CURRENT_SOURCE_DIR}/MinMaxInfo.h)

if(VISIT_PYTHON_FILTERS)
    visit_append_list(
        NAME  avtquery_ser_SOURCES
        ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtPythonQuery.C)
    visit_append_list(
        NAME  avtquery_ser_HEADERS
        ITEMS ${CMAKE_CURRENT_SOURCE_DIR}/avtPythonQuery.h)
endif()

visit_append_list(
    NAME  avtquery_ser_INCLUDES
    ITEMS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/Queries/Queries>)

