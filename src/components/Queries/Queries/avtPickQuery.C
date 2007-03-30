// ************************************************************************* //
//                              avtPickQuery.C                               //
// ************************************************************************* //

#include <avtPickQuery.h>

#include <vtkDataSet.h>
#include <vtkStructuredGrid.h>
#include <vtkRectilinearGrid.h>
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
// ****************************************************************************

avtPickQuery::avtPickQuery()
{
    blockOrigin = cellOrigin = 0;
    invTransform = NULL;
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
// ****************************************************************************

void
avtPickQuery::PreExecute(void)
{
    blockOrigin = GetInput()->GetInfo().GetAttributes().GetBlockOrigin();
    cellOrigin = GetInput()->GetInfo().GetAttributes().GetCellOrigin();
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
// ****************************************************************************

void
avtPickQuery::Execute(vtkDataSet *ds, const int dom)
{
    if (dom != pickAtts.GetDomain() || pickAtts.GetFulfilled())
    {
        return;
    }

    avtDataSpecification_p dspec = 
        GetInput()->GetTerminatingSource()->GetFullDataSpecification();
    int ts = dspec->GetTimestep();
    pickAtts.SetTimeStep(ts);
    intVector dlist;
    dspec->GetSIL().GetDomainList(dlist);

    int foundZone = pickAtts.GetZoneNumber();
    int type = ds->GetDataObjectType();

    if (foundZone == -1)
    {
        //
        //  If a valid zone hasn't already been determined, locate it now.
        //  
        if (ds->GetDataObjectType() != VTK_RECTILINEAR_GRID)
        {
            foundZone = LocatorFindCell(ds);
        }
        else
        {
            foundZone = RGridFindCell((vtkRectilinearGrid*)ds);
        }


        //
        //  If a valid zone still hasn't been found, there's some kind of 
        //  problem, it should have been found for this domain. 
        //  
        if (foundZone == -1)
        {
            pickAtts.SetDomain(-1);
            pickAtts.SetZoneNumber(-1);
            debug5 << "PICK BIG PROBLEM!  "
                   << "Could not find zone corresponding to pick point" << endl;
            return;
        }
        //
        // Retrieve nodes for foundZone.
        // 
        vtkIdList *ptIds = vtkIdList::New();
        intVector nodes;
        stringVector nodeCoords;
        float coord[3];
        int ijk[3];
        char buff[80];
        ds->GetCellPoints(foundZone, ptIds);
        int dim = GetInput()->GetInfo().GetAttributes().GetSpatialDimension();
        pickAtts.SetDimension(dim);
        if (ptIds->GetNumberOfIds() == 0)
        {
            pickAtts.SetDomain(-1);
            pickAtts.SetZoneNumber(-1);
            ptIds->Delete();
            debug5 << "FoundZone contained no points!" << endl;
            return;
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
        }
        pickAtts.SetFulfilled(true);
        pickAtts.SetNodes(nodes);
        pickAtts.SetNodeCoords(nodeCoords);
        pickAtts.SetZoneNumber(foundZone);
 

        //
        //  Retrieve var info.
        // 
        stringVector userVars = pickAtts.GetUserSelectedVars();
        string vName;
        double *temp;
        double mag;
        int nComponents;
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
                nComponents = varArray->GetNumberOfComponents(); 
                temp = new double[nComponents];
                for (int k = 0; k < nodes.size(); k++)
                {
                    sprintf(buff, "(%d)", nodes[k]);
                    names.push_back(buff);
                    varArray->GetTuple(nodes[k], temp);
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
                } // for all nodes
            } 
            else
            { 
                varArray = ds->GetCellData()->GetArray(vName.c_str());
                if (varArray != NULL) // zonal data
                {
                    varInfo.SetCentering(PickVarInfo::Zonal);
                    sprintf(buff, "(%d)", foundZone);
                    names.push_back(buff);
                    nComponents = varArray->GetNumberOfComponents();
                    temp = new double[nComponents];
                    varArray->GetTuple(foundZone, temp);
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
            }  
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

    //
    //  Allow the database to add any missing information.
    // 
    GetInput()->GetTerminatingSource()->Query(&pickAtts);

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

    if (type == VTK_RECTILINEAR_GRID || type == VTK_STRUCTURED_GRID) 
    {
        foundZone = vtkVisItUtility::CalculateRealCellID(foundZone, ds);
    }

    pickAtts.SetZoneNumber(foundZone + cellOrigin);

    //
    // If the points of this dataset have been transformed, and we know the 
    // inverse transform matrix, transform the pick point that will be 
    // displayed in the pick info window.
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
// ****************************************************************************

int
avtPickQuery::LocatorFindCell(vtkDataSet *ds)
{
    //
    // Use the picked point that has been moved towards the cell center.
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
        // Reset the cell point, so that the DB uses the same point. 
        //
        pickAtts.SetCellPoint(cellPoint);
    }


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
// ****************************************************************************

avtDataObject_p
avtPickQuery::ApplyFilters(avtDataObject_p inData)
{
    avtDataAttributes &inAtts = inData->GetInfo().GetAttributes();
    if (inAtts.HasTransform() && inAtts.GetCanUseTransform())
    {
        invTransform = inAtts.GetTransform();
    }
    else
    {
        invTransform = NULL;
    }
    if (inData->GetInfo().GetValidity().GetPointsWereTransformed() &&
        invTransform == NULL)
    {
        pickAtts.SetNeedTransformMessage(true);
    }
    else
    {
        pickAtts.SetNeedTransformMessage(false);
    }

    return inData->GetTerminatingSource()->GetOutput();
}


