// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      avtVariableSkewExpression.C                          //
// ************************************************************************* //

#include <avtVariableSkewExpression.h>

#include <vtkDataArray.h>
#include <ExpressionException.h>
#include <vtkSkew.h>

// ****************************************************************************
//  Method: avtVariableSkewExpression constructor
//
//  Purpose:
//    Defines the constructor.  Note: this should not be inlined in the
//    header because it causes problems for certain compilers.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 2, 2050 
//
// ****************************************************************************

avtVariableSkewExpression::avtVariableSkewExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtVariableSkewExpression destructor
//
//  Purpose:
//    Defines the destructor.  Note: this should not be inlined in the header
//    because it causes problems for certain compilers.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 2, 2005 
//
// ****************************************************************************

avtVariableSkewExpression::~avtVariableSkewExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtVariableSkewExpression::DoOperation
//
//  Purpose:
//    Performs the skew operation on each component,tuple of a data array.
//
//  Arguments:
//    in1           The first input data array.
//    in2           The second data array.
//    out           The output data array.
//    ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                  vectors, etc.)
//    ntuples       The number of tuples (ie 'npoints' or 'ncells')
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 5, 2005 
//
//  Modifications:
//    Brad Whitlock, Fri Dec 19 15:54:36 PST 2008
//    I made it use vtkSkewValue so we have all of the skew definitions in
//    one place.
//
// ****************************************************************************

void
avtVariableSkewExpression::DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                   vtkDataArray *out, int ncomponents, 
                                   int ntuples)
{
    vtkIdType in1ncomps = in1->GetNumberOfComponents();
    vtkIdType in2ncomps = in2->GetNumberOfComponents();
    if (in1ncomps == 1 && in2ncomps == 1)
    {
        double *r = in1->GetRange();
        for (int i = 0 ; i < ntuples ; i++)
        {
            double val1 = in1->GetComponent(i, 0);
            double val2 = in2->GetComponent(i, 0);
            double f = vtkSkewValue(val1, r[0], r[1], val2);
            out->SetComponent(i, 0, f);
        }
    }
    else
    {
        EXCEPTION2(ExpressionException, outputVariableName, "Skew can only be "
                   "used on scalar variables.");
    }
}
