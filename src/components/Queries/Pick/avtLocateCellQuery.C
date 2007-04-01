// ************************************************************************* //
//                           avtLocateCellQuery.C                            //
// ************************************************************************* //

#include <avtLocateCellQuery.h>

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkRectilinearGrid.h>
#include <vtkUnsignedCharArray.h>
#include <vtkVisItUtility.h>

#include <avtTerminatingSource.h>
#include <DebugStream.h>

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

        vtkDataArray *origCells = 
                 ds->GetCellData()->GetArray("avtOriginalCellNumbers");

        if (origCells)
        {
            int comp = origCells->GetNumberOfComponents() -1;
            foundElement = (int) origCells->GetComponent(foundCell, comp);
        }
        else if (GetInput()->GetInfo().GetAttributes().
                 GetContainsOriginalCells())
        {
            debug5 << "PICK PROBLEM! Info says we should have original "
                   << " cells but the array was not found in the dataset."
                   << endl;
        }
        else if (GetInput()->GetInfo().GetValidity().GetZonesPreserved() &&
                 GetInput()->GetInfo().GetAttributes().GetContainsGhostZones() 
                    != AVT_CREATED_GHOSTS)
        {
            foundElement = foundCell;
        }

        //
        // There is no need to 'fudge' the intersection point unless 
        // avtLocateCellQuery will be using it to find the Zone number and 
        // we are in 3D.
        //
        if (foundElement == -1 && dim == 3)
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
// ****************************************************************************

int
avtLocateCellQuery::RGridFindCell(vtkDataSet *ds, float &dist, float *isect)
{
    vtkRectilinearGrid *rgrid = vtkRectilinearGrid::SafeDownCast(ds);

    int ijk[3], cellId = -1;
 
    if (RGridIsect(rgrid, dist, isect, ijk))
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

