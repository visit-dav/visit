// ************************************************************************* //
//                               avtInverseFilter.C                          //
// ************************************************************************* //

#include <avtInverseFilter.h>

#include <vtkDataArray.h>
#include <vtkMath.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtInverseFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtInverseFilter::avtInverseFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtInverseFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtInverseFilter::~avtInverseFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtInverseFilter::DoOperation
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
avtInverseFilter::DoOperation(vtkDataArray *in, vtkDataArray *out,
                          int ncomponents, int ntuples)
{
    if (ncomponents == 9)
    {
        for (int i = 0 ; i < ntuples ; i++)
        {
            float *vals = in->GetTuple9(i);
            float input[3][3];
            input[0][0] = vals[0];
            input[0][1] = vals[1];
            input[0][2] = vals[2];
            input[1][0] = vals[3];
            input[1][1] = vals[4];
            input[1][2] = vals[5];
            input[2][0] = vals[6];
            input[2][1] = vals[7];
            input[2][2] = vals[8];
            float output[3][3];
            vtkMath::Invert3x3(input, output);
            float out_vals[9];
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
        EXCEPTION1(ExpressionException, "Cannot determine tensor type");
    }
}


