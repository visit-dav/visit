// ************************************************************************* //
//                             avtVariableQuery.C                           //
// ************************************************************************* //

#include <avtVariableQuery.h>

#include <snprintf.h>

#include <vector>
#include <float.h>
#include <vtkCellData.h>
#include <vtkIdList.h>
#include <vtkPointData.h>
#include <vtkDataSet.h>
#include <vtkVisItUtility.h>
#include <vtkUnsignedCharArray.h>

#include <avtExpressionEvaluatorFilter.h>
#include <avtTerminatingSource.h>
#include <PickVarInfo.h>
#include <NonQueryableInputException.h>

#include <DebugStream.h>

#ifdef PARALLEL
#include <mpi.h>
#include <BufferConnection.h>
#endif

using std::vector;
using std::string;



// ****************************************************************************
//  Method: avtVariableQuery::avtVariableQuery
//
//  Purpose:
//      Construct an avtVariableQuery object.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     July 23, 2003
//
//  Modifications:
//    Kathleen Bonnell, Thu Apr  1 19:06:07 PST 2004
//    Added eef and src.
//
// ****************************************************************************

avtVariableQuery::avtVariableQuery()
{
    searchDomain = 0;
    searchElement = 0;
    eef = new avtExpressionEvaluatorFilter;
    src = NULL;
}

// ****************************************************************************
//  Method: avtVariableQuery::~avtVariableQuery
//
//  Purpose:
//      Destruct an avtVariableQuery object.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     July 23, 2003 
//
//  Modifications:
//    Kathleen Bonnell, Thu Apr  1 19:06:07 PST 2004
//    Added eef. 
//
// ****************************************************************************

avtVariableQuery::~avtVariableQuery()
{
    if (eef != NULL)
    {
        delete eef;
        eef = NULL;
    }
}

 
// ****************************************************************************
//  Method: avtVariableQuery::VerifyInput
//
//  Purpose:
//    Verify a new input.  Overrides base class in order to allow vectors
//    (topo dim == 0) to be queried.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 31, 2004 
//
// ****************************************************************************

void
avtVariableQuery::VerifyInput()
{
    if (!GetInput()->GetInfo().GetValidity().GetQueryable())
    {
        EXCEPTION0(NonQueryableInputException);
    }
}

 


// ****************************************************************************
//  Method: avtVariableQuery::Execute
//
//  Purpose:
//    Retrieves var information from the dataset, based on a domain and zone.
//
//  Arguments:
//    ds          The input dataset.
//    dom         The domain number.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     July 23, 2003
//
//  Modifications:
//    Kathleen Bonnell, Thu Apr  1 19:06:07 PST 2004
//    Added Calls to RetrieveNodes, RetrieveZones and RetrieveVarInfo.
//
// ****************************************************************************

void 
avtVariableQuery::Execute(vtkDataSet *ds, const int dom)
{
    if (dom != searchDomain || pickAtts.GetFulfilled() || ds == NULL)
    {
        return;
    }

    avtDataSpecification_p dspec = 
        GetInput()->GetTerminatingSource()->GetFullDataSpecification();

    pickAtts.SetTimeStep(queryAtts.GetTimeStep());
    pickAtts.SetActiveVariable(dspec->GetVariable());
    pickAtts.SetDomain(searchDomain);
    pickAtts.SetElementNumber(searchElement);
    pickAtts.SetVariables(queryAtts.GetVariables());

    bool success = false;
    if (queryAtts.GetElementType() == QueryAttributes::Zone)
    {
        pickAtts.SetPickType(PickAttributes::Zone);
        success = RetrieveNodes(ds, searchElement);
    }
    else 
    {
        pickAtts.SetPickType(PickAttributes::Node);
        success = RetrieveZones(ds, searchElement);
    }


    intVector dlist;
    dspec->GetSIL().GetDomainList(dlist);

    if (success)
    {
        RetrieveVarInfo(ds);
        pickAtts.SetFulfilled(true);

        src->Query(&pickAtts);

        if (dlist.size() == 1 && dspec->UsesAllDomains())
        {
            //
            // Indicate that there was only one domain.
            // We don't report the domain number for single-domain problems.
            //
            pickAtts.SetDomain(-1);
        }
        else
            pickAtts.SetDomain(queryAtts.GetDomain());
    
        pickAtts.SetElementNumber(queryAtts.GetElement());
    }
    else
    {
        pickAtts.SetDomain(-1);
        pickAtts.SetElementNumber(-1);
    }
}

// ****************************************************************************
//  Method: avtVariableQuery::PostExecute
//
//  Purpose:
//    This is called after all of the domains are executed.
//    If in parallel, collects the correct pickAtts from the processor that
//    gathered the info, to processor 0.
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 23, 2003 
//
//  Modifications:
//    Kathleen Bonnell, Thu Apr  1 19:06:07 PST 2004
//    Set query result values from PickVarInfo.  Reset picks atts.
//
// ****************************************************************************

void
avtVariableQuery::PostExecute(void)
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

    if (pickAtts.GetFulfilled())
    {
        // Special indication that the pick point should not be displayed.
        float cp[3] = { FLT_MAX, FLT_MAX, FLT_MAX };
        string msg;
        pickAtts.SetCellPoint(cp);
        pickAtts.CreateOutputString(msg);
        SetResultMessage(msg.c_str());
        SetResultValues(pickAtts.GetPickVarInfo(0).GetValues());
    }
    else
    {
        char msg[120]; 
        SNPRINTF(msg, 120, "Could not retrieve information from domain "
                 " %d element %d.", queryAtts.GetDomain(), queryAtts.GetElement());
        SetResultMessage(msg);
    }
    pickAtts.PrepareForNewPick();
}


// ****************************************************************************
//  Method: avtVariableQuery::PreExecute
//
//  Purpose:
//    This is called before any of the domains are executed.
//    Retrieves the correct spatial dimension and sets it in pickAtts. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 23, 2003
//
//  Modifications:
//    Kathleen Bonnell, Thu Apr  1 08:07:13 PST 2004
//    Moved code to ApplyFilters method.
//
// ****************************************************************************

void
avtVariableQuery::PreExecute()
{
}


// ****************************************************************************
//  Method: avtVariableQuery::ApplyFilters
//
//  Purpose:
//    Retrieves the termnating source to use as input. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 23, 2003
//
//  Modifications:
//    Kathleen Bonnell, Thu Apr  1 09:21:22 PST 2004
//    Reworked code so that this query will always return the same results
//    as Pick.  Also allow for time-varying query.
//
//    Hank Childs, Fri Apr  9 16:25:40 PDT 2004
//    Minimize work done by creating new SIL.
//
// ****************************************************************************

avtDataObject_p
avtVariableQuery::ApplyFilters(avtDataObject_p inData)
{
    avtDataAttributes &data = GetInput()->GetInfo().GetAttributes();

    pickAtts.SetDimension(data.GetSpatialDimension());
    searchDomain = queryAtts.GetDomain() - data.GetBlockOrigin();
    searchElement = queryAtts.GetElement(); 
    if (queryAtts.GetElementType() == QueryAttributes::Zone)
        searchElement -=  data.GetCellOrigin();

    src = inData->GetQueryableSource();
    int i;
    avtDataSpecification_p dspec; 
    if (!timeVarying)
    {
        dspec = inData->GetTerminatingSource()->
            GetGeneralPipelineSpecification()->GetDataSpecification();
    }
    else 
    {
        avtDataSpecification_p oldSpec = inData->GetTerminatingSource()->
            GetGeneralPipelineSpecification()->GetDataSpecification();

        dspec = new 
            avtDataSpecification(oldSpec->GetVariable(), queryAtts.GetTimeStep(), 
                                 oldSpec->GetRestriction());

    }
    dspec->GetRestriction()->SuspendCorrectnessChecking();
    dspec->GetRestriction()->TurnOnAll();
    for (i = 0; i < silUseSet.size(); i++)
    {
        if (silUseSet[i] == 0)
            dspec->GetRestriction()->TurnOffSet(i);
    }
    dspec->GetRestriction()->EnableCorrectnessChecking();

    intVector dlist;
    dlist.push_back(searchDomain);
    dspec->GetRestriction()->RestrictDomains(dlist);

    stringVector vars = queryAtts.GetVariables();

    for (i = 0; i < vars.size(); i++)
    {
        if (dspec->GetVariable() != vars[i])
        {
            if (!dspec->HasSecondaryVariable(vars[i].c_str()))
                 dspec->AddSecondaryVariable(vars[i].c_str());
        }
    }
 
    avtPipelineSpecification_p pspec = 
        new avtPipelineSpecification(dspec, queryAtts.GetPipeIndex());

    avtDataObject_p temp;
    CopyTo(temp, inData);
    eef->SetInput(temp);
    avtDataObject_p retObj = eef->GetOutput();
    retObj->Update(pspec);
    return retObj;
}


// ****************************************************************************
//  Method: avtVariableQuery::RetrieveVarInfo
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
//    
// ****************************************************************************

void
avtVariableQuery::RetrieveVarInfo(vtkDataSet* ds)
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
            if (pickAtts.GetPickVarInfo(varNum).HasInfo())
                continue;
            vName = pickAtts.GetPickVarInfo(varNum).GetVariableName();
        }
        else
        {
            vName = userVars[varNum];
        }
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

        if (pickAtts.GetFulfilled())
        {
            if (!names.empty())
            {
                pickAtts.GetPickVarInfo(varNum).SetNames(names);
                pickAtts.GetPickVarInfo(varNum).SetValues(vals);
                pickAtts.GetPickVarInfo(varNum).SetCentering(centering);
                if (nComponents == 1)
                    pickAtts.GetPickVarInfo(varNum).SetVariableType("scalar");
                else if (nComponents == 3)
                    pickAtts.GetPickVarInfo(varNum).SetVariableType("vector");
                else if (nComponents == 9)
                    pickAtts.GetPickVarInfo(varNum).SetVariableType("tensor");
                delete [] temp; 
            }
        }
        else
        {
            PickVarInfo varInfo;
            varInfo.SetVariableName(vName);
            varInfo.SetCentering(centering);
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
//  Method: avtVariableQuery::RetrieveNodes
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
// ****************************************************************************

bool
avtVariableQuery::RetrieveNodes(vtkDataSet *ds, int zone)
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
                        sprintf(buff, "<%d, %d>", ijk[0], ijk[1]);
                    }
                    else 
                    {
                        sprintf(buff, "<%d, %d, %d>", ijk[0], ijk[1], ijk[2]);
                    }
                    dnodeCoords.push_back(buff);
                }
                if (pickAtts.GetShowNodeBlockLogicalCoords())
                {
                    vtkVisItUtility::GetLogicalIndices(ds, false, 
                         ptIds->GetId(i), ijk, true);
                    if (pickAtts.GetDimension() == 2)
                    {
                        sprintf(buff, "<%d, %d>", ijk[0], ijk[1]);
                    }
                    else 
                    {
                        sprintf(buff, "<%d, %d, %d>", ijk[0], ijk[1], ijk[2]);
                    }
                    bnodeCoords.push_back(buff);
                }
            }
            if (pickAtts.GetShowNodePhysicalCoords())
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
                pnodeCoords.push_back(buff);
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
        pickAtts.SetPnodeCoords(pnodeCoords);
        pickAtts.SetDnodeCoords(dnodeCoords);
        pickAtts.SetBnodeCoords(bnodeCoords);
    }
    return success;
}


// ****************************************************************************
//  Method: avtVariableQuery::RetrieveZones
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
// ****************************************************************************

bool
avtVariableQuery::RetrieveZones(vtkDataSet *ds, int foundNode)
{
    vtkIdList *cellIds = vtkIdList::New();
    intVector zones;
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
            GetArray("vtkGhostLevels");
        if (ghostArray)
            ghosts = ghostArray->GetPointer(0);

        vtkIdType *cells = cellIds->GetPointer(0);
        for (int i = 0; i < nCells; i++)
        {
            if (ghosts && ghosts[cells[i]] == 1)
               continue;
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
                        sprintf(buff, "<%d, %d>", ijk[0], ijk[1]);
                    }
                    else 
                    {
                        sprintf(buff, "<%d, %d, %d>", ijk[0], ijk[1], ijk[2]);
                    }
                    dzoneCoords.push_back(buff);
                }
                if (pickAtts.GetShowZoneBlockLogicalCoords())
                {
                    vtkVisItUtility::GetLogicalIndices(ds, true, cells[i], ijk, 
                      true);

                    if (pickAtts.GetDimension() == 2)
                    {
                        sprintf(buff, "<%d, %d>", ijk[0], ijk[1]);
                    }
                    else 
                    {
                        sprintf(buff, "<%d, %d, %d>", ijk[0], ijk[1], ijk[2]);
                    }
                    bzoneCoords.push_back(buff);
                }
            }
        }
        pickAtts.SetIncidentElements(zones);
        pickAtts.SetDzoneCoords(dzoneCoords);
        pickAtts.SetBzoneCoords(bzoneCoords);
    }
    cellIds->Delete();
    return success;
}

