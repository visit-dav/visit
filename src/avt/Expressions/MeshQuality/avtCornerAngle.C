// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtCornerAngle.C                             //
// ************************************************************************* //

#include <avtCornerAngle.h>

#include <float.h>

#include <vtkCell.h>
#include <vtkDataSet.h>
#include <vtkPoints.h>

#include <avtCallback.h>

#include <InvalidDimensionsException.h>


// ****************************************************************************
//  Method: avtCornerAngle constructor
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2008
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

avtCornerAngle::avtCornerAngle()
{
    takeMin = true;
}


// ****************************************************************************
//  Method: avtCornerAngle::DeriveVariable
//
//  Purpose:
//      Find the minimum or maximum corner angle.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2008
//
// ****************************************************************************

vtkDataArray *
avtCornerAngle::DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex)
{
    vtkDataArray *arr = CreateArrayFromMesh(in_ds);
    vtkIdType ncells = in_ds->GetNumberOfCells();
    arr->SetNumberOfTuples(ncells);

    for (vtkIdType i = 0 ; i < ncells ; i++)
    {
        vtkCell *cell = in_ds->GetCell(i);
        double vol = GetCornerAngle(cell);
        arr->SetTuple1(i, vol);
    }

    return arr;
}


// ****************************************************************************
//  Method: avtCornerAngle::GetCornerAngle
//
//  Purpose:
//      Evaluate the corner angle and determine the minimum or
//      maximum.
//
//  Arguments:
//      cell    The input zone.
//
//  Returns:    The corner angle of the zone.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2008
//
// ****************************************************************************
 
double
avtCornerAngle::GetCornerAngle(vtkCell *cell)
{
    int celltype = cell->GetCellType();
    if (celltype == VTK_VERTEX || celltype == VTK_LINE)
        return 0.;

    //
    // Calculate the value of each edge and then return the minimum or maximum.
    //
    vtkIdType nPts = cell->GetNumberOfPoints();
    int nEdges = cell->GetNumberOfEdges();
    double rv = +FLT_MAX;
    if (!takeMin)
        rv = -FLT_MAX;

    for (vtkIdType i = 0 ; i < nPts ; i++)
    {
        vtkIdType ID = cell->GetPointId(i);
        double ptV[3];
        cell->GetPoints()->GetPoint(i, ptV);
        double A[4][3];

        // Identify the edges adjacent to this vertex.
        int numEdges = 0;
        for (int j = 0 ; j < nEdges ; j++)
        {
            vtkCell *edge = cell->GetEdge(j);
            int match = -1;
            if (edge->GetPointId(0) == ID)
                match = 1;
            if (edge->GetPointId(1) == ID)
                match = 0;
            if (match < 0)
                continue;

            // Guard against 4 edges from a vertex.
            if (numEdges > 3)
                return 0.;

            double ptA[3];
            edge->GetPoints()->GetPoint(match, ptA);
            A[numEdges][0] = ptA[0] - ptV[0];
            A[numEdges][1] = ptA[1] - ptV[1];
            A[numEdges][2] = ptA[2] - ptV[2];
            double amag = sqrt(A[numEdges][0]*A[numEdges][0] +
                               A[numEdges][1]*A[numEdges][1] + 
                               A[numEdges][2]*A[numEdges][2]);
            if (amag != 0.)
            {
                A[numEdges][0] /= amag;
                A[numEdges][1] /= amag;
                A[numEdges][2] /= amag;
            }

            numEdges++;
        }

        double angle = 0.;
        if (numEdges == 2)
        {
            double dot = A[0][0]*A[1][0] + A[0][1]*A[1][1] + A[0][2]*A[1][2];
            angle = acos(dot);
        }
        else if (numEdges == 3)
        {
            // According to Tony De Groot, we want this to be asin(sqrt(AxB.C)).
            double cross[3];
            cross[0] = A[0][1]*A[1][2] - A[0][2]*A[1][1];
            cross[1] = A[0][2]*A[1][0] - A[0][0]*A[1][2];
            cross[2] = A[0][0]*A[1][1] - A[0][1]*A[1][0];
            double dot = A[2][0]*cross[0] + A[2][1]*cross[1] + A[2][2]*cross[2];
            double root = sqrt(dot);
            angle = asin(root);
        }

        if (takeMin)
            rv = (rv < angle ? rv : angle);
        else
            rv = (rv > angle ? rv : angle);
    }

    return rv;
}


