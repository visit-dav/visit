// ************************************************************************* //
//                              avtPickQuery.C                               //
// ************************************************************************* //

#include <avtPickQuery.h>

#include <float.h>

#include <vtkDataSet.h>
#include <vtkIdList.h>
#include <vtkMath.h>
#include <vtkPoints.h>
#include <vtkVisItUtility.h>

#include <avtExpressionEvaluatorFilter.h>
#include <avtMatrix.h>
#include <avtTerminatingSource.h>
#include <avtVector.h>

#include <PickVarInfo.h>

#ifdef PARALLEL
#include <mpi.h>
#include <avtParallel.h>
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
//    Kathleen Bonnell, Mon Mar  8 09:23:47 PST 2004 
//    Initialize new mbmber, needTransform, singleDomain and eef. 
//
// ****************************************************************************

avtPickQuery::avtPickQuery()
{
    blockOrigin = cellOrigin = 0;
    transform = NULL;
    needTransform = false;
    ghostType = AVT_NO_GHOSTS;
    singleDomain = false;
}


// ****************************************************************************
//  Method: avtPickQuery destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
//  Modifications:
//    Kathleen Bonnell, Mon Mar  8 09:23:47 PST 2004
//    Destruct eef.
// ****************************************************************************

avtPickQuery::~avtPickQuery()
{
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
//    Kathleen Bonnell, Wed Nov 26 14:08:42 PST 2003 
//    If we are doing a PickByZone or PickByNode (indicated by 
//    ppt[0] == FLT_MAX), get the correct domain number to use in the pipeline.
//
//    Kathleen Bonnell, Mon Mar  8 08:10:09 PST 2004 
//    Moved code to ApplyFilters method. 
// 
// ****************************************************************************

void
avtPickQuery::PreExecute(void)
{
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
//  Modifications:
//    Kathleen Bonnell, Tue Mar 16 15:55:18 PST 2004
//    Remove any "mesh" PickVarInfo's, as they are unnecessary
//
//    Kathleen Bonnell, Wed May  5 13:24:52 PDT 2004 
//    Added error message. 
//
//    Mark C. Miller, Wed Jun  9 21:50:12 PDT 2004
//    Eliminated use of MPI_ANY_TAG and modified to use GetUniqueMessageTags
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
    int mpiHasFullfilledPickTag = GetUniqueMessageTag();
    int mpiSizeTag              = GetUniqueMessageTag();
    int mpiDataTag              = GetUniqueMessageTag();
    if (myRank == 0)
    {
        for (i = 1; i < numProcs; i++)
        {
            MPI_Status stat, stat2;
            MPI_Recv(&hasFulfilledPick, 1, MPI_INT, MPI_ANY_SOURCE,
                     mpiHasFullfilledPickTag, MPI_COMM_WORLD, &stat);
            if (hasFulfilledPick)
            {
                MPI_Recv(&size, 1, MPI_INT, stat.MPI_SOURCE, mpiSizeTag,
                         MPI_COMM_WORLD, &stat2);
                buf = new unsigned char[size];
                MPI_Recv(buf, size, MPI_UNSIGNED_CHAR, stat.MPI_SOURCE, mpiDataTag,
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
        MPI_Send(&hasFulfilledPick, 1, MPI_INT, 0, mpiHasFullfilledPickTag, MPI_COMM_WORLD);
        if (hasFulfilledPick)
        {
            pickAtts.SelectAll();
            pickAtts.Write(b);
            size = pickAtts.CalculateMessageSize(b);
            buf = new unsigned char[size];
            for (int i = 0; i < size; ++i)
                b.Read(buf+i);
 
            MPI_Send(&size, 1, MPI_INT, 0, mpiSizeTag, MPI_COMM_WORLD);
            MPI_Send(buf, size, MPI_UNSIGNED_CHAR, 0, mpiDataTag, MPI_COMM_WORLD);
            delete [] buf;
        }
        return;
    }

#endif

    //
    // Remove any "mesh" PickVarInfo's, as they are unnecessary
    //
    for (int i = pickAtts.GetNumPickVarInfos()-1; i >= 0; i--)
    {
        if (strcmp(pickAtts.GetPickVarInfo(i).GetVariableType().c_str(),
                   "mesh") == 0)
        {
            pickAtts.RemovePickVarInfo(i);
        }
    }

    //
    //  If we haven't been able to get the necessary info, and
    //  no previous error was set, then
    //
    if (!pickAtts.GetFulfilled() && !pickAtts.GetError())
    {
        pickAtts.SetError(true);
        pickAtts.SetErrorMessage("Chosen pick did not intersect surface.");
    }
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
//    Kathleen Bonnell, Mon Mar  8 08:10:09 PST 2004
//    Reworked to use ExpressionEvaluatorFilter and SILRestriction UseSet as
//    passed down by the Engine. 
//    
//    Hank Childs, Fri Apr  9 16:25:40 PDT 2004
//    Minimize work done by creating new SIL.
//
//    Kathleen Bonnell, Mon Apr 19 15:40:23 PDT 2004 
//    Simplified use of pspec and dspec.   No longer use dspec's timestep
//    to set pickAtts' timestep.
//
//    Kathleen Bonnell, Tue May  4 14:25:07 PDT 2004
//    Set SILRestriction via member restriction, instead of SILUseSet. 
//
//    Kathleen Bonnell, Wed Jun  2 10:21:50 PDT 2004 
//    Moved Node and Zone-specific code to Preparation method.
//
// ****************************************************************************

avtDataObject_p
avtPickQuery::ApplyFilters(avtDataObject_p inData)
{
    src = inData->GetQueryableSource();

    avtDataAttributes &inAtts = inData->GetInfo().GetAttributes();
    blockOrigin = inAtts.GetBlockOrigin();
    cellOrigin = inAtts.GetCellOrigin();
    ghostType = inAtts.GetContainsGhostZones();
    pickAtts.SetDimension(inAtts.GetSpatialDimension());

    Preparation();

    if (needTransform && (transform == NULL)) 
    {
        pickAtts.SetNeedTransformMessage(true);
    }
    else
    {
        pickAtts.SetNeedTransformMessage(false);
    }

    avtDataSpecification_p dspec = 
        inData->GetTerminatingSource()->GetFullDataSpecification();

    intVector dlist;
    dspec->GetSIL().GetDomainList(dlist);
    if (dlist.size() == 1 && dspec->UsesAllDomains())
    {
        singleDomain = true;
    }
    else 
    {
        singleDomain = false;
    }

    dspec = new avtDataSpecification(pickAtts.GetActiveVariable().c_str(),
                                     pickAtts.GetTimeStep(), querySILR);

    int i;
    if (!singleDomain)
    {
        intVector dlist;
        dlist.push_back(pickAtts.GetDomain());
        dspec->GetRestriction()->RestrictDomains(dlist);
    }

    stringVector vars = pickAtts.GetVariables();
    for (i = 0; i < vars.size(); i++)
    {
        if (strcmp(dspec->GetVariable(), vars[i].c_str()) != 0)
        {
            if (!dspec->HasSecondaryVariable(vars[i].c_str()))
                dspec->AddSecondaryVariable(vars[i].c_str());
        }
    }
    avtPipelineSpecification_p pspec = new avtPipelineSpecification(dspec, 0);

    avtDataObject_p temp;
    CopyTo(temp, inData);
    eef->SetInput(temp);
    avtDataObject_p retObj = eef->GetOutput();
    retObj->Update(pspec);
    return retObj;
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
//    Kathleen Bonnell, Wed Dec 17 15:06:34 PST 2003 
//    Added logic to support multiple types of coordinates. 
//    
//    Kathleen Bonnell, Mon May 10 08:52:53 PDT 2004 
//    Moved coordinate-getting code to GetNodeCoords. 
//    
// ****************************************************************************

bool
avtPickQuery::DeterminePickedNode(vtkDataSet *ds, int &foundEl)
{
   float *ppoint = pickAtts.GetPickPoint();
   vtkIdType minId = -1; 
   vtkPoints *points = vtkVisItUtility::GetPoints(ds);

   if (ppoint[0] == FLT_MAX)
      return true;

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
//    Kathleen Bonnell, Thu Sep 18 07:38:32 PDT 2003
//    Store the results in 'realElement' and 'realIncidentElements'.
//    
//    Kathleen Bonnell, Thu Nov 20 15:06:49 PST 2003 
//    Swapped 'VarIsMaterial' with check of varType. 
//    
// ****************************************************************************

void
avtPickQuery::SetRealIds(vtkDataSet *ds)
{
    int foundEl= pickAtts.GetElementNumber();
    intVector incEls = pickAtts.GetIncidentElements();

    int i, j; 
    bool forCell = pickAtts.GetPickType() == PickAttributes::Zone || 
                   pickAtts.GetPickType() == PickAttributes::DomainZone;
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
        if (pickAtts.GetPickVarInfo(i).GetVariableType() == "material")
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
   
    pickAtts.SetRealElementNumber(foundEl);
    pickAtts.SetRealIncidentElements(incEls);
}

 
// ****************************************************************************
//  Method: avtPickQuery::VerifyInput
//
//  Purpose:
//    Verify a new input.  Overrides parent class.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 31, 2004 
//
// ****************************************************************************

void
avtPickQuery::VerifyInput()
{
    avtDataObjectQuery::VerifyInput();
}
 
// ****************************************************************************
//  Method: avtPickQuery::GetNodeCoords
//
//  Purpose:
//    Retrieves coordinates for a node. 
//
//  Arguments:
//    ds        The dataset to retrieve information from.
//    nodeId    The picked node .
//
//  Programmer: Kathleen Bonnell  
//  Creation:   May 10, 2004 
//
//  Modifications:
//    
// ****************************************************************************

void
avtPickQuery::GetNodeCoords(vtkDataSet *ds, const int nodeId)
{
   char buff[80];
   float coord[3];
   int type = ds->GetDataObjectType();
   vtkPoints *points = vtkVisItUtility::GetPoints(ds);
   stringVector nodeCoords;

   points->GetPoint(nodeId, coord); 


   if ((pickAtts.GetShowNodeDomainLogicalCoords() ||
        pickAtts.GetShowNodeBlockLogicalCoords())  &&
       (type == VTK_STRUCTURED_GRID || 
        type == VTK_RECTILINEAR_GRID))
   {
       int ijk[3];
       if (pickAtts.GetShowNodeDomainLogicalCoords())
       {
           nodeCoords.clear();
           vtkVisItUtility::GetLogicalIndices(ds, false, nodeId, ijk, 
              false);
           if (pickAtts.GetDimension() == 2)
           {
               sprintf(buff, "<%d, %d>", ijk[0], ijk[1]);
           }
           else 
           {
               sprintf(buff, "<%d, %d, %d>", ijk[0], ijk[1], ijk[2]);
           }
           nodeCoords.push_back(buff);
           pickAtts.SetDnodeCoords(nodeCoords);
       }
       if (pickAtts.GetShowNodeBlockLogicalCoords())
       {
           nodeCoords.clear();
           vtkVisItUtility::GetLogicalIndices(ds, false, nodeId, ijk, 
               true);
           if (pickAtts.GetDimension() == 2)
           {
               sprintf(buff, "<%d, %d>", ijk[0], ijk[1]);
           }
           else 
           {
               sprintf(buff, "<%d, %d, %d>", ijk[0], ijk[1], ijk[2]);
           }
           nodeCoords.push_back(buff);
           pickAtts.SetBnodeCoords(nodeCoords);
       }
   }
   if (pickAtts.GetShowNodePhysicalCoords())
   {
       nodeCoords.clear();
       if (pickAtts.GetDimension() == 2)
       {
           sprintf(buff, "<%g, %g>", coord[0], coord[1]);
       }
       else 
       {
           sprintf(buff, "<%g, %g, %g>", coord[0], coord[1], coord[2]);
       }
       nodeCoords.push_back(buff);
       pickAtts.SetPnodeCoords(nodeCoords);
   }
}


// ****************************************************************************
//  Method: avtPickQuery::GetZoneCoords
//
//  Purpose:
//    Retrieves coordinates for a zone. 
//
//  Arguments:
//    ds        The dataset to retrieve information from.
//    zoneId    The picked zone .
//
//  Programmer: Kathleen Bonnell  
//  Creation:   May 10, 2004 
//
//  Modifications:
//    
// ****************************************************************************

void
avtPickQuery::GetZoneCoords(vtkDataSet *ds, const int zoneId)
{
    int type = ds->GetDataObjectType();
    if ((pickAtts.GetShowZoneDomainLogicalCoords()  ||
         pickAtts.GetShowZoneBlockLogicalCoords())  &&
         (type == VTK_STRUCTURED_GRID || 
         type == VTK_RECTILINEAR_GRID))
    {
        char buff[80];
        int ijk[3];
        stringVector zoneCoords;
        if (pickAtts.GetShowZoneDomainLogicalCoords())
        {
            zoneCoords.clear();
            vtkVisItUtility::GetLogicalIndices(ds, true, zoneId, 
                        ijk, false);
            if (pickAtts.GetDimension() == 2)
            {
                sprintf(buff, "<%d, %d>", ijk[0], ijk[1]);
            }
            else 
            {
                sprintf(buff, "<%d, %d, %d>", ijk[0], ijk[1], ijk[2]);
            }
            zoneCoords.push_back(buff);
            pickAtts.SetDzoneCoords(zoneCoords);
        }
        if (pickAtts.GetShowZoneBlockLogicalCoords())
        {
            zoneCoords.clear();
            vtkVisItUtility::GetLogicalIndices(ds, true, zoneId, 
                        ijk, true);
            if (pickAtts.GetDimension() == 2)
            {
                sprintf(buff, "<%d, %d>", ijk[0], ijk[1]);
            }
            else 
            {
                sprintf(buff, "<%d, %d, %d>", ijk[0], ijk[1], ijk[2]);
            }
            zoneCoords.push_back(buff);
            pickAtts.SetBzoneCoords(zoneCoords);
        }
    }
}

