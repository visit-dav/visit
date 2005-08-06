// ************************************************************************* //
//                          avtArrayComposeFilter.C                          //
// ************************************************************************* //

#include <avtArrayComposeFilter.h>

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
//  Method: avtArrayComposeFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   July 21, 2005
//
// ****************************************************************************

avtArrayComposeFilter::avtArrayComposeFilter()
{
    nargs = 0;
}


// ****************************************************************************
//  Method: avtArrayComposeFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   July 21, 2005
//
// ****************************************************************************

avtArrayComposeFilter::~avtArrayComposeFilter()
{
}


// ****************************************************************************
//  Method: avtArrayComposeFilter::DeriveVariable
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
avtArrayComposeFilter::DeriveVariable(vtkDataSet *in_ds)
{
    int    i, j;

    if (varnames.size() == 0)
        EXCEPTION0(ImproperUseException);

    int nvars = varnames.size();
    vtkDataArray **vars = new vtkDataArray*[nvars];
    avtCentering  *centering = new avtCentering[nvars];

    for (i = 0 ; i < nvars ; i++)
    {
        vars[i] = in_ds->GetPointData()->GetArray(varnames[i]);
        centering[i] = AVT_NODECENT;
        if (vars[i] == NULL)
        {
            vars[i] = in_ds->GetCellData()->GetArray(varnames[i]);
            centering[i] = AVT_ZONECENT;
        }
    }

    for (i = 0 ; i < nvars ; i++)
    {
        if (vars[i] == NULL)
            EXCEPTION1(ExpressionException, 
                  "Cannot create array because: cannot locate all variables");
        if (vars[i]->GetNumberOfComponents() != 1)
            EXCEPTION1(ExpressionException, 
                  "Cannot create array because: all inputs must be scalars");
        if (centering[i] != centering[0])
            EXCEPTION1(ExpressionException,
                  "Cannot create array because: the centering of the "
                  "variables does not agree.");
    }

    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetNumberOfComponents(nvars);
    int nvals = vars[0]->GetNumberOfTuples();
    rv->SetNumberOfTuples(nvals);
    for (i = 0 ; i < nvals ; i++)
        for (j = 0 ; j < nvars ; j++)
            rv->SetComponent(i, j, vars[j]->GetTuple1(i));

    delete [] vars;
    delete [] centering;
    return rv;
}


// ****************************************************************************
//  Method: avtArrayComposeFilter::ProcessArguments
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
avtArrayComposeFilter::ProcessArguments(ArgsExpr *args,
                                        ExprPipelineState *state)
{
    // Check the number of arguments
    std::vector<ArgExpr*> *arguments = args->GetArgs();
    nargs = arguments->size();

    // Let the base class do this processing.  We only had to over-ride this
    // function to determine the number of arguments.
    avtMultipleInputExpressionFilter::ProcessArguments(args, state);
}


// ****************************************************************************
//  Method: avtArrayComposeFilter::RefashionDataObjectInfo
//
//  Purpose:
//      Tell the output what the component names are.
//
//  Programmer:   Hank Childs
//  Creation:     August 5, 2005
//
// ****************************************************************************

void
avtArrayComposeFilter::RefashionDataObjectInfo(void)
{
    avtMultipleInputExpressionFilter::RefashionDataObjectInfo();

    // If we don't know the name of the variable, we can't set it up in the
    // output.
    if (outputVariableName == NULL)
        return;

    std::vector<std::string> subnames(varnames.size());
    for (int i = 0 ; i < varnames.size() ; i++)
        subnames[i] = varnames[i];

    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
    outAtts.SetVariableSubnames(subnames, outputVariableName);
}

