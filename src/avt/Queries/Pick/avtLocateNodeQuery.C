// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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

#include <avtGhostData.h>
#include <DebugStream.h>
#include <vtkVisItPointLocator.h>

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
//    Kathleen Biagas, Wed Jun 28 16:32:38 PDT 2017
//    If the plot was MatSelected, and we can find a cell, but for some reason
//    not the node, pass along enough information for PickNode to possibly
//    resolve the information.
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
    int foundCell = -1;
    int origNode = -1;
    int topodim = info.GetAttributes().GetTopologicalDimension();
    int spatdim = info.GetAttributes().GetSpatialDimension();

    // Find the cell, intersection point, and distance along the ray.
    //
    // Don't use the RectilinearGrid fast-path if there are ghosts.  It
    // takes longer to remove the ghosts than it does to use the slower path.
    if (ds->GetDataObjectType() != VTK_RECTILINEAR_GRID ||
        info.GetAttributes().GetContainsExteriorBoundaryGhosts())
    {
        if (topodim == 1) // LINES
        {
            dist = minDist;
            if (spatdim == 2)
              foundNode = FindClosestPointOnLine(ds, dist, isect);
            else if (spatdim == 3)
              foundNode = ClosestLineToLine(ds, true, dist, isect);
        }
        else
        {
            foundCell = LocatorFindCell(ds, dist, isect);
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
    else if ((foundCell != -1) && (dist < minDist) &&
              pickAtts.GetMatSelected())
    {
        // If we found an intersected cell, we should be able to find the
        // node as well,  so give NodePick a chance to figure this out.
        minDist = dist;
        pickAtts.SetCellPoint(isect);
        pickAtts.SetPickPoint(isect);
        foundDomain = dom;
    }
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
//    Kathleen Biagas, Tue Feb 10 10:16:44 PST 2015
//    Don't include added Nodes, signified by '-1' for an OriginalNode number.
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
   vtkDataArray *origNodes = ds->GetPointData()->
                              GetArray("avtOriginalNodeNumbers");
   int comp = (origNodes != NULL ? origNodes->GetNumberOfComponents() -1 : 0);
   for (int i = 0; i < numPts; i++)
   {
        bool skipNode = false;
        id = ptIds->GetId(i);
        if (origNodes)
        {
            // is this an added node?
            skipNode =  ((int) origNodes->GetComponent(id, comp) == -1);
        }
        if (!skipNode)
        {
            dist2 = vtkMath::Distance2BetweenPoints(ppoint, ds->GetPoint(id));
            if (dist2 < minDist2)
            {
                   minDist2 = dist2;
                   minId = id;
            }
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

    for (size_t i = 0; i < cells.size(); i++)
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
            origNode = (int) origNodes->GetComponent(id, ncomp);
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
//    dist    The current minimum distance.
//    isect   The node coordinates.
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
//    Kathleen Biagas, Fri Jul  7 09:35:22 PDT 2017
//    Changed arg from minDist to dist, to avoid overshadowing member var.
//
// ****************************************************************************

int
avtLocateNodeQuery::FindClosestPointOnLine(vtkDataSet *ds, double &dist,
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
    double dist2, rad = dist;
    foundPoint = pointLocator->FindClosestPointWithinRadius(rad, pt, dist2);

    if (foundPoint >= 0 && dist2 < dist)
    {
        ds->GetPoint(foundPoint, isect);
        dist = dist2;
    }

    pointLocator->Delete();
    return foundPoint;
}
