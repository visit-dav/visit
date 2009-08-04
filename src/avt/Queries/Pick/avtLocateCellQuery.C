/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                           avtLocateCellQuery.C                            //
// ************************************************************************* //

#include <avtLocateCellQuery.h>

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkRectilinearGrid.h>
#include <vtkUnsignedCharArray.h>
#include <vtkVisItCellLocator.h>
#include <vtkVisItUtility.h>

#include <avtOriginatingSource.h>
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
    int dim     = dataAtts.GetSpatialDimension(); 
    int topodim = dataAtts.GetTopologicalDimension(); 


    double dist = minDist, isect[3] = { 0., 0., 0.};
    int foundCell = -1;

    // Find the cell, intersection point, and distance along the ray.
    //
    if (ds->GetDataObjectType() != VTK_RECTILINEAR_GRID)
    {
        if (topodim == 1 && dim == 2) // Lines
            foundCell = FindClosestCell(ds, dist, isect);
        else 
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
        bool canUseCells = dataAtts.CanUseOrigZones();

        if (canUseCells && origCells)
        {
            int comp = origCells->GetNumberOfComponents() -1;
            foundElement = (int) origCells->GetComponent(foundCell, comp);
        }
        else if (canUseCells && dataAtts.GetContainsOriginalCells())
        {
            debug5 << "PICK PROBLEM! Info says we should have original "
                   << " cells but the array was not found in the dataset."
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
        if (foundElement == -1 && dim == 3 && canUseCells)
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
//    minDist The current minimum distance.
//    isect   The intersection point. 
//
//  Returns:
//    The id of the closest cell (-1 if none found).
//
//  Programmer: Kathleen Bonnell  
//  Creation:   July 7, 2004 
//
//  Modifications:
//    
// ****************************************************************************

int
avtLocateCellQuery::FindClosestCell(vtkDataSet *ds, double &minDist, double isect[3])
{
    if (ds->GetNumberOfPoints() == 0)
    {
        return -1;
    }
    int foundCell = -1;
    double *rayPt1 = pickAtts.GetRayPoint1();
    double pt[3] = {rayPt1[0], rayPt1[1], 0.};
    double dist, rad = minDist;

    vtkVisItCellLocator *cellLocator = vtkVisItCellLocator::New();
    cellLocator->SetDataSet(ds);
    cellLocator->IgnoreGhostsOn();
    cellLocator->BuildLocator();

    int subId = 0;
    double cp[3] = {0., 0., 0.};
    int success = cellLocator->FindClosestPointWithinRadius(pt, rad, cp, 
                               foundCell, subId, dist);

    if (success == 1 && dist < minDist)
    {
        isect[0] = cp[0];
        isect[1] = cp[1];
        isect[2] = cp[2];
        minDist = dist;
    }

    cellLocator->Delete();

    return foundCell;
}
