// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      avtArrayComposeExpression.C                          //
// ************************************************************************* //

#include <avtArrayComposeExpression.h>

#include <math.h>

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkUnsignedIntArray.h>

#include <ExprToken.h>
#include <avtExprNode.h>

#include <avtCallback.h>
#include <avtMaterial.h>
#include <avtSpecies.h>

#include <DebugStream.h>
#include <ExpressionException.h>
#include <ImproperUseException.h>


// ****************************************************************************
//  Method: avtArrayComposeExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   July 21, 2005
//
// ****************************************************************************

avtArrayComposeExpression::avtArrayComposeExpression()
{
    nargs = 0;
}


// ****************************************************************************
//  Method: avtArrayComposeExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   July 21, 2005
//
// ****************************************************************************

avtArrayComposeExpression::~avtArrayComposeExpression()
{
}


// ****************************************************************************
//  Method: avtArrayComposeExpression::DeriveVariable
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
avtArrayComposeExpression::DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex)
{
    if (varnames.size() == 0)
        EXCEPTION0(ImproperUseException);

    int nvars = (int)varnames.size();
    vtkDataArray **vars = new vtkDataArray*[nvars];
    avtCentering  *centering = new avtCentering[nvars];

    for (int i = 0 ; i < nvars ; i++)
    {
        vars[i] = in_ds->GetPointData()->GetArray(varnames[i]);
        centering[i] = AVT_NODECENT;
        if (vars[i] == NULL)
        {
            vars[i] = in_ds->GetCellData()->GetArray(varnames[i]);
            centering[i] = AVT_ZONECENT;
        }
    }

    int outType = VTK_FLOAT;
    for (int i = 0 ; i < nvars ; i++)
    {
        if (vars[i] == NULL)
            EXCEPTION2(ExpressionException, outputVariableName,
                  "Cannot create array because: cannot locate all variables");
        if (vars[i]->GetNumberOfComponents() != 1)
            EXCEPTION2(ExpressionException, outputVariableName,
                  "Cannot create array because: all inputs must be scalars");
        if (centering[i] != centering[0])
            EXCEPTION2(ExpressionException, outputVariableName,
                  "Cannot create array because: the centering of the "
                  "variables does not agree.");
        if (vars[i]->GetDataType() == VTK_DOUBLE)
            outType = VTK_DOUBLE;
    }

    vtkDataArray *rv = vtkDataArray::CreateDataArray(outType);
    rv->SetNumberOfComponents(nvars);
    vtkIdType nvals = vars[0]->GetNumberOfTuples();
    rv->SetNumberOfTuples(nvals);
    for (vtkIdType i = 0 ; i < nvals ; i++)
        for (int j = 0 ; j < nvars ; j++)
            rv->SetComponent(i, j, vars[j]->GetTuple1(i));

    delete [] vars;
    delete [] centering;
    return rv;
}


// ****************************************************************************
//  Method: avtArrayComposeExpression::ProcessArguments
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
avtArrayComposeExpression::ProcessArguments(ArgsExpr *args,
                                        ExprPipelineState *state)
{
    // Check the number of arguments
    std::vector<ArgExpr*> *arguments = args->GetArgs();
    nargs = (int)arguments->size();

    // Let the base class do this processing.  We only had to over-ride this
    // function to determine the number of arguments.
    avtMultipleInputExpressionFilter::ProcessArguments(args, state);
}


// ****************************************************************************
//  Method: avtArrayComposeExpression::UpdateDataObjectInfo
//
//  Purpose:
//      Tell the output what the component names are.
//
//  Programmer:   Hank Childs
//  Creation:     August 5, 2005
//
//  Modifications:
//    Jeremy Meredith, Thu Feb  7 18:01:29 EST 2008
//    This wasn't setting the dimension of the output variable, which
//    was necessary, so I added it.
//
// ****************************************************************************

void
avtArrayComposeExpression::UpdateDataObjectInfo(void)
{
    avtMultipleInputExpressionFilter::UpdateDataObjectInfo();

    // If we don't know the name of the variable, we can't set it up in the
    // output.
    if (outputVariableName == NULL)
        return;

    std::vector<std::string> subnames(varnames.size());
    for (size_t i = 0 ; i < varnames.size() ; i++)
        subnames[i] = varnames[i];

    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
    outAtts.SetVariableDimension((int)varnames.size(), outputVariableName);
    outAtts.SetVariableSubnames(subnames, outputVariableName);
}

