// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtLocateCellQuery.C                            //
// ************************************************************************* //

#include <avtLocateCellQuery.h>

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkRectilinearGrid.h>
#include <vtkUnsignedCharArray.h>

#include <avtOriginatingSource.h>
#include <DebugStream.h>
#include <vtkVisItCellLocator.h>
#include <vtkVisItUtility.h>

#include <math.h>
#include <float.h>


// ****************************************************************************
//  Method: avtLocateCellQuery constructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 15, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Jan 31 11:34:03 PST 2003
//    Initialize minDist.
//
//    Kathleen Bonnell, Mon Apr 14 09:43:11 PDT 2003
//    Initialize invTransform.
//
//    Kathleen Bonnell, Thu Apr 17 09:39:19 PDT 2003
//    Removed invTransform.
//
//    Kathleen Bonnell, Wed Jun  2 10:21:50 PDT 2004
//    Moved code to new parent class, avtLocateQuery.
//
// ****************************************************************************

avtLocateCellQuery::avtLocateCellQuery()
{
}


// ****************************************************************************
//  Method: avtLocateCellQuery destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtLocateCellQuery::~avtLocateCellQuery()
{
    ;
}


// ****************************************************************************
//  Method: avtLocateCellQuery::Execute
//
//  Purpose:
//      Processes a single domain.
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 15, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Jan 31 11:34:03 PST 2003
//    Use ray endpoints to determine intersection point, via IntersectWithLine.
//
//    Kathleen Bonnell, Wed Mar 26 13:03:54 PST 2003
//    Test if originalCells is set in DataAttributes, to indicate possible
//    error condition if the array is not found in the dataset.
//
//    Kathleen Bonnell, Mon Apr 14 09:43:11 PDT 2003
//    Use inverse transformation matrix, if available, to determine true
//    intersection point with original data.  Precludes necessity of using
//    avtOriginalCellsArray.
//
//    Kathleen Bonnell, Thu Apr 17 09:39:19 PDT 2003
//    Don't use the transform matrix here, just test for its availability.
//
//    Kathleen Bonnell, Wed May  7 13:06:55 PDT 2003
//    For efficiency, do not use the cell locator for rectilinear grids.
//
//    Kathleen Bonnell, Tue Jun  3 15:20:35 PDT 2003
//    Removed calculation of tolerance, and passing of that value to FindCell
//    methods.  No longer use tolerance when calculating fudgedIsect.
//
//    Kathleen Bonnell, Wed Jun 18 17:52:45 PDT 2003
//    Always use OriginalCellsArray if present.
//
//    Kathleen Bonnell, Fri Oct 10 11:45:24 PDT 2003
//    Determine the picked node if in 'NodePick' mode.
//
//    Kathleen Bonnell, Tue Nov  4 08:18:54 PST 2003
//    Use pickAtts instead of queryAtts.
//
//    Kathleen Bonnell, Thu May  6 14:28:00 PDT 2004
//    Set foundElement (used to set pickAtts.ElementNumber) to the foundCell
//    if the zones have not been invalidated (ZonesPreserved).
//
//    Kathleen Bonnell, Tue May 18 13:12:09 PDT 2004
//    Move node-specific code to avtLocateNodeQuery.
//
//    Kathleen Bonnell, Wed Jul  7 14:48:44 PDT 2004
//    Added call to FindClosestCell for line data.
//
//    Kathleen Bonnell, Tue Oct  5 14:02:31 PDT 2004
//    Terminate early if the ray doesn't intersect the dataset.
//
//    Kathleen Bonnell, Wed Oct 20 17:10:21 PDT 2004
//    Use vtkVisItUtility method for computing cell center.
//
//    Kathleen Bonnell, Fri Jul  8 14:15:21 PDT 2005
//    Added another option when zones are preserved -- to determine
//    whether the ghost zones are mixed (AMR & other types) or simply AMR --
//    will be used by PickQuery in determining the correct zone id to display
//    to the user.
//
//    Kathleen Bonnell, Wed Aug 10 17:00:58 PDT 2005
//    Added some 'canUseCells' tests for whether or not to set the
//    element number or determine cell center -- Added mostly because
//    these things shouldn't be done if IndexSelected (the only operator
//    currently to set this flag) -- the origCell may be invalid, the
//    current cell number is most likely invalid, and the cell center is
//    probably invalid as well, so defer the determination (other than
//    domain and intersection point) until the Pick portion.
//
//    Kathleen Biagas, Thu Jun 29 13:02:07 PDT 2017
//    Remove constraint that lines should only be 2D spatially.
//
//    Kathleen Biagas, Tue Sep 14 09:48:24 PDT 2021
//    Don't use Rectilinear grid fast-path if there are exterior boundary
//    ghosts. They would need to be removed and it takes longer to remove them
//    than to use the slower path.  Resolves pick failure on rectilinear grids
//    with ghosts external to problem completely surrounding real zones.
//
// ****************************************************************************

void
avtLocateCellQuery::Execute(vtkDataSet *ds, const int dom)
{
    if (ds == NULL)
    {
        return;
    }

    if (!RayIntersectsDataSet(ds))
    {
        return;
    }

    avtDataObjectInformation &info = GetInput()->GetInfo();
    avtDataAttributes &dataAtts = info.GetAttributes();
    int spatdim = dataAtts.GetSpatialDimension();
    int topodim = dataAtts.GetTopologicalDimension();


    double dist = minDist, isect[3] = { 0., 0., 0.};
    int foundCell = -1;

    // Find the cell, intersection point, and distance along the ray.
    // Don't use the RectilinearGrid fast-path if there are ghosts, as it
    // takes longer to remove the ghosts than to use the fast-path.
    if ( ds->GetDataObjectType() != VTK_RECTILINEAR_GRID ||
         dataAtts.GetContainsExteriorBoundaryGhosts())
    {
        if (topodim == 1) // Lines
        {
            if (spatdim == 2)
              foundCell = FindClosestCell(ds, dist, isect);
            else if (spatdim == 3)
              foundCell = ClosestLineToLine(ds, false, dist, isect);
        }
        else
        {
            foundCell = LocatorFindCell(ds, dist, isect);
        }
    }
    else
    {
        foundCell = RGridFindCell(ds, dist, isect);
    }
    if ((foundCell != -1) && (dist < minDist))
    {
        minDist = dist;
        pickAtts.SetPickPoint(isect);

        vtkDataArray *origCells =
                 ds->GetCellData()->GetArray("avtOriginalCellNumbers");
        bool canUseCells = dataAtts.CanUseOrigZones();

        if (canUseCells && origCells)
        {
            int comp = origCells->GetNumberOfComponents() -1;
            foundElement = (int) origCells->GetComponent(foundCell, comp);
        }
        else if (dataAtts.GetContainsOriginalCells() && !origCells)
        {
            debug4 << "PICK PROBLEM! Info says we should have original "
                   << "cells but the array was not found in the dataset."
                   << endl;
        }
        else if (info.GetValidity().GetZonesPreserved())
        {
            if (dataAtts.GetContainsGhostZones() != AVT_CREATED_GHOSTS)
            {
                if (canUseCells)
                    foundElement = foundCell;
            }
            else
            {
                pickAtts.SetHasMixedGhostTypes(
                    vtkVisItUtility::ContainsMixedGhostZoneTypes(ds));
            }
        }

        //
        // There is no need to 'fudge' the intersection point unless
        // avtLocateCellQuery will be using it to find the Zone number and
        // we are in 3D.
        //
        if (foundElement == -1 && spatdim == 3 && canUseCells)
        {
            vtkVisItUtility::GetCellCenter(ds->GetCell(foundCell), isect);
        }
        pickAtts.SetCellPoint(isect);
        foundDomain = dom;
    } // if cell was found
}


// ****************************************************************************
//  Method: avtLocateCellQuery::RGridFindCell
//
//  Purpose:
//    Uses rectilinear-grid specific code to find the cell intersected by
//    the pick ray.  Ignores ghost zones.
//
//  Arguments:
//    ds      The dataset to query.
//    dist    A place to store the distance along the ray of the
//            intersection point.
//    isect   A place to store the intersetion point of the ray with the
//            dataset.
//
//  Returns:
//    The id of the cell that was intersected (-1 if none found).
//
//  Programmer: Kathleen Bonnell
//  Creation:   May 7, 2003
//
//  Modifications:
//    Kathleen Bonnell, Tue Jun  3 15:20:35 PDT 2003
//    Removed unused tolerance parameter.
//
//    Kathleen Bonnell, Wed Jul 23 15:51:45 PDT 2003
//    Added logic for World pick (indicated by rayPoint1 == rayPoint2).
//
//    Kathleen Bonnell, Tue Nov  4 08:18:54 PST 2003
//    Use pickAtts instead of queryAtts.
//
//    Kathleen Bonnell, Thu Feb  5 16:17:48 PST 2004
//    Replaced vtkCell::HitBBox with vtkBox::IntersectBox, as the old
//    method is soon to be obsoleted in vtk.
//
//    Kathleen Bonnell, Wed Jun  2 10:21:50 PDT 2004
//    Moved Isect code to RGridIsect.
//
//    Hank Childs, Fri Aug 27 16:02:58 PDT 2004
//    Rename ghost data array.
//
//    Kathleen Bonnell, Thu Oct 21 18:02:50 PDT 2004
//    Correct test for whether a cell is ghost or not.
//
//    Kathleen Bonnell, Thu Jun 23 11:33:53 PDT 2005
//    Moved ghost-checking code to method RGridIsect, compressed code.
//
// ****************************************************************************

int
avtLocateCellQuery::RGridFindCell(vtkDataSet *ds, double &dist, double *isect)
{
    return RGridIsect(vtkRectilinearGrid::SafeDownCast(ds), dist, isect);
}


// ****************************************************************************
//  Method: avtLocateQuery::FindClosestCell
//
//  Purpose:
//    Uses a locator to find the closest cell to the given point.
//
//  Arguments:
//    ds      The dataset to query.
//    dist    The current minimum distance.
//    isect   The intersection point.
//
//  Returns:
//    The id of the closest cell (-1 if none found).
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 7, 2004
//
//  Modifications:
//    Kathleen Biagas, Fri Jul  7 09:35:22 PDT 2017
//    Changed arg from minDist to dist, to avoid overshadowing member var.
//
// ****************************************************************************

int
avtLocateCellQuery::FindClosestCell(vtkDataSet *ds, double &dist, double isect[3])
{
    if (ds->GetNumberOfPoints() == 0)
    {
        return -1;
    }
    vtkIdType foundCell = -1;
    double *rayPt1 = pickAtts.GetRayPoint1();
    double pt[3] = {rayPt1[0], rayPt1[1], 0.};
    double dist2, rad = dist;

    vtkVisItCellLocator *cellLocator = vtkVisItCellLocator::New();
    cellLocator->SetDataSet(ds);
    cellLocator->IgnoreGhostsOn();
    cellLocator->BuildLocator();

    int subId = 0;
    double cp[3] = {0., 0., 0.};
    int success = cellLocator->FindClosestPointWithinRadius(pt, rad, cp,
                                   foundCell, subId, dist2);

    if (success == 1 && dist2 < dist)
    {
        isect[0] = cp[0];
        isect[1] = cp[1];
        isect[2] = cp[2];
        dist = dist2;
    }

    cellLocator->Delete();
    return foundCell;
}
