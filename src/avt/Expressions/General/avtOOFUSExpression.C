// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      avtOOFUSExpression.C                        //
// ************************************************************************* //
#include <avtOOFUSExpression.h>

#include <math.h>

#include <avtDatabaseMetaData.h>
#include <avtExprNode.h>
#include <avtExpressionEvaluatorFilter.h>
#include <avtFacelistFilter.h>
#include <avtIntervalTree.h>
#include <avtMetaData.h>
#include <avtParallel.h>
#include <avtOriginatingSource.h>
#include <avtExecutionManager.h>

#include <vtkAppendFilter.h>
#include <vtkCharArray.h>
#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkCellType.h>
#include <vtkDataSet.h>
#include <vtkDataSetRemoveGhostCells.h>
#include <vtkIntArray.h>
#include <vtkPointData.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnstructuredGridRelevantPointsFilter.h>
#include <vtkUnstructuredGridWriter.h>
#include <vtkUnstructuredGridReader.h>
#include <vtkVisItUtility.h>

#include <sstream>
#include <DebugStream.h>
#include <ExpressionException.h>
#include <TimingsManager.h>
#include <Utility.h>


#ifdef PARALLEL
  #include <mpi.h>
#endif

#include <string>
#include <vector>

// ****************************************************************************
//  Method: avtOOFUSExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Justin Privitera
//  Creation:   August 18, 2025
//
//  Modifications:
// ****************************************************************************

avtOOFUSExpression::avtOOFUSExpression()
{
    nFinalComps = 0;
    enableGhostNeighbors = 0;
    canApplyToDirectDatabaseQOT = false;

    volumeDependent = vtkBitArray::New();
    volumeDependent->SetName("VolumeDependent");
    volumeDependent->SetNumberOfComponents(1);
    volumeDependent->SetNumberOfTuples(1);
    volumeDependent->SetComponent(0, 0, false); // Default volume dependency to false

    totalNodes  = 0;
}


// ****************************************************************************
//  Method: avtOOFUSExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Justin Privitera
//  Creation:   August 18, 2025
//
//  Modifications:
// ****************************************************************************

avtOOFUSExpression::~avtOOFUSExpression()
{
    volumeDependent->Delete();
}


// ****************************************************************************
//  Method: avtOOFUSExpression::GetNumberOfComponents
//
//  Purpose:
//      After expression execution returns the final number of components.
//
//  Programmer: Justin Privitera
//  Creation:   August 18, 2025
//
//  Modifications:
//
// ****************************************************************************
int 
avtOOFUSExpression::GetNumberOfComponents()
{
    return nFinalComps;
}

// ****************************************************************************
// ****************************************************************************
void
avtOOFUSExpression::CalculateWithoutGhosts(vtkDataArray *in, 
                                           vtkDataArray *out,
                                           int ncomponents,
                                           int ntuples)
{
    for (int comp_id = 0; comp_id < ncomponents; comp_id ++)
    {
        double comp_max = in->GetComponent(0, comp_id);
        // start at 1 since we already looked at the 0th element
        for (int tuple_id = 1; tuple_id < ntuples; tuple_id ++)
        {
            const double val = in->GetComponent(tuple_id, comp_id);
            if (val > comp_max)
            {
                comp_max = val;
            }
        }

        for (int tuple_id = 0; tuple_id < ntuples; tuple_id ++)
        {
            out->SetComponent(tuple_id, comp_id, comp_max);
        }
    }
}

// ****************************************************************************
// ****************************************************************************
void
avtOOFUSExpression::CalculateWithGhosts(vtkDataArray *in,
                                        vtkDataArray *out,
                                        int ncomponents,
                                        int ntuples,
                                        int (getNodeOrCellValid)(vtkDataArray *, int *, int),
                                        vtkDataArray *ghostZones,
                                        int *nodeShouldBeIgnoredPtr)
{
    for (int comp_id = 0; comp_id < ncomponents; comp_id ++)
    {
        int start_tuple_id = 0;
        double comp_max = [&]() -> double
        {
            for (int tuple_id = 0; tuple_id < ntuples; tuple_id ++)
            {
                if (0 == getNodeOrCellValid(ghostZones, nodeShouldBeIgnoredPtr, tuple_id))
                {
                    start_tuple_id = tuple_id + 1;
                    return in->GetComponent(tuple_id, comp_id);
                }
            }
            EXCEPTION2(ExpressionException, outputVariableName,
                 "Everything is ghosted so the OOFUS expression is not valid.");
            return 0; // return so the compiler is happy
        }();

        // start at start_tuple_id since it is the second non-ghosted tuple and we
        // have already looked at the first.
        for (int tuple_id = start_tuple_id; tuple_id < ntuples; tuple_id ++)
        {
            if (0 == getNodeOrCellValid(ghostZones, nodeShouldBeIgnoredPtr, tuple_id))
            {
                const double val = in->GetComponent(tuple_id, comp_id);
                if (val > comp_max)
                {
                    comp_max = val;
                }
            }
        }

        for (int tuple_id = 0; tuple_id < ntuples; tuple_id ++)
        {
            out->SetComponent(tuple_id, comp_id, comp_max);
        }
    }
}

// ****************************************************************************
// ****************************************************************************
std::vector<int>
avtOOFUSExpression::IdentifyGhostedNodes(vtkDataSet *in_ds,
                                                       vtkDataArray *ghostZones,
                                                       vtkDataArray *ghostNodes)
{
    const int nPoints = in_ds->GetNumberOfPoints();

    // we create an array to track if this point should be counted
    std::vector<int> nodeShouldBeIgnored(nPoints);
    if (ghostZones)
    {
        // if there are ghost zones, we want to initialize all points to not being counted
        fill(nodeShouldBeIgnored.begin(), nodeShouldBeIgnored.end(), true);
    }
    else
    {
        // If there are ghost nodes and NOT ghost zones, we want to initialize all points
        // to being counted
        // Alternatively, we will hit this case if there are neither ghost zones nor ghost
        // nodes. In that case, we shouldn't even be in this function, but if we are here
        // we might as well say all the points are valid since they are.
        fill(nodeShouldBeIgnored.begin(), nodeShouldBeIgnored.end(), false);
    }

    if (ghostZones)
    {
        const int nCells = in_ds->GetNumberOfCells();
        // iterate through the cells and mark points that are touching non-ghosts
        // as points that should be counted
        for (int cellId = 0; cellId < nCells; cellId ++)
        {
            // if this zone is not a ghost zone
            if (0 == ghostZones->GetComponent(cellId, 0))
            {
                vtkIdType numCellPoints = 0;
                const vtkIdType *cellPoints = nullptr;
                vtkIdList *ptIds = vtkIdList::New();
                // we get the points for this zone
                in_ds->GetCellPoints(cellId, numCellPoints, cellPoints, ptIds);

                // and mark them as valid points
                if (numCellPoints && cellPoints)
                {
                    for (int cellPointId = 0; cellPointId < numCellPoints; cellPointId ++)
                    {
                        const int pointId = cellPoints[cellPointId];
                        nodeShouldBeIgnored[pointId] = false;
                    }
                }
                ptIds->Delete();
            }
        }
    }

    if (ghostNodes)
    {
        // iterate through all points and make sure points marked as ghost
        // nodes are not counted
        for (int pointId = 0; pointId < nPoints; pointId ++)
        {
            // if this node is a ghost node
            if (0 != ghostNodes->GetComponent(pointId, 0))
            {
                nodeShouldBeIgnored[pointId] = true;
            }
        }
    }

    return nodeShouldBeIgnored;
}

// ****************************************************************************
// ****************************************************************************
void
avtOOFUSExpression::DoOperation(vtkDataArray *in, vtkDataArray *out,
                          int ncomponents, int ntuples, vtkDataSet *in_ds)
{
    vtkDataArray *ghostZones = in_ds->GetCellData()->GetArray("avtGhostZones");
    vtkDataArray *ghostNodes = in_ds->GetPointData()->GetArray("avtGhostNodes");
    int *nodeShouldBeIgnoredPtr = nullptr;

    if (AVT_ZONECENT == centering)
    {
        if (ghostZones)
        {
            // we pass a lambda to CalculateWithGhosts() that
            // looks at the ghostZones to determine if a cell
            // is valid and ignores the nodeShouldBeIgnoredPtr.
            CalculateWithGhosts(in, out, ncomponents, ntuples,
                                [](vtkDataArray *ghostZones,
                                   int *nodeShouldBeIgnoredPtr,
                                   int tuple_id) -> int 
                                   { return ghostZones->GetComponent(tuple_id, 0); },
                                ghostZones,
                                nodeShouldBeIgnoredPtr);
        }
        else // no ghosts or just ghost nodes
        {
            CalculateWithoutGhosts(in, out, ncomponents, ntuples);
        }
    }
    else // AVT_NODECENT == centering
    {
        // if we have any kind of ghosts
        if (ghostZones || ghostNodes)
        {
            // we need to identify which nodes should be ignored
            std::vector<int> nodeShouldBeIgnored = IdentifyGhostedNodes(
                in_ds, ghostZones, ghostNodes);
            nodeShouldBeIgnoredPtr = nodeShouldBeIgnored.data();

            // we pass a lambda to CalculateWithGhosts() that
            // looks at the nodeShouldBeIgnoredPtr to determine 
            // if a node is valid and ignores the ghostZones.
            CalculateWithGhosts(in, out, ncomponents, ntuples,
                                [](vtkDataArray *ghostZones,
                                   int *nodeShouldBeIgnoredPtr,
                                   int tuple_id) -> int 
                                   { return nodeShouldBeIgnoredPtr[tuple_id]; },
                                ghostZones,
                                nodeShouldBeIgnoredPtr);
        }
        else // no ghosts
        {
            CalculateWithoutGhosts(in, out, ncomponents, ntuples);
        }
    }
}

// ****************************************************************************
// ****************************************************************************
vtkDataArray *
avtOOFUSExpression::CreateArray(vtkDataArray *in1)
{
    return in1->NewInstance();
}

// TODO justin this is where I left off reproducing the call chain

// ****************************************************************************
// ****************************************************************************
vtkDataArray *
avtOOFUSExpression::DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex)
{
    int  i;

    vtkDataArray *cell_data = NULL;
    vtkDataArray *point_data = NULL;
    vtkDataArray *data = NULL;

    if (activeVariable == NULL)
    {
        //
        // This hack is getting more and more refined.  This situation comes up
        // when we don't know what the active variable is (mostly for the
        // constant creation filter).  We probably need more infrastructure
        // to handle this.
        // Iteration 1 of this hack said take any array.
        // Iteration 2 said take any array that isn't vtkGhostLevels, etc.
        // Iteration 3 says take the first scalar array if one is available,
        //             provided that array is not vtkGhostLevels, etc.
        //             This is because most constants we create are scalar.
        //
        // Note: this hack used to be quite important because we would use
        // the resulting array to determine the centering of the variable.
        // Now we use the IsPointVariable() method.  So this data array is
        // only used to get the type.
        //
        int ncellArray = in_ds->GetCellData()->GetNumberOfArrays();
        for (i = 0 ; i < ncellArray ; i++)
        {
            vtkDataArray *candidate = in_ds->GetCellData()->GetArray(i);
            if (strstr(candidate->GetName(), "vtk") != NULL)
                continue;
            if (strstr(candidate->GetName(), "avt") != NULL)
                continue;
            if (candidate->GetNumberOfComponents() == 1)
            {
                // Definite winner
                cell_data = candidate;
                break;
            }
            else
                // Potential winner -- keep looking
                cell_data = candidate;
        }
        int npointArray = in_ds->GetPointData()->GetNumberOfArrays();
        for (i = 0 ; i < npointArray ; i++)
        {
            vtkDataArray *candidate = in_ds->GetPointData()->GetArray(i);
            if (strstr(candidate->GetName(), "vtk") != NULL)
                continue;
            if (strstr(candidate->GetName(), "avt") != NULL)
                continue;
            if (candidate->GetNumberOfComponents() == 1)
            {
                // Definite winner
                point_data = candidate;
                break;
            }
            else
                // Potential winner -- keep looking
                point_data = candidate;
        }

        if (cell_data != NULL && cell_data->GetNumberOfComponents() == 1)
        {
            data = cell_data;
            centering = AVT_ZONECENT;
        }
        else if (point_data != NULL && point_data->GetNumberOfComponents()== 1)
        {
            data = point_data;
            centering = AVT_NODECENT;
        }
        else if (cell_data != NULL)
        {
            data = cell_data;
            centering = AVT_ZONECENT;
        }
        else
        {
            data = point_data;
            centering = AVT_NODECENT;
        }
    } 
    else
    {
        cell_data = in_ds->GetCellData()->GetArray(activeVariable);
        point_data = in_ds->GetPointData()->GetArray(activeVariable);

        if (cell_data != NULL)
        {
            data = cell_data;
            centering = AVT_ZONECENT;
        }
        else
        {
            data = point_data;
            centering = AVT_NODECENT;
        }
    }

    //
    // Set up a VTK variable reflecting the calculated variable
    //
    int ncomps = 0;
    int nvals = 0;
    if (activeVariable == NULL || data == NULL)
        nvals = (IsPointVariable() ? in_ds->GetNumberOfPoints() 
                                   : in_ds->GetNumberOfCells());
    else
        nvals = data->GetNumberOfTuples();

    vtkDataArray *dv = NULL;
    if (data == NULL)
    {
        //
        // We could not find a single array.  We must be doing something with
        // the mesh.
        //
        ncomps = 1;
        dv = CreateArrayFromMesh(in_ds);
    }
    else
    {
        ncomps = data->GetNumberOfComponents();
        dv = CreateArray(data);
    }

    if (data == NULL)
    {
        // One way to get here is to have vtkPolyData Curve plots.
        EXCEPTION2(ExpressionException, outputVariableName,
             "An internal error occurred when "
             "trying to calculate your expression.  Please contact a "
             "VisIt developer.");
    }

    int noutcomps = ncomps;
    dv->SetNumberOfComponents(noutcomps);
    dv->SetNumberOfTuples(nvals);

    //
    // Should we send in ncomps or noutcomps?  They are the same number 
    // unless the derived type re-defined GetNumberOfComponentsInOutput.
    // If it did, it probably doesn't matter.  If not, then it is the same
    // number.  So send in the input.  Really doesn't matter.
    //
    cur_mesh = in_ds;
    DoOperation(data, dv, ncomps, nvals, in_ds);
    cur_mesh = NULL;

    return dv;
}

// ****************************************************************************
// ****************************************************************************
avtDataRepresentation *
avtOOFUSExpression::ExecuteData_VTK(avtDataRepresentation *in_dr)
{
    //
    // Get the VTK data set and domain number.
    //
    vtkDataSet *in_ds = in_dr->GetDataVTK();
    int domain = in_dr->GetDomain();

    //
    // Sometimes we are asked to calculate a variable twice.  The easiest way
    // to catch this is to see if we already have the requested variable and
    // not re-derive it if we do.
    //
    vtkDataArray *dat = nullptr;
    dat = in_ds->GetPointData()->GetArray(outputVariableName);
    if (dat == nullptr)
    {
        dat = in_ds->GetCellData()->GetArray(outputVariableName);
    }
    if (dat != nullptr)
    {
        debug1 << "NOTE: variable " << outputVariableName 
               << " already exists and it is not being recalculated." << endl;
        dat->Register(nullptr);  // At the end of the routine, we will free this.
    }

    //
    // Start off by having the derived type calculate the derived variable.
    //
    if (dat == nullptr)
    {
        dat = DeriveVariable(in_ds, domain);
        if (dat == nullptr)
        {
            EXCEPTION2(ExpressionException, outputVariableName, "an unknown error occurred while " 
                  "trying to calculate your expression.  Please contact a "
                  "VisIt developer.");
        }
        dat->SetName(outputVariableName);
    }

    int vardim = dat->GetNumberOfComponents();

    //
    // Now make a copy of the input and add the derived variable as its output.
    //
    vtkDataSet *rv = (vtkDataSet *) in_ds->NewInstance();
    rv->ShallowCopy(in_ds);
    int npts   = rv->GetNumberOfPoints();
    int ncells = rv->GetNumberOfCells();
    int ntups  = dat->GetNumberOfTuples();
    bool isPoint = false;
    if ((ntups == npts) && (ntups == ncells))
    {
        isPoint = IsPointVariable();
    }
    else if (ntups == 1) // Constant singleton.
    {
        isPoint = IsPointVariable();
    }
    else if ((ntups == ncells) && (ntups != npts))
    {
        isPoint = false;
    }
    else if ((ntups == npts) && (ntups != ncells))
    {
        isPoint = true;
    }
    else
    {
        debug1 << "Number of tuples cannot be point or cell variable."
               << endl;
        debug1 << "Var = " << dat->GetName() << endl;
        debug1 << "Ntuples = " << ntups << endl;
        debug1 << "Ncells = " << ncells << endl;
        debug1 << "Npts = " << npts << endl;
        dat->Delete();

        avtDataRepresentation *out_dr = new avtDataRepresentation(rv,
            in_dr->GetDomain(), in_dr->GetLabel());

        rv->Delete();

        return out_dr;
    }

    if (isPoint)
    {
        rv->GetPointData()->AddArray(dat);
        if (vardim == 1)
            rv->GetPointData()->SetActiveScalars(outputVariableName);
        else if (vardim == 3)
            rv->GetPointData()->SetActiveVectors(outputVariableName);
        else if (vardim == 9)
            rv->GetPointData()->SetActiveTensors(outputVariableName);
    }
    else
    {
        rv->GetCellData()->AddArray(dat);
        if (vardim == 1)
            rv->GetCellData()->SetActiveScalars(outputVariableName);
        else if (vardim == 3)
            rv->GetCellData()->SetActiveVectors(outputVariableName);
        else if (vardim == 9)
            rv->GetCellData()->SetActiveTensors(outputVariableName);
    }

    //
    // Make sure that we don't have any memory leaks.
    //
    dat->Delete();

    rv->GetFieldData()->AddArray(this->volumeDependent);

    avtDataRepresentation *out_dr = new avtDataRepresentation(rv,
        in_dr->GetDomain(), in_dr->GetLabel());

    rv->Delete();

    return out_dr;
}

avtDataRepresentation *
avtOOFUSExpression::ExecuteData(avtDataRepresentation *in_dr)
{
    avtDataRepresentation *out_dr = nullptr;

// #ifdef HAVE_LIBVTKM
//     if (in_dr->GetDataRepType() == DATA_REP_TYPE_VTKM ||
//         avtCallback::GetBackendType() == GlobalAttributes::VTKM)
//     {
//         out_dr = ExecuteData_VTKm(in_dr);
//     }
//     else
// #endif
    {
        out_dr = ExecuteData_VTK(in_dr); 
    }

    return out_dr;
}

// ****************************************************************************
// ****************************************************************************
avtDataTree_p
avtOOFUSExpression::ExecuteDataTree(avtDataRepresentation *in_dr)
{
    avtDataRepresentation *out_dr = ExecuteData(in_dr);

    if (out_dr == nullptr)
    {
        return nullptr;
    }

    // This code ends up creating a copy of out_dr in the data tree.
    avtDataTree_p retval = new avtDataTree(out_dr);

    // If the derived type created a new avtDataRepresentation instance then
    // it's been copied in the avtDataTree. We need to remove the instance
    // that was returned or we'll end up with a VTK reference count leak.
    if(out_dr != in_dr)
    {
        delete out_dr;
    }

    return retval;
}

// ****************************************************************************
// ****************************************************************************
void
avtOOFUSExpression::ExecuteDataTreeOnThread(avtDataTree_p inDT, avtDataTree_p &outDT)
{
    avtDataTree_p retDT = ExecuteDataTree(&(inDT->GetDataRepresentation()));
    if (*retDT)
    {
        outDT = *retDT;
    }

    UpdateProgress(currentProgress++, totalSteps);
}

// ****************************************************************************
// ****************************************************************************
void
avtOOFUSExpression::execute_2_electric_boogaloo(avtDataTree_p inDT, avtDataTree_p &outDT)
{
    CheckAbort();
    int nc = inDT->GetNChildren();

    if (nc <= 0 && !inDT->HasData())
    {
        return;
    }

    if (nc == 0)
    {
        outDT = new avtDataTree();
        ExecuteDataTreeOnThread(inDT, outDT);
    }
    else
    {
        //
        // there is more than one input dataset to process
        // and we need an output datatree for each
        //
        avtDataTree_p *localOutDT = new avtDataTree_p[nc];
        for (int j = 0; j < nc; j++)
        {
            // are children present?
            if (inDT->ChildIsPresent(j))
            {
                localOutDT[j].SetReference( new avtDataTree );
                execute_2_electric_boogaloo(inDT->GetChild(j), localOutDT[j]);
            }
            else // adults only
            {
                localOutDT[j] = NULL;
            }
        }
        outDT = new avtDataTree(nc, localOutDT);
        delete [] localOutDT;
    }
}


// ****************************************************************************
//  Method: avtOOFUSExpression::Execute
//
//  Purpose:
//      Labels the connected components of an unstructured mesh.
//
//  Programmer: Justin Privitera
//  Creation:   August 18, 2025
//
//  Modifications:
// ****************************************************************************
void
avtOOFUSExpression::Execute()
{
    // taken from avtSIMODataTreeIterator::Execute()

    //
    // This will walk through the data domains in a data tree.
    //
    avtDataTree_p tree = GetInputDataTree();
    avtDataTree_p newTree;


    if (*tree != NULL)
    {
        totalNodes = tree->GetNumberOfLeaves();
        execute_2_electric_boogaloo(tree, newTree);

        // If in threaded mode wait until Execute has completed.
        // TODO do we really need to do this?
        avtExecutionManagerFinishWork();
    }
    else
    {
        // This can happen when a filter serves up an empty data tree.
        // It can also happen when we claim memory from intermediate
        // data objects and then go back to execute them.
        debug1 << "Unusual situation: NULL input tree to SIMO iterator.  Likely "
               << "that an exception occurred previously." << endl;
    }

    if (*newTree == NULL)
    {
        //
        // Lots of code assumes that the root tree is non-NULL.  Put a dummy
        // tree in its place.
        //
        newTree = new avtDataTree();
    }

    SetOutputDataTree(newTree);

    ///////////////////////////////////////////////////////////////

//     int t_full = visitTimer->StartTimer();

//     // loop index
//     int i;
//     // initialize number of components to zero 
//     nFinalComps = 0;

//     // get input data tree to obtain datasets
//     avtDataTree_p tree = GetInputDataTree();
//     // holds number of datasets
//     int nsets;

//     // get datasets
//     vtkDataSet **data_sets = tree->GetAllLeaves(nsets);

//     // get dataset domain ids
//     std::vector<int> domain_ids;
//     tree->GetAllDomainIds(domain_ids);

//     // check for ghosts
//     bool have_ghosts = false;
//     if(nsets > 0)
//         have_ghosts = data_sets[0]->GetCellData()->GetArray("avtGhostZones");

//     // set progress related vars
// #ifdef PARALLEL
//     totalSteps = nsets *4;
//     if(have_ghosts)
//         totalSteps+= nsets;
// #else
//     totalSteps = nsets *2;
// #endif
//     currentProgress = 0;        

// #ifdef PARALLEL

//     debug2 << "avtOOFUSExpression::enableGhostNeighbors = " 
//            << enableGhostNeighbors <<endl;
//     if(enableGhostNeighbors == 0) 
//        if (!  CheckForProperGhostZones(data_sets,nsets))
//        {
//            enableGhostNeighbors = 2;
//        }

//     if (enableGhostNeighbors == 0)
//     {
//         debug2 << "avtOOFUSExpression:: Proper ghost zones found for "
//                << "ghost zone communication enhancement." 
//                << "Labeling ghost zone neighbors."
//                << endl;
//         // if we have ghosts, label ghost neighbors for reduced comm in global
//         // resolve
//         for( i = 0; i <nsets ; i++)
//         {
//             LabelGhostNeighbors(data_sets[i]);
//             UpdateProgress(currentProgress++,totalSteps);
//         }
//     }
// #endif

//     int t_gzrm = visitTimer->StartTimer();
//     // filter out any ghost cells
//     vtkDataSetRemoveGhostCells **ghost_filters = NULL;

//     if(have_ghosts)
//     {
//         ghost_filters = new vtkDataSetRemoveGhostCells*[nsets];

//         for( i = 0 ; i < nsets ; i++)
//         {
//             ghost_filters[i] = vtkDataSetRemoveGhostCells::New();
//             ghost_filters[i]->SetInputData(data_sets[i]);
//             ghost_filters[i]->Update();
//             data_sets[i] = ghost_filters[i]->GetOutput();
//         }
//     }
//     visitTimer->StopTimer(t_gzrm,"Ghost Zone Removal");

// #ifdef PARALLEL
//     if (enableGhostNeighbors == 1)
//     {
//         debug2 << "avtOOFUSExpression:: Proper ghost zones NOT found "
//                << "for ghost zone communication enhancement, using boundary neighbors." << endl;
//         for( i = 0; i <nsets ; i++)
//         {
//             LabelBoundaryNeighbors(data_sets[i]);
//             UpdateProgress(currentProgress++,totalSteps);
//         }
//     }
//     else // enableGhostNeighbors == 2
//     {
//         debug2 << "old method for boundary neighbors." << endl;
//     }
// #endif

//     int t_local_lbl = visitTimer->StartTimer();
//     // array to hold output sets
//     avtDataTree_p *leaves = new avtDataTree_p[nsets];

//     // vectors to hold result sets and their component labels
//     std::vector<vtkDataSet *>  result_sets;
//     std::vector<vtkIntArray *> result_arrays;
//     // vector to hold the number of components per set
//     std::vector<int> results_num_comps;

//     result_sets.resize(nsets);
//     result_arrays.resize(nsets);
//     results_num_comps.resize(nsets);

//     int num_local_comps=0;
//     int num_comps = 0;
//     int num_local_cells=0;

//     // process all local sets
//     for(i = 0; i < nsets ; i++)
//     {
//         // get current set
//         vtkDataSet *curr_set = data_sets[i];
//         num_local_cells += curr_set->GetNumberOfCells();

//         // perform connected components labeling on current set
//         // (this only resolves components within the set)
//         vtkIntArray *res_array = SingleSetLabel(curr_set,results_num_comps[i]);

//         vtkIntArray *res_array = 

//         int ncomponents = res_array->GetNumberOfComponents();
//         int ntuples = res_array->GetNumberOfTuples();
//         for (int comp_id = 0; comp_id < ncomponents; comp_id ++)
//         {
//             for (int tuple_id = 0; tuple_id < ntuples; tuple_id ++)
//             {
//                 res_array->SetComponent(tuple_id, comp_id, 1);
//             }
//         }

//         // update the total number of components found
//         num_local_comps+= results_num_comps[i];

//         // create a shallow copy of the current data set to add to output
//         vtkDataSet *res_set = (vtkDataSet *) curr_set->NewInstance();
//         res_set->ShallowCopy(curr_set);

//         // add array to dataset
//         res_set->GetCellData()->AddArray(res_array);

//         // keep pointers to the result set and labels
//         result_arrays[i] = res_array;
//         result_sets[i]   = res_set;

//         if(res_array->GetNumberOfTuples() > 0) // add result as new leaf
//             leaves[i] = new avtDataTree(res_set,domain_ids[i]);
//         else // if the dataset only contained ghost zones we could end up here
//             leaves[i] = NULL;

//         // update progress
//         UpdateProgress(currentProgress++,totalSteps);
//     }

//     // create a boundary set 
//     // this is used to for fast boundary queries to resolve components across
//     // multiple datasets

//     BoundarySet bset;
//     for(i=0;i<nsets;i++)
//     {
//         // add each local mesh to the boundary set
//         bset.AddMesh(result_sets[i]);
//     }
//     // prepare the boundary set for queries
//     bset.Finalize();

//     // resolve labels across multiple local sets
//     num_local_comps = MultiSetResolve(num_local_comps,
//                                      bset,
//                                      result_sets,
//                                      result_arrays);

//     // update the total number of found components
//     num_comps = num_local_comps;

//     std::ostringstream oss;
//     oss << "Connected Components Labeling of " << nsets << " local datasets (" 
//         << num_local_cells << " cells, " << num_comps << " comps)";
//     visitTimer->StopTimer(t_local_lbl,oss.str());

// #ifdef PARALLEL

//     //
//     // At this point each processor has resolved the labels across all local 
//     // datasets.  Components on each processor are labeled 0 <-> (number of 
//     // local comps - 1).  In the parallel case we need to ensure that every 
//     // component has a unique label so we first perform a global label shift.
//     //

//     // Globally shift labels to make sure they are unique
//     int num_global_comps = GlobalLabelShift(num_local_comps,result_arrays);


//     // We can now globally resolve the labels

//     // Resolve components across processors
//     num_global_comps = GlobalResolve(num_global_comps,
//                                      bset,
//                                      result_sets,
//                                      result_arrays);

//     // update the total number of found components
//     num_comps = num_global_comps;

// #endif

//     // create output data trees
//     if(nsets > 0 )
//     {
//         // create output data tree
//         avtDataTree_p result_tree = new avtDataTree(nsets,leaves);
//         // set output data tree
//         SetOutputDataTree(result_tree);
//     }

//     // cleanup leaves array
//     delete [] leaves;
//     // cleanup data_sets array
//     delete [] data_sets;

//     // cleanup result sets
//     for(i = 0; i< nsets; i++)
//     {
//        // dec ref pointer for each set
//        result_sets[i]->Delete();
//        // dec ref pointer for each set's label array
//        result_arrays[i]->Delete();
//        // cleanup ghost filters
//        if(have_ghosts)
//            ghost_filters[i]->Delete();
//     }
//     // cleanup ghost filters array
//     if(have_ghosts)
//         delete [] ghost_filters;

//     // Set progress to complete
//     UpdateProgress(totalSteps,totalSteps);

//     // set the final number of components
//     nFinalComps  = num_comps;

//     visitTimer->StopTimer(t_full,"Full Connected Components Labeling");
}


// ****************************************************************************
//  Method: avtOOFUSExpression::CheckForProperGhostZones
//
//  Purpose:
//     Checks for ghost zone info that can be used in to reduce parallel 
//     communication. 
//
//  Arguments:
//    sets        Input data sets.
//    nsets       Number of input data sets. 
//
//  Programmer: Cyrus Harrison
//  Creation:   October 10, 20078
//
//  Modifications:
//    Cyrus Harrison, Tue Nov  9 10:32:01 PST 2010
//    Added timing info.
//
//    Cyrus Harrison, Tue Nov  9 10:32:01 PST 2010
//    Loosen the check for valid ghost zones.
//
// ****************************************************************************
bool
avtOOFUSExpression::CheckForProperGhostZones(vtkDataSet **sets,
                                                      int nsets)
{
    int t0 = visitTimer->StartTimer();

    int found_ghosts = 0;
    int total_ncells = 0;
    for(int i=0; i < nsets && found_ghosts == 0; i++)
    {
        int ncells = sets[i]->GetNumberOfCells();
        total_ncells += ncells;
        vtkUnsignedCharArray *gz_array = (vtkUnsignedCharArray *) sets[i]
                                    ->GetCellData()->GetArray("avtGhostZones");
        if(gz_array)
        {
            unsigned char *gz_ptr = (unsigned char *)gz_array->GetPointer(0);
            for(int j=0; j < ncells && found_ghosts == 0; j++)
            {
                if(gz_ptr[j] & 1) // Bit 0 == DUPLICATED_ZONE_INTERNAL_TO_PROBLEM
                    found_ghosts = 1;
            }
        }
    }

    //
    // If we found a single instance of a proper ghost zone
    // we want to try to use the ghost zone neighbors optimization.
    // Note: It would be better if the data attributes simply told
    // us that the proper type of ghost zones were generated ...
    //

    found_ghosts = UnifyMaximumValue(found_ghosts);

    visitTimer->StopTimer(t0,"Check For Proper Ghost Zones");
    return (found_ghosts == 1);
}

// ****************************************************************************
//  Method: avtOOFUSExpression::LabelGhostNeighbors
//
//  Purpose:
//     Identifies cells that have ghost neighbors, storing the info in
//     a vtkUnsignedCharArray named "avtOnBoundary".
//
//  Arguments:
//    data_set     Input mesh
//
//  Programmer: Cyrus Harrison
//  Creation:   August 11, 2007
//
//  Modifications:
//    Cyrus Harrison, Tue Nov  9 10:32:01 PST 2010
//    Added timing info.
//
//    Cyrus Harrison, Tue Nov  9 10:32:01 PST 2010
//    Explicit check for DUPLICATED_ZONE_INTERNAL_TO_PROBLEM.
//
//    Ryan Bleile, Wed Jun 11 09:53:23 CDT 2014
//    Changed avtGhostZoneNeighbors to avtOnBoundary
// ****************************************************************************
void
avtOOFUSExpression::LabelGhostNeighbors(vtkDataSet *data_set)
{
    int t0 = visitTimer->StartTimer();
    // loop indices
    int i,j,k;
    vtkUnsignedCharArray *gz_array = (vtkUnsignedCharArray *) data_set
                             ->GetCellData()->GetArray("avtGhostZones");

    // if the data set does not have ghosts, we are done
    if (!gz_array)
        return;

    unsigned char *gz_ptr = (unsigned char *)gz_array->GetPointer(0);
    int ncells = data_set->GetNumberOfCells();

    vtkUnsignedCharArray *gzn_array = vtkUnsignedCharArray::New();
    gzn_array->SetName("avtOnBoundary");
    gzn_array->SetNumberOfComponents(1);
    gzn_array->SetNumberOfTuples(ncells);

    unsigned char *gzn_ptr = (unsigned char *)gzn_array->GetPointer(0);

    // init the ghost zone neighbors array
    memset(gzn_ptr,0,ncells * sizeof(unsigned char));

    for ( i=0; i < ncells; i++)
    {
        // if this cell has ghost zones, label it's neighbors
        if(gz_ptr[i] & 1) // Bit 0 == DUPLICATED_ZONE_INTERNAL_TO_PROBLEM
        {
            // get cell neighbors
            vtkIdList *gcell_pts = data_set->GetCell(i)->GetPointIds();
            int ngcell_pts = gcell_pts->GetNumberOfIds();
            for( j=0; j < ngcell_pts; j++)
            {
                // neighbors share points with the current cell
                vtkIdList *gpt = vtkIdList::New();
                gpt->SetNumberOfIds(1);
                gpt->SetId(0,gcell_pts->GetId(j));
                vtkIdList *nei_cells = vtkIdList::New();
                data_set->GetCellNeighbors(i,gpt,nei_cells);
                int nnei = nei_cells->GetNumberOfIds();

                // tag neighbors
                for ( k = 0; k < nnei; k++)
                    gzn_ptr[nei_cells->GetId(k)] = 1;

                gpt->Delete();
                nei_cells->Delete();
            }
        }
    }

    data_set->GetCellData()->AddArray(gzn_array);
    gzn_array->Delete();
    visitTimer->StopTimer(t0,"Labeling Ghost Neighbors");
}


// ****************************************************************************
//  Method: avtOOFUSExpression::LabelBoundaryNeighbors
//
//  Purpose:
//     Identifies cells that lie on the boundary, storing the results in 
//     a vtkUnsignedCharArray named "avtOnBoundary".
//
//  Arguments:
//    data_set     Input mesh
//
//  Programmer: Hank Childs
//  Creation:   November 30, 2013
//
//  Modifications:
//    Eric Brugger, Mon Jul 21 12:06:33 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

void
avtOOFUSExpression::LabelBoundaryNeighbors(vtkDataSet *data_set)
{
    int i;
    int t0 = visitTimer->StartTimer();

    int ncells = data_set->GetNumberOfCells();

    // make a clone of the input that has no variable
    // (less variables mean less operations when manipulating it)
    vtkDataSet *clone_ds = data_set->NewInstance();
    clone_ds->ShallowCopy(data_set);
    int numPointArrays = clone_ds->GetPointData()->GetNumberOfArrays();
    for (i = numPointArrays-1 ; i>=0 ; i--)
        clone_ds->GetPointData()->RemoveArray(i);
    int numCellArrays = clone_ds->GetCellData()->GetNumberOfArrays();
    for (i = numCellArrays-1 ; i>=0 ; i--)
        clone_ds->GetCellData()->RemoveArray(i);

    // set up a variable that has the cell ID for each cell.
    vtkIntArray *arr = vtkIntArray::New();
    arr->SetNumberOfTuples(ncells);
    for (vtkIdType i = 0 ; i < ncells ; i++)
        arr->SetValue(i, (int)i);
    const char *varname = "_avt_id";
    arr->SetName(varname);
    clone_ds->GetCellData()->AddArray(arr);
    arr->Delete();

    // use external routine to find which cells are external
    avtFacelistFilter *flf = new avtFacelistFilter();
    avtDataRepresentation clone_dr(clone_ds, -1, "");
    avtDataTree_p tree = flf->FindFaces(&clone_dr,
                                  GetInput()->GetInfo(), false, false,
                                  true, true, NULL);
    delete flf;
    clone_ds->Delete();
    // we do not need to delete tree, since it is a ref_ptr

    // init the boundary neighbors array
    vtkUnsignedCharArray *b_array = vtkUnsignedCharArray::New();
    b_array->SetName("avtOnBoundary");
    b_array->SetNumberOfComponents(1);
    b_array->SetNumberOfTuples(ncells);
    unsigned char *b_ptr = (unsigned char *)b_array->GetPointer(0);
    memset(b_ptr,0,ncells * sizeof(unsigned char));

    // go through external cells and update array for which are on boundary
    vtkDataSet *just_exteriors = tree->GetSingleLeaf();
    vtkIntArray *outsides = (vtkIntArray *) just_exteriors->GetCellData()->GetArray(varname);
    int numOutsideCells = outsides->GetNumberOfTuples();
    for (i = 0 ; i < numOutsideCells ; i++)
        b_ptr[outsides->GetValue(i)] = 1;

    data_set->GetCellData()->AddArray(b_array);
    b_array->Delete();

    visitTimer->StopTimer(t0,"Labeling Boundary Neighbors");
}

// ****************************************************************************
//  Method: avtGradientExpression::ModifyContract
//
//  Purpose:
//      Request ghost zones.
//
//  Programmer: Justin Privitera
//  Creation:   August 18, 2025
//
//  Modifications:
// ****************************************************************************
avtContract_p
avtOOFUSExpression::ModifyContract(avtContract_p in_spec)
{
    avtContract_p spec = 
                            avtExpressionFilter::ModifyContract(in_spec);
    spec->GetDataRequest()->SetDesiredGhostDataType(GHOST_ZONE_DATA);
    return spec;
}

