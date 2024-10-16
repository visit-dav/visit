// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtTransposeExpression.C                        //
// ************************************************************************* //

#include <avtTransposeExpression.h>

#include <math.h>

#include <vtkDataArray.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtTransposeExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 21, 2008
//
// ****************************************************************************

avtTransposeExpression::avtTransposeExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtTransposeExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 21, 2008
//
// ****************************************************************************

avtTransposeExpression::~avtTransposeExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtPrincipalTensorExpression::DoOperation
//
//  Purpose:
//      Calculates the contraction of a tensor
//
//  Programmer: Hank Childs
//  Creation:   February 21, 2008
//
// ****************************************************************************

void
avtTransposeExpression::DoOperation(vtkDataArray *in, vtkDataArray *out,
                                        int ncomps, int ntuples, vtkDataSet *in_ds)
{
    if (ncomps == 9)
    {
        for (int i = 0 ; i < ntuples ; i++)
        {
            double *vals = in->GetTuple9(i);
            out->SetComponent(i, 0, vals[0]);
            out->SetComponent(i, 1, vals[3]);
            out->SetComponent(i, 2, vals[6]);
            out->SetComponent(i, 3, vals[1]);
            out->SetComponent(i, 4, vals[4]);
            out->SetComponent(i, 5, vals[7]);
            out->SetComponent(i, 6, vals[2]);
            out->SetComponent(i, 7, vals[5]);
            out->SetComponent(i, 8, vals[8]);
        }
    }
    else
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                                        "Cannot determine tensor type");
    }
}


