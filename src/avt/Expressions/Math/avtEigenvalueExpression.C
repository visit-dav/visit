// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtEigenvalueExpression.C                      //
// ************************************************************************* //

#include <avtEigenvalueExpression.h>

#include <vtkDataArray.h>
#include <vtkMath.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtEigenvalueExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtEigenvalueExpression::avtEigenvalueExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtEigenvalueExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtEigenvalueExpression::~avtEigenvalueExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtEigenvalueExpression::DoOperation
//
//  Purpose:
//      Calculates the eigenvalues of a matrix.
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
avtEigenvalueExpression::DoOperation(vtkDataArray *in, vtkDataArray *out,
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
            vtkMath::Jacobi(input, eigenvals, eigenvecs);
            out->SetTuple(i, eigenvals);
        }
    }
    else
    {
        EXCEPTION2(ExpressionException, outputVariableName, "Cannot determine tensor type");
    }
}


