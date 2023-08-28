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

set(QUERIES_SOURCES
    avtActualDataMinMaxQuery.C
    avtActualDataNumNodesQuery.C
    avtActualDataNumZonesQuery.C
    avtAggregateChordLengthDistributionQuery.C
    avtAggregateRayLengthDistributionQuery.C
    avtAreaBetweenCurvesQuery.C
    avtAverageMeanCurvatureQuery.C
    avtAverageValueQuery.C
    avtBestFitLineQuery.C
    avtCentroidQuery.C
    avtCompactnessQuery.C
    avtConnComponentsAreaQuery.C
    avtConnComponentsCentroidQuery.C
    avtConnComponentsLengthQuery.C
    avtConnComponentsQuery.C
    avtConnComponentsSummaryQuery.C
    avtConnComponentsVariableQuery.C
    avtConnComponentsVolumeQuery.C
    avtConnComponentsWeightedVariableQuery.C
    avtCycleQuery.C
    avtDistanceFromBoundaryQuery.C
    avtEllipticalCompactnessFactorQuery.C
    avtEulerianQuery.C
    avtExpectedValueQuery.C
    avtFlattenQuery.C
    avtGridInformationQuery.C
    avtGyRadiusQuery.C
    avtHohlraumFluxQuery.C
    avtIndividualChordLengthDistributionQuery.C
    avtIndividualRayLengthDistributionQuery.C
    avtIntegralCurveInfoQuery.C
    avtIntegrateQuery.C
    avtKurtosisQuery.C
    avtL2NormBetweenCurvesQuery.C
    avtL2NormQuery.C
    avtLineScanQuery.C
    avtLineScanTransformQuery.C
    avtLineSamplerInfoQuery.C
    avtLocalizedCompactnessFactorQuery.C
    avtLocateAndPickNodeQuery.C
    avtLocateAndPickZoneQuery.C
    avtMassDistributionQuery.C
    avtMemoryUsageQuery.C
    avtMinMaxQuery.C
    avtMomentOfInertiaQuery.C
    avtNodeCoordsQuery.C
    avtNumNodesQuery.C
    avtNumZonesQuery.C
    avtOriginalDataMinMaxQuery.C
    avtOriginalDataNumNodesQuery.C
    avtOriginalDataNumZonesQuery.C
    avtOriginalDataSpatialExtentsQuery.C
    avtSampleStatisticsQuery.C
    avtShapeletDecompositionQuery.C
    avtSkewnessQuery.C
    avtSphericalCompactnessFactorQuery.C
    avtSummationQuery.C
    avtTimeQuery.C
    avtTotalLengthQuery.C
    avtTotalRevolvedSurfaceAreaQuery.C
    avtTotalRevolvedVolumeQuery.C
    avtTotalSurfaceAreaQuery.C
    avtTotalVolumeQuery.C
    avtTrajectoryByNode.C
    avtTrajectoryByZone.C
    avtVariableByNodeQuery.C
    avtVariableByZoneQuery.C
    avtVariableSummationQuery.C
    avtWatertightQuery.C
    avtWeightedVariableSummationQuery.C
    avtXRayImageQuery.C
    avtZoneCenterQuery.C
    MinMaxInfo.C)

if(VISIT_PYTHON_FILTERS)
    list(APPEND QUERIES_SOURCES avtPythonQuery.C)
endif()

target_sources(avtquery_ser PRIVATE ${QUERIES_SOURCES})
target_include_directories(avtquery_ser PUBLIC .)

if(VISIT_PARALLEL)
    target_sources(avtquery_par PRIVATE ${QUERIES_SOURCES})
    target_include_directories(avtquery_par PUBLIC .)
endif()

unset(QUERIES_SOURCES)

