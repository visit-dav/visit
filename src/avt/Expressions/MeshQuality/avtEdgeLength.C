// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtEdgeLength.C                             //
// ************************************************************************* //

#include <avtEdgeLength.h>

#include <float.h>

#include <vtkCell.h>
#include <vtkDataSet.h>
#include <vtkPoints.h>

#include <avtCallback.h>

#include <InvalidDimensionsException.h>


// ****************************************************************************
//  Method: avtEdgeLength constructor
//
//  Programmer: Hank Childs
//  Creation:   September 22, 2005
//
//  Modifications:
//
//    Alister Maguire, Thu Jun 18 10:02:58 PDT 2020
//    Set canApplyToDirectDatabaseQOT to false.
//
//    Alister Maguire, Fri Nov  6 08:53:20 PST 2020
//    Removed direct database QOT disabler as they are now compatible.
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
avtEdgeLength::DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex)
{
    vtkDataArray *arr = CreateArrayFromMesh(in_ds);
    vtkIdType ncells = in_ds->GetNumberOfCells();
    arr->SetNumberOfTuples(ncells);

    for (vtkIdType i = 0 ; i < ncells ; i++)
    {
        vtkCell *cell = in_ds->GetCell(i);
        double vol = GetEdgeLength(cell);
        arr->SetTuple1(i, vol);
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
//  Modifications:
//
//    Hank Childs, Fri Jun  9 14:34:50 PDT 2006
//    Removed unused variable.
//
//    Hank Childs, Fri Nov  3 16:05:12 PST 2006
//    Add support for actual edges.  Also have vertices return 0.
//
// ****************************************************************************
 
double
avtEdgeLength::GetEdgeLength(vtkCell *cell)
{
    int celltype = cell->GetCellType();
    if (celltype == VTK_VERTEX)
        return 0.;

    if (celltype == VTK_LINE)
    {
        double pt1[3], pt2[3], a[3];

        vtkPoints *pts = cell->GetPoints();
        pts->GetPoint(0, pt1);
        pts->GetPoint(1, pt2);
        a[0] = pt2[0] - pt1[0]; 
        a[1] = pt2[1] - pt1[1]; 
        a[2] = pt2[2] - pt1[2]; 

        double length = sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
        return length;
    }

    //
    // Calculate the value of each edge and then return the minimum or maximum.
    //
    int nEdges = cell->GetNumberOfEdges();
    double rv = +FLT_MAX;
    if (!takeMin)
        rv = -FLT_MAX;
    for (int i = 0 ; i < nEdges ; i++)
    {
        vtkCell *edge = cell->GetEdge(i);
        vtkPoints *pts = edge->GetPoints();

        double pt1[3], pt2[3], a[3];

        pts->GetPoint(0, pt1);
        pts->GetPoint(1, pt2);

        a[0] = pt2[0] - pt1[0]; 
        a[1] = pt2[1] - pt1[1]; 
        a[2] = pt2[2] - pt1[2]; 

        double length = sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
        if (takeMin)
            rv = (rv < length ? rv : length);
        else
            rv = (rv > length ? rv : length);
    }

    return rv;
}


