// ************************************************************************* //
//                           avtLocateCellQuery.C                            //
// ************************************************************************* //

#include <avtLocateCellQuery.h>

#include <vtkDataSet.h>
#include <vtkVisItCellLocator.h>
#include <vtkUnsignedCharArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkVisItUtility.h>
#include <vtkMath.h>
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
// ****************************************************************************

void
avtLocateCellQuery::PostExecute(void)
{
    if (ThisProcessorHasMinimumValue(minDist))
    {
        queryAtts.SetDomain(foundDomain);
        queryAtts.SetZone(foundZone);
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

        queryAtts.SetWorldPoint(isect);

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
        // avtPickQuery will be using it to find the Zone number and 
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
        queryAtts.SetCellPoint(isect);
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
// ****************************************************************************

int
avtLocateCellQuery::LocatorFindCell(vtkDataSet *ds, float &dist, float *isect)
{
    float *rayPt1 = queryAtts.GetRayPoint1();
    float *rayPt2 = queryAtts.GetRayPoint2();

    vtkVisItCellLocator *cellLocator = vtkVisItCellLocator::New(); 
    cellLocator->SetIgnoreGhosts(true);
    cellLocator->SetDataSet(ds);
    cellLocator->BuildLocator();

    float pcoords[3];
    int subId;

    vtkIdType foundCell; 
    int success = cellLocator->IntersectWithLine(rayPt1, rayPt2, dist, 
                                     isect, pcoords, subId, foundCell);

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
// ****************************************************************************

int
avtLocateCellQuery::RGridFindCell(vtkDataSet *ds, float &dist, float *isect)
{
    vtkRectilinearGrid *rgrid = (vtkRectilinearGrid*)ds;

    int i, cellId = -1;
    float t, dsBounds[6], rayDir[3];
    float *rayPt1 = queryAtts.GetRayPoint1();
    float *rayPt2 = queryAtts.GetRayPoint2();
 
    rgrid->GetBounds(dsBounds);
 
    for (i = 0; i < 3; i++)
    {
       rayDir[i] = rayPt2[i] - rayPt1[i];
    }
    if (vtkCell::HitBBox(dsBounds, rayPt1, rayDir, isect, t))
    {
        int ijk[3];

        if (vtkVisItUtility::ComputeStructuredCoordinates(rgrid, isect, ijk) == 1)
        {
            cellId = rgrid->ComputeCellId(ijk);
            vtkUnsignedCharArray *ghosts =
                (vtkUnsignedCharArray *)ds->GetCellData()->GetArray("vtkGhostLevels");
            if (ghosts && ghosts->GetComponent(cellId, 0) == 1 )
            {
                cellId = -1;
            }
            else
            {
                dist = vtkMath::Distance2BetweenPoints(rayPt1, isect);
            }
        }
    }
    return cellId;
}

