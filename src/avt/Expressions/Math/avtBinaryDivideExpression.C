// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtBinaryDivideExpression.C                         //
// ************************************************************************* //

#include <avtBinaryDivideExpression.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtBinaryDivideExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtBinaryDivideExpression::avtBinaryDivideExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtBinaryDivideExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtBinaryDivideExpression::~avtBinaryDivideExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtBinaryDivideExpression::DoOperation
//
//  Purpose:
//      Divides the contents of the first array by the second array and puts
//      the output in a third array.
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
//    Hank Childs, Thu Aug 14 11:18:07 PDT 2003
//    Add support for vector and scalar types mixing.
//
//    Hank Childs, Fri Oct  7 10:43:28 PDT 2005
//    Add support for dividing by zero.
//
//    Hank Childs, Mon Jan 14 17:58:58 PST 2008
//    Add support for singleton constants.
//
// ****************************************************************************

void
avtBinaryDivideExpression::DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                   vtkDataArray *out, int ncomponents,
                                   int ntuples)
{
    bool var1IsSingleton = (in1->GetNumberOfTuples() == 1);
    bool var2IsSingleton = (in2->GetNumberOfTuples() == 1);
    int in1ncomps = in1->GetNumberOfComponents();
    int in2ncomps = in2->GetNumberOfComponents();
    if ((in1ncomps == 1) && (in2ncomps == 1))
    {
        for (int i = 0 ; i < ntuples ; i++)
        {
            vtkIdType tup1 = (var1IsSingleton ? 0 : i);
            vtkIdType tup2 = (var2IsSingleton ? 0 : i);
            double val1 = in1->GetTuple1(tup1);
            double val2 = in2->GetTuple1(tup2);
            if (val1 == 0. && val2 == 0.)
            {
                out->SetTuple1(i, 1.);
            }
            else if (val2 == 0. && val1 != 0.)
            {
                EXCEPTION2(ExpressionException, outputVariableName, 
                           "You can't divide by zero");
            }
            else
                out->SetTuple1(i, val1 / val2);
        }
    }
    else if (in1ncomps > 1 && in2ncomps == 1)
    {
        for (int i = 0 ; i < ntuples ; i++)
        {
            vtkIdType tup1 = (var1IsSingleton ? 0 : i);
            vtkIdType tup2 = (var2IsSingleton ? 0 : i);
            double val2 = in2->GetTuple1(tup2);
            if (val2 == 0)
            {
                EXCEPTION2(ExpressionException, outputVariableName, 
                           "You can't divide by zero");
            }
            for (int j = 0 ; j < in1ncomps ; j++)
            {
                double val1 = in1->GetComponent(tup1, j);
                out->SetComponent(i, j, val1/val2);
            }
        }
    }
    else if (in1ncomps == 1 && in2ncomps > 1)
    {
        for (int i = 0 ; i < ntuples ; i++)
        {
            vtkIdType tup1 = (var1IsSingleton ? 0 : i);
            vtkIdType tup2 = (var2IsSingleton ? 0 : i);
            double val1 = in1->GetTuple1(tup1);
            for (int j = 0 ; j < in2ncomps ; j++)
            {
                double val2 = in2->GetComponent(tup2, j);
                if (val2 == 0)
                {
                    EXCEPTION2(ExpressionException, outputVariableName, 
                               "You can't divide by zero");
                }
                out->SetComponent(i, j, val1/val2);
            }
        }
    }
    else
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                   "Division of vectors in undefined.");
    }
}


