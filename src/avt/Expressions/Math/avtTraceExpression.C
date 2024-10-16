// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtTraceExpression.C                        //
// ************************************************************************* //

#include <avtTraceExpression.h>

#include <vtkDataArray.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtTraceExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtTraceExpression::avtTraceExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtTraceExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtTraceExpression::~avtTraceExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtTraceExpression::DoOperation
//
//  Purpose:
//      Finds the trace (sum of diagonal components) of a tensor.
//
//  Arguments:
//      in            The input data array.
//      out           The output data array.
//      ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                    vectors, etc.)
//      ntuples       The number of tuples (ie 'npoints' or 'ncells')
//
//  Programmer: Hank Childs
//  Creation:   September 19, 2003
//
// ****************************************************************************
 
void
avtTraceExpression::DoOperation(vtkDataArray *in, vtkDataArray *out,
                          int ncomponents, int ntuples, vtkDataSet *in_ds)
{
    if (ncomponents == 9)
    {
        for (int i = 0 ; i < ntuples ; i++)
        {
            double val1 = in->GetComponent(i, 0);
            double val2 = in->GetComponent(i, 4);
            double val3 = in->GetComponent(i, 8);
            out->SetTuple1(i, val1+val2+val3);
        }
    }
    else
    {
        EXCEPTION2(ExpressionException, outputVariableName, "Cannot take trace of non-tensor.");
    }
}


