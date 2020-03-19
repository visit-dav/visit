// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtBinaryPowerExpression.C                         //
// ************************************************************************* //

#include <avtBinaryPowerExpression.h>

#include <vtkDataArray.h>
#include <vtkDataArray.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtBinaryPowerExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtBinaryPowerExpression::avtBinaryPowerExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtBinaryPowerExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtBinaryPowerExpression::~avtBinaryPowerExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtBinaryPowerExpression::DoOperation
//
//  Purpose:
//      Raises the first array to the power in the second array and puts the
//      result into a third array.
//
//  Arguments:
//      in1           The first input data array.
//      in2           The second input data array.
//      out           The output data array.
//      ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                    vectors, etc.)
//      ntuples       The number of tuples (ie 'npoints' or 'ncells')
//
//  Programmer: Sean Ahern          <Header added by Hank Childs>
//  Creation:   November 18, 2002   <Header creation date>
//
//  Modifications:
//
//    Hank Childs, Mon Nov 18 07:35:07 PST 2002
//    Added support for vectors and arbitrary data types.
//
//    Hank Childs, Thu Aug 14 13:40:20 PDT 2003
//    Added support for mixing vectors and scalars.
//
//    Hank Childs, Mon Jan 14 17:58:58 PST 2008
//    Add support for singleton constants.
//
// ****************************************************************************
 
void
avtBinaryPowerExpression::DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                  vtkDataArray *out, int ncomponents,
                                  int ntuples)
{
    bool var1IsSingleton = (in1->GetNumberOfTuples() == 1);
    bool var2IsSingleton = (in2->GetNumberOfTuples() == 1);
    int in1ncomps = in1->GetNumberOfComponents();
    int in2ncomps = in2->GetNumberOfComponents();
    if (in2ncomps == 1)
    {
        for (int i = 0 ; i < ntuples ; i++)
        {
            vtkIdType tup1 = (var1IsSingleton ? 0 : i);
            vtkIdType tup2 = (var2IsSingleton ? 0 : i);
            for (int j = 0 ; j < in1ncomps ; j++)
            {
                double val1 = in1->GetComponent(tup1, j);
                double val2 = in2->GetComponent(tup2, j);
                out->SetComponent(i, j, pow(val1, val2));
            }
        }
    }
    else
    {
        EXCEPTION2(ExpressionException, outputVariableName,
                   "Don't know how to raise a variable by a vector variable.");
    }
}


