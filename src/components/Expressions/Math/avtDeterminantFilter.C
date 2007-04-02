// ************************************************************************* //
//                           avtDeterminantFilter.C                          //
// ************************************************************************* //

#include <avtDeterminantFilter.h>

#include <vtkDataArray.h>
#include <vtkMath.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtDeterminantFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtDeterminantFilter::avtDeterminantFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtDeterminantFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtDeterminantFilter::~avtDeterminantFilter()
{
    ;
}


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
//  Modifications:
//
//    Hank Childs, Fri Mar  3 08:56:52 PST 2006
//    Add support for 2D tensors ['7063].
//
// ****************************************************************************
 
void
avtDeterminantFilter::DoOperation(vtkDataArray *in, vtkDataArray *out,
                          int ncomponents, int ntuples)
{
    if (ncomponents == 9)
    {
        bool is2D = GetInput()->GetInfo().GetAttributes().
                                                    GetSpatialDimension() == 2;
        for (int i = 0 ; i < ntuples ; i++)
        {
            double *vals = in->GetTuple9(i);
            double col1[3];
            col1[0] = vals[0];
            col1[1] = vals[3];
            col1[2] = vals[6];
            double col2[3];
            col2[0] = vals[1];
            col2[1] = vals[4];
            col2[2] = vals[7];
            double col3[3];
            if (is2D)
            {
                col3[0] = 0.;
                col3[1] = 0.;
                col3[2] = 1.;
            }
            else
            {
                col3[0] = vals[2];
                col3[1] = vals[5];
                col3[2] = vals[8];
            }
            double det = vtkMath::Determinant3x3(col1, col2, col3);
            out->SetTuple1(i, det);
        }
    }
    else
    {
        EXCEPTION1(ExpressionException, "Cannot determine tensor type");
    }
}


