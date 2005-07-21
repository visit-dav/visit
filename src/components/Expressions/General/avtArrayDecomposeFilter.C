// ************************************************************************* //
//                          avtArrayDecomposeFilter.C                          //
// ************************************************************************* //

#include <avtArrayDecomposeFilter.h>

#include <math.h>

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkUnsignedIntArray.h>

#include <ExprToken.h>
#include <avtExprNode.h>

#include <avtCallback.h>
#include <avtMaterial.h>
#include <avtMetaData.h>
#include <avtSpecies.h>

#include <DebugStream.h>
#include <ExpressionException.h>
#include <ImproperUseException.h>


// ****************************************************************************
//  Method: avtArrayDecomposeFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   July 21, 2005
//
// ****************************************************************************

avtArrayDecomposeFilter::avtArrayDecomposeFilter()
{
    issuedWarning = false;
    index = -1;
}


// ****************************************************************************
//  Method: avtArrayDecomposeFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   July 21, 2005
//
// ****************************************************************************

avtArrayDecomposeFilter::~avtArrayDecomposeFilter()
{
}


// ****************************************************************************
//  Method: avtArrayDecomposeFilter::DeriveVariable
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
// ****************************************************************************

vtkDataArray *
avtArrayDecomposeFilter::DeriveVariable(vtkDataSet *in_ds)
{
    int    i, j;

    if (activeVariable == NULL)
        EXCEPTION1(ExpressionException, "Asked to decompose an array, but did "
                   "specify which variable to decompose");

    vtkDataArray *data = in_ds->GetPointData()->GetArray(activeVariable);
    if (data == NULL)
        data = in_ds->GetCellData()->GetArray(activeVariable);

    if (data == NULL)
        EXCEPTION1(ExpressionException, "Unable to locate variable to "
                                        "decompose");

    if (index < 0 || index >= data->GetNumberOfComponents())
        EXCEPTION1(ExpressionException, "Index into array is not valid.");

    vtkFloatArray *rv = vtkFloatArray::New();
    int nvals = data->GetNumberOfTuples();
    rv->SetNumberOfTuples(nvals);
    for (i = 0 ; i < nvals ; i++)
        rv->SetTuple1(i, data->GetComponent(i, index));

    return rv;
}


// ****************************************************************************
//  Method: avtArrayDecomposeFilter::ProcessArguments
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
avtArrayDecomposeFilter::ProcessArguments(ArgsExpr *args, 
                                          ExprPipelineState *state)
{
    // Check the number of arguments
    std::vector<ArgExpr*> *arguments = args->GetArgs();
    int nargs = arguments->size();
    if (nargs != 2)
    {
        EXCEPTION1(ExpressionException, 
                        "this expression must be specified with exactly two "
                        "arguments.  Usage: array_decompose(array, #)");
    }

    // Tell the first argument to create its filters.
    ArgExpr *firstarg = (*arguments)[0];
    avtExprNode *firstTree = dynamic_cast<avtExprNode*>(firstarg->GetExpr());
    firstTree->CreateFilters(state);

    ArgExpr *secondarg = (*arguments)[1];
    ExprParseTreeNode *secondTree = secondarg->GetExpr();
    string type = secondTree->GetTypeName();
    if (type == "IntegerConst")
        index = dynamic_cast<IntegerConstExpr*>(secondTree)->GetValue();
    else
    {
        debug5 << "avtArrayDecomposeFilter: Second argument is not an int."
               << endl;
        EXCEPTION1(ExpressionException, "Second argument to array_decompose "
                                        "must be a number.");
    }
}


// ****************************************************************************
//  Method: avtArrayDecomposeFilter::PreExecute
//
//  Purpose:
//      Called before execution.  This sets the issuedWarning flag to false.
//
//  Programmer: Hank Childs
//  Creation:   July 21, 2005
//
// ****************************************************************************

void
avtArrayDecomposeFilter::PreExecute(void)
{
    issuedWarning = false;
}


