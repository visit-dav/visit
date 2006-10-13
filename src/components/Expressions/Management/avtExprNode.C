/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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
#include <avtFloorFilter.h>
#include <avtModuloFilter.h>
#include <avtRoundFilter.h>
#include <avtSinFilter.h>
#include <avtCosFilter.h>
#include <avtDistanceToBestFitLineFilter.h>
#include <avtRandomFilter.h>
#include <avtArctanFilter.h>
#include <avtArcsinFilter.h>
#include <avtArccosFilter.h>
#include <avtDegreeToRadianFilter.h>
#include <avtDeterminantFilter.h>
#include <avtEigenvalueFilter.h>
#include <avtEigenvectorFilter.h>
#include <avtInverseFilter.h>
#include <avtTraceFilter.h>
#include <avtTensorMaximumShearFilter.h>
#include <avtPrincipalDeviatoricTensorFilter.h>
#include <avtPrincipalTensorFilter.h>
#include <avtEffectiveTensorFilter.h>
#include <avtCurvatureExpression.h>
#include <avtGradientFilter.h>
#include <avtCurlFilter.h>
#include <avtDivergenceFilter.h>
#include <avtLaplacianFilter.h>
#include <avtResradFilter.h>
#include <avtMagnitudeFilter.h>
#include <avtNeighborFilter.h>
#include <avtNodeDegreeFilter.h>
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
#include <avtTimeExpression.h>
#include <avtMinMaxExpression.h>

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
// ****************************************************************************
void
avtFunctionExpr::CreateFilters(ExprPipelineState *state)
{
    // Figure out which filter to add and add it.
    string functionName = name;
    avtExpressionFilter *f = NULL;

    if (functionName == "sin")
        f = new avtSinFilter();
    else if (functionName == "cos")
        f = new avtCosFilter();
    else if (functionName == "tan")
        f = new avtTanFilter();
    else if (functionName == "atan")
        f = new avtArctanFilter();
    else if (functionName == "asin")
        f = new avtArcsinFilter();
    else if (functionName == "acos")
        f = new avtArccosFilter();
    else if (functionName == "deg2rad")
        f = new avtDegreeToRadianFilter();
    else if (functionName == "rad2deg")
        f = new avtRadianToDegreeFilter();
    else if (functionName == "abs")
        f = new avtAbsValFilter();
    else if (functionName == "ln")
        f = new avtNaturalLogFilter();
    else if ((functionName == "log") || (functionName == "log10"))
        f = new avtBase10LogFilter();
    else if (functionName == "sqrt")
        f = new avtSquareRootFilter();
    else if ((functionName == "sq") || (functionName == "sqr"))
        f = new avtSquareFilter();
    else if (functionName == "mod")
        f = new avtModuloFilter();
    else if (functionName == "ceil")
        f = new avtCeilingFilter();
    else if (functionName == "floor")
        f = new avtFloorFilter();
    else if (functionName == "round")
        f = new avtRoundFilter();
    else if ((functionName == "random") || (functionName == "rand"))
        f = new avtRandomFilter();
    else if (functionName == "cross")
        f = new avtVectorCrossProductFilter();
    else if (functionName == "dot")
        f = new avtBinaryMultiplyFilter();
    else if (functionName == "array_compose")
        f = new avtArrayComposeFilter();
    else if (functionName == "array_decompose")
        f = new avtArrayDecomposeFilter();
    else if (functionName == "matvf")
        f = new avtMatvfFilter();
    else if (functionName == "mirvf")
        f = new avtMIRvfFilter();
    else if (functionName == "materror")
        f = new avtMatErrorFilter();
    else if (functionName == "specmf")
        f = new avtSpecMFFilter();
    else if (functionName == "nmats")
        f = new avtNMatsFilter();
    else if (functionName == "localized_compactness")
        f = new avtLocalizedCompactnessExpression();
    else if (functionName == "recenter")
        f = new avtRecenterFilter();
    else if (functionName == "det" || functionName == "determinant")
        f = new avtDeterminantFilter();
    else if (functionName == "eigenvalue")
        f = new avtEigenvalueFilter();
    else if (functionName == "eigenvector")
        f = new avtEigenvectorFilter();
    else if (functionName == "inverse")
        f = new avtInverseFilter();
    else if (functionName == "trace")
        f = new avtTraceFilter();
    else if (functionName == "effective_tensor")
        f = new avtEffectiveTensorFilter();
    else if (functionName == "tensor_maximum_shear")
        f = new avtTensorMaximumShearFilter();
    else if (functionName == "principal_tensor")
        f = new avtPrincipalTensorFilter();
    else if (functionName == "principal_deviatoric_tensor")
        f = new avtPrincipalDeviatoricTensorFilter();
    else if (functionName == "degree")
        f = new avtDegreeFilter();
    else if (functionName == "cylindrical")
        f = new avtCylindricalCoordinatesFilter();
    else if (functionName == "polar")
        f = new avtPolarCoordinatesFilter();
    else if (functionName == "coord" || functionName == "coords")
        f = new avtMeshCoordinateFilter();
    else if (functionName == "procid")
        f = new avtProcessorIdFilter();
    else if (functionName == "mean_curvature")
    {
        avtCurvatureExpression *c = new avtCurvatureExpression;
        c->DoGaussCurvature(false);
        f = c;
    }
    else if (functionName == "gauss_curvature")
    {
        avtCurvatureExpression *c = new avtCurvatureExpression;
        c->DoGaussCurvature(true);
        f = c;
    }
    else if (functionName == "ijk_gradient" || functionName == "ij_gradient")
    {
        avtGradientFilter *g = new avtGradientFilter();
        g->SetDoLogicalGradient(true);
        f = g;
    }
    else if (functionName == "gradient")
        f = new avtGradientFilter();
    else if (functionName == "curl")
        f = new avtCurlFilter();
    else if (functionName == "divergence")
        f = new avtDivergenceFilter();
    else if (functionName == "laplacian" || functionName == "Laplacian")
        f = new avtLaplacianFilter();
    else if (functionName == "resrad")
        f = new avtResradFilter();
    else if (functionName == "magnitude")
        f = new avtMagnitudeFilter();
    else if (functionName == "relative_difference")
        f = new avtRelativeDifferenceFilter();
    else if (functionName == "var_skew")
        f = new avtVariableSkewFilter();
    else if (functionName == "apply_ddf")
        f = new avtApplyDDFExpression();
    else if (functionName == "if")
        f = new avtConditionalFilter();
    else if (functionName == "and")
        f = new avtLogicalAndFilter();
    else if (functionName == "or")
        f = new avtLogicalOrFilter();
    else if (functionName == "not")
        f = new avtLogicalNegationFilter();
    else if (functionName == "le" || functionName == "lte")
        f = new avtTestLessThanOrEqualToFilter();
    else if (functionName == "ge" || functionName == "gte")
        f = new avtTestGreaterThanOrEqualToFilter();
    else if (functionName == "lt")
        f = new avtTestLessThanFilter();
    else if (functionName == "gt")
        f = new avtTestGreaterThanFilter();
    else if (functionName == "eq" || functionName == "equal" || 
             functionName == "equals")
        f = new avtTestEqualToFilter();
    else if (functionName == "ne" || functionName == "neq" ||
             functionName == "notequal" || functionName == "notequals")
        f = new avtTestNotEqualToFilter();
    else if (functionName == "neighbor")
        f = new avtNeighborFilter();
    else if (functionName == "external_node")
        f = new avtExternalNodeExpression();
    else if (functionName == "node_degree")
        f = new avtNodeDegreeFilter();
    // Begin Verdict Metrics
    else if (functionName == "area")
        f = new avtVMetricArea();
    else if (functionName == "aspect")
        f = new avtVMetricAspectRatio();
    else if (functionName == "skew")
        f = new avtVMetricSkew();
    else if (functionName == "taper")
        f = new avtVMetricTaper();
    else if (functionName == "volume")
        f = new avtVMetricVolume();
    else if (functionName == "volume2")
    {
        avtVMetricVolume *vol = new avtVMetricVolume();
        vol->UseVerdictHex(false);
        f = vol;
    }
    else if (functionName == "min_edge_length")
    {
        avtEdgeLength *el = new avtEdgeLength();
        el->SetTakeMin(true);
        f = el;
    }
    else if (functionName == "max_edge_length")
    {
        avtEdgeLength *el = new avtEdgeLength();
        el->SetTakeMin(false);
        f = el;
    }
    else if (functionName == "min_side_volume")
    {
        avtSideVolume *sv = new avtSideVolume();
        sv->SetTakeMin(true);
        f = sv;
    }
    else if (functionName == "max_side_volume")
    {
        avtSideVolume *sv = new avtSideVolume();
        sv->SetTakeMin(false);
        f = sv;
    }
    else if (functionName == "stretch")
        f = new avtVMetricStretch();
    else if (functionName == "diagonal")
        f = new avtVMetricDiagonal();
    else if (functionName == "dimension")
        f = new avtVMetricDimension();
    else if (functionName == "oddy")
        f = new avtVMetricOddy();
    else if (functionName == "condition")
        f = new avtVMetricCondition();
    else if (functionName == "jacobian")
        f = new avtVMetricJacobian();
    else if (functionName == "scaled_jacobian")
        f = new avtVMetricScaledJacobian();
    else if (functionName == "shear")
        f = new avtVMetricShear();
    else if (functionName == "shape")
        f = new avtVMetricShape();
    else if (functionName == "relative_size")
        f = new avtVMetricRelativeSize();
    else if (functionName == "shape_and_size")
        f = new avtVMetricShapeAndSize();
    else if (functionName == "aspect_gamma")
        f = new avtVMetricAspectGamma();
    else if (functionName == "warpage")
        f = new avtVMetricWarpage();
    else if (functionName == "largest_angle")
        f = new avtVMetricLargestAngle();
    else if (functionName == "smallest_angle")
        f = new avtVMetricSmallestAngle();
    else if (functionName == "revolved_volume")
        f = new avtRevolvedVolume;
    else if (functionName == "revolved_surface_area")
        f = new avtRevolvedSurfaceArea;
    else if (functionName == "conservative_smoothing")
        f = new avtConservativeSmoothingExpression;
    else if (functionName == "mean_filter")
        f = new avtMeanFilterExpression;
    else if (functionName == "median_filter")
        f = new avtMedianFilterExpression;
    else if (functionName == "abel_inversion")
        f = new avtAbelInversionExpression;
    else if (functionName == "conn_cmfe")
        f = new avtConnCMFEExpression;
    else if (functionName == "pos_cmfe")
        f = new avtPosCMFEExpression;
    else if (functionName == "eval_transform")
        f = new avtEvalTransformExpression;
    else if (functionName == "symm_transform")
        f = new avtSymmTransformExpression;
    else if (functionName == "eval_plane")
        f = new avtEvalPlaneExpression;
    else if (functionName == "symm_plane")
        f = new avtSymmPlaneExpression;
    else if (functionName == "time")
        f = new avtTimeExpression;
    else if (functionName == "surface_normal" || 
             functionName == "point_surface_normal")
    {
        avtSurfaceNormalExpression *ff = new avtSurfaceNormalExpression;
        ff->DoPointNormals(true);
        f = ff;
    }
    else if (functionName == "cell_surface_normal")
    {
        avtSurfaceNormalExpression *ff = new avtSurfaceNormalExpression;
        ff->DoPointNormals(false);
        f = ff;
    }
    else if (functionName == "zoneid")
    {
        avtDataIdFilter *ff = new avtDataIdFilter;
        ff->CreateZoneIds();
        ff->CreateLocalNumbering();
        f = ff;
    }
    else if (functionName == "global_zoneid")
    {
        avtDataIdFilter *ff = new avtDataIdFilter;
        ff->CreateZoneIds();
        ff->CreateGlobalNumbering();
        f = ff;
    }
    else if (functionName == "nodeid")
    {
        avtDataIdFilter *ff = new avtDataIdFilter;
        ff->CreateNodeIds();
        ff->CreateLocalNumbering();
        f = ff;
    }
    else if (functionName == "global_nodeid")
    {
        avtDataIdFilter *ff = new avtDataIdFilter;
        ff->CreateNodeIds();
        ff->CreateGlobalNumbering();
        f = ff;
    }
    else if (functionName == "biggest_neighbor")
    {
        avtNeighborEvaluatorFilter *ff = new avtNeighborEvaluatorFilter;
        ff->SetEvaluationType(avtNeighborEvaluatorFilter::BIGGEST_NEIGHBOR);
        f = ff;
    }
    else if (functionName == "smallest_neighbor")
    {
        avtNeighborEvaluatorFilter *ff = new avtNeighborEvaluatorFilter;
        ff->SetEvaluationType(avtNeighborEvaluatorFilter::SMALLEST_NEIGHBOR);
        f = ff;
    }
    else if (functionName == "neighbor_average")
    {
        avtNeighborEvaluatorFilter *ff = new avtNeighborEvaluatorFilter;
        ff->SetEvaluationType(avtNeighborEvaluatorFilter::AVERAGE_NEIGHBOR);
        f = ff;
    }
    else if (functionName == "cylindrical_radius")
    {
        avtExpressionComponentMacro *ecm = new avtExpressionComponentMacro;
        ecm->SetMacro("cylindrical", 0);
        f = ecm;
    }
    else if (functionName == "cylindrical_theta")
    {
        avtExpressionComponentMacro *ecm = new avtExpressionComponentMacro;
        ecm->SetMacro("cylindrical", 1);
        f = ecm;
    }
    else if (functionName == "polar_radius")
    {
        avtExpressionComponentMacro *ecm = new avtExpressionComponentMacro;
        ecm->SetMacro("polar", 0);
        f = ecm;
    }
    else if (functionName == "polar_theta")
    {
        avtExpressionComponentMacro *ecm = new avtExpressionComponentMacro;
        ecm->SetMacro("polar", 1);
        f = ecm;
    }
    else if (functionName == "polar_phi")
    {
        avtExpressionComponentMacro *ecm = new avtExpressionComponentMacro;
        ecm->SetMacro("polar", 2);
        f = ecm;
    }
    else if (functionName == "distance_to_best_fit_line")
        f = new avtDistanceToBestFitLineFilter(true);
    else if (functionName == "distance_to_best_fit_line2")
        f = new avtDistanceToBestFitLineFilter(false);
    else if (functionName == "min" || functionName == "minimum")
    {
        avtMinMaxExpression *mm = new avtMinMaxExpression;
        mm->SetDoMinimum(true);
        f = mm;
    }
    else if (functionName == "max" || functionName == "maximum")
    {
        avtMinMaxExpression *mm = new avtMinMaxExpression;
        mm->SetDoMinimum(false);
        f = mm;
    }
    else
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
