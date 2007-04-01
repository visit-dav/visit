// ************************************************************************* //
//                          avtDegreeToRadianFilter.C                        //
// ************************************************************************* //

#include <avtDegreeToRadianFilter.h>

#include <math.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>

#if defined(_WIN32) && !defined(M_PI)
// Windows does not seem to have this in math.h
#define M_PI 3.14159265358979323846
#endif


// ****************************************************************************
//  Method: avtDegreeToRadianFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtDegreeToRadianFilter::avtDegreeToRadianFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtDegreeToRadianFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtDegreeToRadianFilter::~avtDegreeToRadianFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtDegreeToRadianFilter::DoOperation
//
//  Purpose:
//      Converts each degree angle to radians.
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
avtDegreeToRadianFilter::DoOperation(vtkDataArray *in, vtkDataArray *out, 
                                     int ncomponents, int ntuples)
{
    for (int i = 0 ; i < ntuples ; i++)
    {
        for (int j = 0 ; j < ncomponents ; j++)
        {
            float val = in->GetComponent(i, j);
            out->SetComponent(i, j, val*((2.*M_PI)/360.));
        }
    }
}


