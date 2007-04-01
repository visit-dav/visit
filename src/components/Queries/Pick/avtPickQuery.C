// ************************************************************************* //
//                              avtPickQuery.C                               //
// ************************************************************************* //

#include <avtPickQuery.h>

#include <float.h>
#include <snprintf.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkIdList.h>
#include <vtkIntArray.h>
#include <vtkMath.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnsignedIntArray.h>
#include <vtkVisItUtility.h>

#include <avtExpressionEvaluatorFilter.h>
#include <avtMatrix.h>
#include <avtParallel.h>
#include <avtTerminatingSource.h>
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
// ****************************************************************************

avtPickQuery::avtPickQuery()
{
    blockOrigin = cellOrigin = 0;
    transform = NULL;
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
//    Kathleen Bonnell, Thu Jul  1 16:41:57 PDT 2004 
//    Removed mpi calls, use GetAttToRootProc instead. 
//
// ****************************************************************************

void
avtPickQuery::PostExecute(void)
{
    int hasFulfilledPick = (int) pickAtts.GetFulfilled();
    GetAttToRootProc(pickAtts, hasFulfilledPick);

    if (PAR_Rank() == 0)
    {
        //
        // Remove any "mesh" PickVarInfo's, as they are unnecessary
        //
        for (int i = pickAtts.GetNumPickVarInfos()-1; i >= 0; i--)
        {
            if (pickAtts.GetPickVarInfo(i).GetVariableType() == "mesh")
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
    //  
    //  If maxDom == -1, then all procs will be doing real work.  Otherwise,
    //  only the proc with pickAtt.domain == maxDom will do real work.
    //  
    int maxDom = UnifyMaximumValue(pickAtts.GetDomain());

    if (!singleDomain && maxDom != -1)
    {
        intVector dlist;
        if (maxDom == pickAtts.GetDomain())
        {
            dlist.push_back(pickAtts.GetDomain());
        }
        dspec->GetRestriction()->RestrictDomains(dlist);
    }

    // 
    // Only set vars and turn on zone/node numbers if this process
    // will be doing real work.
    // 
    if (maxDom == -1 || maxDom == pickAtts.GetDomain())
    {
        stringVector vars = pickAtts.GetVariables();
        for (int i = 0; i < vars.size(); i++)
        {
            if (dspec->GetVariable() != vars[i]) 
            {
                if (!dspec->HasSecondaryVariable(vars[i].c_str()))
                    dspec->AddSecondaryVariable(vars[i].c_str());
            }
        }

        if (pickAtts.GetMatSelected())
        {
            dspec->TurnZoneNumbersOn();
            dspec->TurnNodeNumbersOn();
        }
        if (pickAtts.GetDisplayGlobalIds() || pickAtts.GetElementIsGlobal()) 
        {
            dspec->TurnGlobalZoneNumbersOn();
            dspec->TurnGlobalNodeNumbersOn();
            dspec->TurnZoneNumbersOn();
            dspec->TurnNodeNumbersOn();
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
//    Brad Whitlock, Thu Jul 29 17:26:26 PST 2004
//    I made it use SNPRINTF.
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
               SNPRINTF(buff, 80, "<%d, %d>", ijk[0], ijk[1]);
           }
           else 
           {
               SNPRINTF(buff, 80, "<%d, %d, %d>", ijk[0], ijk[1], ijk[2]);
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
               SNPRINTF(buff, 80, "<%d, %d>", ijk[0], ijk[1]);
           }
           else 
           {
               SNPRINTF(buff, 80, "<%d, %d, %d>", ijk[0], ijk[1], ijk[2]);
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
           SNPRINTF(buff, 80, "<%g, %g>", coord[0], coord[1]);
       }
       else 
       {
           SNPRINTF(buff, 80, "<%g, %g, %g>", coord[0], coord[1], coord[2]);
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
//    Brad Whitlock, Thu Jul 29 17:23:14 PST 2004
//    I made it use the cell origin for the zonal domain logical coordinates.
//    I also made it use SNPRINTF.
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
                SNPRINTF(buff, 80, "<%d, %d>", ijk[0] + cellOrigin,
                         ijk[1] + cellOrigin);
            }
            else 
            {
                SNPRINTF(buff, 80, "<%d, %d, %d>",
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
                        ijk, true);
            if (pickAtts.GetDimension() == 2)
            {
                SNPRINTF(buff, 80, "<%d, %d>", ijk[0], ijk[1]);
            }
            else 
            {
                SNPRINTF(buff, 80, "<%d, %d, %d>", ijk[0], ijk[1], ijk[2]);
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
    RetrieveVarInfo(ds, pickAtts.GetElementNumber(), pickAtts.GetIncidentElements());
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
    char buff[80];
    intVector incidentElements = pickAtts.GetIncidentElements();
    double *temp;
    double mag;
    int nComponents;
    bool zonePick = pickAtts.GetPickType() == PickAttributes::Zone ||
                    pickAtts.GetPickType() == PickAttributes::DomainZone;
    bool zoneCent;
    bool foundData = true;



    int numVars;
    if (pickAtts.GetFulfilled())
        numVars = pickAtts.GetNumPickVarInfos();
    else 
        numVars = userVars.size();
    for (int varNum = 0; varNum < numVars; varNum++)
    {
        stringVector names; 
        doubleVector vals; 
        PickVarInfo::Centering centering;
        if (pickAtts.GetFulfilled())
        {
            if (pickAtts.GetPickVarInfo(varNum).HasInfo() &&
                pickAtts.GetPickVarInfo(varNum).GetVariableType() != "species" &&
                pickAtts.GetPickVarInfo(varNum).GetVariableType() != "scalar") 
                continue;

            vName = pickAtts.GetPickVarInfo(varNum).GetVariableName();
        }
        else
        {
            vName = userVars[varNum];
        }

        if (data.ValidVariable(vName))
            treatAsASCII = data.GetTreatAsASCII(vName.c_str());
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
        if (foundData)
        {
            nComponents = varArray->GetNumberOfComponents(); 
            temp = new double[nComponents];
            intVector globalIncEl = pickAtts.GetGlobalIncidentElements();
            if (zoneCent != zonePick)
            {
                // data we want is associated with incidentElements
                for (int k = 0; k < incidentElements.size(); k++)
                {
                    if (pickAtts.GetDisplayGlobalIds() && 
                        globalIncEl.size() == incidentElements.size())
                    {
                        SNPRINTF(buff, 80, "(%d)", globalIncEl[k]);
                    }
                    else 
                    {
                        SNPRINTF(buff, 80, "(%d)", incidentElements[k]);
                    }
                    names.push_back(buff);
                    varArray->GetTuple(findIncidentElements[k], temp);
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
                if (pickAtts.GetDisplayGlobalIds() && 
                    pickAtts.GetGlobalElement() != -1)
                    SNPRINTF(buff, 80, "(%d)", pickAtts.GetGlobalElement());
                else 
                    SNPRINTF(buff, 80, "(%d)", element);
                names.push_back(buff);
                varArray->GetTuple(findElement, temp);
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

        if (pickAtts.GetFulfilled())
        {
            if (!names.empty())
            {
                pickAtts.GetPickVarInfo(varNum).SetNames(names);
                pickAtts.GetPickVarInfo(varNum).SetValues(vals);
                pickAtts.GetPickVarInfo(varNum).SetCentering(centering);
                pickAtts.GetPickVarInfo(varNum).SetTreatAsASCII(treatAsASCII);
                if (pickAtts.GetPickVarInfo(varNum).GetVariableType() != "species")
                { 
                    if (nComponents == 1)
                        pickAtts.GetPickVarInfo(varNum).SetVariableType("scalar");
                    else if (nComponents == 3)
                        pickAtts.GetPickVarInfo(varNum).SetVariableType("vector");
                    else if (nComponents == 9)
                        pickAtts.GetPickVarInfo(varNum).SetVariableType("tensor");
                } 
                delete [] temp; 
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
                delete [] temp; 
                if (nComponents == 1)
                    varInfo.SetVariableType("scalar");
                else if (nComponents == 3)
                    varInfo.SetVariableType("vector");
                else if (nComponents == 9)
                    varInfo.SetVariableType("tensor");
            }
            pickAtts.AddPickVarInfo(varInfo);
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
//    Made it use SNPRINTF.
//
//    Kathleen Bonnell, Thu Sep 23 17:38:15 PDT 2004 
//    Added logic to search for ghost nodes, if ghostType == AVT_HAS_GHOSTS. 
//
//    Kathleen Bonnell, Wed Dec 15 09:19:39 PST 2004 
//    Added call to 'SetGlobalIds'. 
//
// ****************************************************************************

bool
avtPickQuery::RetrieveNodes(vtkDataSet *ds, int zone)
{
    vtkIdList *ptIds = vtkIdList::New();
    intVector nodes;
    stringVector pnodeCoords;
    stringVector dnodeCoords;
    stringVector bnodeCoords;
    float coord[3];
    int ijk[3];
    char buff[80];
    int type = ds->GetDataObjectType();
    bool success = true;
    ds->GetCellPoints(zone, ptIds);
    intVector ghostNodes;

    if (ptIds->GetNumberOfIds() == 0)
    {
        ptIds->Delete();
        debug5 << "FoundZone contained no points!" << endl;
        success = false;
    }
    else
    {
        unsigned char *gNodes = NULL; 
        unsigned char *gZones = NULL; 
        vtkIdList *cells;
        bool findGhosts = (ghostType == AVT_HAS_GHOSTS &&
                         ((ds->GetPointData()->GetArray("avtGhostNodes") != NULL) ||
                          (ds->GetCellData()->GetArray("avtGhostZones") != NULL)));
        pickAtts.SetIncludeGhosts(findGhosts);
        if (findGhosts)
        {
            vtkUnsignedCharArray *gn = 
               (vtkUnsignedCharArray*)ds->GetPointData()->GetArray("avtGhostNodes");
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
        
        int nGnodes = 0; 
        for (int i = 0; i < ptIds->GetNumberOfIds(); i++)
        {
            vtkIdType ptId = ptIds->GetId(i);
            nodes.push_back(ptId);
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
                         ptIds->GetId(i), ijk, false);
                    if (pickAtts.GetDimension() == 2)
                    {
                        SNPRINTF(buff, 80, "<%d, %d>", ijk[0], ijk[1]);
                    }
                    else 
                    {
                        SNPRINTF(buff, 80, "<%d, %d, %d>", ijk[0], ijk[1], ijk[2]);
                    }
                    dnodeCoords.push_back(buff);
                }
                if (pickAtts.GetShowNodeBlockLogicalCoords())
                {
                    vtkVisItUtility::GetLogicalIndices(ds, false, 
                         ptIds->GetId(i), ijk, true);
                    if (pickAtts.GetDimension() == 2)
                    {
                        SNPRINTF(buff, 80, "<%d, %d>", ijk[0], ijk[1]);
                    }
                    else 
                    {
                        SNPRINTF(buff, 80, "<%d, %d, %d>", ijk[0], ijk[1], ijk[2]);
                    }
                    bnodeCoords.push_back(buff);
                }
            }
            if (pickAtts.GetShowNodePhysicalCoords())
            {
                ds->GetPoint(ptIds->GetId(i), coord); 
                if (pickAtts.GetDimension() == 2)
                {
                    SNPRINTF(buff, 80, "<%g, %g>", coord[0], coord[1]);
                }
                else 
                {
                    SNPRINTF(buff, 80, "<%g, %g, %g>", coord[0], coord[1], coord[2]);
                }
                pnodeCoords.push_back(buff);
            }
        }

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
        pickAtts.SetPnodeCoords(pnodeCoords);
        pickAtts.SetDnodeCoords(dnodeCoords);
        pickAtts.SetBnodeCoords(bnodeCoords);
        pickAtts.SetGhosts(ghostNodes);
        pickAtts.SetElementIsGhost((gZones && gZones[zone]) ||
            (nGnodes > 0 && nGnodes == ptIds->GetNumberOfIds()));
        ptIds->Delete();
    }
    if (success && pickAtts.GetDisplayGlobalIds())
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
//    changed sprintf to SNPRINTF.
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
// ****************************************************************************

bool
avtPickQuery::RetrieveZones(vtkDataSet *ds, int foundNode)
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
            GetArray("avtGhostZones");
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
            zones.push_back(cells[i]);
            if ((pickAtts.GetShowZoneBlockLogicalCoords() ||
                 pickAtts.GetShowZoneDomainLogicalCoords()) &&
                (type == VTK_STRUCTURED_GRID || 
                 type == VTK_RECTILINEAR_GRID))
            {
                if (pickAtts.GetShowZoneDomainLogicalCoords())
                {
                    vtkVisItUtility::GetLogicalIndices(ds, true, cells[i], ijk, 
                      false);

                    if (pickAtts.GetDimension() == 2)
                    {
                        SNPRINTF(buff, 80, "<%d, %d>", ijk[0] + cellOrigin,
                                 ijk[1] + cellOrigin);
                    }
                    else 
                    {
                        SNPRINTF(buff, 80, "<%d, %d, %d>", ijk[0] + cellOrigin,
                                 ijk[1] + cellOrigin, ijk[2] + cellOrigin);
                    }
                    dzoneCoords.push_back(buff);
                }
                if (pickAtts.GetShowZoneBlockLogicalCoords())
                {
                    vtkVisItUtility::GetLogicalIndices(ds, true, cells[i], ijk, 
                      true);

                    if (pickAtts.GetDimension() == 2)
                    {
                        SNPRINTF(buff, 80, "<%d, %d>", ijk[0], ijk[1]);
                    }
                    else 
                    {
                        SNPRINTF(buff, 80, "<%d, %d, %d>", ijk[0], ijk[1], ijk[2]);
                    }
                    bzoneCoords.push_back(buff);
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
    if (success && pickAtts.GetDisplayGlobalIds())
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
///   matches that of the passed node.
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
            if (on[i*nComp+comp] == origNode)
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
            if (oc[i*nComp+comp] == origZone)
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
//
// ****************************************************************************

intVector
avtPickQuery::GetCurrentZoneForOriginal(vtkDataSet *ds, const intVector &origZones)
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
        for (int i = 0; i < nTuples && nFound < origZones.size(); i++)
        {
            for (int j = 0; j < currentZones.size(); j++)
            {
                if (oc[i*nComp+comp] == origZones[j])
                {
                    currentZones[j] = i; 
                    nFound++;
                    break;
                }
            }
        }
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
            for (int i = 0; i < incEls.size(); i++)
                gie.push_back(globalNodes->GetValue(incEls[i]));
    
    }
    else 
    {
        if (globalNodes)
            pickAtts.SetGlobalElement(globalNodes->GetValue(element));
        if (globalZones)
            for (int i = 0; i < incEls.size(); i++)
                gie.push_back(globalZones->GetValue(incEls[i]));
    
    }
    pickAtts.SetGlobalIncidentElements(gie);
}
