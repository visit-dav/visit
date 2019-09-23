// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                    avtArraySumExpression.C                          //
// ************************************************************************* //

#include <avtArraySumExpression.h>

#include <math.h>

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>

#include <ExprToken.h>
#include <avtExprNode.h>


#include <DebugStream.h>
#include <ExpressionException.h>
#include <ImproperUseException.h>

#include <string>
#include <vector>

// ****************************************************************************
//  Method: avtArraySumExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   July 21, 2005
//
// ****************************************************************************

avtArraySumExpression::avtArraySumExpression()
{
    issuedWarning = false;
    canApplyToDirectDatabaseQOT = true;
}


// ****************************************************************************
//  Method: avtArraySumExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   July 21, 2005
//
// ****************************************************************************

avtArraySumExpression::~avtArraySumExpression()
{
}


// ****************************************************************************
//  Method: avtArraySumExpression::DeriveVariable
//
//  Purpose:
//      Creates an array.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Hank Childs
//  Creation:     July 21, 2005
//
//  Modifications:
//
//    Hank Childs, Fri Jun  9 14:22:43 PDT 2006
//    Remove unused variable.
//
// ****************************************************************************

vtkDataArray *
avtArraySumExpression::DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex)
{
    if (activeVariable == NULL)
        EXCEPTION2(ExpressionException, outputVariableName, 
                   "Asked to sum an array, but did "
                   "specify which variable to sum");

    vtkDataArray *data = in_ds->GetPointData()->GetArray(activeVariable);
    if (data == NULL)
        data = in_ds->GetCellData()->GetArray(activeVariable);

    if (data == NULL)
        EXCEPTION2(ExpressionException, outputVariableName,
                   "Unable to locate variable to sum");

    vtkDataArray *rv = data->NewInstance();
    vtkIdType nvals = data->GetNumberOfTuples();
    rv->SetNumberOfTuples(nvals);
    for (vtkIdType i = 0 ; i < nvals ; ++i)
    {
        double val = 0.0;
        for (vtkIdType index = 0; index < data->GetNumberOfComponents(); ++index)
            val += data->GetComponent(i, index);
        rv->SetTuple1(i, val);
    }

    return rv;
}


// ****************************************************************************
//  Method: avtArraySumExpression::ProcessArguments
//
//  Purpose:
//      Tells the first argument to go generate itself.  Parses the second
//      argument into a list of material names.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Hank Childs
//  Creation:     July 21, 2005
//
// ****************************************************************************

void
avtArraySumExpression::ProcessArguments(ArgsExpr *args, 
                                          ExprPipelineState *state)
{
    // Check the number of arguments
    std::vector<ArgExpr*> *arguments = args->GetArgs();
    size_t nargs = arguments->size();
    if (nargs != 1)
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                        "this expression must be specified with exactly one "
                        "arguments.  Usage: array_sum(array)");
    }

    // Tell the first argument to create its filters.
    ArgExpr *firstarg = (*arguments)[0];
    avtExprNode *firstTree = dynamic_cast<avtExprNode*>(firstarg->GetExpr());
    firstTree->CreateFilters(state);
}


// ****************************************************************************
//  Method: avtArraySumExpression::PreExecute
//
//  Purpose:
//      Called before execution.  This sets the issuedWarning flag to false.
//
//  Programmer: Hank Childs
//  Creation:   July 21, 2005
//
//  Modifications:
//    Jeremy Meredith, Thu Feb 15 12:02:51 EST 2007
//    Call inherited PreExecute before everything else.
//
// ****************************************************************************

void
avtArraySumExpression::PreExecute(void)
{
    avtSingleInputExpressionFilter::PreExecute();
    issuedWarning = false;
}


