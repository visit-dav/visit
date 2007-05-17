/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                                avtExprNode.C                              //
// ************************************************************************* //

#include <avtUnaryMinusFilter.h>
#include <avtBinaryAddFilter.h>
#include <avtBinarySubtractFilter.h>
#include <avtBinaryMultiplyFilter.h>
#include <avtBinaryDivideFilter.h>
#include <avtBinaryPowerFilter.h>
#include <avtCeilingFilter.h>
#include <avtColorComposeFilter.h>
#include <avtFloorFilter.h>
#include <avtModuloFilter.h>
#include <avtRoundFilter.h>
#include <avtSinFilter.h>
#include <avtCosFilter.h>
#include <avtDistanceToBestFitLineFilter.h>
#include <avtRandomFilter.h>
#include <avtArctanFilter.h>
#include <avtArctan2Filter.h>
#include <avtArcsinFilter.h>
#include <avtArccosFilter.h>
#include <avtDegreeToRadianFilter.h>
#include <avtDeterminantFilter.h>
#include <avtEigenvalueFilter.h>
#include <avtEigenvectorFilter.h>
#include <avtExpressionFilter.h>
#include <avtInverseFilter.h>
#include <avtTraceFilter.h>
#include <avtTensorMaximumShearFilter.h>
#include <avtPrincipalDeviatoricTensorFilter.h>
#include <avtPrincipalTensorFilter.h>
#include <avtEffectiveTensorFilter.h>
#include <avtStrainAlmansiFilter.h>
#include <avtStrainGreenLagrangeFilter.h>
#include <avtStrainInfinitesimalFilter.h>
#include <avtStrainRateFilter.h>
#include <avtDisplacementFilter.h>
#include <avtCurvatureExpression.h>
#include <avtGradientFilter.h>
#include <avtCurlFilter.h>
#include <avtDivergenceFilter.h>
#include <avtLaplacianFilter.h>
#include <avtResradFilter.h>
#include <avtMagnitudeFilter.h>
#include <avtNeighborFilter.h>
#include <avtNodeDegreeFilter.h>
#include <avtNormalizeFilter.h>
#include <avtMatvfFilter.h>
#include <avtMIRvfFilter.h>
#include <avtMatErrorFilter.h>
#include <avtNMatsFilter.h>
#include <avtRadianToDegreeFilter.h>
#include <avtRevolvedVolume.h>
#include <avtSideVolume.h>
#include <avtEdgeLength.h>
#include <avtRevolvedSurfaceArea.h>
#include <avtSpecMFFilter.h>
#include <avtTanFilter.h>
#include <avtAbsValFilter.h>
#include <avtNaturalLogFilter.h>
#include <avtBase10LogFilter.h>
#include <avtSquareRootFilter.h>
#include <avtSquareFilter.h>
#include <avtCylindricalCoordinatesFilter.h>
#include <avtPolarCoordinatesFilter.h>
#include <avtVariableSkewFilter.h>
#include <avtVectorComposeFilter.h>
#include <avtVectorDecomposeFilter.h>
#include <avtVectorCrossProductFilter.h>
#include <avtArrayComposeFilter.h>
#include <avtArrayComposeWithBinsFilter.h>
#include <avtArrayDecomposeFilter.h>
#include <avtMeshCoordinateFilter.h>
#include <avtProcessorIdFilter.h>
#include <avtDegreeFilter.h>
#include <avtVMetrics.h>
#include <avtRecenterFilter.h>
#include <avtLocalizedCompactnessExpression.h>
#include <avtApplyDDFExpression.h>
#include <avtConstantCreatorFilter.h>
#include <avtRelativeDifferenceFilter.h>
#include <avtConditionalFilter.h>
#include <avtLogicalAndFilter.h>
#include <avtLogicalNegationFilter.h>
#include <avtLogicalOrFilter.h>
#include <avtTestEqualToFilter.h>
#include <avtTestGreaterThanFilter.h>
#include <avtTestGreaterThanOrEqualToFilter.h>
#include <avtTestLessThanFilter.h>
#include <avtTestLessThanOrEqualToFilter.h>
#include <avtTestNotEqualToFilter.h>
#include <avtNeighborEvaluatorFilter.h>
#include <avtDataIdFilter.h>
#include <avtZoneTypeFilter.h>
#include <avtExpressionComponentMacro.h>
#include <avtAbelInversionExpression.h>
#include <avtConservativeSmoothingExpression.h>
#include <avtMeanFilterExpression.h>
#include <avtMedianFilterExpression.h>
#include <avtConnCMFEExpression.h>
#include <avtPosCMFEExpression.h>
#include <avtExternalNodeExpression.h>
#include <avtSurfaceNormalExpression.h>
#include <avtEvalTransformExpression.h>
#include <avtSymmTransformExpression.h>
#include <avtEvalPlaneExpression.h>
#include <avtSymmPlaneExpression.h>
#include <avtEvalPointExpression.h>
#include <avtSymmPointExpression.h>
#include <avtTimeExpression.h>
#include <avtMinMaxExpression.h>
#include <avtConnComponentsExpression.h>

#include <stdio.h>
#include <ExpressionException.h>
#include <ExprToken.h>
#include <ExprPipelineState.h>
#include <avtExprNode.h>
#include <DebugStream.h>
#include <snprintf.h>

using std::string;

// ****************************************************************************
// Method: avtIntegerConstExpr::CreateFilters
//
// Purpose:
//     Creates the avt filters that are necessary to complete the given
//     constant
//
// Programmer: Jeremy Meredith
// Creation:   June 13, 2005
//
// Note:  taken roughly from the old ConstExpr::CreateFilters
//
// Modifications:
//
// ****************************************************************************
void
avtIntegerConstExpr::CreateFilters(ExprPipelineState *state)
{
    avtConstantCreatorFilter *f = new avtConstantCreatorFilter();
    f->SetValue(value);
    char strrep[30];
    SNPRINTF(strrep, 30, "'%d'", value);
    state->PushName(string(strrep));
    f->SetOutputVariableName(strrep);

    // Keep track of the current dataObject.
    f->SetInput(state->GetDataObject());
    state->SetDataObject(f->GetOutput());
    state->AddFilter(f);
}

// ****************************************************************************
// Method: avtFloatConstExpr::CreateFilters
//
// Purpose:
//     Creates the avt filters that are necessary to complete the given
//     constant
//
// Programmer: Jeremy Meredith
// Creation:   June 13, 2005
//
// Note:  taken roughly from the old ConstExpr::CreateFilters
//
// Modifications:
//
// ****************************************************************************
void
avtFloatConstExpr::CreateFilters(ExprPipelineState *state)
{
    avtConstantCreatorFilter *f = new avtConstantCreatorFilter();
    f->SetValue(value);
    char strrep[30];
    SNPRINTF(strrep, 30, "'%e'", value);
    state->PushName(string(strrep));
    f->SetOutputVariableName(strrep);

    // Keep track of the current dataObject.
    f->SetInput(state->GetDataObject());
    state->SetDataObject(f->GetOutput());
    state->AddFilter(f);
}

// ****************************************************************************
// Method: avtStringConstExpr::CreateFilters
//
// Purpose:
//     Issue an error -- we don't support string constants like this.
//
// Programmer: Jeremy Meredith
// Creation:   June 13, 2005
//
// Modifications:
//
// ****************************************************************************
void
avtStringConstExpr::CreateFilters(ExprPipelineState *state)
{
    EXCEPTION1(ExpressionException,
               "avtStringConstExpr::CreateFilters: "
               "Unsupported constant type: String");
}

// ****************************************************************************
// Method: avtBooleanConstExpr::CreateFilters
//
// Purpose:
//     Issue an error -- we don't support boolean constants like this.
//
// Programmer: Jeremy Meredith
// Creation:   June 13, 2005
//
// Modifications:
//
// ****************************************************************************
void
avtBooleanConstExpr::CreateFilters(ExprPipelineState *state)
{
    EXCEPTION1(ExpressionException,
               "avtStringConstExpr::CreateFilters: "
               "Unsupported constant type: Bool");
}


void
avtUnaryExpr::CreateFilters(ExprPipelineState *state)
{
    dynamic_cast<avtExprNode*>(expr)->CreateFilters(state);

    avtSingleInputExpressionFilter *f = NULL;
    if (op == '-')
        f = new avtUnaryMinusFilter();
    else
    {
        string error =
            string("avtUnaryExpr::CreateFilters: "
                   "Unknown unary operator:\"") + op + string("\".");
        EXCEPTION1(ExpressionException, error);
    }

    // Set the variable the function should process.
    string inputName = state->PopName();
    f->AddInputVariableName(inputName.c_str());

    // Set the variable the function should output.
    string outputName = string() + op + "(" + inputName + ")";
    state->PushName(outputName);
    f->SetOutputVariableName(outputName.c_str());

    // Keep track of the current dataObject.
    f->SetInput(state->GetDataObject());
    state->SetDataObject(f->GetOutput());
    state->AddFilter(f);
}

void
avtBinaryExpr::CreateFilters(ExprPipelineState *state)
{
    dynamic_cast<avtExprNode*>(left)->CreateFilters(state);
    dynamic_cast<avtExprNode*>(right)->CreateFilters(state);

    avtMultipleInputExpressionFilter *f = NULL;
    if (op == '+')
        f = new avtBinaryAddFilter();
    else if (op == '-')
        f = new avtBinarySubtractFilter();
    else if (op == '*')
        f = new avtBinaryMultiplyFilter();
    else if (op == '/')
        f = new avtBinaryDivideFilter();
    else if (op == '^')
        f = new avtBinaryPowerFilter();
    else
    {
        string error =
            string("avtBinaryExpr::CreateFilters: "
                   "Unknown binary operator:\"") + op + string("\".");
        EXCEPTION1(ExpressionException, error);
    }

    // Set the variable the function should process.
    string inputName2 = state->PopName();
    string inputName1 = state->PopName();
    f->AddInputVariableName(inputName1.c_str());
    f->AddInputVariableName(inputName2.c_str());

    // Set the variable the function should output>
    string outputName = inputName1 + op + inputName2;
    state->PushName(outputName);
    f->SetOutputVariableName(outputName.c_str());

    // Keep track of the current input.
    f->SetInput(state->GetDataObject());
    state->SetDataObject(f->GetOutput());
    state->AddFilter(f);
}

void
avtIndexExpr::CreateFilters(ExprPipelineState *state)
{
    dynamic_cast<avtExprNode*>(expr)->CreateFilters(state);

    avtVectorDecomposeFilter *f = new avtVectorDecomposeFilter(ind);

    // Set the variable the function should process.
    string inputName = state->PopName();
    f->AddInputVariableName(inputName.c_str());

    // Set the variable the function should output.
    char value_name[200];
    SNPRINTF(value_name, 200, "%d", ind);
    string outputName = inputName + "[" + value_name + "]";
    state->PushName(outputName);
    f->SetOutputVariableName(outputName.c_str());

    // Keep track of the current dataObject.
    f->SetInput(state->GetDataObject());
    state->SetDataObject(f->GetOutput());
    state->AddFilter(f);
}

// ****************************************************************************
//  Method:  avtVectorExpr::CreateFilters
//
//  Purpose:
//    Creates the avt filters necessary to evaluate a vector.
//
//  Arguments:
//    state      The pipeline state
//
//  Programmer:  Sean Ahern
//
//  Modifications:
//    Jeremy Meredith, Thu Aug 14 09:52:12 PDT 2003
//    Allow 2D vectors.
//
// ****************************************************************************
void
avtVectorExpr::CreateFilters(ExprPipelineState *state)
{
    dynamic_cast<avtExprNode*>(x)->CreateFilters(state);
    dynamic_cast<avtExprNode*>(y)->CreateFilters(state);
    if (z)
        dynamic_cast<avtExprNode*>(z)->CreateFilters(state);

    avtVectorComposeFilter *f = new avtVectorComposeFilter();

    // Set the variable the function should process.
    string inputName3 = z ? state->PopName() : "";
    string inputName2 = state->PopName();
    string inputName1 = state->PopName();
    f->AddInputVariableName(inputName1.c_str());
    f->AddInputVariableName(inputName2.c_str());
    if (z)
        f->AddInputVariableName(inputName3.c_str());

    // Set the variable the function should output>
    string outputName;
    if (z)
        outputName = string("{") + inputName1 + "," + inputName2 + "," +
                                                             inputName3 + "}";
    else
        outputName = string("{") + inputName1 + "," + inputName2 + "}";

    state->PushName(outputName);
    f->SetOutputVariableName(outputName.c_str());

    // Keep track of the current input.
    f->SetInput(state->GetDataObject());
    state->SetDataObject(f->GetOutput());
    state->AddFilter(f);
}

// ****************************************************************************
// Method: avtFunctionExpr::CreateFilters
//
// Purpose:
//     Creates the avt filters that are necessary to complete the given
//     function.
//
// Notes:  
//     Moved from public CreateFilters(ExprPipelineState) method, so as to 
//     remove nested-if blocks.  Too many nested blocks cause compile failure 
//     on Win32.
//
// Programmer: Kathleen Bonnell 
// Creation:   November 17, 2006
//
// Modifications:
//
//    Thomas R. Treadway, Tue Dec  5 15:09:08 PST 2006
//    added avtStrainAlmansiFilter, avtStrainGreenLagrangeFilter,
//    avtStrainInfinitesimalFilter, avtStrainRateFilter, and
//    avtDisplacementFilter
//
//    Hank Childs, Fri Dec 22 10:03:40 PST 2006
//    Added eval_point and symm_point.
//
//    Hank Childs, Fri Jan 12 13:45:04 PST 2007
//    Added array_compose_with_bins.
// 
//    Cyrus Harrison, Wed Feb 21 09:37:38 PST 2007
//    Added conn_components  
//
//    Brad Whitlock, Mon Apr 23 17:32:14 PST 2007
//    Added color.
//
//    Sean Ahern, Tue May  8 13:11:47 EDT 2007
//    Added atan2.
// 
// ****************************************************************************

avtExpressionFilter *
avtFunctionExpr::CreateFilters(string functionName)
{
    if (functionName == "sin")
        return new avtSinFilter();
    if (functionName == "cos")
        return new avtCosFilter();
    if (functionName == "tan")
        return new avtTanFilter();
    if (functionName == "atan")
        return new avtArctanFilter();
    if (functionName == "atan2")
        return new avtArctan2Filter();
    if (functionName == "asin")
        return new avtArcsinFilter();
    if (functionName == "acos")
        return new avtArccosFilter();
    if (functionName == "deg2rad")
        return new avtDegreeToRadianFilter();
    if (functionName == "rad2deg")
        return new avtRadianToDegreeFilter();
    if (functionName == "abs")
        return new avtAbsValFilter();
    if (functionName == "ln")
        return new avtNaturalLogFilter();
    if ((functionName == "log") || (functionName == "log10"))
        return new avtBase10LogFilter();
    if (functionName == "sqrt")
        return new avtSquareRootFilter();
    if ((functionName == "sq") || (functionName == "sqr"))
        return new avtSquareFilter();
    if (functionName == "mod")
        return new avtModuloFilter();
    if (functionName == "ceil")
        return new avtCeilingFilter();
    if (functionName == "floor")
        return new avtFloorFilter();
    if (functionName == "round")
        return new avtRoundFilter();
    if ((functionName == "random") || (functionName == "rand"))
        return new avtRandomFilter();
    if (functionName == "cross")
        return new avtVectorCrossProductFilter();
    if (functionName == "dot")
        return new avtBinaryMultiplyFilter();
    if (functionName == "array_compose")
        return new avtArrayComposeFilter();
    if (functionName == "array_compose_with_bins")
        return new avtArrayComposeWithBinsFilter();
    if (functionName == "array_decompose")
        return new avtArrayDecomposeFilter();
    if (functionName == "matvf")
        return new avtMatvfFilter();
    if (functionName == "mirvf")
        return new avtMIRvfFilter();
    if (functionName == "materror")
        return new avtMatErrorFilter();
    if (functionName == "specmf")
        return new avtSpecMFFilter();
    if (functionName == "nmats")
        return new avtNMatsFilter();
    if (functionName == "zonetype")
        return new avtZoneTypeFilter();
    if (functionName == "localized_compactness")
        return new avtLocalizedCompactnessExpression();
    if (functionName == "recenter")
        return new avtRecenterFilter();
    if (functionName == "det" || functionName == "determinant")
        return new avtDeterminantFilter();
    if (functionName == "eigenvalue")
        return new avtEigenvalueFilter();
    if (functionName == "eigenvector")
        return new avtEigenvectorFilter();
    if (functionName == "inverse")
        return new avtInverseFilter();
    if (functionName == "trace")
        return new avtTraceFilter();
    if (functionName == "effective_tensor")
        return new avtEffectiveTensorFilter();
    if (functionName == "tensor_maximum_shear")
        return new avtTensorMaximumShearFilter();
    if (functionName == "principal_tensor")
        return new avtPrincipalTensorFilter();
    if (functionName == "principal_deviatoric_tensor")
        return new avtPrincipalDeviatoricTensorFilter();
    if (functionName == "strain_almansi")
        return new avtStrainAlmansiFilter();
    if (functionName == "strain_green_lagrange")
        return new avtStrainGreenLagrangeFilter();
    if (functionName == "strain_infinitesimal")
        return new avtStrainInfinitesimalFilter();
    if (functionName == "strain_rate")
        return new avtStrainRateFilter();
    if (functionName == "displacement")
        return new avtDisplacementFilter();
    if (functionName == "degree")
        return new avtDegreeFilter();
    if (functionName == "cylindrical")
        return new avtCylindricalCoordinatesFilter();
    if (functionName == "polar")
        return new avtPolarCoordinatesFilter();
    if (functionName == "coord" || functionName == "coords")
        return new avtMeshCoordinateFilter();
    if (functionName == "procid")
        return new avtProcessorIdFilter();
    if (functionName == "mean_curvature")
    {
        avtCurvatureExpression *c = new avtCurvatureExpression;
        c->DoGaussCurvature(false);
        return c;
    }
    if (functionName == "gauss_curvature")
    {
        avtCurvatureExpression *c = new avtCurvatureExpression;
        c->DoGaussCurvature(true);
        return c;
    }
    if (functionName == "ijk_gradient" || functionName == "ij_gradient")
    {
        avtGradientFilter *g = new avtGradientFilter();
        g->SetDoLogicalGradient(true);
        return g;
    }
    if (functionName == "conn_components")
        return new avtConnComponentsExpression();
    if (functionName == "gradient")
        return new avtGradientFilter();
    if (functionName == "curl")
        return new avtCurlFilter();
    if (functionName == "divergence")
        return new avtDivergenceFilter();
    if (functionName == "laplacian" || functionName == "Laplacian")
        return new avtLaplacianFilter();
    if (functionName == "resrad")
        return new avtResradFilter();
    if (functionName == "magnitude")
        return new avtMagnitudeFilter();
    if (functionName == "normalize")
        return new avtNormalizeFilter();
    if (functionName == "relative_difference")
        return new avtRelativeDifferenceFilter();
    if (functionName == "var_skew")
        return new avtVariableSkewFilter();
    if (functionName == "apply_ddf")
        return new avtApplyDDFExpression();
    if (functionName == "if")
        return new avtConditionalFilter();
    if (functionName == "and")
        return new avtLogicalAndFilter();
    if (functionName == "or")
        return new avtLogicalOrFilter();
    if (functionName == "not")
        return new avtLogicalNegationFilter();
    if (functionName == "le" || functionName == "lte")
        return new avtTestLessThanOrEqualToFilter();
    if (functionName == "ge" || functionName == "gte")
        return new avtTestGreaterThanOrEqualToFilter();
    if (functionName == "lt")
        return new avtTestLessThanFilter();
    if (functionName == "gt")
        return new avtTestGreaterThanFilter();
    if (functionName == "eq" || functionName == "equal" || 
             functionName == "equals")
        return new avtTestEqualToFilter();
    if (functionName == "ne" || functionName == "neq" ||
             functionName == "notequal" || functionName == "notequals")
        return new avtTestNotEqualToFilter();
    if (functionName == "neighbor")
        return new avtNeighborFilter();
    if (functionName == "external_node")
        return new avtExternalNodeExpression();
    if (functionName == "node_degree")
        return new avtNodeDegreeFilter();
    // Begin Verdict Metrics
    if (functionName == "area")
        return new avtVMetricArea();
    if (functionName == "aspect")
        return new avtVMetricAspectRatio();
    if (functionName == "skew")
        return new avtVMetricSkew();
    if (functionName == "taper")
        return new avtVMetricTaper();
    if (functionName == "volume")
        return new avtVMetricVolume();
    if (functionName == "volume2")
    {
        avtVMetricVolume *vol = new avtVMetricVolume();
        vol->UseVerdictHex(false);
        return vol;
    }
    if (functionName == "min_edge_length")
    {
        avtEdgeLength *el = new avtEdgeLength();
        el->SetTakeMin(true);
        return el;
    }
    if (functionName == "max_edge_length")
    {
        avtEdgeLength *el = new avtEdgeLength();
        el->SetTakeMin(false);
        return el;
    }
    if (functionName == "min_side_volume")
    {
        avtSideVolume *sv = new avtSideVolume();
        sv->SetTakeMin(true);
        return sv;
    }
    if (functionName == "max_side_volume")
    {
        avtSideVolume *sv = new avtSideVolume();
        sv->SetTakeMin(false);
        return sv;
    }
    if (functionName == "stretch")
        return new avtVMetricStretch();
    if (functionName == "diagonal")
        return new avtVMetricDiagonal();
    if (functionName == "dimension")
        return new avtVMetricDimension();
    if (functionName == "oddy")
        return new avtVMetricOddy();
    if (functionName == "condition")
        return new avtVMetricCondition();
    if (functionName == "jacobian")
        return new avtVMetricJacobian();
    if (functionName == "scaled_jacobian")
        return new avtVMetricScaledJacobian();
    if (functionName == "shear")
        return new avtVMetricShear();
    if (functionName == "shape")
        return new avtVMetricShape();
    if (functionName == "relative_size")
        return new avtVMetricRelativeSize();
    if (functionName == "shape_and_size")
        return new avtVMetricShapeAndSize();
    if (functionName == "aspect_gamma")
        return new avtVMetricAspectGamma();
    if (functionName == "warpage")
        return new avtVMetricWarpage();
    if (functionName == "largest_angle")
        return new avtVMetricLargestAngle();
    if (functionName == "smallest_angle")
        return new avtVMetricSmallestAngle();
    if (functionName == "revolved_volume")
        return new avtRevolvedVolume;
    if (functionName == "revolved_surface_area")
        return new avtRevolvedSurfaceArea;
    if (functionName == "conservative_smoothing")
        return new avtConservativeSmoothingExpression;
    if (functionName == "mean_filter")
        return new avtMeanFilterExpression;
    if (functionName == "median_filter")
        return new avtMedianFilterExpression;
    if (functionName == "abel_inversion")
        return new avtAbelInversionExpression;
    if (functionName == "conn_cmfe")
        return new avtConnCMFEExpression;
    if (functionName == "pos_cmfe")
        return new avtPosCMFEExpression;
    if (functionName == "eval_transform")
        return new avtEvalTransformExpression;
    if (functionName == "symm_transform")
        return new avtSymmTransformExpression;
    if (functionName == "eval_plane")
        return new avtEvalPlaneExpression;
    if (functionName == "symm_plane")
        return new avtSymmPlaneExpression;
    if (functionName == "eval_point")
        return new avtEvalPointExpression;
    if (functionName == "symm_point")
        return new avtSymmPointExpression;
    if (functionName == "time")
        return new avtTimeExpression;
    if (functionName == "surface_normal" || 
             functionName == "point_surface_normal")
    {
        avtSurfaceNormalExpression *ff = new avtSurfaceNormalExpression;
        ff->DoPointNormals(true);
        return ff;
    }
    if (functionName == "cell_surface_normal")
    {
        avtSurfaceNormalExpression *ff = new avtSurfaceNormalExpression;
        ff->DoPointNormals(false);
        return ff;
    }
    if (functionName == "zoneid")
    {
        avtDataIdFilter *ff = new avtDataIdFilter;
        ff->CreateZoneIds();
        ff->CreateLocalNumbering();
        return ff;
    }
    if (functionName == "global_zoneid")
    {
        avtDataIdFilter *ff = new avtDataIdFilter;
        ff->CreateZoneIds();
        ff->CreateGlobalNumbering();
        return ff;
    }
    if (functionName == "nodeid")
    {
        avtDataIdFilter *ff = new avtDataIdFilter;
        ff->CreateNodeIds();
        ff->CreateLocalNumbering();
        return ff;
    }
    if (functionName == "global_nodeid")
    {
        avtDataIdFilter *ff = new avtDataIdFilter;
        ff->CreateNodeIds();
        ff->CreateGlobalNumbering();
        return ff;
    }
    if (functionName == "biggest_neighbor")
    {
        avtNeighborEvaluatorFilter *ff = new avtNeighborEvaluatorFilter;
        ff->SetEvaluationType(avtNeighborEvaluatorFilter::BIGGEST_NEIGHBOR);
        return ff;
    }
    if (functionName == "smallest_neighbor")
    {
        avtNeighborEvaluatorFilter *ff = new avtNeighborEvaluatorFilter;
        ff->SetEvaluationType(avtNeighborEvaluatorFilter::SMALLEST_NEIGHBOR);
        return ff;
    }
    if (functionName == "neighbor_average")
    {
        avtNeighborEvaluatorFilter *ff = new avtNeighborEvaluatorFilter;
        ff->SetEvaluationType(avtNeighborEvaluatorFilter::AVERAGE_NEIGHBOR);
        return ff;
    }
    if (functionName == "cylindrical_radius")
    {
        avtExpressionComponentMacro *ecm = new avtExpressionComponentMacro;
        ecm->SetMacro("cylindrical", 0);
        return ecm;
    }
    if (functionName == "cylindrical_theta")
    {
        avtExpressionComponentMacro *ecm = new avtExpressionComponentMacro;
        ecm->SetMacro("cylindrical", 1);
        return ecm;
    }
    if (functionName == "polar_radius")
    {
        avtExpressionComponentMacro *ecm = new avtExpressionComponentMacro;
        ecm->SetMacro("polar", 0);
        return ecm;
    }
    if (functionName == "polar_theta")
    {
        avtExpressionComponentMacro *ecm = new avtExpressionComponentMacro;
        ecm->SetMacro("polar", 1);
        return ecm;
    }
    if (functionName == "polar_phi")
    {
        avtExpressionComponentMacro *ecm = new avtExpressionComponentMacro;
        ecm->SetMacro("polar", 2);
        return ecm;
    }
    if (functionName == "distance_to_best_fit_line")
        return new avtDistanceToBestFitLineFilter(true);
    if (functionName == "distance_to_best_fit_line2")
        return new avtDistanceToBestFitLineFilter(false);
    if (functionName == "min" || functionName == "minimum")
    {
        avtMinMaxExpression *mm = new avtMinMaxExpression;
        mm->SetDoMinimum(true);
        return mm;
    }
    if (functionName == "max" || functionName == "maximum")
    {
        avtMinMaxExpression *mm = new avtMinMaxExpression;
        mm->SetDoMinimum(false);
        return mm;
    }
    if (functionName == "color")
        return new avtColorComposeFilter(3);
    if (functionName == "color4")
        return new avtColorComposeFilter(4);

    return NULL;
}

// ****************************************************************************
// Method: avtFunctionExpr::CreateFilters
//
// Purpose:
//     Creates the avt filters that are necessary to complete the given
//     function.
//
// Programmer: Sean Ahern
//
// Modifications:
//      Hank Childs, Tue Mar 18 21:36:26 PST 2003
//      Added revolved surface area.
//
//      Sean Ahern, Wed Jun 11 13:44:52 PDT 2003
//      Added vector cross product.
//
//      Hank Childs, Thu Aug 21 09:54:51 PDT 2003
//      Added conditionals/comparisons.
//
//      Hank Childs, Fri Sep 19 16:18:37 PDT 2003
//      Added matrix operations.
//
//      Hank Childs, Wed Dec 10 11:01:07 PST 2003
//      Added recenter.
//
//      Jeremy Meredith, Wed Jun  9 09:16:25 PDT 2004
//      Added specmf.
//
//      Hank Childs, Sat Sep 18 08:54:51 PDT 2004
//      Added neighbor evaluation expressions.
//
//      Hank Childs, Thu Sep 23 09:17:26 PDT 2004
//      Added zone id, node id, and global variants.
//
//      Hank Childs, Sun Jan  2 15:27:00 PST 2005
//      Added macro expressions -- curl, divergence, laplacian, and materror.
//      Also added support functions for materror -- MIRvf and relative diff.
//
//      Hank Childs, Thu Jan 20 15:51:16 PST 2005
//      Added side volume, resrad.
//
//      Kathleen Bonnell, Thu Mar  3 11:13:22 PST 2005 
//      Added var_skew. 
//
//      Jeremy Meredith, Mon Jun 13 17:20:44 PDT 2005
//      This class now holds its name directly.  Use that instead.
//
//      Hank Childs, Thu Jun 30 15:55:06 PDT 2005
//      Added cylindrical, components of cylindrical and polar, mod, floor,
//      round, and ceil.
//
//      Hank Childs, Thu Jul 21 12:34:02 PDT 2005
//      Added array_compose, array_decompose.
//
//      Hank Childs, Tue Aug 16 09:05:03 PDT 2005
//      Added mean_filter, median_filter.
//
//      Hank Childs, Fri Aug 26 13:51:39 PDT 2005
//      Added conn_cmfe.
//
//      Hank Childs, Wed Sep 21 17:30:53 PDT 2005
//      Added external_node, surface normals, min_side_volume, max_side_volume,
//      min_edge_length, and max_edge_length.
//
//      Hank Childs, Mon Oct 10 17:08:05 PDT 2005
//      Added pos_cmfe.
//
//      Brad Whitlock, Fri Nov 18 15:59:27 PST 2005
//      Added distance_to_best_fit_line.
//
//      Hank Childs, Thu Jan  5 08:44:12 PST 2006
//      Remove redundant polar_phi entry that was added with cut-n-paste.
//
//      Hank Childs, Sat Jan 21 14:43:45 PST 2006
//      Added symm_eval_transform.
//
//      Hank Childs, Tue Feb 14 14:03:47 PST 2006
//      Added logical gradient.
//
//      Hank Childs, Sat Feb 18 10:24:30 PST 2006
//      Added apply_ddf.
//
//      Hank Childs, Sun Mar  5 16:01:34 PST 2006
//      Added time.
//
//      Hank Childs, Mon Mar 13 16:43:55 PST 2006
//      Added min and max expressions.
//
//      Hank Childs, Sat Apr 29 14:40:47 PDT 2006
//      Added localized compactness expression.
//
//      Hank Childs, Thu May 11 12:14:51 PDT 2006
//      Added curvature.
//
//      Hank Childs, Fri Aug 25 17:23:34 PDT 2006
//      Make the parsing more robust when the wrong number of arguments are
//      specified.
//
//      Hank Childs, Fri Oct  6 15:45:26 PDT 2006
//      Add inverse of Abel transform.
//
//      Kathleen Bonnell, Fri Sep 15 09:55:55 PDT 2006 
//      Added volume2.  (does a different hex-volume calculation).
//
//      Kathleen Bonnell, Fri Nov 17 08:32:54 PST 2006
//      Moved actual creation of filters to private CreateFilters method,
//      which does not have nested-if blocks, too many of which cause
//      a compile failure on Win32.
//
// ****************************************************************************
void
avtFunctionExpr::CreateFilters(ExprPipelineState *state)
{
    // Figure out which filter to add and add it.
    string functionName = name;
    avtExpressionFilter *f = CreateFilters(functionName);

    if (f == NULL)
    {
        string error =
            string("avtFunctionExpr::CreateFilters: "
                   "Unknown function:\"") + functionName +
                   string("\".");
        EXCEPTION1(ExpressionException, error);
    }

    // Have the filter process arguments as necessary.  It's important
    // that this happen before the next bit of code so that the pipeline
    // state is correct.
    f->ProcessArguments(args, state);

    // Ask the filter how many variable arguments it has.  Pop that number
    // of arguments off the stack and store them on a stack.
    int nvars = f->NumVariableArguments();
    string outputName = functionName + "(";
    vector<string> inputStack;
    int i;
    for (i = 0; i < nvars; i++)
    {
        string inputName;
        if (state->GetNumNames() > 0)
            inputName = state->PopName();
        else
        {
            EXCEPTION1(ExpressionException, "Parsing of your expression has "
                       "failed.  Failures of the type VisIt's parser has "
                       "encountered are often caused when an expression is "
                       "given less arguments than that expression expects.");
        }

        inputStack.push_back(inputName);
        outputName += inputName;
        if (i != 0)
            outputName += ",";
    }
    outputName += ")";

    // Take the stack of variable names and feed them to the function in
    // reverse order.
    int size = inputStack.size();
    for (i = 0; i < size; i++)
    {
        string inputName = inputStack.back();
        inputStack.pop_back();
        f->AddInputVariableName(inputName.c_str());
    }

    // Set the variable the function should output>
    state->PushName(outputName);
    f->SetOutputVariableName(outputName.c_str());
    
    // Keep track of the current input.
    f->SetInput(state->GetDataObject());
    state->SetDataObject(f->GetOutput());
    state->AddFilter(f);
}

void
avtVarExpr::CreateFilters(ExprPipelineState *state)
{
    state->PushName(var->GetFullpath());
}
