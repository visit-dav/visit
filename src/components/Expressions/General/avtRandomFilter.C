// ************************************************************************* //
//                              avtRandomFilter.C                            //
// ************************************************************************* //

#include <Token.h>
#include <EngineExprNode.h>
#include <avtRandomFilter.h>
#include <DebugStream.h>
#include <ExpressionException.h>

#include <math.h>

#include <vtkDataSet.h>
#include <vtkFloatArray.h>


// ****************************************************************************
//  Method: avtRandomFilter::DeriveVariable
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
// ****************************************************************************

vtkDataArray *
avtRandomFilter::DeriveVariable(vtkDataSet *in_ds)
{
    int npts   = in_ds->GetNumberOfPoints();

    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetNumberOfTuples(npts);
    for (int i = 0 ; i < npts ; i++)
    {
        rv->SetTuple1(i, (rand() % 1024) / 1024.);
    }

    return rv;
}

// ****************************************************************************
//  Method: avtRandomFilter::ProcessArguments
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
// ****************************************************************************
void
avtRandomFilter::ProcessArguments(ArgsExpr *args, ExprPipelineState *state)
{
    // Check the number of arguments
    std::vector<ArgExpr*> *arguments = args->GetArgs();
    int nargs = arguments->size();
    if (nargs == 0)
    {
        EXCEPTION1(ExpressionException, "avtRandomFilter: No arguments given.");
    }

    // Tell the first argument to create its filters.
    ArgExpr *firstarg = (*arguments)[0];
    EngineExprNode *firstTree = dynamic_cast<EngineExprNode*>(firstarg->GetExpr());
    firstTree->CreateFilters(state);

    // Check if there's a second argument.
    if (nargs == 1)
    {
        debug5 << "avtRandomFilter: No second argument." << endl;
        return;
    }

    // Pull off the second argument and make sure it's a constant.
    ArgExpr *secondarg = (*arguments)[1];
    EngineExprNode *secondTree = dynamic_cast<EngineExprNode*>(secondarg->GetExpr());
    if (secondTree->GetTypeName() != "Const")
    {
        debug5 << "avtRandomFilter: Second argument is not a constant: "
               << secondTree->GetTypeName() << endl;
        EXCEPTION1(ExpressionException, "avtRandomFilter: Second argument is not a constant.");
    }
    EngineConstExpr *con = dynamic_cast<EngineConstExpr*>(secondTree);

    // Now check that it's an int.
    Token *t = con->GetToken();
    if (t->GetType() != TT_IntegerConst)
    {
        debug5 << "avtRandomFilter: Second argument is not an integer: "
               << GetTokenTypeString(t->GetType()) << endl;
        EXCEPTION1(ExpressionException, "avtRandomFilter: Second argument is not an integer.");
    }

    int val = dynamic_cast<IntegerConst*>(t)->GetValue();
    debug5 << "avtRandomFilter: Setting random seed to: " << val << endl;
    srand(val);

    // See if there are other arguments.
    if (nargs > 2)
    {
        EXCEPTION1(ExpressionException, "avtRandomFilter: Too many arguments.");
    }
}
