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
#include <vtkVisItUtility.h>

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
//    Handle finding the NOde for mat-selected plots differently, as they
//    may have created new points in the mesh.
//    
// ****************************************************************************

void
avtLocateNodeQuery::Execute(vtkDataSet *ds, const int dom)
{
    if (ds == NULL)
    {
        return;
    }


    float dist, isect[3] = { 0., 0., 0.};
    int foundNode = -1;
    int origNode = -1;

    // Find the cell, intersection point, and distance along the ray.
    //
    if (ds->GetDataObjectType() != VTK_RECTILINEAR_GRID)
    {
        int foundCell = LocatorFindCell(ds, dist, isect); 
        if (foundCell != -1)
        {
            if (!pickAtts.GetMatSelected())
                foundNode = DeterminePickedNode(ds, foundCell, isect);
            else 
                foundNode = FindClosestPoint(ds, isect, origNode);
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
        }
        pickAtts.SetCellPoint(isect);
        pickAtts.SetNodePoint(vtkVisItUtility::GetPoints(ds)->GetPoint(foundNode));
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
                     GetPointData()->GetArray("vtkGhostLevels");
        if (ghosts && ghosts->GetComponent(nodeId, 0) == 1 )
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
// ****************************************************************************

int
avtLocateNodeQuery::DeterminePickedNode(vtkDataSet *ds, int foundCell, float *ppoint)
{
   vtkIdType minId = -1; 
   vtkPoints *points = vtkVisItUtility::GetPoints(ds);

   vtkIdList *ptIds = vtkIdList::New();
   vtkIdType id;
   ds->GetCellPoints(foundCell, ptIds);
   int numPts = ptIds->GetNumberOfIds();
   float dist2;
   float minDist2 = FLT_MAX;

   for (int i = 0; i < numPts; i++)
   {
        id = ptIds->GetId(i);

        dist2 = vtkMath::Distance2BetweenPoints(ppoint, points->GetPoint(id));
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
//    
// ****************************************************************************

int
avtLocateNodeQuery::FindClosestPoint(vtkDataSet *ds, float *isect, int &origNode)
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
            int comp = origNodes->GetNumberOfComponents()-1;
            int oNode = -1;
            for (int i = 0; i < ncp && oNode == -1; i++)
            {
                id = closestPoints->GetId(i);
                oNode = (int)origNodes->GetComponent(id, comp);
            }
            if (oNode != -1)
                origNode = oNode; 
            else
                id = closestPoints->GetId(0);
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
