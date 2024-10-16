// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      avtLogicalNegationExpression.C                       //
// ************************************************************************* //

#include <avtLogicalNegationExpression.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtLogicalNegationExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtLogicalNegationExpression::avtLogicalNegationExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtLogicalNegationExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtLogicalNegationExpression::~avtLogicalNegationExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtLogicalNegationExpression::DoOperation
//
//  Purpose:
//      Takes the logical negation of an array.
//
//  Arguments:
//      in1           The input data array.
//      out           The output data array.
//      ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                    vectors, etc.)
//      ntuples       The number of tuples (ie 'npoints' or 'ncells')
//
//  Programmer: Hank Childs
//  Creation:   August 21, 2003
//
// ****************************************************************************
 
void
avtLogicalNegationExpression::DoOperation(vtkDataArray *in, vtkDataArray *out,
                                      int ncomponents, int ntuples, vtkDataSet *in_ds)
{
    int inncomps = in->GetNumberOfComponents();
    if (inncomps != 1)
    {
        EXCEPTION2(ExpressionException, outputVariableName,
                   "Cannot logically negate a vector variable.");
    }

    for (int i = 0 ; i < ntuples ; i++)
    {
        bool val1;

        if (in->GetDataType() == VTK_UNSIGNED_CHAR)
        {
            val1 = (unsigned char) in->GetTuple1(i);
        }
        else
        {
            val1 = (in->GetTuple1(i) != 0. ? true : false);
        }

        unsigned char outval = !val1;
        out->SetTuple1(i, outval);
    }
}


