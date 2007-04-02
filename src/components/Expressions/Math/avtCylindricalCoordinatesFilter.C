// ************************************************************************* //
//                      avtCylindricalCoordinatesFilter.C                    //
// ************************************************************************* //

#include <avtCylindricalCoordinatesFilter.h>

#include <math.h>

#include <vtkDataSet.h>
#include <vtkFloatArray.h>


// ****************************************************************************
//  Method: avtCylindricalCoordinatesFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   June 30, 2005
//
// ****************************************************************************

avtCylindricalCoordinatesFilter::avtCylindricalCoordinatesFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtCylindricalCoordinatesFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   June 30, 2005
//
// ****************************************************************************

avtCylindricalCoordinatesFilter::~avtCylindricalCoordinatesFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtCylindricalCoordinatesFilter::DeriveVariable
//
//  Purpose:
//      Derives a variable based on the input dataset.  The variable is the
//      cylindrical coordinates of the point list.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Hank Childs
//  Creation:     June 30, 2005
//
// ****************************************************************************

vtkDataArray *
avtCylindricalCoordinatesFilter::DeriveVariable(vtkDataSet *in_ds)
{
    int npts = in_ds->GetNumberOfPoints();
    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetNumberOfComponents(3);
    rv->SetNumberOfTuples(npts);
    for (int i = 0 ; i < npts ; i++)
    {
        float pt[3];
        in_ds->GetPoint(i, pt);
        
        float r = sqrt(pt[0]*pt[0] + pt[1]*pt[1]);
        rv->SetComponent(i, 0, r);

        float theta = atan2(pt[1], pt[0]);
        rv->SetComponent(i, 1, theta);

        rv->SetComponent(i, 2, pt[2]);
    }
    
    return rv;
}


