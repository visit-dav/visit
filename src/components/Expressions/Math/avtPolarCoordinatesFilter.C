// ************************************************************************* //
//                         avtPolarCoordinatesFilter.C                       //
// ************************************************************************* //

#include <avtPolarCoordinatesFilter.h>

#include <math.h>

#include <vtkDataSet.h>
#include <vtkFloatArray.h>


// ****************************************************************************
//  Method: avtPolarCoordinatesFilter::DeriveVariable
//
//  Purpose:
//      Derives a variable based on the input dataset.  The variable is the
//      polar coordinates of the point list.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Hank Childs
//  Creation:     November 18, 2002
//
// ****************************************************************************

vtkDataArray *
avtPolarCoordinatesFilter::DeriveVariable(vtkDataSet *in_ds)
{
    int npts = in_ds->GetNumberOfPoints();
    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetNumberOfComponents(3);
    rv->SetNumberOfTuples(npts);
    for (int i = 0 ; i < npts ; i++)
    {
        float pt[3];
        in_ds->GetPoint(i, pt);
        
        float r = sqrt(pt[0]*pt[0] + pt[1]*pt[1] + pt[2]*pt[2]);
        rv->SetComponent(i, 0, r);

        float theta = atan2(pt[1], pt[0]);
        rv->SetComponent(i, 1, theta);

        float phi = acos(pt[2] / r);
        rv->SetComponent(i, 2, phi);
    }
    
    return rv;
}


