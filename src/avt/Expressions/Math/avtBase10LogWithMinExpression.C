// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                  avtBase10LogWithMinExpression.C                          //
// ************************************************************************* //

#include <avtBase10LogWithMinExpression.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>

#include <avtExprNode.h>
#include <ExprToken.h>
#include <DebugStream.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtBase10LogWithMinExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtBase10LogWithMinExpression::avtBase10LogWithMinExpression()
{
}


// ****************************************************************************
//  Method: avtBase10LogWithMinExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtBase10LogWithMinExpression::~avtBase10LogWithMinExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtBase10LogWithMinExpression::DoOperation
//
//  Purpose:
//      Performs the base-10 logarithm to each component,tuple of a data array.
//      Also checks against a minimum value.
//
//  Arguments:
//      in1           Input data array #1.
//      in2           Input data array #2.
//      out           The output data array.
//      ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                    vectors, etc.)
//      ntuples       The number of tuples (ie 'npoints' or 'ncells')
//
//  Programmer: Hank Childs
//  Creation:   May 20, 2010
//
//  Modifications:
//
//    Hank Childs, Thu Sep 23 14:10:45 PDT 2010
//    Correct minimum test.
//
// ****************************************************************************

void
avtBase10LogWithMinExpression::DoOperation(vtkDataArray *in1,vtkDataArray *in2,
                                           vtkDataArray *out,
                                           int ncomponents, int ntuples)
{
    if (in2->GetNumberOfComponents() != 1)
    {
        EXCEPTION2(ExpressionException, outputVariableName,  
                   "The minimum value must be a scalar");
    }

    bool var1IsSingleton = (in1->GetNumberOfTuples() == 1);
    bool var2IsSingleton = (in2->GetNumberOfTuples() == 1);
    int in1ncomps = in1->GetNumberOfComponents();

    for (int i = 0 ; i < ntuples ; i++)
    {
        vtkIdType tup2 = (var2IsSingleton ? 0 : i);
        double f2 = in2->GetTuple1(tup2);
        for (int j = 0 ; j < in1ncomps ; j++)
        {
            vtkIdType tup1 = (var1IsSingleton ? 0 : i);
            double f1 = in1->GetComponent(tup1, j);
            double f = (f1 < f2 ? f2 : f1);
            out->SetComponent(i, j, log10(f));
        }
    }
}


