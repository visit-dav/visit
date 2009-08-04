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
//                           avtLocateNodeQuery.C                            //
// ************************************************************************* //

#include <avtLocateNodeQuery.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkIdList.h>
#include <vtkMath.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkRectilinearGrid.h>
#include <vtkUnsignedCharArray.h>
#include <vtkVisItPointLocator.h>

#include <avtGhostData.h>
#include <DebugStream.h>

#include <math.h>
#include <float.h>


// ****************************************************************************
//  Method: avtLocateNodeQuery constructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   May 18, 2004 
//
//  Modifications:
//
// ****************************************************************************

avtLocateNodeQuery::avtLocateNodeQuery()
{
}


// ****************************************************************************
//  Method: avtLocateNodeQuery destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 18, 2004 
//
// ****************************************************************************

avtLocateNodeQuery::~avtLocateNodeQuery()
{
    ;
}


// ****************************************************************************
//  Method: avtLocateNodeQuery::Execute
//
//  Purpose:
//      Processes a single domain.
//
//  Programmer: Kathleen Bonnell  
//  Creation:   May 18, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Thu Jun 17 14:05:22 PDT 2004
//    Handle finding the Node for mat-selected plots differently, as they
//    may have created new points in the mesh.
//    
//    Kathleen Bonnell, Thu Jul 29 08:34:18 PDT 2004 
//    Make sure that node found for mat-slected plots is actually used! 
//
//    Kathleen Bonnell, Wed Oct  6 10:43:10 PDT 2004
//    Terminate early if the ray doesn't intersect the dataset. 
//
//    Kathleen Bonnell, Tue Nov 30 09:25:28 PST 2004
//    Set foundElement = foundNode when MatSelected and No subdivision 
//    occurred, or when not MatSelected and Points not transformed. 
//
//    Hank Childs, Thu Mar 10 10:27:57 PST 2005
//    Fix memory leak.
//
//    Kathleen Bonnell, Fri Jul  8 14:15:21 PDT 2005 
//    Set foundElement = foundNode when points not transformed only if
//    ghost zones not created.
//
//    Kathleen Bonnell, Tue Aug 29 08:04:41 PDT 2006 
//    Set foundElement = foundNode only if OriginalZones intact (eg not
//    clipped, contoured, etc). 
//
//    Kathleen Bonnell, Thu Oct 26 10:27:36 PDT 2006 
//    Added test for NodesPreserved.
//
// ****************************************************************************

void
avtLocateNodeQuery::Execute(vtkDataSet *ds, const int dom)
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
    double dist, isect[3] = { 0., 0., 0.};
    int foundNode = -1;
    int origNode = -1;
    int topodim = info.GetAttributes().GetTopologicalDimension();
    int spatdim = info.GetAttributes().GetSpatialDimension();

    // Find the cell, intersection point, and distance along the ray.
    //
    if (ds->GetDataObjectType() != VTK_RECTILINEAR_GRID)
    {
        if (topodim == 1 && spatdim == 2) // LINES
        {
            dist = minDist;
            foundNode = FindClosestPointOnLine(ds, dist, isect);
        }
        else
        {
            int foundCell = LocatorFindCell(ds, dist, isect); 
            if (foundCell != -1)
            {
                if (!pickAtts.GetMatSelected())
                    foundNode = DeterminePickedNode(ds, foundCell, isect);
                else 
                    foundNode = FindClosestPoint(ds,foundCell,isect,origNode);
            }
        }
    }
    else
    {
        foundNode = RGridFindNode(ds, dist, isect); 
    }

    if ((foundNode != -1) && (dist < minDist))
    {
        minDist = dist;

        pickAtts.SetPickPoint(isect);
        if (!pickAtts.GetMatSelected())
        {
            vtkDataArray *origNodes = 
                 ds->GetPointData()->GetArray("avtOriginalNodeNumbers");

            if (origNodes)
            {
                int comp = origNodes->GetNumberOfComponents() -1;
                foundElement = (int) origNodes->GetComponent(foundNode, comp);
            }
            else if (info.GetValidity().GetNodesPreserved() &&
                     info.GetValidity().GetOriginalZonesIntact() &&
                     (info.GetValidity().GetZonesPreserved()   ||
                     !info.GetValidity().GetPointsWereTransformed()) &&
                     info.GetAttributes().GetContainsGhostZones() 
                        != AVT_CREATED_GHOSTS)
            {
                foundElement = foundNode;
            }
            // else ... Zones not preserved or we created ghosts, or points 
            // were transformed, so node id found here is not valid, so don't 
            // set it.
        }
        else if (origNode != -1)
        {
            // MaterialSelection occurred, but we found an original node, 
            // use that
            foundElement = origNode; 
        }
        else if (!info.GetValidity().SubdivisionOccurred())
        {
            // MaterialSelection occurred without subdivision, can use the
            // node id found here.
            foundElement = foundNode; 
        }
        // else ... MaterialSelection occurred with subdivision, and the 
        // original nodes array was not present, so the node id found here 
        // will not be valid, so don't set it.
   
        pickAtts.SetCellPoint(isect);
        pickAtts.SetNodePoint(ds->GetPoint(foundNode));
        foundDomain = dom;
    } // if node was found
}


// ****************************************************************************
//  Method: avtLocateNodeQuery::RGridFindNode
//
//  Purpose:
//    Uses rectilinear-grid specific code to find the node closest by 
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
//    The id of the node that was cosest (-1 if none found).
//
//  Programmer: Kathleen Bonnell  
//  Creation:   May 18, 2004
//
//  Modifications:
//    
//    Hank Childs, Fri Aug 27 16:02:58 PDT 2004
//    Rename ghost data array.
//
//    Kathleen Bonnell, Thu Oct 21 18:02:50 PDT 2004 
//    Correct test for whether a node is ghost or not. 
//
//    Kathleen Bonnell, Mon Jun 27 15:54:52 PDT 2005
//    Match new interface for RGridIsect. 
//
//    Kathleen Bonnell, Tue Oct  2 08:30:04 PDT 2007 
//    Don't throw away a 'duplicated node' ghost node. 
//
// ****************************************************************************

int
avtLocateNodeQuery::RGridFindNode(vtkDataSet *ds, double &dist, double *isect)
{
    vtkRectilinearGrid *rgrid = vtkRectilinearGrid::SafeDownCast(ds);

    int nodeId = -1;

    if (RGridIsect(rgrid, dist, isect) != -1)
    {
        nodeId = ds->FindPoint(isect);

        vtkUnsignedCharArray *ghostNodes = (vtkUnsignedCharArray *)ds->
                     GetPointData()->GetArray("avtGhostNodes");
        unsigned char gn = ghostNodes ? ghostNodes->GetValue(nodeId): 0;
        if (ghostNodes && gn > 0 && 
            !avtGhostData::IsGhostNodeType(gn, DUPLICATED_NODE))
        {
            nodeId = -1;
        }
    }
    return nodeId;
}


// ****************************************************************************
//  Method: avtLocateNodeQuery::DeterminePickedNode
//
//  Purpose:
//    Finds the closest node-point to the picked point.  
//
//  Arguments:
//    ds        The dataset to retrieve information from.
//    foundCell the picked ZONE 
//    ppoint    the node's coordinates.
//
//  Notes:  Moved from avtLocateCellQuery.
//
//  Programmer: Kathleen Bonnell  
//  Creation:   June 2, 2004 
//
//  Modifications:
//    
//    Hank Childs, Thu Mar 10 10:27:57 PST 2005
//    Fix memory leak.
//
// ****************************************************************************

int
avtLocateNodeQuery::DeterminePickedNode(vtkDataSet *ds, int foundCell, 
                                        double *ppoint)
{
   vtkIdType minId = -1; 

   vtkIdList *ptIds = vtkIdList::New();
   vtkIdType id;
   ds->GetCellPoints(foundCell, ptIds);
   int numPts = ptIds->GetNumberOfIds();
   double dist2;
   double minDist2 = FLT_MAX;

   for (int i = 0; i < numPts; i++)
   {
        id = ptIds->GetId(i);

        dist2 = vtkMath::Distance2BetweenPoints(ppoint, ds->GetPoint(id));
        if (dist2 < minDist2)
        {
               minDist2 = dist2; 
               minId = id; 
        }
    }

    ptIds->Delete();

   return minId;
}


// ****************************************************************************
//  Method: avtLocateNodeQuery::FindClosestPoint
//
//  Purpose:
//    Finds the closest node-point to the intersection point.  
//
//  Arguments:
//    ds        The dataset to retrieve information from.
//    isect       the intersection point.
//    origNode  the original node Id, if any.
//
//  Programmer: Kathleen Bonnell  
//  Creation:   June 17, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Thu Jul 29 08:34:18 PDT 2004
//    Test for coincident points, return the one that is part of the
//    original intersected cell.
//    
//    Kathleen Bonnell, Tue Aug 10 09:06:54 PDT 2004 
//    Set id when validOrigNodes size is 1. 
//
//    Kathleen Bonnell, Mon Aug 30 18:20:50 PDT 2004
//    Revamped: FindClosestNPoints orders the points in increasing distance
//    from the isect point, so don't need to recalculate distances.  Simply
//    keep the first node that has a valid OrigNode associated with it AND is
//    part of the isectedCell.
//
//    Kathleen Bonnell, Fri May 13 15:16:12 PDT 2005 
//    Fix memory leak. 
//
//    Kathleen Bonnell, Mon May 23 12:17:37 PDT 2005 
//    Arg origNode was not getting set.
//
//    Kathleen Bonnell, Thu Nov  8 09:01:31 PST 2007 
//    Rework to only consider 'original' nodes from cells that have same
//    original cell as isectedCell.
//
// ****************************************************************************

int
avtLocateNodeQuery::FindClosestPoint(vtkDataSet *ds, const int isectedCell,
    double *isect, int &origNode)
{
    vtkDataArray *origCells = ds->GetCellData()->
                              GetArray("avtOriginalCellNumbers");

    if (!origCells)
    {
        debug5 << "avtLocateNodeQuery::FindClosestPoint could not find "
               << "avtOriginalCellNumbers, cannot continue." << endl;
        return -1;
    }

    vtkDataArray *origNodes = ds->GetPointData()->
                              GetArray("avtOriginalNodeNumbers");

    if (!origNodes)
    {
        debug5 << "avtLocateNodeQuery::FindClosestPoint could not find "
               << "avtOriginalNodeNumbers, cannot continue." << endl;
        return -1;
    }

    int zcomp = origCells->GetNumberOfComponents()-1;
    int ncomp = origNodes->GetNumberOfComponents()-1;
    int oc = (int)origCells->GetComponent(isectedCell, zcomp);
    intVector cells;

    for (int i = 0; i < ds->GetNumberOfCells(); i++)
    {
        if ((int)origCells->GetComponent(i, zcomp) == oc)
            cells.push_back(i);
    }

    vtkIdList *nodesFromCells = vtkIdList::New();
    vtkIdList *cellPts = vtkIdList::New();

    for (int i = 0; i < cells.size(); i++)
    {
        ds->GetCellPoints(cells[i], cellPts);
        for (int j = 0; j < cellPts->GetNumberOfIds(); j++)
        {
            int node = cellPts->GetId(j);
            if ((int)origNodes->GetComponent(node, ncomp) != -1)
                nodesFromCells->InsertUniqueId(node);
        }
        cellPts->Reset();
    }

    double d2, minD = FLT_MAX;
    double pt[3];
    int id = -1;
    for (int i = 0; i < nodesFromCells->GetNumberOfIds(); i++)
    {
        ds->GetPoint(nodesFromCells->GetId(i), pt);
        d2 = vtkMath::Distance2BetweenPoints(isect, pt);
        if (d2 < minD)
        {
            id = nodesFromCells->GetId(i);
            minD = d2;
            origNode = origNodes->GetComponent(id, ncomp);
        }
    }

    nodesFromCells->Delete();
    cellPts->Delete();
    return id;
}


// ****************************************************************************
//  Method: avtLocateQuery::FindClosestPointOnLine
//
//  Purpose:
//    Uses a locator to find the closest point to the given point. 
//
//  Arguments:
//    ds      The dataset to query.
//    minDist The current minimum distance.
//
//  Returns:
//    The id of the closest point (-1 if none found).
//
//  Programmer: Kathleen Bonnell  
//  Creation:   June 10, 2004 
//
//  Modifications:
//    
//    Hank Childs, Thu Mar 10 10:27:57 PST 2005
//    Fix memory leak.
//
// ****************************************************************************

int
avtLocateNodeQuery::FindClosestPointOnLine(vtkDataSet *ds, double &minDist, 
                                           double isect[3])
{
    if (ds->GetNumberOfPoints() == 0)
    {
        return -1;
    }
    double *rayPt1 = pickAtts.GetRayPoint1();

    vtkVisItPointLocator *pointLocator = vtkVisItPointLocator::New(); 
    pointLocator->SetDataSet(ds);
    pointLocator->IgnoreDisconnectedPointsOn();
    pointLocator->BuildLocator();

    vtkIdType foundPoint = -1; 
    double pt[3] = {rayPt1[0], rayPt1[1], 0.};
    double dist, rad = minDist;
    foundPoint = pointLocator->FindClosestPointWithinRadius(rad, pt, dist);

    if (foundPoint >= 0 && dist < minDist)
    {
        ds->GetPoint(foundPoint, isect);
        minDist = dist;
    }

    pointLocator->Delete();
    return foundPoint;
}
