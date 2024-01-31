// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtVMetricArea.C                             //
// ************************************************************************* //

#include "avtVMetricArea.h"

#include <vtkCellType.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkRectilinearGrid.h>

#include <verdict.h>


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
//    Eddie Rusu, Wed Feb 19 16:33:46 PST 2020
//    Returns 0 instead of -1 if no verdict or unchecked cell.
//
// ****************************************************************************

double avtVMetricArea::Metric (double coords[][3], int type)
{
    double out_value = 0.0;
#ifdef HAVE_VERDICT
    switch(type)
    {
        case VTK_TRIANGLE:
            out_value = v_tri_area(3, coords);
            break;
        case VTK_QUAD:
            out_value = v_quad_area(4, coords);
            break;
        default:
            out_value = 0.;
    }
#endif
    return out_value;
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
    if (ds->GetDataObjectType() != VTK_RECTILINEAR_GRID)
        EXCEPTION0(ImproperUseException);

    vtkRectilinearGrid *rg = (vtkRectilinearGrid *) ds;
    vtkDataArray *X = rg->GetXCoordinates();
    vtkDataArray *Y = rg->GetYCoordinates();
    int dims[3];
    rg->GetDimensions(dims);
    double *Xdist = new double[dims[0]-1];
    for (int i = 0 ; i < dims[0]-1 ; i++)
        Xdist[i] = X->GetTuple1(i+1) - X->GetTuple1(i);
    double *Ydist = new double[dims[1]-1];
    for (int i = 0 ; i < dims[1]-1 ; i++)
        Ydist[i] = Y->GetTuple1(i+1) - Y->GetTuple1(i);

    for (int j = 0 ; j < dims[1]-1 ; j++)
        for (int i = 0 ; i < dims[0]-1 ; i++)
        {
            int idx = j*(dims[0]-1) + i;
            double area = Xdist[i]*Ydist[j];
            rv->SetTuple1(idx, area);
        }

    delete [] Xdist;
    delete [] Ydist;
}


