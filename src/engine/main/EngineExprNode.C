// ************************************************************************* //
//                              EngineExprNode.C                             //
// ************************************************************************* //

#include <avtUnaryMinusFilter.h>
#include <avtBinaryAddFilter.h>
#include <avtBinarySubtractFilter.h>
#include <avtBinaryMultiplyFilter.h>
#include <avtBinaryDivideFilter.h>
#include <avtBinaryPowerFilter.h>
#include <avtSinFilter.h>
#include <avtCosFilter.h>
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
#include <avtGradientFilter.h>
#include <avtMagnitudeFilter.h>
#include <avtNeighborFilter.h>
#include <avtNodeDegreeFilter.h>
#include <avtMatvfFilter.h>
#include <avtNMatsFilter.h>
#include <avtRadianToDegreeFilter.h>
#include <avtRevolvedVolume.h>
#include <avtRevolvedSurfaceArea.h>
#include <avtSpecMFFilter.h>
#include <avtTanFilter.h>
#include <avtAbsValFilter.h>
#include <avtNaturalLogFilter.h>
#include <avtBase10LogFilter.h>
#include <avtSquareRootFilter.h>
#include <avtSquareFilter.h>
#include <avtPolarCoordinatesFilter.h>
#include <avtVectorComposeFilter.h>
#include <avtVectorDecomposeFilter.h>
#include <avtVectorCrossProductFilter.h>
#include <avtMeshCoordinateFilter.h>
#include <avtProcessorIdFilter.h>
#include <avtDegreeFilter.h>
#include <avtVMetrics.h>
#include <avtRecenterFilter.h>
#include <avtConstantCreatorFilter.h>
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

#include <stdio.h>
#include <ExpressionException.h>
#include <Token.h>
#include <ExprPipelineState.h>
#include <EngineExprNode.h>
#include <DebugStream.h>
#include <snprintf.h>

using std::string;

void
EngineConstExpr::CreateFilters(ExprPipelineState *state)
{
    // First, check if this is a constant that we can handle.  Only numbers
    // are supported right now.
    double value;
    switch(token->GetType())
    {
    case TT_IntegerConst:
        value = dynamic_cast<IntegerConst*>(token)->GetValue();
        break;
    case TT_FloatConst:
        value = dynamic_cast<FloatConst*>(token)->GetValue();
        break;
    default:
        string error =
            string("EngineConstExpr::CreateFilters: "
                   "Unsupported constant type: ") +
                   GetTokenTypeString(token->GetType());

        EXCEPTION1(ExpressionException, error);
    }

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

void
EngineUnaryExpr::CreateFilters(ExprPipelineState *state)
{
    dynamic_cast<EngineExprNode*>(expr)->CreateFilters(state);

    avtSingleInputExpressionFilter *f = NULL;
    if (op == '-')
        f = new avtUnaryMinusFilter();
    else
    {
        string error =
            string("EngineUnaryExpr::CreateFilters: "
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
EngineBinaryExpr::CreateFilters(ExprPipelineState *state)
{
    dynamic_cast<EngineExprNode*>(left)->CreateFilters(state);
    dynamic_cast<EngineExprNode*>(right)->CreateFilters(state);

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
            string("EngineBinaryExpr::CreateFilters: "
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
EngineIndexExpr::CreateFilters(ExprPipelineState *state)
{
    dynamic_cast<EngineExprNode*>(expr)->CreateFilters(state);

    //avtVectorDecomposeFilter *f = new avtVectorDecomposeFilter(ind);
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
//  Method:  EngineVectorExpr::CreateFilters
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
EngineVectorExpr::CreateFilters(ExprPipelineState *state)
{
    dynamic_cast<EngineExprNode*>(x)->CreateFilters(state);
    dynamic_cast<EngineExprNode*>(y)->CreateFilters(state);
    if (z)
        dynamic_cast<EngineExprNode*>(z)->CreateFilters(state);

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
// Method: EngineFunctionExpr::CreateFilters
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
// ****************************************************************************
void
EngineFunctionExpr::CreateFilters(ExprPipelineState *state)
{
    // Figure out which filter to add and add it.
    string functionName = name->GetVal();
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
    else if ((functionName == "random") || (functionName == "rand"))
        f = new avtRandomFilter();
    else if (functionName == "cross")
        f = new avtVectorCrossProductFilter();
    else if (functionName == "dot")
        f = new avtBinaryMultiplyFilter();
    else if (functionName == "matvf")
        f = new avtMatvfFilter();
    else if (functionName == "specmf")
        f = new avtSpecMFFilter();
    else if (functionName == "nmats")
        f = new avtNMatsFilter();
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
    else if (functionName == "polar")
        f = new avtPolarCoordinatesFilter();
    else if (functionName == "coord" || functionName == "coords")
        f = new avtMeshCoordinateFilter();
    else if (functionName == "procid")
        f = new avtProcessorIdFilter();
    else if (functionName == "gradient")
        f = new avtGradientFilter();
    else if (functionName == "magnitude")
        f = new avtMagnitudeFilter();
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
    else
    {
        string error =
            string("EngineFunctionExpr::CreateFilters: "
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
        string inputName = state->PopName();
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
EngineVarExpr::CreateFilters(ExprPipelineState *state)
{
    state->PushName(var->GetFullpath());
}
