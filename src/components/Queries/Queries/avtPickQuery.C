// ************************************************************************* //
//                              avtPickQuery.C                               //
// ************************************************************************* //

#include <avtPickQuery.h>

#include <float.h>

#include <vtkDataSet.h>
#include <vtkMath.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkVisItCellLocator.h>
#include <vtkVisItUtility.h>

#include <avtMatrix.h>
#include <avtTerminatingSource.h>
#include <avtVector.h>

#include <PickVarInfo.h>
#include <DebugStream.h>

#ifdef PARALLEL
#include <mpi.h>
#include <BufferConnection.h>
#endif

using     std::string;


// ****************************************************************************
//  Method: avtPickQuery constructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 15, 2002
//
//  Modifications:
//    Kathleen Bonnell, Thu Apr 17 09:39:19 PDT 2003  
//    Initialize invTransform.
//
//    Kathleen Bonnell, Wed Jun 18 17:55:49 PDT 2003 
//    Initialize ghostType. 
//
// ****************************************************************************

avtPickQuery::avtPickQuery()
{
    blockOrigin = cellOrigin = 0;
    invTransform = NULL;
    ghostType = AVT_NO_GHOSTS;
}


// ****************************************************************************
//  Method: avtPickQuery::SetPickAtts
//
//  Purpose:
//      Sets the pickAtts to the passed values. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 15, 2002
//
// ****************************************************************************

void
avtPickQuery::SetPickAtts(const PickAttributes *pa)
{
    pickAtts =  *pa;
}


// ****************************************************************************
//  Method: avtPickQuery::GetPickAtts
//
//  Purpose:
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 15, 2002
//
// ****************************************************************************

const PickAttributes *
avtPickQuery::GetPickAtts() 
{
    return &pickAtts; 
}


// ****************************************************************************
//  Method: avtPickQuery::PreExecute
//
//  Purpose:
//      This is called before any of the domains are executed.
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 15, 2002
//
//  Modifications:
//    Kathleen Bonnell, Wed Jun 18 17:55:49 PDT 2003 
//    Retreive value of ContainsGhostZones.
//
// ****************************************************************************

void
avtPickQuery::PreExecute(void)
{
    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
    blockOrigin = atts.GetBlockOrigin();
    cellOrigin = atts.GetCellOrigin();
    ghostType = atts.GetContainsGhostZones();
    pickAtts.SetDimension(atts.GetSpatialDimension());
}


// ****************************************************************************
//  Method: avtPickQuery::PostExecute
//
//  Purpose:
//      This is called after all of the domains are executed.
//      If in parallel, collects the correct pickAtts from the processor that 
//      gathered the pick info, to processor 0.
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 15, 2002
//
// ****************************************************************************

void
avtPickQuery::PostExecute(void)
{
#ifdef PARALLEL
    int myRank, numProcs;
    int hasFulfilledPick;
    int size, i;
    BufferConnection b;
    unsigned char *buf;
 
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
    if (myRank == 0)
    {
        for (i = 1; i < numProcs; i++)
        {
            MPI_Status stat, stat2;
            MPI_Recv(&hasFulfilledPick, 1, MPI_INT, MPI_ANY_SOURCE,
                     MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
            if (hasFulfilledPick)
            {
                MPI_Recv(&size, 1, MPI_INT, stat.MPI_SOURCE, MPI_ANY_TAG,
                         MPI_COMM_WORLD, &stat2);
                buf = new unsigned char[size];
                MPI_Recv(buf, size, MPI_UNSIGNED_CHAR, stat.MPI_SOURCE, MPI_ANY_TAG,
                         MPI_COMM_WORLD, &stat2);
                b.Append(buf, size);
                pickAtts.Read(b);
                delete [] buf;
            }
        }
    }
    else
    {
        hasFulfilledPick = (int) pickAtts.GetFulfilled();
        MPI_Send(&hasFulfilledPick, 1, MPI_INT, 0, myRank, MPI_COMM_WORLD);
        if (hasFulfilledPick)
        {
            pickAtts.SelectAll();
            pickAtts.Write(b);
            size = pickAtts.CalculateMessageSize(b);
            buf = new unsigned char[size];
            for (int i = 0; i < size; ++i)
                b.Read(buf+i);
 
            MPI_Send(&size, 1, MPI_INT, 0, myRank, MPI_COMM_WORLD);
            MPI_Send(buf, size, MPI_UNSIGNED_CHAR, 0, myRank, MPI_COMM_WORLD);
            delete [] buf;
        }
    }
#endif
}


// ****************************************************************************
//  Method: avtPickQuery::Execute
//
//  Purpose:
//      Processes a single domain.
//
//  Programmer: Kathleen Bonnell  
//  Creation:   November 15, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec  6 16:25:20 PST 2002 
//    If the zone number is from avtOriginalCellNumbers, (foundZone != -1)
//    have the database perform the entire query ... retrieve nodes, query
//    variable info.  Otherwise mat-selected plots return erroneous info.
//
//    Kathleen Bonnell, Fri Dec 27 14:09:40 PST 2002 
//    Fill in new pickAtts member nodeCoords, if requested. 
//    
//    Kathleen Bonnell, Tue Apr  8 11:08:15 PDT 2003  
//    Added call to CalculateRealCellID. 
//    
//    Kathleen Bonnell, Thu Apr 17 15:12:12 PDT 2003 
//    Use spatial dimension of input instead of from viewer.
//    pickAtts.CellPoint is now used to display the pick point, ensure it
//    has the correct data;  use invTransform if necessary.
//
//    Hank Childs, Thu May  8 09:56:44 PDT 2003
//    Make sure that the timestep is correct by getting a valid one from the
//    data specification.
//
//    Kathleen Bonnell, Thu May  8 10:24:37 PDT 2003 
//    For efficiency, use rectilinear-specific code to find the cellId
//    when applicable. 
//    
//    Kathleen Bonnell, Wed Jun 18 17:55:49 PDT 2003 
//    Restrict calculation of 'RealCellId' (Reason:  if VisIt calculates
//    ghost zones, they are created AFTER the OriginalCellsArray, so the
//    cell Id retrieved from the array is valid.  This is designated by
//    avtGhostType AVT_CREATED_GHOSTS.  GhostZones determined by the file
//    format are designated by AVT_HAS_GHOSTS).
//
//    Kathleen Bonnell, Thu Jun 19 16:50:41 PDT 2003  
//    Test for null ds. 
//    
//    Kathleen Bonnell, Fri Jun 27 17:28:17 PDT 2003 
//    Rework code to support node picking. 
//    
//    Hank Childs, Tue Jul 29 11:54:11 PDT 2003
//    Use the queryable source rather than the terminating source.
//
// ****************************************************************************

void
avtPickQuery::Execute(vtkDataSet *ds, const int dom)
{
    if (dom != pickAtts.GetDomain() || pickAtts.GetFulfilled() || ds == NULL)
    {
        return;
    }

    avtDataSpecification_p dspec = 
        GetInput()->GetTerminatingSource()->GetFullDataSpecification();
    int ts = dspec->GetTimestep();
    pickAtts.SetTimeStep(ts);
    intVector dlist;
    dspec->GetSIL().GetDomainList(dlist);

    int foundElement = pickAtts.GetElementNumber();
    int type = ds->GetDataObjectType();
    bool needRealId = (ghostType == AVT_HAS_GHOSTS || foundElement == -1) &&
                 (type == VTK_STRUCTURED_GRID || type == VTK_RECTILINEAR_GRID);

    if (foundElement == -1)
    {
        //
        //  If a valid zone hasn't already been determined, locate it now.
        //  
        if (type != VTK_RECTILINEAR_GRID)
        {
            foundElement = LocatorFindCell(ds);
        }
        else
        {
            foundElement = RGridFindCell((vtkRectilinearGrid*)ds);
        }


        //
        //  If a valid zone still hasn't been found, there's some kind of 
        //  problem, it should have been found for this domain. 
        //  
        if (foundElement == -1)
        {
            pickAtts.SetDomain(-1);
            pickAtts.SetElementNumber(-1);
            debug5 << "PICK BIG PROBLEM!  "
                   << "Could not find zone corresponding to pick point" << endl;
            return;
        }
        
        bool success = false;
        if (pickAtts.GetPickType() == PickAttributes::Zone)
        {
            success = RetrieveNodes(ds, foundElement);
        }
        else if (pickAtts.GetPickType() == PickAttributes::Node)
        {
            if (DeterminePickedNode(ds, foundElement))
            {
                success =  RetrieveZones(ds, foundElement);
            }
        }
        if (success)
        {
            pickAtts.SetFulfilled(true);
            pickAtts.SetElementNumber(foundElement);
            RetrieveVarInfo(ds);
        }
        else
        {
            // the zone/node could not be found, no further processing required.
            // SetDomain and ElementNumber to -1 to indicate failure. 
            pickAtts.SetDomain(-1);
            pickAtts.SetElementNumber(-1);
            return; 
        }
    } 

    //
    //  Allow the database to add any missing information.
    // 
    GetInput()->GetQueryableSource()->Query(&pickAtts);

    //
    // Set the domain and zone of pickAtts in relation to the
    // blockOrigin and cellOrigin of the problem.  
    // 
    if (dlist.size() == 1 && dspec->UsesAllDomains())
    {
        //
        // Indicate that there was only one domain.
        // We don't report the domain number for single-domain problems.
        //
        pickAtts.SetDomain(-1);
    }
    else
    {
        pickAtts.SetDomain(dom+blockOrigin);
    }

    if (needRealId)
        SetRealIds(ds);

    if (pickAtts.GetPickType() == PickAttributes::Zone)
        pickAtts.SetElementNumber(pickAtts.GetElementNumber() + cellOrigin);

    if (pickAtts.GetPickType() == PickAttributes::Zone)
    {
        //
        // If the points of this dataset have been transformed, and we know 
        // the inverse transform matrix, transform the pick point that will 
        // be displayed in the pick info window.
        //
        if (invTransform != NULL)
        {
            avtVector v1(pickAtts.GetPickPoint());
            v1 = (*invTransform) * v1;
            // 
            // PickPoint is used for placing the pick letter, so set
            // this tranformed point in CellPoint instead.
            // 
            float ppt[3] = { v1.x, v1.y, v1.z };
            pickAtts.SetCellPoint(ppt);
        }
        else 
        {
            // 
            // CellPoint may have been 'fudged' towards the cell center by
            // avtLocateCellQuery.  Make sure the point that will be displayed
            // is not that 'fudged' point.
            // 
            pickAtts.SetCellPoint(pickAtts.GetPickPoint());
        }
    }
    else
    {
        //
        // CellPoint should now contain the actual node coordinates.
        // This is the value displayed to the user in the PickAtts output.
        // PickPoint determines where on the screen the pick letter will
        // be placed.  This should be the actual node coordinates (CellPoint)
        // if the plot was NOT transformed.
        //
        // If the plot was transformed && the inverseTransform is available,
        // transform CellPoint according to how the whole plot was transformed: 
        // apply the inverse of the inverseTransform and set the value in 
        // PickPoint.
        //
        // If the plot was transformed && inverseTransform is NOT available,
        // there is no way to determine the location of the picked node in
        // transformed space, so leave PickPoint set to the intersection point
        // with the ray as determined by avtLocateCellQuery.
        //
        if (invTransform != NULL)
        {
            avtMatrix mat = *(invTransform);
            mat.Inverse(); 
            avtVector v1(pickAtts.GetCellPoint());
            v1 = mat * v1;
            float ppt[3] = { v1.x, v1.y, v1.z };
            pickAtts.SetPickPoint(ppt);
        }
        else if (pickAtts.GetNeedTransformMessage())
        {
            //
            // Points were transformed, but we don't need the message because
            // we are displaying the node coords to the user in pick output.
            //
            pickAtts.SetNeedTransformMessage(false);
        }
        else 
        {
            pickAtts.SetPickPoint(pickAtts.GetCellPoint());
        }
    }
}


// ****************************************************************************
//  Method: avtPickQuery::LocatorFindCell
//
//  Purpose:
//      Uses a locator to find the cell containing the picked point. 
//
//  Returns:
//      The id of the cell containing the picked point. (-1 if point is
//      not contained within the ds).
//
//  Programmer: Kathleen Bonnell  
//  Creation:   November 15, 2002
//
//  Modifications:
//    Kathleen Bonnell, Mon Dec 30 11:18:24 PST 2002
//    Use new vtkVisItUtility method GetLogicalIndices.
//
//    Kathleen Bonnell, Tue Apr  8 11:08:15 PDT 2003  
//    The logic calculating 'real' cellId did not belong in this method. 
//
//    Kathleen Bonnell, Thu Apr 17 15:14:27 PDT 2003  
//    Use inverse transformation matrix, if available, to determine true
//    intersection point with original data.  
//
//    Kathleen Bonnell, Wed May  7 13:24:37 PDT 2003
//    Renamed from LocateCell.
//    
//    Kathleen Bonnell, Wed Jun 18 18:03:41 PDT 2003 
//    Moved transformation of cellPoint to ApplyFilters method, so that
//    the transformed point is available to RGridFindCell if necessary. 
//    
//    Kathleen Bonnell, Thu Jun 19 16:50:41 PDT 2003  
//    Test for no cells in ds. 
//    
// ****************************************************************************

int
avtPickQuery::LocatorFindCell(vtkDataSet *ds)
{
    if (ds->GetNumberOfCells() == 0)
    {
        return -1;
    }

    //
    // Use the picked point that has been moved towards the cell center.
    //
    float *cellPoint  = pickAtts.GetCellPoint();

    float tol, dist, ptLine[3], diagLen;
    int subId, found = -1;
    vtkIdType cellId;
 
    vtkVisItCellLocator *cellLocator = vtkVisItCellLocator::New();
    cellLocator->SetIgnoreGhosts(true);
    cellLocator->SetDataSet(ds);
    cellLocator->BuildLocator();

    int nCells = ds->GetNumberOfCells();
    diagLen = ds->GetLength();
    if (nCells != 0)
        tol = diagLen / (float) nCells;
    else
        tol = 1e-6;

    cellLocator->FindClosestPoint(cellPoint,ptLine,cellId,subId,dist);
    if (cellId >= 0 && dist <= tol)
    {
        found = cellId;
    }
    cellLocator->Delete();
    return found;
}


// ****************************************************************************
//  Method: avtPickQuery::RGridFindCell
//
//  Purpose:
//      Uses rectilinear-grid specific code to find the cell containing 
//      the picked point. 
//
//  Returns:
//      The id of the cell containing the picked point. (-1 if point is
//      not contained within the rgrid).
//
//  Programmer: Kathleen Bonnell  
//  Creation:   May 7, 2003 
//
//  Modifications:
//    Kathleen Bonnell, Wed May 14 17:48:52 PDT 2003  
//    Removed unnecessary (repeated) code segment.
//
// ****************************************************************************

int
avtPickQuery::RGridFindCell(vtkRectilinearGrid *rgrid)
{
    int ijk[3];
    float *x  = pickAtts.GetCellPoint();
    if (vtkVisItUtility::ComputeStructuredCoordinates(rgrid, x, ijk) == 0)
        return -1;
    return rgrid->ComputeCellId(ijk);
}


// ****************************************************************************
//  Method: avtPickQuery::ApplyFilters
//
//  Purpose:
//      Retrieves the terminating source to use as input. 
//
//  Programmer: Kathleen Bonnell  
//  Creation:   November 15, 2002
//
//  Modifications:
//    Kathleen Bonnell, Thu Apr 17 11:00:28 PDT 2003
//    Retrieve inverse transformation matrix if available.  
//    This is done here, rather than PreExecute, because the input used
//    for this Query will not have that information.
// 
//    Kathleen Bonnell, Thu Jun  5 13:37:11 PDT 2003 
//    pickAtts' NeedTransformMessage wasn't always getting set to false
//    when needed.
//    
//    Kathleen Bonnell, Wed Jun 18 18:03:41 PDT 2003 
//    Transform pickAtts.cellPoint.
//    
//    Kathleen Bonnell, Fri Jun 27 17:28:17 PDT 2003 
//    Transform message not needed for node pick. 
//    
// ****************************************************************************

avtDataObject_p
avtPickQuery::ApplyFilters(avtDataObject_p inData)
{
    avtDataAttributes &inAtts = inData->GetInfo().GetAttributes();
    if (inAtts.HasTransform() && inAtts.GetCanUseTransform())
    {
        invTransform = inAtts.GetTransform();
        //
        // Transform the point that will be used in locating the cell. 
        //
        float *cellPoint  = pickAtts.GetCellPoint();
        if (invTransform != NULL)  
        {
            //
            // Transform the intersection point back to original space.
            //
            avtVector v1(cellPoint);
            v1 = (*invTransform) * v1;
            cellPoint[0] = v1.x;
            cellPoint[1] = v1.y;
            cellPoint[2] = v1.z;
            //
            // Reset the cell point to the transformed point.
            //
            pickAtts.SetCellPoint(cellPoint);
        }
    }
    else
    {
        invTransform = NULL;
    }
    if (inData->GetInfo().GetValidity().GetPointsWereTransformed() &&
        (invTransform == NULL)) 
    {
        pickAtts.SetNeedTransformMessage(true);
    }
    else
    {
        pickAtts.SetNeedTransformMessage(false);
    }
    return inData->GetQueryableSource()->GetOutput();
}


// ****************************************************************************
//  Method: avtPickQuery::RetrieveNodes
//
//  Purpose:
//    Retrieves the nodes incident to the passed zone. 
//    Stores them in pickAtts.
//
//  Arguments:
//    ds    The dataset to retrieve information from.
//    zone  The zone in question. 
//
//  Returns:
//    True if node-retrieval was successful, false otherwise.
//
//  Notes:
//    Will also set the node Coordinates if needed.
//
//  Programmer: Kathleen Bonnell  
//  Creation:   June 27, 2003 
//
//  Modifications:
//    
// ****************************************************************************

bool
avtPickQuery::RetrieveNodes(vtkDataSet *ds, int zone)
{
    vtkIdList *ptIds = vtkIdList::New();
    intVector nodes;
    stringVector nodeCoords;
    float coord[3];
    int ijk[3];
    char buff[80];
    int type = ds->GetDataObjectType();
    bool success = true;
    ds->GetCellPoints(zone, ptIds);

    if (ptIds->GetNumberOfIds() == 0)
    {
        ptIds->Delete();
        debug5 << "FoundZone contained no points!" << endl;
        success = false;
    }
    else
    {
        for (int i = 0; i < ptIds->GetNumberOfIds(); i++)
        {
            nodes.push_back(ptIds->GetId(i));
            if (pickAtts.GetUseNodeCoords())
            {
                if (pickAtts.GetLogicalCoords() && 
                    (type == VTK_STRUCTURED_GRID || 
                    type == VTK_RECTILINEAR_GRID))
                {
                    vtkVisItUtility::GetLogicalIndices(ds, false, ptIds->GetId(i), ijk);
                    if (pickAtts.GetDimension() == 2)
                    {
                        sprintf(buff, "<%d, %d>", ijk[0], ijk[1]);
                    }
                    else 
                    {
                        sprintf(buff, "<%d, %d, %d>", ijk[0], ijk[1], ijk[2]);
                    }
                }
                else
                {
                    ds->GetPoint(ptIds->GetId(i), coord); 
                    if (pickAtts.GetDimension() == 2)
                    {
                        sprintf(buff, "<%g, %g>", coord[0], coord[1]);
                    }
                    else 
                    {
                        sprintf(buff, "<%g, %g, %g>", coord[0], coord[1], coord[2]);
                    }
                }
                nodeCoords.push_back(buff);
            }
        }
        ptIds->Delete();

        if (nodes.size() == 1) // point mesh
        {
            //
            //  Set pick point to be the same as the cell
            //  node for point mesh.
            //
            float pt[3];
            ds->GetPoint(nodes[0], pt);
            pickAtts.SetPickPoint(pt);
        }
        pickAtts.SetIncidentElements(nodes);
        pickAtts.SetNodeCoords(nodeCoords);
    }
    return success;
}


// ****************************************************************************
//  Method: avtPickQuery::RetrieveZones
//
//  Purpose:
//    Retrieves the zones incident to the passed node. 
//    Stores them in pickAtts. 
//
//  Arguments:
//    ds    The dataset to retrieve information from.
//    node  The node in question. 
//
//  Returns:
//    True if zone-retrieval was successful, false otherwise.
//
//  Notes:
//    This method will not return any zone designated as a ghost-zone.
// 
//  Programmer: Kathleen Bonnell  
//  Creation:   June 27, 2003 
//
//  Modifications:
//    
// ****************************************************************************

bool
avtPickQuery::RetrieveZones(vtkDataSet *ds, int foundNode)
{
    vtkIdList *cellIds = vtkIdList::New();
    intVector zones;
    ds->GetPointCells(foundNode, cellIds);
    int nCells = cellIds->GetNumberOfIds();
    bool success = true;
    vtkUnsignedCharArray *ghostArray; 
    unsigned char *ghosts = NULL;
    if (nCells == 0)
    {
        debug5 << "FoundNode has no incident cells!" << endl;
        success = false;
    }
    else
    {
        ghostArray  = (vtkUnsignedCharArray *)ds->GetCellData()->
            GetArray("vtkGhostLevels");
        if (ghostArray)
            ghosts = ghostArray->GetPointer(0);

        vtkIdType *cells = cellIds->GetPointer(0);
        for (int i = 0; i < nCells; i++)
        {
            if (ghosts && ghosts[cells[i]] == 1)
               continue;
            zones.push_back(cells[i]);
        }
        pickAtts.SetIncidentElements(zones);
    }
    cellIds->Delete();
    return success;
}


// ****************************************************************************
//  Method: avtPickQuery::DeterminePickedNode
//
//  Purpose:
//    Finds the closest node-point to the picked point.  
//
//  Arguments:
//    ds        The dataset to retrieve information from.
//    foundEl   IN:   the picked ZONE 
//              OUT:  the picked NODE 
//
//  Returns:
//    True if node-determination was successful, false otherwise. 
//
//  Programmer: Kathleen Bonnell  
//  Creation:   June 27, 2003 
//
//  Modifications:
//    
// ****************************************************************************

bool
avtPickQuery::DeterminePickedNode(vtkDataSet *ds, int &foundEl)
{
   float *ppoint = pickAtts.GetPickPoint();
   vtkIdType minId = -1; 
   vtkPoints *points = vtkVisItUtility::GetPoints(ds);
   stringVector nodeCoords;

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

   if ( minId == -1)
       return false;
   
   pickAtts.SetCellPoint(points->GetPoint(minId));

   // change the foundEl (a zone) to the min pt id (node)
   foundEl = minId;

   if (pickAtts.GetUseNodeCoords())
   {
       char buff[80];
       int ijk[3];
       float coord[3];
       int type = ds->GetDataObjectType();
       stringVector nodeCoords;
       if (pickAtts.GetLogicalCoords() && 
           (type == VTK_STRUCTURED_GRID || 
           type == VTK_RECTILINEAR_GRID))
       {
           vtkVisItUtility::GetLogicalIndices(ds, false, minId, ijk);
           if (pickAtts.GetDimension() == 2)
           {
               sprintf(buff, "<%d, %d>", ijk[0], ijk[1]);
           }
           else 
           {
               sprintf(buff, "<%d, %d, %d>", ijk[0], ijk[1], ijk[2]);
           }
       }
       else
       {
           points->GetPoint(minId, coord); 
           if (pickAtts.GetDimension() == 2)
           {
               sprintf(buff, "<%g, %g>", coord[0], coord[1]);
           }
           else 
           {
               sprintf(buff, "<%g, %g, %g>", coord[0], coord[1], coord[2]);
           }
       }
       nodeCoords.push_back(buff);
       pickAtts.SetNodeCoords(nodeCoords);
   }
   return true;
}


// ****************************************************************************
//  Method: avtPickQuery::SetRealIds
//
//  Purpose:
//    Converts node/zone numbers to the 'correct' ids for the original mesh
//    with no-ghost zones. 
//
//  Arguments:
//    ds          The dataset to retrieve information from.
//
//  Programmer: Kathleen Bonnell  
//  Creation:   June 27, 2003 
//
//  Modifications:
//    
// ****************************************************************************

void
avtPickQuery::SetRealIds(vtkDataSet *ds)
{
    int foundEl= pickAtts.GetElementNumber();
    intVector incEls = pickAtts.GetIncidentElements();

    int i, j; 
    bool forCell = pickAtts.GetPickType() == PickAttributes::Zone;
    char fString[20], tmp[20];

    stringVector elStrings;
    foundEl = vtkVisItUtility::CalculateRealID(foundEl, forCell, ds);
    sprintf(fString, "(%d)", foundEl);
    for (i = 0; i < incEls.size(); i++)
    {
        incEls[i] = vtkVisItUtility::CalculateRealID(incEls[i], !forCell, ds);
        sprintf(tmp, "(%d)", incEls[i]);
        elStrings.push_back(tmp);
    }

    // need to change the zone/node names stored in all PickVarInfo
    int numVars = pickAtts.GetNumPickVarInfos();
    for (i = 0; i < numVars; i++)
    {
        if (pickAtts.GetPickVarInfo(i).GetVarIsMaterial())
            continue;

        stringVector &names = pickAtts.GetPickVarInfo(i).GetNames();
        if (names.size() == 0) 
            continue;
        if (names.size() == incEls.size())
        {
            for (j = 0; j < names.size(); j++)
            {
                names[j] = elStrings[j]; 
            }
        }
        else 
        {
            names[0] = fString;
        }
    }
   
    pickAtts.SetElementNumber(foundEl);
    pickAtts.SetIncidentElements(incEls);
}


// ****************************************************************************
//  Method: avtPickQuery::RetrieveVarInfo
//
//  Purpose:
//    Retrieves the variable information from the dataset and stores it
//    in pickAtts.
//
//  Arguments:
//    ds    The dataset to retrieve information from.
//
//
//  Programmer: Kathleen Bonnell  
//  Creation:   June 27, 2003 
//
//  Modifications:
//    
// ****************************************************************************

void
avtPickQuery::RetrieveVarInfo(vtkDataSet* ds)
{
    int element = pickAtts.GetElementNumber();
    stringVector userVars = pickAtts.GetVariables();
    string vName;
    char buff[80];
    intVector incidentElements = pickAtts.GetIncidentElements();
    double *temp;
    double mag;
    int nComponents;
    bool zonePick = pickAtts.GetPickType() == PickAttributes::Zone;
    bool zoneCent;
    bool foundData = true;

    for (int varNum = 0; varNum < userVars.size(); varNum++)
    {
        stringVector names; 
        doubleVector vals; 
        vName = userVars[varNum];
        PickVarInfo varInfo;
        vtkDataArray *varArray = ds->GetPointData()->GetArray(vName.c_str());
        if (varArray != NULL) // nodal data
        {
            varInfo.SetCentering(PickVarInfo::Nodal);
            zoneCent = false;
        }
        else
        {
            varArray = ds->GetCellData()->GetArray(vName.c_str());
            if (varArray != NULL) // zonal data
            {
                varInfo.SetCentering(PickVarInfo::Zonal);
                zoneCent = true;
            }
            else 
            {
                foundData = false;
            }
        }
        if (foundData)
        {
            nComponents = varArray->GetNumberOfComponents(); 
            temp = new double[nComponents];
            if (zoneCent != zonePick)
            {
                // data we want is associated with incidentElements
                for (int k = 0; k < incidentElements.size(); k++)
                {
                    sprintf(buff, "(%d)", incidentElements[k]);
                    names.push_back(buff);
                    varArray->GetTuple(incidentElements[k], temp);
                    mag = 0;
                    for (int i = 0; i < nComponents; i++)
                    {
                        vals.push_back(temp[i]);
                        if (nComponents > 1) // assume its a vector, get its mag.
                            mag += (temp[i] * temp[i]);
                    }     
                    if (nComponents > 1)
                    {
                        mag = sqrt(mag);
                        vals.push_back(mag); 
                    }         
                } // for all incidentElements
            }
            else  
            {
                // data we want is associated with element
                sprintf(buff, "(%d)", element);
                names.push_back(buff);
                varArray->GetTuple(element, temp);
                mag = 0.;
                for (int i = 0; i < nComponents; i++)
                {
                    vals.push_back(temp[i]);
                    if (nComponents > 1)
                        mag +=  (temp[i] * temp[i]);
                }
                if (nComponents > 1) 
                {
                    mag = sqrt(mag);
                    vals.push_back(mag);
                }
            } 
        }  // foundData
        varInfo.SetVariableName(vName);
        if (!names.empty())
        {
            varInfo.SetNames(names);
            varInfo.SetValues(vals);
            delete [] temp; 
        }
        pickAtts.AddPickVarInfo(varInfo);
    } // for all vars  
}


