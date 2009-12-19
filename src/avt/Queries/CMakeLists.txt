#*****************************************************************************
#
# Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
# Produced at the Lawrence Livermore National Laboratory
# LLNL-CODE-400142
# All rights reserved.
#
# This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
# full copyright notice is contained in the file COPYRIGHT located at the root
# of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
#
# Redistribution  and  use  in  source  and  binary  forms,  with  or  without
# modification, are permitted provided that the following conditions are met:
#
#  - Redistributions of  source code must  retain the above  copyright notice,
#    this list of conditions and the disclaimer below.
#  - Redistributions in binary form must reproduce the above copyright notice,
#    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
#    documentation and/or other materials provided with the distribution.
#  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
#    be used to endorse or promote products derived from this software without
#    specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
# ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
# LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
# DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
# SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
# CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
# LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
# OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
# DAMAGE.
#
# Modifications:
#
#****************************************************************************/

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
Misc/avtQueryFactory.C
Misc/avtQueryOverTimeFilter.C
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
Queries/avtBestFitLineQuery.C
Queries/avtCentroidQuery.C
Queries/avtCompactnessQuery.C
Queries/avtConnComponentsQuery.C
Queries/avtConnComponentsAreaQuery.C
Queries/avtConnComponentsCentroidQuery.C
Queries/avtConnComponentsSummaryQuery.C
Queries/avtConnComponentsVolumeQuery.C
Queries/avtConnComponentsVariableQuery.C
Queries/avtConnComponentsWeightedVariableQuery.C
Queries/avtCycleQuery.C
Queries/avtDistanceFromBoundaryQuery.C
Queries/avtEllipticalCompactnessFactorQuery.C
Queries/avtEulerianQuery.C
Queries/avtExpectedValueQuery.C
Queries/avtHohlraumFluxQuery.C
Queries/avtIndividualChordLengthDistributionQuery.C
Queries/avtIndividualRayLengthDistributionQuery.C
Queries/avtIntegrateQuery.C
Queries/avtKurtosisQuery.C
Queries/avtLineScanQuery.C
Queries/avtLineScanTransformQuery.C
Queries/avtL2NormQuery.C
Queries/avtL2NormBetweenCurvesQuery.C
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
Queries/avtZoneCenterQuery.C
Queries/MinMaxInfo.C
)

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

# Add link directories
LINK_DIRECTORIES(${LIBRARY_OUTPUT_PATH} ${VTK_LIBRARY_DIRS} ${MESA_LIBRARY_DIR} ${GLEW_LIBRARY_DIR})

#********************************* SERIAL ************************************
ADD_LIBRARY(avtquery_ser ${AVTQUERY_SOURCES})
TARGET_LINK_LIBRARIES(avtquery_ser visitcommon avtmath avtshapelets avtexpressions_ser avtfilters_ser visit_vtk)
VISIT_INSTALL_TARGETS(avtquery_ser)

#********************************* PARALLEL **********************************
IF(VISIT_PARALLEL)
    ADD_PARALLEL_LIBRARY(avtquery_par ${AVTQUERY_SOURCES})
    TARGET_LINK_LIBRARIES(avtquery_par visitcommon avtmath avtshapelets avtexpressions_par avtfilters_par visit_vtk)
    VISIT_INSTALL_TARGETS(avtquery_par)
ENDIF(VISIT_PARALLEL)
