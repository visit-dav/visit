// ************************************************************************* //
//                           avtLocateCellQuery.C                            //
// ************************************************************************* //

#include <avtLocateCellQuery.h>

#include <vtkBox.h>
#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkMath.h>
#include <vtkPoints.h>
#include <vtkRectilinearGrid.h>
#include <vtkUnsignedCharArray.h>
#include <vtkVisItCellLocator.h>
#include <vtkVisItUtility.h>

#include <DebugStream.h>

#include <math.h>
#include <float.h>

#include <avtParallel.h>


using     std::string;


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
// ****************************************************************************

avtLocateCellQuery::avtLocateCellQuery()
{
    foundZone = foundDomain = -1;
    minDist = +FLT_MAX;
}


// ****************************************************************************
//  Method: avtLocateCellQuery::PreExecute
//
//  Purpose:
//      This is called before any of the domains are executed.
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 15, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Jan 31 11:34:03 PST 2003 
//    Initialize minDist.
//
//    Kathleen Bonnell, Mon Apr 14 09:26:46 PDT 2003 
//    Retrieve the inverse transformation matrix if it is available and
//    can be used.
//
//    Kathleen Bonnell, Thu Apr 17 09:39:19 PDT 2003 
//    Removed code to retrieve the inverse transformation matrix. 
//
// ****************************************************************************

void
avtLocateCellQuery::PreExecute(void)
{
    foundZone = foundDomain = -1; 
    minDist = +FLT_MAX;
}


// ****************************************************************************
//  Method: avtLocateCellQuery::PostExecute
//
//  Purpose:
//      This is called after all of the domains are executed.
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 15, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Jan 31 11:34:03 PST 2003 
//    Ensure that only the processor that found the intersection point with
//    minimum distance will pass it's information to the queryAtts. 
//    Removed "message" code, as the messaging is handled in the Viewer.
//
//    Kathleen Bonnell, Tue Nov  4 08:18:54 PST 2003 
//    Use pickAtts instead of queryAtts. 
//    
// ****************************************************************************

void
avtLocateCellQuery::PostExecute(void)
{
    if (ThisProcessorHasMinimumValue(minDist))
    {
        pickAtts.SetDomain(foundDomain);
        pickAtts.SetElementNumber(foundZone);
    }
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
// ****************************************************************************

void
avtLocateCellQuery::Execute(vtkDataSet *ds, const int dom)
{
    if (ds == NULL)
    {
        return;
    }
    int dim = GetInput()->GetInfo().GetAttributes().GetSpatialDimension(); 
    float dist, isect[3] = { 0., 0., 0.};
    int foundCell;

    // Find the cell, intersection point, and distance along the ray.
    //
    if (ds->GetDataObjectType() != VTK_RECTILINEAR_GRID)
    {
        foundCell = LocatorFindCell(ds, dist, isect); 
    }
    else
    {
        foundCell = RGridFindCell(ds, dist, isect); 
    }

    if ((foundCell != -1) && (dist < minDist))
    {
        minDist = dist;

        pickAtts.SetPickPoint(isect);
        if (pickAtts.GetPickType() == PickAttributes::Node)
            DeterminePickedNode(ds, foundCell, isect);

        vtkDataArray *origCells = 
                 ds->GetCellData()->GetArray("avtOriginalCellNumbers");
    
        if (origCells)
        {
            int comp = origCells->GetNumberOfComponents() -1;
            foundZone = (int) origCells->GetComponent(foundCell, comp);
        }
        else if (GetInput()->GetInfo().GetAttributes().
                 GetContainsOriginalCells())
        {
            debug5 << "PICK PROBLEM! Info says we should have original "
                   << " cells but the array was not found in the dataset."
                   << endl;
        }
        //
        // There is no need to 'fudge' the intersection point unless 
        // avtLocateCellQuery will be using it to find the Zone number and 
        // we are in 3D.
        //
        if (foundZone == -1 && dim == 3)
        {
            vtkCell *cell = ds->GetCell(foundCell);
            float parametricCenter[3];
            float *weights = new float[cell->GetNumberOfPoints()];
            int subId = cell->GetParametricCenter(parametricCenter);
            cell->EvaluateLocation(subId, parametricCenter, isect, weights);
            delete [] weights;
        }
        pickAtts.SetCellPoint(isect);
        foundDomain = dom;
    } // if cell was found
}


// ****************************************************************************
//  Method: avtLocateCellQuery::LocatorFindCell
//
//  Purpose:
//    Uses a locator to find the cell intersected by the pick ray.
//    Ignores ghost zones.
//
//  Arguments:
//    ds      The dataset to query.
//    dist    A place to store the distance along the ray of the 
//            intersection point. 
//    isect   A place to store the intersection point of the ray with the 
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
//    Removed tolerance parameter (based on the diagonal length of the dataset)
//    in favor of a tolerance calculated from the MinimumCellLength as 
//    determined by the locator.  
//
//    Kathleen Bonnell, Wed Jun 18 17:52:45 PDT 2003 
//    Use new IntersectWithLine routine from cellLocator.  It doesn't require
//    the calculation of a tolerance. 
//    
//    Kathleen Bonnell, Thu Jun 19 16:50:41 PDT 2003  
//    Test for no cells in ds.  
//    
//    Kathleen Bonnell, Wed Jul 23 15:51:45 PDT 2003 
//    Added logic for World pick (indicated by rayPoint1 == rayPoint2). 
//    
//    Kathleen Bonnell, Tue Nov  4 15:00:04 PST 2003 
//    Use pickAtts insteead of queryAtts.
//    Specify the bounds for cellLocator to use when doing spatial decomp. 
//    
// ****************************************************************************

int
avtLocateCellQuery::LocatorFindCell(vtkDataSet *ds, float &dist, float *isect)
{
    if (ds->GetNumberOfCells() == 0)
    {
        return -1;
    }
    float *rayPt1 = pickAtts.GetRayPoint1();
    float *rayPt2 = pickAtts.GetRayPoint2();

    vtkVisItCellLocator *cellLocator = vtkVisItCellLocator::New(); 
    cellLocator->SetIgnoreGhosts(true);
    cellLocator->SetDataSet(ds);
    //
    // Cells may have been removed, and unused points may still exist,
    // giving the dataset larger bounds than just the cell bounds, so
    // tell the locator to use the actual bounds retrieved from the
    // plot that originated this query.  The locator will use these
    // bounds only if they are smaller than the dataset bounds.
    //
    cellLocator->SetUserBounds(pickAtts.GetPlotBounds());
    cellLocator->BuildLocator();

    float pcoords[3], ptLine[3];
    int subId, success;

    vtkIdType foundCell; 
    if (rayPt1[0] == rayPt2[0] &&
        rayPt1[1] == rayPt2[1] &&
        rayPt1[2] == rayPt2[2])
    {
        cellLocator->FindClosestPoint(rayPt1, ptLine, foundCell,
                                     subId, dist);
        if (foundCell >= 0)
        {
            success = 1;
            isect[0] = rayPt1[0];
            isect[1] = rayPt1[1];
            isect[2] = rayPt1[2];
        }
    }
    else 
    {
        success = cellLocator->IntersectWithLine(rayPt1, rayPt2, dist, 
                                     isect, pcoords, subId, foundCell);
    }

    cellLocator->Delete();
    if (success)
        return foundCell;
    else
        return -1;
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
// ****************************************************************************

int
avtLocateCellQuery::RGridFindCell(vtkDataSet *ds, float &dist, float *isect)
{
    vtkRectilinearGrid *rgrid = (vtkRectilinearGrid*)ds;

    int i, cellId = -1;
    float t, dsBounds[6], rayDir[3];
    float *rayPt1 = pickAtts.GetRayPoint1();
    float *rayPt2 = pickAtts.GetRayPoint2();
    int ijk[3], success = 0;

 
    rgrid->GetBounds(dsBounds);

    if (rayPt1[0] == rayPt2[0] &&
        rayPt1[1] == rayPt2[1] &&
        rayPt1[2] == rayPt2[2])
    {
        success = vtkVisItUtility::ComputeStructuredCoordinates(rgrid, rayPt1, ijk);
        if (success)
        {
            isect[0] = rayPt1[0];
            isect[1] = rayPt1[1];
            isect[2] = rayPt1[2];
            dist = 0; 
        }
    }
    else
    {
        for (i = 0; i < 3; i++)
        {
           rayDir[i] = rayPt2[i] - rayPt1[i];
        }
        if (vtkBox::IntersectBox(dsBounds, rayPt1, rayDir, isect, t))
        {
            success = vtkVisItUtility::ComputeStructuredCoordinates(rgrid, 
                          isect, ijk); 
            if (success)
            {
                dist = vtkMath::Distance2BetweenPoints(rayPt1, isect);
            }
        }
    }
    if (success)
    {
        cellId = rgrid->ComputeCellId(ijk);
        vtkUnsignedCharArray *ghosts = (vtkUnsignedCharArray *)ds->
                     GetCellData()->GetArray("vtkGhostLevels");
        if (ghosts && ghosts->GetComponent(cellId, 0) == 1 )
        {
            cellId = -1;
        }
    }
    return cellId;
}



// ****************************************************************************
//  Method: avtLocateCellQuery::DeterminePickedNode
//
//  Purpose:
//    Finds the closest node-point to the picked point.  
//
//  Arguments:
//    ds        The dataset to retrieve information from.
//    foundEl   the picked ZONE 
//    ppoint    the node's coordinates.
//
//
//  Programmer: Kathleen Bonnell  
//  Creation:   June 27, 2003 
//
//  Modifications:
//    Kathleen Bonnell, Tue Nov  4 08:18:54 PST 2003
//    Use pickAtts instead of queryAtts.
//    
// ****************************************************************************

void
avtLocateCellQuery::DeterminePickedNode(vtkDataSet *ds, int foundEl, float *ppoint)
{
   vtkIdType minId = -1; 
   vtkPoints *points = vtkVisItUtility::GetPoints(ds);

   //
   // VTK's FindPoint method is faster than the brute force method
   // in the else-part, but only for Rectilinear grids.
   // 
   if (ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
   {
       minId = ds->FindPoint(ppoint);
   }
   else
   {
       vtkIdList *ptIds = vtkIdList::New();
       vtkIdType id;
       ds->GetCellPoints(foundEl, ptIds);
       int numPts = ptIds->GetNumberOfIds();
       float dist2;
       float minDist2 = FLT_MAX;

       for (int i = 0; i < numPts; i++)
       {
           id = ptIds->GetId(i);

           dist2 = vtkMath::Distance2BetweenPoints(ppoint, 
                   points->GetPoint(id));
           if (dist2 < minDist2)
           {
               minDist2 = dist2; 
               minId = id; 
           }
       }

       ptIds->Delete();
   }

   if ( minId != -1)
       pickAtts.SetNodePoint(points->GetPoint(minId));
}


// ****************************************************************************
//  Method: avtLocateCellQuery::SetPickAtts
//
//  Purpose:
//      Sets the pickAtts to the passed values. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 4, 2003
//
// ****************************************************************************

void
avtLocateCellQuery::SetPickAtts(const PickAttributes *pa)
{
    pickAtts =  *pa;
}


// ****************************************************************************
//  Method: avtLocateCellQuery::GetPickAtts
//
//  Purpose:
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 4, 2003
//
// ****************************************************************************

const PickAttributes *
avtLocateCellQuery::GetPickAtts() 
{
    return &pickAtts; 
}

