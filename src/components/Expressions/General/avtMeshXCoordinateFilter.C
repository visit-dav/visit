// ************************************************************************* //
//                          avtMeshXCoordinateFilter.C                       //
// ************************************************************************* //

#include <avtMeshXCoordinateFilter.h>

#include <vtkDataSet.h>

#include <vtkFloatArray.h>


// ****************************************************************************
//  Method: avtMeshXCoordinateFilter::DeriveVariable
//
//  Purpose:
//      Pulls out the X-coordinates a mesh.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Hank Childs
//  Creation:     November 19, 2002
//
// ****************************************************************************

vtkDataArray *
avtMeshXCoordinateFilter::DeriveVariable(vtkDataSet *in_ds)
{
    int npts = in_ds->GetNumberOfPoints();

    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetNumberOfComponents(1);
    rv->SetNumberOfTuples(npts);

    for (int i = 0 ; i < npts ; i++)
    {
         float pt[3];
         in_ds->GetPoint(i, pt);
         rv->SetTuple1(i, pt[0]);
    }

    return rv;
}


