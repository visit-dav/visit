// ************************************************************************* //
//                              avtVMetricArea.C                             //
// ************************************************************************* //

#include "avtVMetricArea.h"

#include <vtkCellType.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>

#include <verdict.h>

#include <DebugStream.h>


// ****************************************************************************
//  Method: avtVMetricArea::Metric
//
//  Purpose:
//      Inspect an element and calculate the Area.
//
//  Arguments:
//      coords    The set of xyz points for the cell.
//      numPoints The number of xyz points for the cell.
//      type      The vtk type of the cell.
//
//  Returns:      The Area of the cell, or defaultValue if not supported.
//
//  Programmer:   Akira Haddox
//  Creation:     June 13, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Oct 17 08:07:53 PDT 2002
//    Update for new verdict interface.
//
// ****************************************************************************

double avtVMetricArea::Metric (double coords[][3], int type)
{
#ifdef HAVE_VERDICT
    switch(type)
    {
        case VTK_TRIANGLE:
            return v_tri_area(3, coords);
        
        case VTK_QUAD:
            return v_quad_area(4, coords);
    }
#endif
    return -1;
}


// ****************************************************************************
//  Method: avtVMetricArea::OperateDirectlyOnMesh
//
//  Purpose:
//      Determines if we want to speed up the operation by operating directly
//      on the mesh.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2005
//
// ****************************************************************************

bool
avtVMetricArea::OperateDirectlyOnMesh(vtkDataSet *ds)
{
    if (ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        int dims[3];
        ((vtkRectilinearGrid *) ds)->GetDimensions(dims);
        if (dims[0] > 1 && dims[1] > 1 && dims[2] == 1)
            return true;
    }

    return false;
}


// ****************************************************************************
//  Method: avtVMetricArea::MetricForWholeMesh
//
//  Purpose:
//      Determines the area for each cell in the mesh.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2005
//
// ****************************************************************************

void
avtVMetricArea::MetricForWholeMesh(vtkDataSet *ds, vtkDataArray *rv)
{
    int  i, j;

    if (ds->GetDataObjectType() != VTK_RECTILINEAR_GRID)
        EXCEPTION0(ImproperUseException);

    vtkRectilinearGrid *rg = (vtkRectilinearGrid *) ds;
    vtkDataArray *X = rg->GetXCoordinates();
    vtkDataArray *Y = rg->GetYCoordinates();
    int dims[3];
    rg->GetDimensions(dims);
    float *Xdist = new float[dims[0]-1];
    for (i = 0 ; i < dims[0]-1 ; i++)
        Xdist[i] = X->GetTuple1(i+1) - X->GetTuple1(i);
    float *Ydist = new float[dims[1]-1];
    for (i = 0 ; i < dims[1]-1 ; i++)
        Ydist[i] = Y->GetTuple1(i+1) - Y->GetTuple1(i);

    for (j = 0 ; j < dims[1]-1 ; j++)
        for (i = 0 ; i < dims[0]-1 ; i++)
        {
            int idx = j*(dims[0]-1) + i;
            float area = Xdist[i]*Ydist[j];
            rv->SetTuple1(idx, area);
        }

    delete [] Xdist;
    delete [] Ydist;
}


