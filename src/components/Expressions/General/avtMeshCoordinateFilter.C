// ************************************************************************* //
//                          avtMeshCoordinateFilter.C                       //
// ************************************************************************* //

#include <avtMeshCoordinateFilter.h>

#include <vtkDataSet.h>

#include <vtkFloatArray.h>


// ****************************************************************************
//  Method: avtMeshCoordinateFilter::DeriveVariable
//
//  Purpose:
//      Pulls out the coordinates a mesh.
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
//  Modifications:
//      Sean Ahern, Thu Mar  6 01:49:31 America/Los_Angeles 2003
//      Merged this with the other coordinate filters.
//
//      Sean Ahern, Fri Mar  7 21:20:29 America/Los_Angeles 2003
//      Made this return a vector of coordinates, rather than just one of them.
//
// ****************************************************************************
vtkDataArray *
avtMeshCoordinateFilter::DeriveVariable(vtkDataSet *in_ds)
{
    int npts = in_ds->GetNumberOfPoints();

    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetNumberOfComponents(3);
    rv->SetNumberOfTuples(npts);

    for (int i = 0 ; i < npts ; i++)
    {
         float pt[3];
         in_ds->GetPoint(i, pt);
         rv->SetTuple3(i, pt[0], pt[1], pt[2]);
    }

    return rv;
}


