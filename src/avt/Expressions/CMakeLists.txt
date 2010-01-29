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
Abstract/avtBinaryMathExpression.C
Abstract/avtComparisonExpression.C
Abstract/avtExpressionFilter.C
Abstract/avtExpressionDataTreeIterator.C
Abstract/avtMacroExpressionFilter.C
Abstract/avtMultipleInputExpressionFilter.C
Abstract/avtSingleInputExpressionFilter.C
Abstract/avtUnaryMathExpression.C
)

SET(CONDITIONAL_SOURCES
Conditional/avtConditionalExpression.C
Conditional/avtLogicalAndExpression.C
Conditional/avtLogicalOrExpression.C
Conditional/avtLogicalNegationExpression.C
Conditional/avtTestEqualToExpression.C
Conditional/avtTestGreaterThanExpression.C
Conditional/avtTestGreaterThanOrEqualToExpression.C
Conditional/avtTestLessThanExpression.C
Conditional/avtTestLessThanOrEqualToExpression.C
Conditional/avtTestNotEqualToExpression.C
)

SET(CMFE_SOURCES
CMFE/avtCMFEExpression.C
CMFE/avtConnCMFEExpression.C
CMFE/avtCurveCMFEExpression.C
CMFE/avtEvalPlaneExpression.C
CMFE/avtEvalPointExpression.C
CMFE/avtEvalTransformExpression.C
CMFE/avtPosCMFEAlgorithm.C
CMFE/avtPosCMFEExpression.C
CMFE/avtSymmEvalExpression.C
CMFE/avtSymmPlaneExpression.C
CMFE/avtSymmPointExpression.C
CMFE/avtSymmTransformExpression.C
)

SET(DERIVATIONS_SOURCES
Derivations/avtCurvatureExpression.C
Derivations/avtDisplacementExpression.C
Derivations/avtEffectiveTensorExpression.C
Derivations/avtLocalizedCompactnessExpression.C
Derivations/avtPrincipalDeviatoricTensorExpression.C
Derivations/avtPrincipalTensorExpression.C
Derivations/avtStrainAlmansiExpression.C
Derivations/avtStrainGreenLagrangeExpression.C
Derivations/avtStrainInfinitesimalExpression.C
Derivations/avtStrainRateExpression.C
Derivations/avtStrainTensorExpression.C
Derivations/avtTensorMaximumShearExpression.C
)

SET(GENERAL_SOURCES
General/avtApplyDDFExpression.C
General/avtApplyEnumerationExpression.C
General/avtApplyMapExpression.C 
General/avtArrayComposeExpression.C
General/avtArrayComposeWithBinsExpression.C
General/avtArrayDecomposeExpression.C
General/avtColorComposeExpression.C
General/avtConnComponentsExpression.C
General/avtConstantFunctionExpression.C
General/avtCurlExpression.C
General/avtCurveDomainExpression.C
General/avtCurveExpression.C
General/avtDataIdExpression.C
General/avtDegreeExpression.C
General/avtDistanceToBestFitLineExpression.C
General/avtDivergenceExpression.C
General/avtDominantMaterialExpression.C
General/avtExpressionComponentMacro.C
General/avtExternalNodeExpression.C
General/avtGeodesicVectorQuantizeExpression.C
General/avtGradientExpression.C
General/avtHSVColorComposeExpression.C
General/avtKeyAggregatorExpression.C
General/avtLaplacianExpression.C
General/avtMIRvfExpression.C
General/avtMagnitudeExpression.C
General/avtMatErrorExpression.C
General/avtMatvfExpression.C
General/avtMeshCoordinateExpression.C
General/avtMinMaxExpression.C
General/avtNMatsExpression.C
General/avtNeighborEvaluatorExpression.C
General/avtNormalizeExpression.C
General/avtPerMaterialValueExpression.C
General/avtPerformColorTableLookupExpression.C
General/avtProcessorIdExpression.C
General/avtRandomExpression.C
General/avtRecenterExpression.C
General/avtRectilinearLaplacianExpression.C
General/avtRelativeDifferenceExpression.C
General/avtResradExpression.C
General/avtSpecMFExpression.C
General/avtSurfaceNormalExpression.C
General/avtTensorContractionExpression.C
General/avtTransposeExpression.C
General/avtVariableSkewExpression.C
General/avtVectorComposeExpression.C
General/avtVectorDecomposeExpression.C
General/avtViscousStressExpression.C
General/avtZoneTypeExpression.C
)

SET(IMAGEPROCESSING_SOURCES
ImageProcessing/avtAbelInversionExpression.C
ImageProcessing/avtConservativeSmoothingExpression.C
ImageProcessing/avtMeanFilterExpression.C
ImageProcessing/avtMedianFilterExpression.C
)

SET(MANAGEMENT_SOURCES
Management/avtConstantCreatorExpression.C
Management/avtExecuteThenTimeLoopFilter.C
Management/avtExpressionEvaluatorFilter.C
Management/avtExprNode.C
Management/avtExprNodeFactory.C
Management/avtFunctionExpr_CreateConditionalFilters.C 
Management/avtFunctionExpr_CreateMeshQualityFilters.C 
Management/avtFunctionExpr_CreateCMFEFilters.C 
Management/avtFunctionExpr_CreateTimeAndValueFilters.C 
Management/avtFunctionExpr_CreateImageProcessingFilters.C 
Management/avtFunctionExpr_CreateVectorMatrixFilters.C 
Management/avtFunctionExpr_CreateMathFilters.C 
Management/avtFunctionExpr_CreateMeshFilters.C 
Management/avtFunctionExpr_CreateMaterialFilters.C 
Management/avtTimeExpression.C 
Management/ExprPipelineState.C
)

SET(MATH_SOURCES
Math/avtAbsValExpression.C
Math/avtArccosExpression.C
Math/avtArcsinExpression.C
Math/avtArctanExpression.C
Math/avtArctan2Expression.C
Math/avtBase10LogExpression.C
Math/avtBinaryAddExpression.C
Math/avtBinaryAndExpression.C
Math/avtBinaryDivideExpression.C
Math/avtBinaryMultiplyExpression.C
Math/avtBinaryPowerExpression.C
Math/avtBinarySubtractExpression.C
Math/avtCeilingExpression.C
Math/avtCosExpression.C
Math/avtCoshExpression.C
Math/avtCylindricalCoordinatesExpression.C
Math/avtCylindricalRadiusExpression.C
Math/avtDegreeToRadianExpression.C
Math/avtDeterminantExpression.C
Math/avtEigenvalueExpression.C
Math/avtEigenvectorExpression.C
Math/avtExpExpression.C
Math/avtFloorExpression.C
Math/avtIdentityExpression.C
Math/avtModuloExpression.C
Math/avtInverseExpression.C
Math/avtNaturalLogExpression.C
Math/avtPolarCoordinatesExpression.C
Math/avtRadianToDegreeExpression.C
Math/avtRoundExpression.C
Math/avtSinExpression.C
Math/avtSinhExpression.C
Math/avtSquareExpression.C
Math/avtSquareRootExpression.C
Math/avtTanExpression.C
Math/avtTanhExpression.C
Math/avtTraceExpression.C
Math/avtUnaryMinusExpression.C
Math/avtVectorCrossProductExpression.C
)

SET(MESHQUALITY_SOURCES
MeshQuality/avtCornerAngle.C
MeshQuality/avtEdgeLength.C
MeshQuality/avtFacePlanarity.C
MeshQuality/avtNeighborExpression.C
MeshQuality/avtNodeDegreeExpression.C
MeshQuality/avtRevolvedSurfaceArea.C
MeshQuality/avtRevolvedVolume.C
MeshQuality/avtSideVolume.C
MeshQuality/avtVMetricArea.C
MeshQuality/avtVMetricAspectGamma.C
MeshQuality/avtVMetricAspectRatio.C
MeshQuality/avtVMetricCondition.C
MeshQuality/avtVMetricDiagonalRatio.C
MeshQuality/avtVMetricDimension.C
MeshQuality/avtVMetricJacobian.C
MeshQuality/avtVMetricMaxDiagonal.C
MeshQuality/avtVMetricMaximumAngle.C
MeshQuality/avtVMetricMinDiagonal.C
MeshQuality/avtVMetricMinimumAngle.C
MeshQuality/avtVMetricOddy.C
MeshQuality/avtVMetricRelativeSize.C
MeshQuality/avtVMetricScaledJacobian.C
MeshQuality/avtVMetricShape.C
MeshQuality/avtVMetricShapeAndSize.C
MeshQuality/avtVMetricShear.C
MeshQuality/avtVMetricSkew.C
MeshQuality/avtVMetricStretch.C
MeshQuality/avtVMetricTaper.C
MeshQuality/avtVMetricVolume.C
MeshQuality/avtVMetricWarpage.C
MeshQuality/avtVerdictExpression.C
)

SET(TIMEITERATORS_SOURCES
TimeIterators/avtAverageOverTimeExpression.C
TimeIterators/avtMaxOverTimeExpression.C
TimeIterators/avtMinOverTimeExpression.C
TimeIterators/avtSumOverTimeExpression.C
TimeIterators/avtTimeIteratorDataTreeIteratorExpression.C
TimeIterators/avtTimeIteratorExpression.C
TimeIterators/avtValueAtExtremaExpression.C
TimeIterators/avtWhenConditionIsTrueExpression.C
)

SET(AVTEXPRESSIONS_SOURCES
${ABSTRACT_SOURCES}
${CONDITIONAL_SOURCES}
${CMFE_SOURCES}
${DERIVATIONS_SOURCES}
${GENERAL_SOURCES}
${IMAGEPROCESSING_SOURCES}
${MANAGEMENT_SOURCES}
${MATH_SOURCES}
${MESHQUALITY_SOURCES}
${TIMEITERATORS_SOURCES}
)

INCLUDE_DIRECTORIES(
${VISIT_COMMON_INCLUDES}
${CMAKE_CURRENT_SOURCE_DIR}/Abstract
${CMAKE_CURRENT_SOURCE_DIR}/CMFE
${CMAKE_CURRENT_SOURCE_DIR}/Conditional
${CMAKE_CURRENT_SOURCE_DIR}/Derivations
${CMAKE_CURRENT_SOURCE_DIR}/General
${CMAKE_CURRENT_SOURCE_DIR}/ImageProcessing
${CMAKE_CURRENT_SOURCE_DIR}/Management
${CMAKE_CURRENT_SOURCE_DIR}/Math
${CMAKE_CURRENT_SOURCE_DIR}/MeshQuality
${CMAKE_CURRENT_SOURCE_DIR}/TimeIterators
${VISIT_SOURCE_DIR}/avt/Math
${VISIT_SOURCE_DIR}/avt/VisWindow/VisWindow
${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
${VISIT_SOURCE_DIR}/avt/Database/Database
${VISIT_SOURCE_DIR}/avt/DDF
${VISIT_SOURCE_DIR}/avt/Filters
${VISIT_SOURCE_DIR}/avt/MIR/Base
${VISIT_SOURCE_DIR}/avt/MIR/Tet
${VISIT_SOURCE_DIR}/avt/MIR/Zoo
${VISIT_SOURCE_DIR}/avt/Pipeline/AbstractFilters
${VISIT_SOURCE_DIR}/avt/Pipeline/Data
${VISIT_SOURCE_DIR}/avt/Pipeline/Pipeline
${VISIT_SOURCE_DIR}/avt/Pipeline/Sinks
${VISIT_SOURCE_DIR}/avt/Pipeline/Sources
${VISIT_SOURCE_DIR}/avt/Plotter
${VISIT_SOURCE_DIR}/visit_vtk/full
${VISIT_SOURCE_DIR}/visit_vtk/lightweight
${VISIT_SOURCE_DIR}/third_party_builtin/verdict
${VTK_INCLUDE_DIRS}
)

# Add link directories
LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY} ${VTK_LIBRARY_DIRS} ${MESA_LIBRARY_DIR} ${GLEW_LIBRARY_DIR})

#********************************* SERIAL ************************************
ADD_LIBRARY(avtexpressions_ser ${AVTEXPRESSIONS_SOURCES})
TARGET_LINK_LIBRARIES(avtexpressions_ser visitcommon avtdbatts avtdatabase_ser avtddf_ser avtplotter_ser avtfilters_ser avtpipeline_ser)
VISIT_INSTALL_TARGETS(avtexpressions_ser)

#********************************* PARALLEL **********************************
IF(VISIT_PARALLEL)
    ADD_PARALLEL_LIBRARY(avtexpressions_par ${AVTEXPRESSIONS_SOURCES})
    TARGET_LINK_LIBRARIES(avtexpressions_par visitcommon avtdbatts avtdatabase_par avtddf_par avtplotter_par avtfilters_par avtpipeline_par)
    VISIT_INSTALL_TARGETS(avtexpressions_par)
ENDIF(VISIT_PARALLEL)
