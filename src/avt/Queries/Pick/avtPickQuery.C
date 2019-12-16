// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtPickQuery.C                               //
// ************************************************************************* //

#include <avtPickQuery.h>

#include <algorithm>
#include <float.h>

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkEdgeTable.h>
#include <vtkIdList.h>
#include <vtkIntArray.h>
#include <vtkMath.h>
#include <vtkElementLabelArray.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnsignedIntArray.h>
#include <vtkVisItUtility.h>

#include <avtExpressionEvaluatorFilter.h>
#include <avtCommonDataFunctions.h>
#include <avtMatrix.h>
#include <avtParallel.h>
#include <avtOriginatingSource.h>
#include <avtVector.h>

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <PickVarInfo.h>

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
//    Initialize new member, needTransform, singleDomain and eef.
//
//    Kathleen Bonnell, Thu Jul 29 17:10:48 PDT 2004
//    Initialize eef and src.
//
//    Kathleen Bonnell, Mon Aug 30 17:56:29 PDT 2004
//    Initialize skippedLocate.
//
//    Mark C. Miller, Tue Mar 27 08:39:55 PDT 2007
//    Added support for node origin
//
//    Kathleen Biagas, Wed Jun 28 15:36:26 PDT 2017
//    Init invTransform.
//
// ****************************************************************************

avtPickQuery::avtPickQuery()
{
    blockOrigin = cellOrigin = nodeOrigin = 0;
    transform = NULL;
    invTransform = NULL;
    needTransform = false;
    ghostType = AVT_NO_GHOSTS;
    singleDomain = false;
    eef = new avtExpressionEvaluatorFilter;
    src = NULL;
    skippedLocate = false;
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
//
// ****************************************************************************

avtPickQuery::~avtPickQuery()
{
    if (eef != NULL)
    {
        delete eef;
        eef = NULL;
    }
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
//    Jeremy Meredith, Thu Feb 15 11:55:03 EST 2007
//    Call inherited PreExecute before everything else.
//
// ****************************************************************************

void
avtPickQuery::PreExecute(void)
{
    avtDatasetQuery::PreExecute();
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
//    Kathleen Bonnell, Thu Jul  1 16:41:57 PDT 2004
//    Removed mpi calls, use GetAttToRootProc instead.
//
//    Kathleen Bonnell, Mon Oct 23 08:37:09 PDT 2006
//    Changed error message.
//
//    Kathleen Bonnell, Tue Feb 13 12:48:14 PST 2007
//    Send the MeshCoordType to the returning PickAtts.
//
//    Kathleen Bonnell, Thu Feb 24 11:54:13 PST 2011
//    Use rectilinearGridTransform matrix to transform cell/pick points to
//    correct location if necessary.
//
//    Kathleen Biagas, Wed Oct 26 13:37:59 PDT 2011
//    Invalidate the timeStep if DB is not time-varying.
//
//    Kathleen Biagas, Wed Jun 28 09:43:10 PDT 2017
//    Ensure error message from non-root gets passsed to root.
//
// ****************************************************************************

void
avtPickQuery::PostExecute(void)
{
    GetAttToRootProc(pickAtts, (int) pickAtts.GetFulfilled());

    int hasFulfilledPick = UnifyMaximumValue((int)pickAtts.GetFulfilled());

    if (!hasFulfilledPick)
    {
        GetAttToRootProc(pickAtts, (int)pickAtts.GetError());
    }

    if (PAR_Rank() == 0)
    {
        //
        // Remove any "mesh" PickVarInfo's, as they are unnecessary
        //
        for (int i = pickAtts.GetNumVarInfos()-1; i >= 0; i--)
        {
            if (pickAtts.GetVarInfo(i).GetVariableType() == "mesh")
            {
                pickAtts.RemoveVarInfo(i);
            }
        }

        //
        //  If we haven't been able to get the necessary info, and
        //  no previous error was set, then
        //
        if (!pickAtts.GetFulfilled() && !pickAtts.GetError())
        {
            pickAtts.SetError(true);
            pickAtts.SetErrorMessage("Pick could not retrieve the requested "
              "information, possibly due to a bad expression in its variables "
              "list. Please check the requested variables list and try again.");
        }
        if (pickAtts.GetFulfilled())
        {
            pickAtts.SetMeshCoordType((PickAttributes::CoordinateType)GetInput()->GetInfo().GetAttributes().GetMeshCoordType());

            if (GetInput()->GetInfo().GetAttributes().GetRectilinearGridHasTransform())
            {
                avtMatrix m(GetInput()->GetInfo().GetAttributes().GetRectilinearGridTransform());
                avtVector pp(pickAtts.GetPickPoint());
                avtVector cp(pickAtts.GetPickPoint());
                pp = m * pp;
                cp = m * cp;
                double ppt[3] = { pp.x, pp.y, pp.z };
                double cpt[3] = { cp.x, cp.y, cp.z };
                //
                // Need to set both the point used for info, and the point used
                // for the visual cue
                //
                pickAtts.SetCellPoint(cpt);
                pickAtts.SetPickPoint(ppt);
            }
            // Invalidate the timeStep if DB is not time-varying.
            if (GetInput()->GetInfo().GetAttributes().GetNumStates() == 1)
                pickAtts.SetTimeStep(-1);
        }
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
//    Simplified use of contract and dataRequest.   No longer use
//    dataRequest's timestep to set pickAtts' timestep.
//
//    Kathleen Bonnell, Tue May  4 14:25:07 PDT 2004
//    Set SILRestriction via member restriction, instead of SILUseSet.
//
//    Kathleen Bonnell, Wed Jun  2 10:21:50 PDT 2004
//    Moved Node and Zone-specific code to Preparation method.
//
//    Kathleen Bonnell, Tue Aug 10 09:15:58 PDT 2004
//    When material selection has been applied, request OriginalZoneNumbers
//    and OriginalNodeNumbers.
//
//    Kathleen Bonnell, Thu Aug 26 09:50:31 PDT 2004
//    Don't restrict domains if pickAtts.domain has not yet been set.
//
//    Kathleen Bonnell, Wed Oct  6 09:58:49 PDT 2004
//    Don't perform certain tasks unless this process will be doing real work.
//
//    Kathleen Bonnell, Wed Dec 15 09:19:39 PST 2004
//    Turn on GlobalZone/Node numbers when appropriate.
//
//    Kathleen Bonnell, Mon Dec 20 14:09:21 PST 2004
//    Turn on OriginalZone/Node numbers when turning on Global versions.
//
//    Kathleen Bonnell, Wed May 11 16:46:51 PDT 2005
//    Reworked so that an 'Update' is issued only if secondary variables
//    are needed, or input has been MatSelected, or global ids are necessary.
//
//    Kathleen Bonnell, Tue Jun 28 15:49:03 PDT 2005
//    Change where we get ghostType from, as the inData's atts aren't always
//    accurate. (What is stored in PickAtts was taken from the networkCache.)
//
//    Kathleen Bonnell, Tue Nov  8 10:45:43 PST 2005
//    Added avtDatAttributes arg.
//
//    Kathleen Bonnell, Wed Jun 14 16:41:03 PDT 2006
//    Require update if Pick's timestep doesn't match the pipeline timestep.
//
//    Mark C. Miller, Tue Mar 27 08:39:55 PDT 2007
//    Added support for node origin
//
//    Hank Childs, Tue Dec 11 15:26:30 PST 2012
//    Add data selections from original executions, which makes index
//    calculations be consistent and also reduces amount of data loaded.
//
//    Kathleen Biagas, Wed Mar 18 15:38:36 PDT 2015
//    Provide secondary vars and request Node/Zones on all procs, to prevent
//    engine crash/hang on pipeline re-execution.
//
// ****************************************************************************

avtDataObject_p
avtPickQuery::ApplyFilters(avtDataObject_p inData)
{
    src = inData->GetQueryableSource();

    avtDataAttributes &inAtts = inData->GetInfo().GetAttributes();
    blockOrigin = inAtts.GetBlockOrigin();
    cellOrigin = inAtts.GetCellOrigin();
    nodeOrigin = inAtts.GetNodeOrigin();
    ghostType = (avtGhostType)pickAtts.GetGhostType();
    pickAtts.SetDimension(inAtts.GetSpatialDimension());

    Preparation(inAtts);

    if (needTransform && (transform == NULL))
    {
        pickAtts.SetNeedTransformMessage(true);
    }
    else
    {
        pickAtts.SetNeedTransformMessage(false);
    }

    avtDataRequest_p dataRequest =
        inData->GetOriginatingSource()->GetFullDataRequest();
    int currentTime = dataRequest->GetTimestep();
    intVector dlist;
    dataRequest->GetSIL().GetDomainList(dlist);
    if (dlist.size() == 1 && dataRequest->UsesAllDomains())
    {
        singleDomain = true;
    }
    else
    {
        singleDomain = false;
    }

    bool requiresUpdate = false;

    dataRequest = new avtDataRequest(pickAtts.GetActiveVariable().c_str(),
                                     pickAtts.GetTimeStep(), querySILR);

    stringVector vars = pickAtts.GetVariables();
    for (size_t i = 0; i < vars.size(); i++)
    {
        if (dataRequest->GetVariable() != vars[i])
        {
            if (!dataRequest->HasSecondaryVariable(vars[i].c_str()))
            {
                dataRequest->AddSecondaryVariable(vars[i].c_str());
                requiresUpdate = true;
            }
        }
    }

    if (pickAtts.GetMatSelected())
    {
        dataRequest->TurnZoneNumbersOn();
        dataRequest->TurnNodeNumbersOn();
        requiresUpdate = true;
    }
    if (pickAtts.GetShowGlobalIds() || pickAtts.GetElementIsGlobal())
    {
        dataRequest->TurnGlobalZoneNumbersOn();
        dataRequest->TurnGlobalNodeNumbersOn();
        dataRequest->TurnZoneNumbersOn();
        dataRequest->TurnNodeNumbersOn();
        requiresUpdate = true;
    }

    if (currentTime != pickAtts.GetTimeStep())
    {
        requiresUpdate = true;
    }

    requiresUpdate = (bool)UnifyMaximumValue((int)requiresUpdate);
    if (requiresUpdate)
    {
        int maxDom = UnifyMaximumValue(pickAtts.GetDomain());
        if (!singleDomain && maxDom != -1)
        {
            intVector dlist;
            if (maxDom == pickAtts.GetDomain())
            {
                dlist.push_back(pickAtts.GetDomain());
            }
            dataRequest->GetRestriction()->RestrictDomains(dlist);
        }
        else
        {
            // if we don't use the same data selections then:
            // (1) we may load too much data
            // (2) the indexing of the new data won't match the
            //     original data.
            std::vector<avtDataSelection_p> selList =
                inData->GetOriginatingSource()->GetSelectionsForLastExecution();
            for (unsigned int i = 0 ; i < selList.size() ; i++)
            {
                dataRequest->AddDataSelectionRefPtr(selList[i]);
            }
        }
    }

    if (!requiresUpdate)
        return inData;

    avtContract_p contract = new avtContract(dataRequest, 0);
    // We don't want to disturb the original pipeline, so get the
    // terminating source's output, and tack on an EEF, in case any of
    // the vars are Expressions.
    avtDataObject_p t1 = inData->GetOriginatingSource()->GetOutput();
    avtDataObject_p temp;
    CopyTo(temp, t1);
    eef->SetInput(temp);
    avtDataObject_p retObj = eef->GetOutput();
    retObj->Update(contract);
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
//    Hank Childs, Thu Mar 10 11:03:37 PST 2005
//    Fix memory leak.
//
// ****************************************************************************

bool
avtPickQuery::DeterminePickedNode(vtkDataSet *ds, int &foundEl)
{
   double *ppoint = pickAtts.GetPickPoint();
   vtkIdType minId = -1;

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
   }

   if ( minId == -1)
       return false;

   pickAtts.SetCellPoint(ds->GetPoint(minId));

   // change the foundEl (a zone) to the min pt id (node)
   foundEl = minId;

   return true;
}


// ****************************************************************************
//  Method: avtPickQuery::GetNodeCoords
//
//  Purpose:
//    Retrieves coordinates for a node.
//
//  Arguments:
//    ds        The dataset to retrieve information from.
//    nodeId    The picked node.
//
//  Programmer: Kathleen Bonnell
//  Creation:   May 10, 2004
//
//  Modifications:
//    Brad Whitlock, Thu Jul 29 17:26:26 PST 2004
//    I made it use snprintf.
//
//    Hank Childs, Thu Mar 10 11:03:37 PST 2005
//    Fix memory leak.
//
//    Mark C. Miller, Tue Mar 27 08:39:55 PDT 2007
//    Added support for node origin
//
//    Cyrus Harrison, Mon Sep 17 11:35:32 PDT 2007
//    Added support for user settable floating point format string
//
//    Gunther H. Weber, Thu Aug 22 10:19:37 PDT 2013
//    Allow GetLogicalIndices to return negative indices.
//
// ****************************************************************************


void
avtPickQuery::GetNodeCoords(vtkDataSet *ds, const int nodeId)
{
   char buff[80];
   double coord[3];
   int type = ds->GetDataObjectType();
   stringVector nodeCoords;

   ds->GetPoint(nodeId, coord);


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
              false, true, true);
           if (pickAtts.GetDimension() == 2)
           {
               snprintf(buff, 80, "<%d, %d>",
                   ijk[0]+nodeOrigin, ijk[1]+nodeOrigin);
           }
           else
           {
               snprintf(buff, 80, "<%d, %d, %d>",
                   ijk[0]+nodeOrigin, ijk[1]+nodeOrigin, ijk[2]+nodeOrigin);
           }
           nodeCoords.push_back(buff);
           pickAtts.SetDnodeCoords(nodeCoords);
       }
       if (pickAtts.GetShowNodeBlockLogicalCoords())
       {
           nodeCoords.clear();
           vtkVisItUtility::GetLogicalIndices(ds, false, nodeId, ijk,
               true, true, true);
           if (pickAtts.GetDimension() == 2)
           {
               snprintf(buff, 80, "<%d, %d>", ijk[0], ijk[1]);
           }
           else
           {
               snprintf(buff, 80, "<%d, %d, %d>", ijk[0], ijk[1], ijk[2]);
           }
           nodeCoords.push_back(buff);
           pickAtts.SetBnodeCoords(nodeCoords);
       }
   }
   if (pickAtts.GetShowNodePhysicalCoords())
   {
       std::string format = "";
       std::string floatFormat = pickAtts.GetFloatFormat();
       nodeCoords.clear();
       if (pickAtts.GetDimension() == 2)
       {
           format = "<" + floatFormat + ", " + floatFormat + ">";
           snprintf(buff, 80, format.c_str(), coord[0], coord[1]);
       }
       else
       {
           format = "<" + floatFormat + ", "
                        + floatFormat + ", "
                        + floatFormat + ">";
           snprintf(buff, 80, format.c_str(), coord[0], coord[1], coord[2]);
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
//    zoneId    The picked zone.
//
//  Programmer: Kathleen Bonnell
//  Creation:   May 10, 2004
//
//  Modifications:
//    Brad Whitlock, Thu Jul 29 17:23:14 PST 2004
//    I made it use the cell origin for the zonal domain logical coordinates.
//    I also made it use snprintf.
//
//    Gunther H. Weber, Thu Aug 22 10:19:37 PDT 2013
//    Allow GetLogicalIndices to return negative indices.
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
                        ijk, false, true, true);
            if (pickAtts.GetDimension() == 2)
            {
                snprintf(buff, 80, "<%d, %d>", ijk[0] + cellOrigin,
                         ijk[1] + cellOrigin);
            }
            else
            {
                snprintf(buff, 80, "<%d, %d, %d>",
                         ijk[0] + cellOrigin,
                         ijk[1] + cellOrigin,
                         ijk[2] + cellOrigin);
            }
            zoneCoords.push_back(buff);
            pickAtts.SetDzoneCoords(zoneCoords);
        }
        if (pickAtts.GetShowZoneBlockLogicalCoords())
        {
            zoneCoords.clear();
            vtkVisItUtility::GetLogicalIndices(ds, true, zoneId,
                        ijk, true, true, true);
            if (pickAtts.GetDimension() == 2)
            {
                snprintf(buff, 80, "<%d, %d>", ijk[0], ijk[1]);
            }
            else
            {
                snprintf(buff, 80, "<%d, %d, %d>", ijk[0], ijk[1], ijk[2]);
            }
            zoneCoords.push_back(buff);
            pickAtts.SetBzoneCoords(zoneCoords);
        }
    }
}


// ****************************************************************************
//  Method: avtPickQuery::RetrieveVarInfo
//
//  Purpose:
//    Convenience method, so derived types don't need to pass new args.
//
//  Arguments:
//    ds    The dataset to retrieve information from.
//
//  Programmer: Kathleen Bonnell
//  Creation:   August 11, 2004
//
//  Modifications:
//
// ****************************************************************************

void
avtPickQuery::RetrieveVarInfo(vtkDataSet* ds)
{
    RetrieveVarInfo(ds, pickAtts.GetElementNumber(),
                    pickAtts.GetIncidentElements());
}


// ****************************************************************************
//  Method: avtPickQuery::RetrieveVarInfo
//
//  Purpose:
//    Convenience method, so derived types don't need to pass new args.
//
//  Arguments:
//    ds        The dataset to retrieve information from.
//    elNum     The element number in question.
//
//  Programmer: Kathleen Bonnell
//  Creation:   Januay 30, 3007
//
//  Modifications:
//
// ****************************************************************************

void
avtPickQuery::RetrieveVarInfo(vtkDataSet* ds, const int elNum)
{
    RetrieveVarInfo(ds, elNum, pickAtts.GetIncidentElements());
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
//  Programmer: Kathleen Bonnell
//  Creation:   March 31, 2004
//
//  Modifications:
//    Kathleen Bonnell, Thu May  6 17:46:59 PDT 2004
//    Allow species and scalar vars to be looked at again if previously set.
//
//    Kathleen Bonnell, Tue Jun  1 15:26:10 PDT 2004
//    Allow 'DomainZone' pick type to indicate a ZonePick.
//
//    Kathleen Bonnell, Thu Jul 22 12:10:19 PDT 2004
//    Set PickVarInfo's treatAsASCII from DataAttributes' treatAsASCII.
//
//    Kathleen Bonnell, Wed Aug 11 09:21:07 PDT 2004
//    Added args findElement, and findIncidentElements, used when the
//    elements stored in pickAtts don't correspond to the element numbers
//    used by ds.
//
//    Kathleen Bonnell, Mon Nov  8 15:47:32 PST 2004
//    Instead of wrapping 'GetTreatAsASCII' in TRY-CATCH, ensure the variable
//    is valid before retrieving the flag to avoid EXCEPTION throwing
//    altogether.
//
//    Kathleen Bonnell, Wed Dec 15 09:19:39 PST 2004
//    Use GlobalZone/Node numbers when present.
//
//    Brad Whitlock, Mon Apr 4 16:47:06 PST 2005
//    Added support for label data.
//
//    Kathleen Bonnell, Tue Aug 30 09:35:44 PDT 2005
//    Compute MajorEigenvalue for tensors.
//
//    Hank Childs, Thu May 25 08:31:28 PDT 2006
//    Added support for array variables.
//
//    Kathleen Bonnell, Fri Jun  9 09:59:49 PDT 2006
//    Fix UMR.
//
//    Cyrus Harrison, Fri Aug 14 11:10:03 PDT 2009
//    Expanded use of 'Treat As ASCII' to handle label string case, removed
//    use of avtLabelVariableSize.
//
//    Kathleen Biagas, Thu Dec 12 12:07:17 PST 2019
//    Set pickVarInfo's VariableType from the one stored in avtDataAttributes.
//    Don't overwrite what was set if the pick is marked as 'fulfilled'.
//
// ****************************************************************************

void
avtPickQuery::RetrieveVarInfo(vtkDataSet* ds, const int findElement,
    const intVector &findIncidentElements)
{
    bool treatAsASCII = false;
    avtDataAttributes &data = GetInput()->GetInfo().GetAttributes();

    int element = pickAtts.GetElementNumber();
    stringVector userVars = pickAtts.GetVariables();
    string vName;
    string vType;
    char buff[80];
    intVector incidentElements = pickAtts.GetIncidentElements();
    double *temp = NULL;
    double mag;
    int nComponents = 0;
    bool zonePick = pickAtts.GetPickType() == PickAttributes::Zone ||
                    pickAtts.GetPickType() == PickAttributes::DomainZone;
    bool zoneCent;
    bool foundData = true;

    int numVars;
    if (pickAtts.GetFulfilled())
        numVars = pickAtts.GetNumVarInfos();
    else
        numVars = (int)userVars.size();
    for (int varNum = 0; varNum < numVars; varNum++)
    {
        stringVector names;
        doubleVector vals;
        PickVarInfo::Centering centering(PickVarInfo::None);
        if (pickAtts.GetFulfilled())
        {
            if (pickAtts.GetVarInfo(varNum).HasInfo()
             && pickAtts.GetVarInfo(varNum).GetVariableType() != "species"
             && pickAtts.GetVarInfo(varNum).GetVariableType() != "scalar")
                continue;

            vName = pickAtts.GetVarInfo(varNum).GetVariableName();
        }
        else
        {
            vName = userVars[varNum];
        }

        if (data.ValidVariable(vName))
        {
            treatAsASCII = data.GetTreatAsASCII(vName.c_str());
            vType = avtVarTypeToString(data.GetVariableType(vName.c_str()));
        }
        else
            treatAsASCII = false;

        vtkDataArray *varArray = ds->GetPointData()->GetArray(vName.c_str());
        if (varArray != NULL) // nodal data
        {
            centering = PickVarInfo::Nodal;
            foundData = true;
            zoneCent = false;
        }
        else
        {
            varArray = ds->GetCellData()->GetArray(vName.c_str());
            if (varArray != NULL) // zonal data
            {
                centering = PickVarInfo::Zonal;
                foundData = true;
                zoneCent = true;
            }
            else
            {
                foundData = false;
            }
        }

        bool labelData = false;
        if (foundData)
        {
            nComponents = varArray->GetNumberOfComponents();

            // Determine if the data is a label.
            labelData = (nComponents > 1) &&
                        data.GetTreatAsASCII(vName.c_str());

            temp = new double[nComponents];
            intVector globalIncEl = pickAtts.GetGlobalIncidentElements();
            if (zoneCent != zonePick)
            {
                // data we want is associated with incidentElements
                for (size_t k = 0; k < incidentElements.size(); k++)
                {
                    if (pickAtts.GetShowGlobalIds() &&
                        globalIncEl.size() == incidentElements.size())
                    {
                        snprintf(buff, 80, "(%d)", globalIncEl[k]);
                    }
                    else
                    {
                        snprintf(buff, 80, "(%d)", incidentElements[k]);
                    }
                    names.push_back(buff);
                    varArray->GetTuple(findIncidentElements[k], temp);
                    mag = 0;
                    for (int i = 0; i < nComponents; i++)
                    {
                        vals.push_back(temp[i]);
                        // assume its a vector, get its mag.
                        if (nComponents > 1 &&  nComponents < 9 && !labelData)
                            mag += (temp[i] * temp[i]);
                    }
                    if (nComponents > 1 && !labelData)
                    {
                        if (nComponents < 9)
                           mag = sqrt(mag);
                        else
                           mag = MajorEigenvalue(temp);
                        vals.push_back(mag);
                    }
                } // for all incidentElements
            }
            else
            {
                // data we want is associated with element
                if (pickAtts.GetShowGlobalIds() &&
                    pickAtts.GetGlobalElement() != -1)
                    snprintf(buff, 80, "(%d)", pickAtts.GetGlobalElement());
                else
                    snprintf(buff, 80, "(%d)", element);
                names.push_back(buff);
                varArray->GetTuple(findElement, temp);
                mag = 0.;
                for (int i = 0; i < nComponents; i++)
                {
                    vals.push_back(temp[i]);
                    if (nComponents > 1 && !labelData)
                        mag +=  (temp[i] * temp[i]);
                }
                if (nComponents > 1 && !labelData)
                {
                    mag = sqrt(mag);
                    vals.push_back(mag);
                }
            }
            delete [] temp;
        }  // foundData

        if (pickAtts.GetFulfilled())
        {
            if (!names.empty())
            {
                pickAtts.GetVarInfo(varNum).SetNames(names);
                pickAtts.GetVarInfo(varNum).SetValues(vals);
                pickAtts.GetVarInfo(varNum).SetCentering(centering);
                pickAtts.GetVarInfo(varNum).SetTreatAsASCII(treatAsASCII);
            }
        }
        else
        {
            PickVarInfo varInfo;
            varInfo.SetVariableName(vName);
            varInfo.SetCentering(centering);
            varInfo.SetTreatAsASCII(treatAsASCII);
            if (!names.empty())
            {
                varInfo.SetNames(names);
                varInfo.SetValues(vals);
                varInfo.SetVariableType(vType);
            }
            pickAtts.AddVarInfo(varInfo);
        }
    } // for all vars
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
//    Kathleen Bonnell, Wed Dec 17 15:06:34 PST 2003
//    Added logic to support multiple types of coordinates.
//
//    Brad Whitlock, Fri Jul 30 08:58:27 PDT 2004
//    Made it use snprintf.
//
//    Kathleen Bonnell, Thu Sep 23 17:38:15 PDT 2004
//    Added logic to search for ghost nodes, if ghostType == AVT_HAS_GHOSTS.
//
//    Kathleen Bonnell, Wed Dec 15 09:19:39 PST 2004
//    Added call to 'SetGlobalIds'.
//
//    Kathleen Bonnell, Mon May 16 07:35:27 PDT 2005
//    Fix memory leak.
//
//    Mark C. Miller, Tue Mar 27 08:39:55 PDT 2007
//    Added support for node origin
//
//    Cyrus Harrison, Mon Sep 17 11:35:32 PDT 2007
//    Added support for user settable floating point format string
//
//    Gunther H. Weber, Thu Aug 22 10:19:37 PDT 2013
//    Allow GetLogicalIndices to return negative indices.
//
//    Kathleen Biagas, Wed Feb 11 15:23:24 PST 2015
//    Use all cells corresponding to original when looking for incident nodes,
//    but only when original zones weren't preserved and we have the original
//    cells array.  Also, don't include 'added' nodes (original node id -1).
//
// ****************************************************************************

bool
avtPickQuery::RetrieveNodes(vtkDataSet *ds, int zone, bool needRealId)
{
    vtkIdList *ptIds = vtkIdList::New();
    intVector nodes;
    stringVector pnodeCoords;
    stringVector dnodeCoords;
    stringVector bnodeCoords;
    double coord[3];
    int ijk[3];
    char buff[80];
    int type = ds->GetDataObjectType();
    bool success = true;

    vtkDataArray *origZones = ds->GetCellData()->
            GetArray("avtOriginalCellNumbers");
    if (!GetInput()->GetInfo().GetValidity().GetZonesPreserved() && origZones)
    {
        vtkIdList *tmpIds = vtkIdList::New();
        int comp = origZones->GetNumberOfComponents() -1;
        for (int i = 0; i < ds->GetNumberOfCells(); ++i)
        {
            if ((int)origZones->GetComponent(i, comp) == zone)
            {
                tmpIds->Reset();
                ds->GetCellPoints(i, tmpIds);
                for (int j = 0; j < tmpIds->GetNumberOfIds(); j++)
                    ptIds->InsertUniqueId(tmpIds->GetId(j));
            }
        }
    }
    else
    {
        ds->GetCellPoints(zone, ptIds);
    }

    intVector ghostNodes;

    if (ptIds->GetNumberOfIds() == 0)
    {
        ptIds->Delete();
        debug4 << "FoundZone contained no points!" << endl;
        success = false;
    }
    else
    {
        unsigned char *gNodes = NULL;
        unsigned char *gZones = NULL;
        vtkIdList *cells = NULL;
        bool findGhosts = (ghostType == AVT_HAS_GHOSTS &&
                 ((ds->GetPointData()->GetArray("avtGhostNodes") != NULL) ||
                  (ds->GetCellData()->GetArray("avtGhostZones") != NULL)));
        pickAtts.SetIncludeGhosts(findGhosts);
        if (findGhosts)
        {
            vtkUnsignedCharArray *gn = (vtkUnsignedCharArray*)ds->
                GetPointData()->GetArray("avtGhostNodes");
            if (gn)
            {
                gNodes = gn->GetPointer(0);
            }
            vtkUnsignedCharArray *gz = (vtkUnsignedCharArray*)
                ds->GetCellData()->GetArray("avtGhostZones");
            if (gz)
            {
                gZones = gz->GetPointer(0);
                cells = vtkIdList::New();
            }
        }
        vtkDataArray *origNodes = ds->GetPointData()->GetArray("avtOriginalNodeNumbers");
        int ocomp = (origNodes ? origNodes->GetNumberOfComponents()-1 :0);
        int nGnodes = 0;
        for (int i = 0; i < ptIds->GetNumberOfIds(); i++)
        {
            vtkIdType ptId = ptIds->GetId(i);
            bool skipNode = false;
            // counterintuitive, but if needRealId is true, they will
            // actually be grabbed elsewhere, so don't do it here.
            if (origNodes && !needRealId)
            {
                int oNode = origNodes->GetComponent(ptId, ocomp);
                skipNode = oNode == -1;
                if(!skipNode)
                {
                    std::vector<int>::iterator it =
                        std::find(nodes.begin(), nodes.end(), oNode);
                    if (it == nodes.end())
                    {
                        nodes.push_back(oNode);
                    }
                    else
                    {
                        skipNode = true;
                    }
                }
            }
            else
            {
                nodes.push_back(ptId);
            }
            if (!skipNode)
            {
                if (findGhosts)
                {
                    if (gNodes && gNodes[ptId])
                    {
                        ghostNodes.push_back(1);
                        nGnodes++;
                    }
                    else if (gZones)
                    {
                        ds->GetPointCells(ptId, cells);
                        int nGhosts = 0;
                        for (int j = 0; j < cells->GetNumberOfIds(); j++)
                        {
                            nGhosts += (int)gZones[cells->GetId(j)];
                        }
                        if (nGhosts > 0 && nGhosts == cells->GetNumberOfIds())
                        {
                            ghostNodes.push_back(1);
                            nGnodes++;
                        }
                        else
                        {
                            ghostNodes.push_back(0);
                        }
                    }
                    else
                    {
                        ghostNodes.push_back(0);
                    }
                }
                if ((pickAtts.GetShowNodeDomainLogicalCoords() ||
                    pickAtts.GetShowNodeBlockLogicalCoords()) &&
                    (type == VTK_STRUCTURED_GRID ||
                     type == VTK_RECTILINEAR_GRID))
                {
                    if (pickAtts.GetShowNodeDomainLogicalCoords())
                    {
                        vtkVisItUtility::GetLogicalIndices(ds, false,
                             ptIds->GetId(i), ijk, false, true, true);
                        if (pickAtts.GetDimension() == 2)
                        {
                            snprintf(buff, 80, "<%d, %d>",
                                ijk[0]+nodeOrigin, ijk[1]+nodeOrigin);
                        }
                        else
                        {
                            snprintf(buff, 80, "<%d, %d, %d>",
                                ijk[0]+nodeOrigin, ijk[1]+nodeOrigin, ijk[2]+nodeOrigin);
                        }
                        dnodeCoords.push_back(buff);
                    }
                    if (pickAtts.GetShowNodeBlockLogicalCoords())
                    {
                        vtkVisItUtility::GetLogicalIndices(ds, false,
                             ptIds->GetId(i), ijk, true, true, true);
                        if (pickAtts.GetDimension() == 2)
                        {
                            snprintf(buff, 80, "<%d, %d>", ijk[0], ijk[1]);
                        }
                        else
                        {
                            snprintf(buff, 80, "<%d, %d, %d>",
                                     ijk[0], ijk[1], ijk[2]);
                        }
                        bnodeCoords.push_back(buff);
                    }
                }
                if (pickAtts.GetShowNodePhysicalCoords())
                {
                    std::string format = "";
                    std::string floatFormat = pickAtts.GetFloatFormat();

                    ds->GetPoint(ptIds->GetId(i), coord);
                    if (pickAtts.GetDimension() == 2)
                    {
                        format = "<" + floatFormat + ", " + floatFormat + ">";
                        snprintf(buff, 80, format.c_str(), coord[0], coord[1]);
                    }
                    else
                    {
                        format = "<" + floatFormat + ", "
                                     + floatFormat + ", "
                                     + floatFormat + ">";
                        snprintf(buff, 80, format.c_str(),
                                 coord[0], coord[1], coord[2]);
                    }
                    pnodeCoords.push_back(buff);
                }
            }
        }

        if (nodes.size() == 1) // point mesh
        {
            //
            //  Set pick point to be the same as the cell
            //  node for point mesh.
            //
            double pt[3];
            ds->GetPoint(nodes[0], pt);
            pickAtts.SetPickPoint(pt);
        }
        pickAtts.SetIncidentElements(nodes);
        pickAtts.SetPnodeCoords(pnodeCoords);
        pickAtts.SetDnodeCoords(dnodeCoords);
        pickAtts.SetBnodeCoords(bnodeCoords);
        pickAtts.SetGhosts(ghostNodes);
        pickAtts.SetElementIsGhost((gZones && gZones[zone]) ||
            (nGnodes > 0 && nGnodes == ptIds->GetNumberOfIds()));
        ptIds->Delete();
        if (cells)
            cells->Delete();
    }
    if (success && pickAtts.GetShowGlobalIds())
    {
        SetGlobalIds(ds, zone);
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
//    Kathleen Bonnell, Tue Nov 18 14:14:05 PST 2003
//    Retrieve logical zone coordinates if specified by pick atts.
//
//    Kathleen Bonnell, Wed Dec 17 15:06:34 PST 2003
//    Added logic to support multiple types of coordinates.
//
//    Brad Whitlock, Fri Jul 30 09:00:37 PDT 2004
//    I made it use the cell origin for the domain logical indices and I
//    changed sprintf to snprintf.
//
//    Hank Childs, Fri Aug 27 16:54:45 PDT 2004
//    Rename ghost data array.
//
//    Kathleen Bonnell, Thu Sep 23 17:38:15 PDT 2004
//    Added logic to search for ghost zones, if ghostType == AVT_HAS_GHOSTS.
//
//    Kathleen Bonnell, Thu Oct 21 18:02:50 PDT 2004
//    Correct test for whether a zone is ghost or not.
//
//    Kathleen Bonnell, Wed Dec 15 09:19:39 PST 2004
//    Added call to 'SetGlobalIds'.
//
//    Kathleen Biagas, Thu Oct 16 13:18:51 PDT 2012
//    Account for cellOrigin.
//
//    Gunther H. Weber, Thu Aug 22 10:19:37 PDT 2013
//    Allow GetLogicalIndices to return negative indices.
//
//    Kathleen Biagas, Tue Feb 10 11:19:30 PST 2015
//    Consider avtOriginalCellNumbers, and don't inlude duplicates.
//
// ****************************************************************************

bool
avtPickQuery::RetrieveZones(vtkDataSet *ds, int foundNode, bool needRealId)
{
    vtkIdList *cellIds = vtkIdList::New();
    intVector zones;
    intVector ghostZones;
    stringVector dzoneCoords;
    stringVector bzoneCoords;
    ds->GetPointCells(foundNode, cellIds);
    int nCells = cellIds->GetNumberOfIds();
    int type = ds->GetDataObjectType();
    int ijk[3];
    char buff[80];
    bool success = true;

    if (nCells == 0)
    {
        debug4 << "FoundNode has no incident cells!" << endl;
        success = false;
    }
    else
    {
        vtkDataArray *origZones =
            ds->GetCellData()->GetArray("avtOriginalCellNumbers");
        int comp = (origZones) ? origZones->GetNumberOfComponents()-1 : 0;

        vtkUnsignedCharArray *ghostArray =
          (vtkUnsignedCharArray *)ds->GetCellData()->GetArray("avtGhostZones");
        unsigned char *ghosts = NULL;
        if (ghostArray)
        {
            ghosts = ghostArray->GetPointer(0);
            if (ghostType == AVT_HAS_GHOSTS)
            {
                pickAtts.SetIncludeGhosts(true);
            }
        }

        vtkIdType *cells = cellIds->GetPointer(0);
        int nGhosts = 0;
        for (int i = 0; i < nCells; i++)
        {
            bool skipCell = false;
            int zoneId = cells[i] + cellOrigin;
            int origZoneId = zoneId;

            // counter-intuitive.  But needRealId indicates that the
            // 'real ids' will be found later.
            if (origZones && !needRealId)
            {
                origZoneId = cellOrigin +
                             (int)origZones->GetComponent(cells[i], comp);
                std::vector<int>::iterator it =
                    std::find(zones.begin(), zones.end(), origZoneId);
                if (it == zones.end())
                {
                    zoneId = origZoneId;
                }
                else
                {
                    skipCell = true;
                }
            }
            if (!skipCell)
            {
                if (ghosts && ghosts[cells[i]] > 0)
                {
                    if (ghostType == AVT_HAS_GHOSTS)
                    {
                        ghostZones.push_back(1);
                        nGhosts++;
                    }
                    else
                    {
                        continue;
                    }
                }
                else if (ghostType == AVT_HAS_GHOSTS)
                {
                    ghostZones.push_back(0);
                }
                zones.push_back(zoneId);
                if ((pickAtts.GetShowZoneBlockLogicalCoords() ||
                     pickAtts.GetShowZoneDomainLogicalCoords()) &&
                    (type == VTK_STRUCTURED_GRID ||
                     type == VTK_RECTILINEAR_GRID))
                {
                    if (pickAtts.GetShowZoneDomainLogicalCoords())
                    {
                        vtkVisItUtility::GetLogicalIndices(ds, true, cells[i],
                                                           ijk, false, true, true);

                        if (pickAtts.GetDimension() == 2)
                        {
                            snprintf(buff, 80, "<%d, %d>", ijk[0] + cellOrigin,
                                     ijk[1] + cellOrigin);
                        }
                        else
                        {
                            snprintf(buff, 80, "<%d, %d, %d>", ijk[0] + cellOrigin,
                                     ijk[1] + cellOrigin, ijk[2] + cellOrigin);
                        }
                        dzoneCoords.push_back(buff);
                    }
                    if (pickAtts.GetShowZoneBlockLogicalCoords())
                    {
                        vtkVisItUtility::GetLogicalIndices(ds, true, cells[i],
                                                           ijk, true, true, true);

                        if (pickAtts.GetDimension() == 2)
                        {
                            snprintf(buff, 80, "<%d, %d>", ijk[0], ijk[1]);
                        }
                        else
                        {
                            snprintf(buff, 80, "<%d, %d, %d>",
                                     ijk[0], ijk[1], ijk[2]);
                        }
                        bzoneCoords.push_back(buff);
                    }
                }
            }
        }
        if (nGhosts == nCells)
            pickAtts.SetElementIsGhost(true);
        pickAtts.SetIncidentElements(zones);
        pickAtts.SetDzoneCoords(dzoneCoords);
        pickAtts.SetBzoneCoords(bzoneCoords);
        pickAtts.SetGhosts(ghostZones);
    }
    cellIds->Delete();
    if (success && pickAtts.GetShowGlobalIds())
    {
        SetGlobalIds(ds, foundNode);
    }
    return success;
}


// ****************************************************************************
//  Method: avtPickQuery::GetCurrentNodeForOriginal
//
//  Purpose:
//    Determines the zone in the dataset whose original node designation
//    matches that of the passed node.
//
//  Arguments:
//    ds         The dataset to retrieve information from.
//    origNode   An 'original' node id.
//
//  Returns:
//    The node id in ds whose original node id matches the arg.
//    If 'avtOriginalNodeNumbers' is not present, then the value is equal to
//    the arg value.
//
//  Programmer: Kathleen Bonnell
//  Creation:   December 15, 2004
//
//  Modifications:
//
// ****************************************************************************

int
avtPickQuery::GetCurrentNodeForOriginal(vtkDataSet *ds, const int origNode)
{
    int currentNode = origNode;
    vtkUnsignedIntArray *origNodes = vtkUnsignedIntArray::SafeDownCast(
        ds->GetPointData()->GetArray("avtOriginalNodeNumbers"));
    if (origNodes)
    {
        int nTuples = origNodes->GetNumberOfTuples();
        int nComp = origNodes->GetNumberOfComponents();
        int comp = nComp -1;
        unsigned int *on = origNodes->GetPointer(0);
        for (int i = 0; i < nTuples; i++)
        {
            if (on[i*nComp+comp] == (unsigned int)origNode)
            {
                currentNode = i;
                break;
            }
        }
    }
    return currentNode;
}

// ****************************************************************************
//  Method: avtPickQuery::GetCurrentZoneForOriginal
//
//  Purpose:
//    Determines the zone in the dataset whose original zone designation
//    matches that of the passed zone.
//
//  Arguments:
//    ds         The dataset to retrieve information from.
//    origZone   An 'original' zone id.
//
//  Returns:
//    The zone id in ds whose original zone id matches the arg.
//    If 'avtOriginalCellNumbers' is not present, then the value is equal to
//    the arg value.
//
//  Programmer: Kathleen Bonnell
//  Creation:   August 11, 2004
//
//  Modifications:
//
// ****************************************************************************

int
avtPickQuery::GetCurrentZoneForOriginal(vtkDataSet *ds, const int origZone)
{
    int currentZone = origZone;
    vtkUnsignedIntArray *origCells = vtkUnsignedIntArray::SafeDownCast(
        ds->GetCellData()->GetArray("avtOriginalCellNumbers"));
    if (origCells)
    {
        int nTuples = origCells->GetNumberOfTuples();
        int nComp = origCells->GetNumberOfComponents();
        int comp = nComp -1;
        unsigned int *oc = origCells->GetPointer(0);
        for (int i = 0; i < nTuples; i++)
        {
            if (oc[i*nComp+comp] == (unsigned int)origZone)
            {
                currentZone = i;
                break;
            }
        }
    }
    return currentZone;
}

// ****************************************************************************
//  Method: avtPickQuery::GetCurrentZoneForOriginal
//
//  Purpose:
//    Determines the  zones in the dataset whose original zone designation
///   matches those of the passed list.
//
//  Arguments:
//    ds         The dataset to retrieve information from.
//    origZones  A list of 'original' zone ids.
//
//  Returns:
//    The list of zone ids in ds whose original zone ids match those
//    of the passed list.  If 'avtOriginalCellNumbers' is not present,
//    then the returned list is equialent to the passed list.
//
//  Programmer: Kathleen Bonnell
//  Creation:   August 11, 2004
//
//  Modifications:
//    Kathleen Bonnell, Thu Jun 30 15:39:03 PDT 2005
//    Correct the logic for finding current zones.
//
//    Jeremy Meredith, Thu Jul  7 09:20:39 PDT 2005
//    Made the zoneFound array dynamically allocated to make more
//    compilers happy.
//
// ****************************************************************************

intVector
avtPickQuery::GetCurrentZoneForOriginal(vtkDataSet *ds,
                                        const intVector &origZones)
{
    intVector currentZones = origZones;
    vtkUnsignedIntArray *origCells = vtkUnsignedIntArray::SafeDownCast(
        ds->GetCellData()->GetArray("avtOriginalCellNumbers"));
    if (origCells)
    {
        int nTuples = origCells->GetNumberOfTuples();
        int nComp = origCells->GetNumberOfComponents();
        int comp = nComp -1;

        unsigned int *oc = origCells->GetPointer(0);
        int nFound = 0;
        bool *zoneFound = new bool[origZones.size()];
        for (size_t i = 0; i < origZones.size(); i++)
            zoneFound[i] = false;
        for (int i = 0; i < nTuples && nFound < (int)origZones.size(); i++)
        {
            for (size_t j = 0; j < currentZones.size(); j++)
            {
                if (!zoneFound[j] && oc[i*nComp+comp] == (unsigned int)origZones[j])
                {
                    currentZones[j] = i;
                    zoneFound[j]= true;
                    nFound++;
                    break;
                }
            }
        }
        delete[] zoneFound;
    }
    return currentZones;
}

// ****************************************************************************
//  Method: avtPickQuery::SetGlobalIds
//
//  Purpose:
//    Determines the global ids associated with element and incidentelements,
//    and stores them in pickatts.
//
//  Arguments:
//    ds         The dataset to retrieve information from.
//    element    A zone or node id.
//
//  Programmer: Kathleen Bonnell
//  Creation:   December 15, 2004
//
//  Modifications:
//
// ****************************************************************************

void
avtPickQuery::SetGlobalIds(vtkDataSet *ds, int element)
{
    vtkIntArray *globalZones = vtkIntArray::SafeDownCast(
        ds->GetCellData()->GetArray("avtGlobalZoneNumbers"));
    vtkIntArray *globalNodes = vtkIntArray::SafeDownCast(
        ds->GetPointData()->GetArray("avtGlobalNodeNumbers"));

    intVector gie;
    intVector incEls = pickAtts.GetIncidentElements();
    if (pickAtts.GetPickType() == PickAttributes::Zone ||
        pickAtts.GetPickType() == PickAttributes::DomainZone)
    {
        if (globalZones)
            pickAtts.SetGlobalElement(globalZones->GetValue(element));
        if (globalNodes)
            for (size_t i = 0; i < incEls.size(); i++)
                gie.push_back(globalNodes->GetValue(incEls[i]));
    }
    else
    {
        if (globalNodes)
            pickAtts.SetGlobalElement(globalNodes->GetValue(element));
        if (globalZones)
            for (size_t i = 0; i < incEls.size(); i++)
                gie.push_back(globalZones->GetValue(incEls[i]));
    }
    pickAtts.SetGlobalIncidentElements(gie);
}


// ****************************************************************************
//  Method: avtPickQuery::ConvertElNamesToGlobal
//
//  Purpose:
//    Converts the element names stored in pick atts to the global form.
//
//  Programmer: Kathleen Bonnell
//  Creation:   May 11, 2005
//
//  Modifications:
//
// ****************************************************************************

void
avtPickQuery::ConvertElNamesToGlobal(void)
{
    // var names were retrieved from DB, not global, so convert them.
    char buff[24];
    stringVector globalElName;
    snprintf(buff, 24, "(%d)", pickAtts.GetGlobalElement());
    globalElName.push_back(buff);
    intVector globalIds = pickAtts.GetGlobalIncidentElements();

    stringVector names;
    for (size_t i = 0; i < globalIds.size(); i++)
    {
        snprintf(buff, 24, "(%d)", globalIds[i]);
        names.push_back(buff);
    }

    bool zonePick = pickAtts.GetPickType() == PickAttributes::Zone ||
                    pickAtts.GetPickType() == PickAttributes::DomainZone;

    for (int i = 0; i < pickAtts.GetNumVarInfos(); i++)
    {
        if (zonePick)
        {
            if (pickAtts.GetVarInfo(i).GetCentering() ==
                PickVarInfo::Zonal)
                pickAtts.GetVarInfo(i).SetNames(globalElName);
            else
                pickAtts.GetVarInfo(i).SetNames(names);
        }
        else
        {
            if (pickAtts.GetVarInfo(i).GetCentering() ==
                PickVarInfo::Zonal)
                pickAtts.GetVarInfo(i).SetNames(names);
            else
                pickAtts.GetVarInfo(i).SetNames(globalElName);
        }
    }
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
//  Creation:   June 28, 2005
//
//  Modifications:
//
// ****************************************************************************

void
avtPickQuery::SetRealIds(vtkDataSet *ds)
{
    int foundEl= pickAtts.GetElementNumber();
    intVector incEls = pickAtts.GetIncidentElements();


    bool forCell = pickAtts.GetPickType() == PickAttributes::Zone ||
                   pickAtts.GetPickType() == PickAttributes::DomainZone;
    char fString[20], tmp[20];

    stringVector elStrings;
    foundEl = vtkVisItUtility::CalculateRealID(foundEl, forCell, ds);
    snprintf(fString, 20, "(%d)", foundEl);
    for (size_t i = 0; i < incEls.size(); i++)
    {
        incEls[i] = vtkVisItUtility::CalculateRealID(incEls[i], !forCell, ds);
        snprintf(tmp, 20, "(%d)", incEls[i]);
        elStrings.push_back(tmp);
    }

    // need to change the zone/node names stored in all PickVarInfo
    int numVars = pickAtts.GetNumVarInfos();
    for (int i = 0; i < numVars; i++)
    {
        if (pickAtts.GetVarInfo(i).GetVariableType() == "material")
            continue;

        stringVector &names = pickAtts.GetVarInfo(i).GetNames();
        if (names.size() == 0)
            continue;
        if (names.size() == incEls.size())
        {
            for (size_t j = 0; j < names.size(); j++)
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
//  Method: avtPickQuery::SetPickAttsForTimeQuery
//
//  Purpose:
//    Sets the pickAtts to specific passed values necessary for a time query.
//    These values would have been set during the Pick that is preliminary
//    to performing the time query.
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 24, 2006
//
// ****************************************************************************

void
avtPickQuery::SetPickAttsForTimeQuery(const PickAttributes *pa)
{
    pickAtts.SetMatSelected(pa->GetMatSelected());
    pickAtts.SetElementIsGlobal(pa->GetElementIsGlobal());
}

// ****************************************************************************
//  Method: avtPickQuery::ExtractZonePickHighlights
//
//  Purpose:
//    Extract the zone edges for pick highlighting. If the data set, has been
//    decomposed from something like arbitrary polyhedra, the zone id should
//    be the original zone id.
//
//  Programmer: Matt Larsen
//  Creation:   July 8, 2016
//
// ****************************************************************************

void
avtPickQuery::ExtractZonePickHighlights(const int &zoneId,
                                        vtkDataSet *ds,
                                        const int &dom)
{
   // Clear anything left over
   pickAtts.ClearLines();
   // Bail if highlights are not on
   if(!pickAtts.GetShowPickHighlight()) return;

   // Check to see if the cells were decomposed in some way
    vtkDataArray* origCellsArr = ds->GetCellData()->
        GetArray("avtOriginalCellNumbers");
    if ( (!origCellsArr) || (origCellsArr->GetDataType() != VTK_UNSIGNED_INT)
      || (origCellsArr->GetNumberOfComponents() != 2 ))
    {
        // this is a normal cell or could not find the proper
        // original cell information. Just extract the lines from the edges
        vtkCell *cell = ds->GetCell(zoneId);
        const int numEdges = cell->GetNumberOfEdges();

        for(int i = 0; i < numEdges; ++i)
        {
            vtkCell *edge = cell->GetEdge(i);
            vtkPoints *edgePoints = edge->GetPoints();
            const int numPoints = edgePoints->GetNumberOfPoints();
            if(numPoints != 2) continue;
            double p1[3];
            double p2[3];
            edgePoints->GetPoint(0, p1);
            edgePoints->GetPoint(1, p2);
            pickAtts.AddLine(p1, p2,i);
        }
        return;
    }
    else
    {
        // Find the cells that make up the original cell
        const int numCells = ds->GetNumberOfCells();
        unsigned int* origCellNums =
            ((vtkUnsignedIntArray*)origCellsArr)->GetPointer(0);
        std::vector<int> relatedCells;
        const unsigned int origCell = zoneId;
        const unsigned int origDom = dom;
        for(int i = 0; i < numCells; ++i)
        {
            if(origCell == origCellNums[i*2+1] &&
               origDom == origCellNums[i*2+0])
            {
                relatedCells.push_back(i);
            }
        }

        // loop over related cells and eliminate duplicate egdes
        vtkSmartPointer<vtkEdgeTable> edgeTable = vtkSmartPointer<vtkEdgeTable>::New();
        const int numRelated = relatedCells.size();

        for(int i = 0; i < numRelated; ++i)
        {
            const int cellId = relatedCells[i];
            vtkCell *cell = ds->GetCell(cellId);
            const int numEdges = cell->GetNumberOfEdges();
            // estimate some space requirements
            if(i == 0) edgeTable->InitEdgeInsertion(numEdges*numRelated);
            for(int j = 0; j < numEdges; ++j)
            {
                vtkCell *edge = cell->GetEdge(j);
                vtkPoints *edgePoints = edge->GetPoints();
                const int numPoints = edgePoints->GetNumberOfPoints();
                if(numPoints != 2) continue;
                vtkIdType p1 = edge->GetPointIds()->GetId(0);
                vtkIdType p2 = edge->GetPointIds()->GetId(1);

                vtkIdType idx = edgeTable->IsEdge(p1,p2);
                if(idx == -1) edgeTable->InsertEdge(p1,p2);
            }
        }

        // Iterate through the table and set the points in pictAtts
        const int totEdges = edgeTable->GetNumberOfEdges();

        edgeTable->InitTraversal();
        for(int i = 0; i < totEdges; ++i)
        {
            vtkIdType p1Id,p2Id;
            edgeTable->GetNextEdge(p1Id,p2Id);
            double p1[3],p2[3];
            ds->GetPoint(p1Id, p1);
            ds->GetPoint(p2Id, p2);
            pickAtts.AddLine(p1, p2,i);
        }
    }
}

// ****************************************************************************
//  Method: avtPickQuery::GetElementIdByLabel
//
//  Purpose:
//    This method translates between and element label and its actual element
//    id so the rest of the pick can proceed normally
//
//  Programmer: Matt Larsen
//  Creation:   May 4, 2017
//
// ****************************************************************************
bool
avtPickQuery::GetElementIdByLabel(const std::string &elementLabel, 
                                  bool isZone, 
                                  int &elementId,
                                  int dom)
{
    int timestep = 0;
    int chunk = dom;
    elementId = -1;
    avtDataRequest_p labelRequest;
    if(isZone)
    {
        labelRequest  = new avtDataRequest("OriginalZoneLabels", timestep, chunk);
    }
    else
    {
        labelRequest  = new avtDataRequest("OriginalNodeLabels", timestep, chunk);
    }
    int pipelineIndex = 0;
    avtContract_p contract_p = new avtContract(labelRequest, pipelineIndex);
    avtDataObject_p dataObject = this->GetInput();
    VoidRefList result;
    void * args = NULL;
    const char * type ="AUXILIARY_DATA_IDENTIFIERS";
    TRY
    {
        dataObject->GetOriginatingSource()->GetVariableAuxiliaryData(type, args, contract_p, result);
    }
    CATCH2(VisItException, e)
    {
        debug3<<"avtPickQuery: failed to find original zone/node labels";
        debug3<<e.Message()<<"\n";

        bool error = true;
        return error; 
    }
    ENDTRY
    for(int i = 0; i < result.nList; ++i)
    {
        vtkElementLabelArray *labels = reinterpret_cast<vtkElementLabelArray*>(*result.list[i]);
        if(labels == NULL)
        {
            continue;
        } 
        bool success = labels->GetElementId(elementLabel, elementId);
        if(success)
        {
            const char *raw_labels = reinterpret_cast<char *>(labels->GetVoidPointer(0));
            int offset = labels->GetNumberOfComponents() * elementId;
            const char *label = raw_labels + offset;
            if(pickAtts.GetUseLabelAsPickLetter())
            {
                pickAtts.SetPickLetter(std::string(label));
            }
        }
    }
    
    bool error = false;
    return error;
}
