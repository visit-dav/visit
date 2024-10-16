// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                     avtTensorContractionExpression.C                      //
// ************************************************************************* //

#include <avtTensorContractionExpression.h>

#include <math.h>

#include <vtkDataArray.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtTensorContractionExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Cyrus Harrison
//  Creation:   June 1, 2007
//
// ****************************************************************************

avtTensorContractionExpression::avtTensorContractionExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtTensorContractionExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Cyrus Harrison
//  Creation:   June 1, 2007
//
// ****************************************************************************

avtTensorContractionExpression::~avtTensorContractionExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtPrincipalTensorExpression::DoOperation
//
//  Purpose:
//      Calculates the contraction of a tensor
//
//  Programmer: Cyrus Harrison
//  Creation:   June 1, 2007
//
// ****************************************************************************

void
avtTensorContractionExpression::DoOperation(vtkDataArray *in, vtkDataArray *out,
                                        int ncomps, int ntuples, vtkDataSet *in_ds)
{
    if (ncomps == 9)
    {
        for (int i = 0 ; i < ntuples ; i++)
        {
            double ctract = 0.0;
            double *vals = in->GetTuple9(i);

            //
            // For a rank 2 tensor, the contraction collapses to a scalar.
            // Conceptually it is like as doting each column vector with
            // itself and adding the column results
            //

            ctract +=vals[0] * vals[0] + vals[1] * vals[1] + vals[2] * vals[2];
            ctract +=vals[3] * vals[3] + vals[4] * vals[4] + vals[5] * vals[5];
            ctract +=vals[6] * vals[6] + vals[7] * vals[7] + vals[8] * vals[8];

            out->SetTuple(i, &ctract);
        }
    }
    else
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                   "Cannot determine tensor type");
    }
}


