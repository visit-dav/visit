// ************************************************************************* //
//                            avtEigenvalueFilter.C                          //
// ************************************************************************* //

#include <avtEigenvalueFilter.h>

#include <vtkDataArray.h>
#include <vtkMath.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtEigenvalueFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtEigenvalueFilter::avtEigenvalueFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtEigenvalueFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtEigenvalueFilter::~avtEigenvalueFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtEigenvalueFilter::DoOperation
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
avtEigenvalueFilter::DoOperation(vtkDataArray *in, vtkDataArray *out,
                                 int ncomponents, int ntuples)
{
    if (ncomponents == 9)
    {
        for (int i = 0 ; i < ntuples ; i++)
        {
            float *vals = in->GetTuple9(i);
            float *input[3];
            float row1[3];
            float row2[3];
            float row3[3];
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
            float *eigenvecs[3];
            float outrow1[3];
            float outrow2[3];
            float outrow3[3];
            eigenvecs[0] = outrow1;
            eigenvecs[1] = outrow2;
            eigenvecs[2] = outrow3;
            float eigenvals[3];
            vtkMath::Jacobi(input, eigenvals, eigenvecs);
            out->SetTuple(i, eigenvals);
        }
    }
    else
    {
        EXCEPTION1(ExpressionException, "Cannot determine tensor type");
    }
}


