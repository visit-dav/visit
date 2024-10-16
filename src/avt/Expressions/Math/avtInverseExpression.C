// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtInverseExpression.C                      //
// ************************************************************************* //

#include <avtInverseExpression.h>

#include <vtkDataArray.h>
#include <vtkMath.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtInverseExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtInverseExpression::avtInverseExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtInverseExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtInverseExpression::~avtInverseExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtInverseExpression::DoOperation
//
//  Purpose:
//      Inverts a tensor matrix.
//
//  Arguments:
//      in            The input data array.
//      out           The output data array.
//      ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                    vectors, etc.)
//      ntuples       The number of tuples (ie 'npoints' or 'ncells')
//
//  Programmer: Hank Childs
//  Creation:   September 22, 2003
//
// ****************************************************************************
 
void
avtInverseExpression::DoOperation(vtkDataArray *in, vtkDataArray *out,
                                  int ncomponents, int ntuples, vtkDataSet *in_ds)
{
    if (ncomponents == 9)
    {
        for (int i = 0 ; i < ntuples ; i++)
        {
            double *vals = in->GetTuple9(i);
            double input[3][3];
            input[0][0] = vals[0];
            input[0][1] = vals[1];
            input[0][2] = vals[2];
            input[1][0] = vals[3];
            input[1][1] = vals[4];
            input[1][2] = vals[5];
            input[2][0] = vals[6];
            input[2][1] = vals[7];
            input[2][2] = vals[8];
            double output[3][3];
            vtkMath::Invert3x3(input, output);
            double out_vals[9];
            out_vals[0] = output[0][0];
            out_vals[1] = output[0][1];
            out_vals[2] = output[0][2];
            out_vals[3] = output[1][0];
            out_vals[4] = output[1][1];
            out_vals[5] = output[1][2];
            out_vals[6] = output[2][0];
            out_vals[7] = output[2][1];
            out_vals[8] = output[2][2];
            out->SetTuple(i, out_vals);
        }
    }
    else
    {
        EXCEPTION2(ExpressionException, outputVariableName, "Cannot determine tensor type");
    }
}


