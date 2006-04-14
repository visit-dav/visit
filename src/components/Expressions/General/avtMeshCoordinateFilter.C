// ************************************************************************* //
//                          avtMeshCoordinateFilter.C                       //
// ************************************************************************* //

#include <avtMeshCoordinateFilter.h>

#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>


// ****************************************************************************
//  Method: avtMeshCoordinateFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtMeshCoordinateFilter::avtMeshCoordinateFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtMeshCoordinateFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtMeshCoordinateFilter::~avtMeshCoordinateFilter()
{
    ;
}


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
//      Hank Childs, Fri Mar 31 08:49:06 PST 2006
//      Add special handling for rectilinear grids.
//
// ****************************************************************************

vtkDataArray *
avtMeshCoordinateFilter::DeriveVariable(vtkDataSet *in_ds)
{
    int npts = in_ds->GetNumberOfPoints();

    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetNumberOfComponents(3);
    rv->SetNumberOfTuples(npts);

    if (in_ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        vtkRectilinearGrid *rg = (vtkRectilinearGrid *) in_ds;
        int dims[3];
        rg->GetDimensions(dims);
        float *X = (float *) rg->GetXCoordinates()->GetVoidPointer(0);
        float *Y = (float *) rg->GetYCoordinates()->GetVoidPointer(0);
        float *Z = (float *) rg->GetZCoordinates()->GetVoidPointer(0);
        float *ptr = rv->GetPointer(0);
        for (int k = 0 ; k < dims[2] ; k++)
            for (int j = 0 ; j < dims[1] ; j++)
                for (int i = 0 ; i < dims[0] ; i++)
                {
                    *ptr++ = X[i];
                    *ptr++ = Y[j];
                    *ptr++ = Z[k];
                }
    }
    else
    {
        for (int i = 0 ; i < npts ; i++)
        {
            double pt[3];
            in_ds->GetPoint(i, pt);
            rv->SetTuple3(i, pt[0], pt[1], pt[2]);
        }
    }

    return rv;
}


