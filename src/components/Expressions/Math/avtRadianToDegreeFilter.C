// ************************************************************************* //
//                          avtRadianToDegreeFilter.C                        //
// ************************************************************************* //

#include <avtRadianToDegreeFilter.h>

#include <math.h>

#include <vtkDataSet.h>

#if defined(_WIN32) && !defined(M_PI)
// Windows does not seem to have this in math.h
#define M_PI 3.14159265358979323846
#endif

// ****************************************************************************
//  Method: avtRadianToDegreeFilter::DoOperation
//
//  Purpose:
//      Converts radian angles to degree angles.
//
//  Arguments:
//      in            The input data array.
//      out           The output data array.
//      ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                    vectors, etc.)
//      ntuples       The number of tuples (ie 'npoints' or 'ncells')
//
//  Programmer: Hank Childs
//  Creation:   November 18, 2002
//
// ****************************************************************************

void
avtRadianToDegreeFilter::DoOperation(vtkDataArray *in, vtkDataArray *out, 
                                     int ncomponents, int ntuples)
{
    for (int i = 0 ; i < ntuples ; i++)
    {
        for (int j = 0 ; j < ncomponents ; j++)
        {
            float val = in->GetComponent(i, j);
            out->SetComponent(i, j, val*(360./(2.*M_PI)));
        }
    }
}


