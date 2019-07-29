// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtBinaryAndExpression.C                         //
// ************************************************************************* //

#include <avtBinaryAndExpression.h>

#include <vtkDataArray.h>
#include <vtkDataArray.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtBinaryAndExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Mark C. Miller 
//  Creation:   Mon Mar 31 19:39:25 PDT 2008
//
// ****************************************************************************

avtBinaryAndExpression::avtBinaryAndExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtBinaryAndExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Mark C. Miller 
//  Creation:   Mon Mar 31 19:39:25 PDT 2008
//
// ****************************************************************************

avtBinaryAndExpression::~avtBinaryAndExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtBinaryAndExpression::DoOperation
//
//  Purpose:
//      Sums two arrays into a third array.
//
//  Arguments:
//      in1           The first input data array.
//      in2           The second input data array.
//      out           The output data array.
//      ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                    vectors, etc.)
//      ntuples       The number of tuples (ie 'npoints' or 'ncells')
//
//  Programmer: Mark C. Miller 
//  Creation:   Mon Mar 31 19:39:25 PDT 2008
//
// ****************************************************************************

void
avtBinaryAndExpression::DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                vtkDataArray *out, int ncomponents,int ntuples)
{
    int in1ncomps = in1->GetNumberOfComponents();
    int in2ncomps = in2->GetNumberOfComponents();
    if (in1ncomps == in2ncomps)
    {
        for (int i = 0 ; i < ntuples ; i++)
        {
            for (int j = 0 ; j < in1ncomps ; j++)
            {
                int val1 = (int) in1->GetComponent(i, j);
                int val2 = (int) in2->GetComponent(i, j);
                out->SetComponent(i, j, val1 & val2);
            }
        }
    }
    else if (in1ncomps > 1 && in2ncomps == 1)
    {
        for (int i = 0 ; i < ntuples ; i++)
        {
            int val2 = (int) in2->GetTuple1(i);
            for (int j = 0 ; j < in1ncomps ; j++)
            {
                int val1 = (int) in1->GetComponent(i, j);
                out->SetComponent(i, j, val1 & val2);
            }
        }
    }
    else if (in1ncomps == 1 && in2ncomps > 1)
    {
        for (int i = 0 ; i < ntuples ; i++)
        {
            int val1 = (int) in1->GetTuple1(i);
            for (int j = 0 ; j < in2ncomps ; j++)
            {
                int val2 = (int) in2->GetComponent(i, j);
                out->SetComponent(i, j, val1 & val2);
            }
        }
    }
    else
    {
        EXCEPTION2(ExpressionException, outputVariableName,
            "Don't know how to and vectors of differing dimensions.");
    }
}
