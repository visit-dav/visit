// ************************************************************************* //
//                              avtRandomFilter.C                            //
// ************************************************************************* //

#include <ExprToken.h>
#include <avtExprNode.h>
#include <avtRandomFilter.h>
#include <DebugStream.h>
#include <ExpressionException.h>

#include <math.h>

#include <vtkDataSet.h>
#include <vtkFloatArray.h>


// ****************************************************************************
//  Method: avtRandomFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtRandomFilter::avtRandomFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtRandomFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtRandomFilter::~avtRandomFilter()
{
    ;
}


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
    avtExprNode *firstTree = dynamic_cast<avtExprNode*>(firstarg->GetExpr());
    firstTree->CreateFilters(state);

    // Check if there's a second argument.
    if (nargs == 1)
    {
        debug5 << "avtRandomFilter: No second argument." << endl;
        return;
    }

    // Pull off the second argument and make sure it's a constant.
    ArgExpr *secondarg = (*arguments)[1];
    avtExprNode *secondTree = dynamic_cast<avtExprNode*>(secondarg->GetExpr());
    if (secondTree->GetTypeName() != "IntegerConst")
    {
        debug5 << "avtRandomFilter: Second argument is not a constant: "
               << secondTree->GetTypeName().c_str() << endl;
        EXCEPTION1(ExpressionException, "avtRandomFilter: Second argument is "
                   "not an integer constant.");
    }

    // Get its value
    int val = dynamic_cast<IntegerConstExpr*>(secondTree)->GetValue();
    debug5 << "avtRandomFilter: Setting random seed to: " << val << endl;
    srand(val);

    // See if there are other arguments.
    if (nargs > 2)
    {
        EXCEPTION1(ExpressionException, "avtRandomFilter: Too many arguments.");
    }
}
