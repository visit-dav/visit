# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#    Cyrus Harrison,Tue Feb  2 16:35:18 PST 2010
#    Added the python filter query.
#
#    Cyrus Harrison, Wed Mar 10 14:28:33 PST 2010   
#    Use proper python include path ${PYTHON_INCLUDE_PATH}, instead of 
#    ${PYTHON_INCLUDE_DIRS}
#
#    Eric Brugger, Wed Jun 30 13:37:54 PDT 2010
#    Added avtXRayQuery.
#
#    Hank Childs, Thu May 12 15:37:21 PDT 2011
#    Add avtAverageValueQuery.
#
#    Kevin Griffin, Thu Aug 11 10:53:13 PDT 2016
#    Add avtGyRadiusQuery.
#
#    Alister Maguire, Tue Sep 24 13:46:56 MST 2019
#    Added avtDirectDatabaseQOTFilter and avtTimeLoopQOTFilter.
#
#    Cyrus Harrison, Tue Dec 10 08:51:21 PST 2019
#    Add xml tools code gen targets
#****************************************************************************/


# Add XML Code Gen Targets

set(QUERIES_GEN_OBJECTS MinMaxInfo)

# Metadata Objects
foreach(obj_name ${QUERIES_GEN_OBJECTS})
    ADD_CPP_GEN_TARGET(${obj_name}
                       ${CMAKE_CURRENT_SOURCE_DIR}/Queries/
                       ${CMAKE_CURRENT_SOURCE_DIR}/Queries/)
endforeach()

SET(ABSTRACT_SOURCES
Abstract/avtCurveQuery.C
Abstract/avtCurveComparisonQuery.C
Abstract/avtDataObjectQuery.C
Abstract/avtDatasetQuery.C
Abstract/avtGeneralQuery.C
Abstract/avtMultipleInputQuery.C
Abstract/avtTwoPassDatasetQuery.C
)

SET(MISC_SOURCES
Misc/avtQueryOverTimeFilter.C
Misc/avtQueryFactory.C
Misc/avtTimeLoopQOTFilter.C
Misc/avtDirectDatabaseQOTFilter.C
)

SET(PICK_SOURCES
Pick/avtActualCoordsQuery.C
Pick/avtActualNodeCoordsQuery.C
Pick/avtActualZoneCoordsQuery.C
Pick/avtCurvePickQuery.C
Pick/avtLocateCellQuery.C
Pick/avtLocateNodeQuery.C
Pick/avtLocateQuery.C
Pick/avtNodePickQuery.C
Pick/avtPickByNodeQuery.C
Pick/avtPickByZoneQuery.C
Pick/avtPickQuery.C
Pick/avtZonePickQuery.C
)

SET(QUERIES_SOURCES
Queries/avtActualDataMinMaxQuery.C
Queries/avtActualDataNumNodesQuery.C
Queries/avtActualDataNumZonesQuery.C
Queries/avtAggregateChordLengthDistributionQuery.C
Queries/avtAggregateRayLengthDistributionQuery.C
Queries/avtAreaBetweenCurvesQuery.C
Queries/avtAverageMeanCurvatureQuery.C
Queries/avtAverageValueQuery.C
Queries/avtBestFitLineQuery.C
Queries/avtCentroidQuery.C
Queries/avtCompactnessQuery.C
Queries/avtConnComponentsQuery.C
Queries/avtConnComponentsAreaQuery.C
Queries/avtConnComponentsCentroidQuery.C
Queries/avtConnComponentsLengthQuery.C
Queries/avtConnComponentsSummaryQuery.C
Queries/avtConnComponentsVolumeQuery.C
Queries/avtConnComponentsVariableQuery.C
Queries/avtConnComponentsWeightedVariableQuery.C
Queries/avtCycleQuery.C
Queries/avtDistanceFromBoundaryQuery.C
Queries/avtEllipticalCompactnessFactorQuery.C
Queries/avtEulerianQuery.C
Queries/avtExpectedValueQuery.C
Queries/avtGridInformationQuery.C
Queries/avtHohlraumFluxQuery.C
Queries/avtIndividualChordLengthDistributionQuery.C
Queries/avtIndividualRayLengthDistributionQuery.C
Queries/avtIntegrateQuery.C
Queries/avtKurtosisQuery.C
Queries/avtLineScanQuery.C
Queries/avtLineScanTransformQuery.C
Queries/avtL2NormQuery.C
Queries/avtL2NormBetweenCurvesQuery.C
Queries/avtLineSamplerInfoQuery.C
Queries/avtLocalizedCompactnessFactorQuery.C
Queries/avtLocateAndPickNodeQuery.C
Queries/avtLocateAndPickZoneQuery.C
Queries/avtMassDistributionQuery.C
Queries/avtMemoryUsageQuery.C
Queries/avtMinMaxQuery.C
Queries/avtMomentOfInertiaQuery.C
Queries/avtNodeCoordsQuery.C
Queries/avtNumNodesQuery.C
Queries/avtNumZonesQuery.C
Queries/avtOriginalDataMinMaxQuery.C
Queries/avtOriginalDataNumNodesQuery.C
Queries/avtOriginalDataNumZonesQuery.C
Queries/avtOriginalDataSpatialExtentsQuery.C
Queries/avtSampleStatisticsQuery.C
Queries/avtShapeletDecompositionQuery.C
Queries/avtSkewnessQuery.C
Queries/avtSphericalCompactnessFactorQuery.C
Queries/avtIntegralCurveInfoQuery.C
Queries/avtSummationQuery.C
Queries/avtTimeQuery.C
Queries/avtTotalLengthQuery.C
Queries/avtTotalRevolvedSurfaceAreaQuery.C
Queries/avtTotalRevolvedVolumeQuery.C
Queries/avtTotalSurfaceAreaQuery.C
Queries/avtTotalVolumeQuery.C
Queries/avtTrajectoryByNode.C
Queries/avtTrajectoryByZone.C
Queries/avtVariableByNodeQuery.C
Queries/avtVariableByZoneQuery.C
Queries/avtVariableSummationQuery.C
Queries/avtWatertightQuery.C
Queries/avtWeightedVariableSummationQuery.C
Queries/avtXRayImageQuery.C
Queries/avtZoneCenterQuery.C
Queries/MinMaxInfo.C
Queries/avtGyRadiusQuery.C
)

IF(VISIT_PYTHON_FILTERS)
    LIST(APPEND QUERIES_SOURCES Queries/avtPythonQuery.C)
ENDIF(VISIT_PYTHON_FILTERS)

SET(AVTQUERY_SOURCES
${ABSTRACT_SOURCES}
${MISC_SOURCES}
${PICK_SOURCES}
${QUERIES_SOURCES}
)

INCLUDE_DIRECTORIES(
${VISIT_COMMON_INCLUDES}
${CMAKE_CURRENT_SOURCE_DIR}/Abstract
${CMAKE_CURRENT_SOURCE_DIR}/Misc
${CMAKE_CURRENT_SOURCE_DIR}/Pick
${CMAKE_CURRENT_SOURCE_DIR}/Queries
${VISIT_SOURCE_DIR}/avt/Math
${VISIT_SOURCE_DIR}/avt/VisWindow/VisWindow
${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
${VISIT_SOURCE_DIR}/avt/Database/Database
${VISIT_SOURCE_DIR}/avt/Expressions/Abstract
${VISIT_SOURCE_DIR}/avt/Expressions/CMFE
${VISIT_SOURCE_DIR}/avt/Expressions/Conditional
${VISIT_SOURCE_DIR}/avt/Expressions/Derivations
${VISIT_SOURCE_DIR}/avt/Expressions/General
${VISIT_SOURCE_DIR}/avt/Expressions/ImageProcessing
${VISIT_SOURCE_DIR}/avt/Expressions/Management
${VISIT_SOURCE_DIR}/avt/Expressions/Math
${VISIT_SOURCE_DIR}/avt/Expressions/MeshQuality
${VISIT_SOURCE_DIR}/avt/Filters
${VISIT_SOURCE_DIR}/avt/MIR/Base
${VISIT_SOURCE_DIR}/avt/MIR/Tet
${VISIT_SOURCE_DIR}/avt/MIR/Zoo
${VISIT_SOURCE_DIR}/avt/Pipeline/AbstractFilters
${VISIT_SOURCE_DIR}/avt/Pipeline/Data
${VISIT_SOURCE_DIR}/avt/Pipeline/Pipeline
${VISIT_SOURCE_DIR}/avt/Pipeline/Sinks
${VISIT_SOURCE_DIR}/avt/Pipeline/Sources
${VISIT_SOURCE_DIR}/avt/Shapelets
${VISIT_SOURCE_DIR}/visit_vtk/full
${VISIT_SOURCE_DIR}/visit_vtk/lightweight
${VTK_INCLUDE_DIRS}
)

IF(VISIT_PYTHON_FILTERS)
    INCLUDE_DIRECTORIES(
        ${PYTHON_INCLUDE_PATH}
        ${VISIT_SOURCE_DIR}/avt/PythonFilters
    )
ENDIF(VISIT_PYTHON_FILTERS)

# Add link directories
LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY})

IF(VISIT_PYTHON_FILTERS)
    LINK_DIRECTORIES(${PYTHON_LIBRARY_DIRS})
ENDIF(VISIT_PYTHON_FILTERS)

#********************************* SERIAL ************************************
ADD_LIBRARY(avtquery_ser ${AVTQUERY_SOURCES})
TARGET_LINK_LIBRARIES(avtquery_ser visitcommon avtmath avtshapelets avtexpressions_ser avtfilters_ser visit_vtk)
IF(VISIT_PYTHON_FILTERS)
    TARGET_LINK_LIBRARIES(avtquery_ser avtpythonfilters_ser)
ENDIF(VISIT_PYTHON_FILTERS)
VISIT_INSTALL_TARGETS(avtquery_ser)

#********************************* PARALLEL **********************************
IF(VISIT_PARALLEL)
    ADD_PARALLEL_LIBRARY(avtquery_par ${AVTQUERY_SOURCES})
    TARGET_LINK_LIBRARIES(avtquery_par visitcommon avtmath avtshapelets avtexpressions_par avtfilters_par visit_vtk)
    IF(VISIT_PYTHON_FILTERS)
        TARGET_LINK_LIBRARIES(avtquery_par avtpythonfilters_par)
    ENDIF(VISIT_PYTHON_FILTERS)
    VISIT_INSTALL_TARGETS(avtquery_par)
ENDIF(VISIT_PARALLEL)
