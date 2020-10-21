// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtRandomExpression.C                            //
// ************************************************************************* //

#include <ExprToken.h>
#include <avtExprNode.h>
#include <avtRandomExpression.h>
#include <DebugStream.h>
#include <ExpressionException.h>

#include <math.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>


// ****************************************************************************
//  Method: avtRandomExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
//  Modifications:
//
//    Alister Maguire, Fri Oct  9 11:46:22 PDT 2020
//    Set canApplyToDirectDatabaseQOT to false.
//
// ****************************************************************************

avtRandomExpression::avtRandomExpression()
{
    canApplyToDirectDatabaseQOT = false;
}


// ****************************************************************************
//  Method: avtRandomExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtRandomExpression::~avtRandomExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtRandomExpression::DeriveVariable
//
//  Purpose:
//      Assigns a random number to each point.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Hank Childs
//  Creation:     March 7, 2003
//
//  Modifications:
//
//    Hank Childs, Thu Aug 24 11:24:41 PDT 2006
//    Initialize the random seed so that we can have reproducible results.
//
// ****************************************************************************

vtkDataArray *
avtRandomExpression::DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex)
{
    srand(currentDomainsIndex);

    vtkIdType npts   = in_ds->GetNumberOfPoints();

    vtkDataArray *rv = CreateArrayFromMesh(in_ds);
    rv->SetNumberOfTuples(npts);
    for (vtkIdType i = 0 ; i < npts ; i++)
    {
        rv->SetTuple1(i, (rand() % 1024) / 1024.);
    }

    return rv;
}

// ****************************************************************************
//  Method: avtRandomExpression::ProcessArguments
//
//  Purpose:
//      Tells the first argument to go generate itself.  Uses the second
//      argument (if there) as a seed value for rand().
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Sean Ahern
//  Creation:     Sat Mar  8 00:18:28 America/Los_Angeles 2003
//
//  Modifications:
//    Jeremy Meredith, Wed Nov 24 12:26:21 PST 2004
//    Renamed EngineExprNode to avtExprNode due to a refactoring.
//    Also renamed Token to ExprToken for the same reason.
//
//    Jeremy Meredith, Mon Jun 13 11:42:38 PDT 2005
//    Changed the way constant expressions work.
//
// ****************************************************************************
void
avtRandomExpression::ProcessArguments(ArgsExpr *args, ExprPipelineState *state)
{
    // Check the number of arguments
    std::vector<ArgExpr*> *arguments = args->GetArgs();
    size_t nargs = arguments->size();
    if (nargs == 0)
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                   "avtRandomExpression: No arguments given.");
    }

    // Tell the first argument to create its filters.
    ArgExpr *firstarg = (*arguments)[0];
    avtExprNode *firstTree = dynamic_cast<avtExprNode*>(firstarg->GetExpr());
    firstTree->CreateFilters(state);

    // Check if there's a second argument.
    if (nargs == 1)
    {
        debug5 << "avtRandomExpression: No second argument." << endl;
        return;
    }

    // Pull off the second argument and make sure it's a constant.
    ArgExpr *secondarg = (*arguments)[1];
    avtExprNode *secondTree = dynamic_cast<avtExprNode*>(secondarg->GetExpr());
    if (secondTree->GetTypeName() != "IntegerConst")
    {
        debug5 << "avtRandomExpression: Second argument is not a constant: "
               << secondTree->GetTypeName().c_str() << endl;
        EXCEPTION2(ExpressionException, outputVariableName, 
                   "avtRandomExpression: Second argument is not an integer constant.");
    }

    // Get its value
    int val = dynamic_cast<IntegerConstExpr*>(secondTree)->GetValue();
    debug5 << "avtRandomExpression: Setting random seed to: " << val << endl;
    srand(val);

    // See if there are other arguments.
    if (nargs > 2)
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                   "avtRandomExpression: Too many arguments.");
    }
}
