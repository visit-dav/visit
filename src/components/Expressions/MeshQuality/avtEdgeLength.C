// ************************************************************************* //
//                               avtEdgeLength.C                             //
// ************************************************************************* //

#include <avtEdgeLength.h>

#include <float.h>

#include <vtkCell.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkPoints.h>

#include <avtCallback.h>

#include <InvalidDimensionsException.h>


// ****************************************************************************
//  Method: avtEdgeLength constructor
//
//  Programmer: Hank Childs
//  Creation:   September 22, 2005
//
// ****************************************************************************

avtEdgeLength::avtEdgeLength()
{
    takeMin = true;
}


// ****************************************************************************
//  Method: avtEdgeLength::DeriveVariable
//
//  Purpose:
//      Find the minimum or maximum edge length.
//
//  Programmer: Hank Childs
//  Creation:   September 22, 2005
//
// ****************************************************************************

vtkDataArray *
avtEdgeLength::DeriveVariable(vtkDataSet *in_ds)
{
    vtkFloatArray *arr = vtkFloatArray::New();
    int ncells = in_ds->GetNumberOfCells();
    arr->SetNumberOfTuples(ncells);

    for (int i = 0 ; i < ncells ; i++)
    {
        vtkCell *cell = in_ds->GetCell(i);
        float vol = (float) GetEdgeLength(cell);
        arr->SetTuple(i, &vol);
    }

    return arr;
}


// ****************************************************************************
//  Method: avtEdgeLength::GetEdgeLength
//
//  Purpose:
//      Evaluate the length of each edge and determine the minimum or
//      maximum.
//
//  Arguments:
//      cell    The input zone.
//
//  Returns:    The side volume of the zone.
//
//  Programmer: Hank Childs
//  Creation:   September 22, 2005
//
// ****************************************************************************
 
double
avtEdgeLength::GetEdgeLength(vtkCell *cell)
{
    int  i, j;

    //
    // Calculate the value of each edge and then return the minimum or maximum.
    //
    int nEdges = cell->GetNumberOfEdges();
    double rv = +FLT_MAX;
    if (!takeMin)
        rv = -FLT_MAX;
    for (i = 0 ; i < nEdges ; i++)
    {
        vtkCell *edge = cell->GetEdge(i);
        vtkPoints *pts = edge->GetPoints();
        float *pts_ptr = (float *) pts->GetVoidPointer(0);
        int npts = edge->GetNumberOfPoints();

        double a[3];
        int id1 = 0;
        int id2 = 1;
        a[0] = pts_ptr[3*id2] - pts_ptr[3*id1];
        a[1] = pts_ptr[3*id2+1] - pts_ptr[3*id1+1];
        a[2] = pts_ptr[3*id2+2] - pts_ptr[3*id1+2];
        double length = sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
        if (takeMin)
            rv = (rv < length ? rv : length);
        else
            rv = (rv > length ? rv : length);
    }

    return rv;
}


