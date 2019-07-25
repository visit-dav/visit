// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      avtTestGreaterThanExpression.C                       //
// ************************************************************************* //

#include <avtTestGreaterThanExpression.h>

#include <vtkDataArray.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtTestGreaterThanExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtTestGreaterThanExpression::avtTestGreaterThanExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtTestGreaterThanExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtTestGreaterThanExpression::~avtTestGreaterThanExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtTestGreaterThanExpression::DoOperation
//
//  Purpose:
//      Tests whether one array is greater than another.
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
//  Creation:   August 21, 2003
//
//  Modifications:
//
//    Hank Childs, Mon Jan 14 20:01:04 PST 2008
//    Add support for singleton constants.
//
// ****************************************************************************
 
void
avtTestGreaterThanExpression::DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                  vtkDataArray *out, int ncomponents,
                                  int ntuples)
{
    bool var1IsSingleton = (in1->GetNumberOfTuples() == 1);
    bool var2IsSingleton = (in2->GetNumberOfTuples() == 1);
    int in1ncomps = in1->GetNumberOfComponents();
    int in2ncomps = in2->GetNumberOfComponents();
    if (in1ncomps != 1 || in2ncomps != 1)
    {
        EXCEPTION2(ExpressionException, outputVariableName,
                   "Cannot compare vector variables.");
    }

    for (int i = 0 ; i < ntuples ; i++)
    {
        int tup1 = (var1IsSingleton ? 0 : i);
        int tup2 = (var2IsSingleton ? 0 : i);
        unsigned char outval = (in1->GetTuple1(tup1) > in2->GetTuple1(tup2)
                                ? '\1' : '\0');
        out->SetTuple1(i, outval);
    }
}


