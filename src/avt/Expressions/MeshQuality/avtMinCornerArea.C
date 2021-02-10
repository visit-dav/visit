// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtMinCornerArea.C                           //
// ************************************************************************* //

#include <avtMinCornerArea.h>

#include <float.h>

#include <vtkCell.h>
#include <vtkDataSet.h>
#include <vtkPoints.h>

#include <avtCallback.h>

#include <InvalidDimensionsException.h>


// ****************************************************************************
//  Method: avtMinCornerArea constructor
//
//  Programmer: Matthew Wheeler
//  Creation:   20 May, 2013
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

avtMinCornerArea::avtMinCornerArea()
{
    orderCCW = true;
}


// ****************************************************************************
//  Method: avtMinCornerArea::DeriveVariable
//
//  Purpose:
//      Find the minimum Relative Corner Areas for each zone of a 2D mesh.
//
//  Programmer: Matthew Wheeler
//  Creation:   20 May, 2013
//
// ****************************************************************************

vtkDataArray *
avtMinCornerArea::DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex)
{
    vtkDataArray *arr = CreateArrayFromMesh(in_ds);
    vtkIdType ncells = in_ds->GetNumberOfCells();
    arr->SetNumberOfTuples(ncells);

    for (vtkIdType i = 0 ; i < ncells ; i++)
    {
        vtkCell *cell = in_ds->GetCell(i);
        double mcar = GetMinCornerArea(cell);
        arr->SetTuple1(i, mcar);
    }

    return arr;
}


// ****************************************************************************
//  Method: avtMinCornerArea::GetMinCornerArea
//
//  Purpose:
//      Evaluate the Relative Corner Areas and determine the minimum.
//
//  Arguments:
//      cell    The input zone of a 2D mesh.
//
//  Returns:    The Minimum Relative Corner Area of the zone.
//              Require answer in the range 0.0-1.0
//              where 0.0 denotes poor cell quality and 1.0 is good.
//              Zero also returned for pathological cases.
//
//  Comments:   Relative Corner Area:
//              = Area bounded by each Corner and Intercept of Mid-Points
//                / Total Area of the Zone.
//              Calculated from node vertices using parametric formulae.
//
//              This version assumes Nodes are supplied in a cyclic order -
//              (generally Counter-Clockwise but Clockwise also works)
//
//  Programmer: Matthew Wheeler
//  Creation:   20 May, 2013
//
// ****************************************************************************

double
avtMinCornerArea::GetMinCornerArea(vtkCell *cell)
{
    double rv = 0.0;

    int celltype = cell->GetCellType();
    if (celltype != VTK_QUAD)
        return rv;

    //
    // Get the Cell parameters:
    //
    vtkIdType nPts = cell->GetNumberOfPoints();
    // Return zero for anything other than a quad
    if (nPts != 4 )
        return rv;

    double ptV[3];
    double ptX[4], ptY[4];

    for (vtkIdType i = 0 ; i < nPts ; i++)
    {
        cell->GetPoints()->GetPoint(i, ptV);
        ptX[i] = ptV[0];
        ptY[i] = ptV[1];
    }

    double dx20, dx31, dx03, dx10, dx21, dx32;

    dx20 = ptX[2] - ptX[0];
    dx31 = ptX[3] - ptX[1];

    double CellAreaTimes2, Cscale;

    CellAreaTimes2 = dx20*(ptY[3] - ptY[1])  -  dx31*(ptY[2] - ptY[0]);

    // return 0.0 if Cell Area is practically zero
    if (fabs(CellAreaTimes2) <= FLT_MIN)
        return rv;
    else
        Cscale = 2.0/CellAreaTimes2;

    dx03 = ptX[0] - ptX[3];
    dx10 = ptX[1] - ptX[0];
    dx21 = ptX[2] - ptX[1];
    dx32 = ptX[3] - ptX[2];

    // Relative Corner Area calculations:
    double CA[4], CAMin;

    CA[0] = (ptY[0]*dx31  + ptY[1]*dx03 + ptY[3]*dx10) * Cscale;

    CA[1] = (ptY[1]*-dx20 + ptY[2]*dx10 + ptY[0]*dx21) * Cscale;

    CA[2] = (ptY[2]*-dx31 + ptY[3]*dx21 + ptY[1]*dx32) * Cscale;

    CA[3] = (ptY[3]*dx20  + ptY[0]*dx32 + ptY[2]*dx03) * Cscale;


    // What is the Minimum of these Corner Areas?

    CAMin = CA[0];
    for (int i = 1 ; i < 4 ; i++)
        if (CA[i] < CAMin)
            CAMin = CA[i];

    // Require answer in the range 0.0-1.0
    // Return 0.0 for anything negative.
    if (CAMin > 0.0)
        rv = CAMin;

    return rv;
}
