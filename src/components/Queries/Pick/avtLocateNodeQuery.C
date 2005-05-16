// ************************************************************************* //
//                           avtLocateNodeQuery.C                            //
// ************************************************************************* //

#include <avtLocateNodeQuery.h>

#include <vtkDataSet.h>
#include <vtkIdList.h>
#include <vtkIntArray.h>
#include <vtkMath.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkRectilinearGrid.h>
#include <vtkUnsignedCharArray.h>
#include <vtkVisItPointLocator.h>

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

    float dist, isect[3] = { 0., 0., 0.};
    int foundNode = -1;
    int origNode = -1;
    int topodim = GetInput()->GetInfo().GetAttributes().GetTopologicalDimension();
    int spatdim = GetInput()->GetInfo().GetAttributes().GetSpatialDimension();

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
                    foundNode = FindClosestPoint(ds, foundCell, isect, origNode);
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
            else if (GetInput()->GetInfo().GetValidity().GetZonesPreserved() &&
                     GetInput()->GetInfo().GetAttributes().GetContainsGhostZones() 
                        != AVT_CREATED_GHOSTS)
            {
                foundElement = foundNode;
            }
            else if (!GetInput()->GetInfo().GetValidity().GetPointsWereTransformed())
            {
                // Points were not transformed, so node id found here is valid.
                foundElement = foundNode; 
            }
            // else ... Zones not preserved or we created ghosts, or points were
            // transformed, so node id found here is not valid, so don't set it.
        }
        else if (origNode != -1)
        {
            // MaterialSelection occurred, but we found an original node, use that
            foundElement = origNode; 
        }
        else if (!GetInput()->GetInfo().GetValidity().SubdivisionOccurred())
        {
            // MaterialSelection occurred without subdivision, can use the
            // node id found here.
            foundElement = foundNode; 
        }
        // else ... MaterialSelection occurred with subdivision, and the original
        // nodes array was not present, so the node id found here will not be
        // valid, so don't set it.
   
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
// ****************************************************************************

int
avtLocateNodeQuery::RGridFindNode(vtkDataSet *ds, float &dist, float *isect)
{
    vtkRectilinearGrid *rgrid = vtkRectilinearGrid::SafeDownCast(ds);

    int ijk[3], nodeId = -1;

    if (RGridIsect(rgrid, dist, isect, ijk))
    {
        nodeId = ds->FindPoint(isect);
        vtkUnsignedCharArray *ghosts = (vtkUnsignedCharArray *)ds->
                     GetPointData()->GetArray("avtGhostZones");
        if (ghosts && ghosts->GetValue(nodeId) > 0)
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
avtLocateNodeQuery::DeterminePickedNode(vtkDataSet *ds, int foundCell, float *ppoint)
{
   vtkIdType minId = -1; 

   vtkIdList *ptIds = vtkIdList::New();
   vtkIdType id;
   ds->GetCellPoints(foundCell, ptIds);
   int numPts = ptIds->GetNumberOfIds();
   float dist2;
   float minDist2 = FLT_MAX;

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
// ****************************************************************************

int
avtLocateNodeQuery::FindClosestPoint(vtkDataSet *ds, const int isectedCell,
    float *isect, int &origNode)
{
    vtkVisItPointLocator *locator = vtkVisItPointLocator::New();
    locator->SetDataSet(ds);
    locator->SetIgnoreDisconnectedPoints(0);
    locator->BuildLocator();

    vtkIdList *closestPoints = vtkIdList::New();
    //
    //  Some of the nearest points may not be valid (e.g. created during
    //  material interface reconstruction, so make sure we retrieve enough
    //  information to get to a valid point.
    //
    locator->FindClosestNPoints(8, isect, closestPoints);

    int id = -1;
    int ncp = closestPoints->GetNumberOfIds();

    if (ncp > 0)
    {
        vtkIntArray *origNodes = vtkIntArray::SafeDownCast(
            ds->GetPointData()->GetArray("avtOriginalNodeNumbers"));
        if (origNodes)
        {
            // some close nodes are same distance, find the one
            // that belongs to the isected cell.
            vtkIdList *cellPts = vtkIdList::New();
            ds->GetCellPoints(isectedCell, cellPts);
            int comp = origNodes->GetNumberOfComponents()-1;
            int oNode = -1;
            for (int i = 0; i < ncp && oNode == -1; i++)
            {
                id = closestPoints->GetId(i);
                oNode = (int)origNodes->GetComponent(id, comp);
                if (oNode != -1 && (cellPts->IsId(id) == -1))
                {
                    //
                    // We only want to consider nodes that are part of
                    // the isected cell.
                    //
                    oNode = -1;
                }
            }
            cellPts->Delete();
        }
        else 
        {
            //
            // This method is called when MateriaSelection has taken place,
            // so avtOriginalNodeNumbers should be here.
            //
            id = closestPoints->GetId(0);
            debug5 << "avtLocateNodeQuery::FindClosestPoint could not find "
                   << "avtOriginalNodeNumbers, possible error." << endl;
        }
    }

    closestPoints->Delete();
    locator->Delete();
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
avtLocateNodeQuery::FindClosestPointOnLine(vtkDataSet *ds, float &minDist, 
                                           float isect[3])
{
    if (ds->GetNumberOfPoints() == 0)
    {
        return -1;
    }
    float *rayPt1 = pickAtts.GetRayPoint1();

    vtkVisItPointLocator *pointLocator = vtkVisItPointLocator::New(); 
    pointLocator->SetDataSet(ds);
    pointLocator->IgnoreDisconnectedPointsOn();
    pointLocator->BuildLocator();

    vtkIdType foundPoint = -1; 
    float pt[3] = {rayPt1[0], rayPt1[1], 0.};
    float dist, rad = minDist;
    foundPoint = pointLocator->FindClosestPointWithinRadius(rad, pt, dist);

    if (foundPoint >= 0 && dist < minDist)
    {
        ds->GetPoint(foundPoint, isect);
        minDist = dist;
    }

    pointLocator->Delete();
    return foundPoint;
}


