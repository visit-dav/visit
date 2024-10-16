// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtEigenvectorExpression.C                     //
// ************************************************************************* //

#include <avtEigenvectorExpression.h>

#include <vtkDataArray.h>
#include <vtkMath.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtEigenvectorExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtEigenvectorExpression::avtEigenvectorExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtEigenvectorExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtEigenvectorExpression::~avtEigenvectorExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtEigenvectorExpression::DoOperation
//
//  Purpose:
//      Calculates the eigenvectors of a tensor matrix.
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
avtEigenvectorExpression::DoOperation(vtkDataArray *in, vtkDataArray *out,
                                      int ncomponents, int ntuples, vtkDataSet *in_ds)
{
    if (ncomponents == 9)
    {
        for (int i = 0 ; i < ntuples ; i++)
        {
            double *vals = in->GetTuple9(i);
            double *input[3];
            double row1[3];
            double row2[3];
            double row3[3];
            input[0] = row1;
            input[1] = row2;
            input[2] = row3;
            input[0][0] = vals[0];
            input[0][1] = vals[1];
            input[0][2] = vals[2];
            input[1][0] = vals[3];
            input[1][1] = vals[4];
            input[1][2] = vals[5];
            input[2][0] = vals[6];
            input[2][1] = vals[7];
            input[2][2] = vals[8];
            double *eigenvecs[3];
            double outrow1[3];
            double outrow2[3];
            double outrow3[3];
            eigenvecs[0] = outrow1;
            eigenvecs[1] = outrow2;
            eigenvecs[2] = outrow3;
            double eigenvals[3];

            // vtkMath::Jacobi and vtkMath::JacobiN return the vectors
            // in a column ordering. That is the first vector is in:
            // eigenvecs[0][0] eigenvecs[1][0] eigenvecs[2][0] and
            // that output[0] output[1] output[1] does NOT contain the
            // first eigen value but the first component of each eigen
            // vector.
            
            vtkMath::Jacobi(input, eigenvals, eigenvecs);
            double output[9];
            output[0] = eigenvecs[0][0];
            output[1] = eigenvecs[0][1];
            output[2] = eigenvecs[0][2];
            output[3] = eigenvecs[1][0];
            output[4] = eigenvecs[1][1];
            output[5] = eigenvecs[1][2];
            output[6] = eigenvecs[2][0];
            output[7] = eigenvecs[2][1];
            output[8] = eigenvecs[2][2];
            out->SetTuple(i, output);
        }
    }
    else
    {
        EXCEPTION2(ExpressionException, outputVariableName, "Cannot determine tensor type");
    }
}
