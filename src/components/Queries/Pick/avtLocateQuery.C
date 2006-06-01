/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                             avtLocateQuery.C                              //
// ************************************************************************* //

#include <avtLocateQuery.h>

#include <float.h>
#include <vtkBox.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkMath.h>
#include <vtkRectilinearGrid.h>
#include <vtkVisItCellLocator.h>
#include <vtkVisItUtility.h>
#include <NonQueryableInputException.h>


#include <avtParallel.h>



// ****************************************************************************
//  Method: avtLocateQuery constructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   May 18, 2004 
//
//  Modifications:
//
// ****************************************************************************

avtLocateQuery::avtLocateQuery()
{
    foundElement = foundDomain = -1;
    minDist = +FLT_MAX;
}


// ****************************************************************************
//  Method: avtLocateQuery destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 18, 2004 
//
// ****************************************************************************

avtLocateQuery::~avtLocateQuery()
{
    ;
}

// ****************************************************************************
//  Method: avtLocateQuery::VerifyInput
//
//  Purpose:
//    Rejects non-queryable input && input that has topological dimension == 0
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 3, 2004
//
//  Modifications:
//    Kathleen Bonnell, Thu Nov 17 13:39:42 PST 2005
//    Don't test Input's topodim, may be unreliable.  Use value stored in
//    pickatts instead, it came directly from the plot, not the pipeline.
//
// ****************************************************************************

void
avtLocateQuery::VerifyInput()
{
    avtDataObjectQuery::VerifyInput();
    if (pickAtts.GetInputTopoDim() == 0)
    {
        EXCEPTION1(NonQueryableInputException,
            "Requires plot with topological dimension > 0.");
    }
}

// ****************************************************************************
//  Method: avtLocateQuery::PreExecute
//
//  Purpose:
//      This is called before any of the domains are executed.
//
//  Programmer: Kathleen Bonnell
//  Creation:   May 18, 2004
//
//  Modifications:
//
// ****************************************************************************

void
avtLocateQuery::PreExecute(void)
{
    foundElement = foundDomain = -1; 
    minDist = +FLT_MAX;
}


// ****************************************************************************
//  Method: avtLocateQuery::PostExecute
//
//  Purpose:
//      This is called after all of the domains are executed.
//
//  Programmer: Kathleen Bonnell
//  Creation:   May 18, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Thu Nov  4 15:18:15 PST 2004
//    Set a flag in pick atts specifiying whether or not this code was
//    successful in finding an intersection.
//    
// ****************************************************************************

void
avtLocateQuery::PostExecute(void)
{
    int foundIsect = 0;
    if (ThisProcessorHasMinimumValue(minDist) && minDist != +FLT_MAX)
    {
        pickAtts.SetDomain(foundDomain);
        pickAtts.SetElementNumber(foundElement);
        foundIsect = 1;
    }
    //
    // Make sure all processors are on the same page.
    //
    foundIsect = UnifyMaximumValue(foundIsect);
    pickAtts.SetLocationSuccessful(foundIsect);
}


// ****************************************************************************
//  Method: avtLocateQuery::SetPickAtts
//
//  Purpose:
//      Sets the pickAtts to the passed values. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   May 18, 2004 
//
// ****************************************************************************

void
avtLocateQuery::SetPickAtts(const PickAttributes *pa)
{
    pickAtts =  *pa;
}


// ****************************************************************************
//  Method: avtLocateQuery::GetPickAtts
//
//  Purpose:
//    Retrieve the PickAttributes being used in this query.
//
//  Programmer: Kathleen Bonnell
//  Creation:   May 18, 2004
//
// ****************************************************************************

const PickAttributes *
avtLocateQuery::GetPickAtts() 
{
    return &pickAtts; 
}



// ****************************************************************************
//  Method: avtLocateQuery::RGRidIsect
//
//  Purpose:
//    Determine the intersection of a point or ray with a rectilinear grid.
//
//  Arguments:
//    ds        The dataset to search.
//    dist      The distance from the isect to the ray-origin. 
//    isect     The intersection point. 
//
//  Returns:
//    Cell id of intersected cell. (-1 if ds not intersected, or cell
//    was a ghost).
//
//  Programmer: Kathleen Bonnell
//  Creation:   May 18, 2004
//
//  Modifications:
//    Kathleen Bonnell, Mon Jun 27 15:46:29 PDT 2005
//    Made method return int, removed ijk from arguments.  Test for ghosts.
//
// ****************************************************************************

int                            
avtLocateQuery::RGridIsect(vtkRectilinearGrid *rgrid, double &dist, 
                           double isect[3])
{
    int i, ijk[3];
    double t, dsBounds[6], rayDir[3];
    double *rayPt1 = pickAtts.GetRayPoint1();
    double *rayPt2 = pickAtts.GetRayPoint2();
    int success = 0;
    int cellId = -1;

 
    rgrid->GetBounds(dsBounds);

    if (rayPt1[0] == rayPt2[0] &&
        rayPt1[1] == rayPt2[1] &&
        rayPt1[2] == rayPt2[2])
    { /* WORLD COORDINATE LOCATE */
        success = vtkVisItUtility::ComputeStructuredCoordinates(rgrid, rayPt1,
                                                                ijk);
        if (success)
        {
            isect[0] = rayPt1[0];
            isect[1] = rayPt1[1];
            isect[2] = rayPt1[2];
            dist = 0; 
        }
    }
    else
    { /* RAY-INTERSECTION LOCATE */
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
        vtkDataArray *ghosts = rgrid->GetCellData()->GetArray("avtGhostZones");
        if (ghosts && ghosts->GetTuple1(cellId) > 0)
        {
            cellId = -1;
        }
    }

    return cellId;
}


// ****************************************************************************
//  Method: avtLocateQuery::LocatorFindCell
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
//  Notes:  Moved vrom avtLocateCellQuery.
//
//  Programmer: Kathleen Bonnell  
//  Creation:   June 2, 2004
//
//  Modifications:
//
// ****************************************************************************

int
avtLocateQuery::LocatorFindCell(vtkDataSet *ds, double &dist, double *isect)
{
    if (ds->GetNumberOfPoints() == 0)
    {
        return -1;
    }
    double *rayPt1 = pickAtts.GetRayPoint1();
    double *rayPt2 = pickAtts.GetRayPoint2();
    dist = -1;

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

    double pcoords[3] = {0., 0., 0.}, ptLine[3] = {0., 0., 0.};
    int subId = 0, success = 0;

    vtkIdType foundCell; 
    if (rayPt1[0] == rayPt2[0] &&
        rayPt1[1] == rayPt2[1] &&
        rayPt1[2] == rayPt2[2])
    { /* WORLD COORD LOCATE */
        cellLocator->FindClosestPoint(rayPt1, ptLine, foundCell,
                                     subId, dist);
        if (foundCell >= 0 && dist >= 0)
        {
            success = 1;
            isect[0] = rayPt1[0];
            isect[1] = rayPt1[1];
            isect[2] = rayPt1[2];
        }
    }
    else 
    { /* RAY-INTERSECT LOCATE */
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
//  Method: avtLocateQuery::RayIntersectsDataSet
//
//  Purpose:
//    Determines if the ray stored in PickAtts intersects the passed dataset. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 6, 2004
//
//  Modifications:
//
// ****************************************************************************

bool
avtLocateQuery::RayIntersectsDataSet(vtkDataSet *ds)
{
    double *bnds = ds->GetBounds();
    double *pt1 = pickAtts.GetRayPoint1();
    double *pt2 = pickAtts.GetRayPoint2();
    double dir[3];
    for (int i = 0; i < 3; i++)
    {
        dir[i] = pt2[i] - pt1[i];
    }
    double dummy1[3], dummy2;
    return (vtkBox::IntersectBox(bnds, pt1, dir, dummy1, dummy2));
}
