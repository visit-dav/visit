// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                    avtConstantFunctionExpression.C                        //
// ************************************************************************* //

#include <avtConstantFunctionExpression.h>

#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <ExpressionException.h>
#include <avtExprNode.h>
#include <ExprPipelineState.h>

#include <string>

// ****************************************************************************
//  Method: avtConstantFunctionExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Jeremy Meredith
//  Creation:   February 19, 2008
//
//  Modifications:
//    Jeremy Meredith, Wed Feb 20 10:00:31 EST 2008
//    Support either nodal or zonal values.
//
//    Alister Maguire, Fri Oct  9 11:04:05 PDT 2020
//    Setting canApplyToDirectDatabaseQOT to false.
//
// ****************************************************************************

avtConstantFunctionExpression::avtConstantFunctionExpression(bool n)
{
    nodal = n;
    value = 0;
    canApplyToDirectDatabaseQOT = false;
}


// ****************************************************************************
//  Method: avtConstantFunctionExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Jeremy Meredith
//  Creation:   February 19, 2008
//
// ****************************************************************************

avtConstantFunctionExpression::~avtConstantFunctionExpression()
{
}


// ****************************************************************************
//  Method: avtConstantFunctionExpression::DeriveVariable
//
//  Purpose:
//      Assigns a constant value to the variable.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Jeremy Meredith
//  Creation:     February 19, 2008
//
//  Modifications:
//    Jeremy Meredith, Wed Feb 20 10:00:31 EST 2008
//    Support either nodal or zonal values.
//
// ****************************************************************************

vtkDataArray *
avtConstantFunctionExpression::DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex)
{
    vtkIdType nvals = nodal ? in_ds->GetNumberOfPoints() : 
                      in_ds->GetNumberOfCells();

    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetNumberOfTuples(nvals);
    for (vtkIdType i = 0 ; i < nvals ; i++)
    {
        rv->SetTuple1(i, value);
    }

    return rv;
}

// ****************************************************************************
//  Method:  avtConstantFunctionExpression::ProcessArguments
//
//  Purpose:
//    Process the arguments for this function.
//
//  Arguments:
//    args       the arguments to the function
//    state      the pipeline state
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 19, 2008
//
//  Modifications:
//    Jeremy Meredith, Tue Feb 19 16:04:11 EST 2008
//    By having this function specify it only had one argument, it was
//    possible to clobber other constant creations.  I upped it to 2 and
//    had the second arg push a textual representation on the state.
//
// ****************************************************************************
void
avtConstantFunctionExpression::ProcessArguments(ArgsExpr *args,
                                                ExprPipelineState *state)
{
    if (!args || args->GetArgs()->size() != 2)
    {
        EXCEPTION2(ExpressionException, outputVariableName,
                   "the function expected two arguments");
    }

    std::vector<ArgExpr*> *arguments = args->GetArgs();
    ArgExpr *arg0 = (*arguments)[0];
    ArgExpr *arg1 = (*arguments)[1];
    ExprParseTreeNode *node0 = arg0->GetExpr();
    ExprParseTreeNode *node1 = arg1->GetExpr();
    avtExprNode *expr0 = dynamic_cast<avtExprNode*>(node0);
    avtExprNode *expr1 = dynamic_cast<avtExprNode*>(node1);

    if (!expr0)
    {
        EXCEPTION2(ExpressionException, outputVariableName,
                   "the first argument was not an expression");
    }
    if (!expr1 ||
        (expr1->GetTypeName() != "IntegerConst" &&
         expr1->GetTypeName() != "FloatConst"))
    {
        EXCEPTION2(ExpressionException, outputVariableName,
                   "the second argument was not an int or float constant");
    }

    expr0->CreateFilters(state);

    if (expr1->GetTypeName() == "IntegerConst")
        value = dynamic_cast<avtIntegerConstExpr*>(expr1)->GetValue();
    else
        value = dynamic_cast<avtFloatConstExpr*>(expr1)->GetValue();

    char strrep[30];
    snprintf(strrep, 30, "'%e'", value);
    state->PushName(std::string(strrep));    
}

