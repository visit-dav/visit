// ************************************************************************* //
//                           avtDeterminantFilter.C                          //
// ************************************************************************* //

#include <avtDeterminantFilter.h>

#include <vtkDataArray.h>
#include <vtkMath.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtDeterminantFilter::DoOperation
//
//  Purpose:
//      Finds the determinant of a tensor.
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
avtDeterminantFilter::DoOperation(vtkDataArray *in, vtkDataArray *out,
                          int ncomponents, int ntuples)
{
    if (ncomponents == 9)
    {
        for (int i = 0 ; i < ntuples ; i++)
        {
            float *vals = in->GetTuple9(i);
            float col1[3];
            col1[0] = vals[0];
            col1[1] = vals[3];
            col1[2] = vals[6];
            float col2[3];
            col2[0] = vals[1];
            col2[1] = vals[4];
            col2[2] = vals[7];
            float col3[3];
            col3[0] = vals[2];
            col3[1] = vals[5];
            col3[2] = vals[8];
            float det = vtkMath::Determinant3x3(col1, col2, col3);
            out->SetTuple1(i, det);
        }
    }
    else
    {
        EXCEPTION1(ExpressionException, "Cannot determine tensor type");
    }
}


