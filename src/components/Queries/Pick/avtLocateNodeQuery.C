// ************************************************************************* //
//                           avtLocateNodeQuery.C                            //
// ************************************************************************* //

#include <avtLocateNodeQuery.h>

#include <vtkDataSet.h>
#include <vtkIdList.h>
#include <vtkMath.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkRectilinearGrid.h>
#include <vtkUnsignedCharArray.h>
#include <vtkVisItUtility.h>


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

    // Find the cell, intersection point, and distance along the ray.
    //
    if (ds->GetDataObjectType() != VTK_RECTILINEAR_GRID)
    {
        int foundCell = LocatorFindCell(ds, dist, isect); 
        if (foundCell != -1)
            foundNode = DeterminePickedNode(ds, foundCell, isect);
    }
    else
    {
        foundNode = RGridFindNode(ds, dist, isect); 
    }

    if ((foundNode != -1) && (dist < minDist))
    {
        minDist = dist;

        pickAtts.SetPickPoint(isect);
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

