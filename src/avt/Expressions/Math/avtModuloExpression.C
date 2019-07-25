// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtModuloExpression.C                       //
// ************************************************************************* //

#include <avtModuloExpression.h>

#include <vtkDataArray.h>
#include <vtkDataArray.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtModuloExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   June 30, 2005
//
// ****************************************************************************

avtModuloExpression::avtModuloExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtModuloExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   June 30, 2005
//
// ****************************************************************************

avtModuloExpression::~avtModuloExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtModuloExpression::DoOperation
//
//  Purpose:
//      Performs a modulo operation of the first by the second array and 
//      puts the result into a third array.
//
//  Arguments:
//      in1           The first input data array.
//      in2           The second input data array.
//      out           The output data array.
//      ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                    vectors, etc.)
//      ntuples       The number of tuples (ie 'npoints' or 'ncells')
//
//  Programmer: Hank Childs
//  Creation:   June 30, 2005
//
// ****************************************************************************
 
void
avtModuloExpression::DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                 vtkDataArray *out, int ncomponents,
                                 int ntuples)
{
    vtkIdType in1ncomps = in1->GetNumberOfComponents();
    vtkIdType in2ncomps = in2->GetNumberOfComponents();
    if (in2ncomps == 1)
    {
        for (int i = 0 ; i < ntuples ; i++)
        {
            for (vtkIdType j = 0 ; j < in1ncomps ; j++)
            {
                double val1 = in1->GetComponent(i, j);
                int v1 = (int) val1;
                double val2 = in2->GetComponent(i, j);
                int v2 = (int) val2;
                int output = (v2 <= 0 ? 1 : v1 % v2);
                out->SetComponent(i, j, output);
            }
        }
    }
    else
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                   "Don't know how to perform modulo "
                   "operation with a vector variable.");
    }
}


