// ************************************************************************* //
//                           avtPosCMFEExpression.C                          //
// ************************************************************************* //

#include <avtPosCMFEExpression.h>

#include <float.h>
#include <math.h>

#ifdef PARALLEL
#include <mpi.h>
#endif

#include <vtkAppendFilter.h>
#include <vtkCellData.h>
#include <vtkCellLocator.h>
#include <vtkCharArray.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkGenericCell.h>
#include <vtkPointData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnstructuredGridRelevantPointsFilter.h>
#include <vtkUnstructuredGridWriter.h>
#include <vtkUnstructuredGridReader.h>

#include <Utility.h>
#include <vtkVisItUtility.h>

#include <avtCommonDataFunctions.h>
#include <avtDatasetExaminer.h>
#include <avtIntervalTree.h>
#include <avtParallel.h>
#include <avtSILRestrictionTraverser.h>

#include <DebugStream.h>
#include <ExpressionException.h>
#include <InvalidMergeException.h>
#include <TimingsManager.h>


// ****************************************************************************
//  Method: avtPosCMFEExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   October 11, 2005
//
// ****************************************************************************

avtPosCMFEExpression::avtPosCMFEExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtPosCMFEExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   October 11, 2005
//
// ****************************************************************************

avtPosCMFEExpression::~avtPosCMFEExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtPosCMFEExpression::PerformCMFE
//
//  Purpose:
//      Performs a cross-mesh field evaluation based on position-based 
//      differencing.
//
//  Programmer: Hank Childs
//  Creation:   October 11, 2005
// 
//  Modifications:
//
//    Hank Childs, Thu Jan 12 11:24:23 PST 2006
//    Modify index of default variable.
//
// ****************************************************************************

avtDataTree_p
avtPosCMFEExpression::PerformCMFE(avtDataTree_p output_mesh,
                                  avtDataTree_p mesh_to_be_sampled,
                                  const std::string &invar,
                                  const std::string &outvar)
{
    int   i, j;

    //
    // Get all of the mesh to sample and get information about the variable
    // we are sampling.
    //
    GetAllDatasetsArgs sample_list;
    bool unused = false;
    mesh_to_be_sampled->Traverse(CGetAllDatasets, 
                                 (void *) &sample_list, unused);
    int centering = 0;
    int nComp     = 0;
    if (sample_list.datasets.size() > 0)
    {
        const char *varname = invar.c_str();
        if (sample_list.datasets[0]->GetPointData()->GetArray(varname) != NULL)
        {
            centering = 1;
            nComp = sample_list.datasets[0]->GetPointData()->GetArray(varname)
                         ->GetNumberOfComponents();
        }
        else if (sample_list.datasets[0]->GetCellData()->GetArray(varname) 
                 != NULL)
        {
            centering = 2;
            nComp = sample_list.datasets[0]->GetCellData()->GetArray(varname)
                         ->GetNumberOfComponents();
        }
    }
    centering = UnifyMaximumValue(centering);
    if (centering == 0)
    {
        EXCEPTION1(ExpressionException, "Could not do database comparison, "
                       "because the secondary database had no data.");
    }
    nComp = UnifyMaximumValue(nComp);
    if (nComp == 0)
    {
        EXCEPTION1(ExpressionException, "Could not do database comparison, "
                       "because the specified variable is degenerate.");
    }
    bool isNodal = (centering == 1);

    //
    // Set up the data structure so that we can locate sample points in the
    // mesh to be sampled quickly.
    //
    FastLookupGrouping flg(invar, isNodal);
    for (i = 0 ; i < sample_list.datasets.size() ; i++)
        flg.AddMesh(sample_list.datasets[i]);

    //
    // Now get all the datasets for the mesh we are supposed to sample onto.
    //
    int t0 = visitTimer->StartTimer();
    GetAllDatasetsArgs output_list;
    output_mesh->Traverse(CGetAllDatasets, (void *) &output_list, unused);

    //
    // Set up the data structure that keeps track of the sample points we need.
    //
    DesiredPoints dp(isNodal, nComp);
    for (i = 0 ; i < output_list.datasets.size() ; i++)
        dp.AddDataset(output_list.datasets[i]);

#ifdef PARALLEL
    //
    // There is no guarantee that the "dp" and "flg" overlap spatially.  It's
    // likely that the parts of the "flg" mesh that the points in "dp" are
    // interested in are located on different processors.  So we do a large
    // communication phase to get all of the points on the right processors.
    //
    int t3 = visitTimer->StartTimer();
    SpatialPartition spat_part;
    double bounds[6];
    std::vector<avtDataTree_p> tree_list;
    tree_list.push_back(output_mesh);
    tree_list.push_back(mesh_to_be_sampled);
    avtDatasetExaminer::GetSpatialExtents(tree_list, bounds);
    UnifyMinMax(bounds, 6);

    // Need to "finalize" in pre-partitioned form so that the spatial
    // partitioner can access their data.
    dp.Finalize();

    spat_part.CreatePartition(dp, flg, bounds);
    dp.RelocatePointsUsingPartition(spat_part);
    flg.RelocateDataUsingPartition(spat_part);
    visitTimer->StopTimer(t3, "Spatial re-distribution");
#endif
    flg.Finalize();
    dp.Finalize();

    //
    // Now, for each sample, locate the sample point in the mesh to be sampled
    // and evaluate that point.
    //
    int t1 = visitTimer->StartTimer();
    int npts = dp.GetNumberOfPoints();
    float *comps = new float[nComp];
    for (i = 0 ; i < npts ; i++)
    {
        float pt[3];
        dp.GetPoint(i, pt);
        bool gotValue = flg.GetValue(pt, comps);
        if (!gotValue)
            comps[0] = +FLT_MAX;
        dp.SetValue(i, comps);
    }
    delete [] comps;
    visitTimer->StopTimer(t1, "Sampling variable");

#ifdef PARALLEL
    //
    // We had to distribute the "dp" and "flg" structures across all 
    // processors (see comments in sections above).  So now we need to
    // get the correct values back to this processor so that we can set
    // up the output variable array.
    //
    int t4 = visitTimer->StartTimer();
    dp.UnRelocatePoints(spat_part);
    visitTimer->StopTimer(t4, "Collecting sample points back");
#endif

    //
    // Now create the variable that contains all of the values for the sample
    // points we evaluated.
    //
    int t2 = visitTimer->StartTimer();
    avtDataTree_p *leaves = new avtDataTree_p[output_list.datasets.size()];
    for (i = 0 ; i < output_list.datasets.size() ; i++)
    {
        vtkDataSet *in_ds1 = output_list.datasets[i];
        vtkDataArray *defaultVar = NULL;
        bool deleteDefault = false;
        const char *defaultName = varnames[1].c_str();
        defaultVar = (isNodal
                       ? in_ds1->GetPointData()->GetArray(defaultName)
                       : in_ds1->GetCellData()->GetArray(defaultName));
        if (defaultVar == NULL)
        {
             defaultVar = (!isNodal
                       ? in_ds1->GetPointData()->GetArray(defaultName)
                       : in_ds1->GetCellData()->GetArray(defaultName));
             if (defaultVar == NULL)
             {
                 EXCEPTION1(ExpressionException, "Cannot perform pos_cmfe "
                         "because VisIt cannot locate the default variable."
                         "   Please contact a VisIt developer.");
             }
             else
             {
                 deleteDefault = true;
                 defaultVar = Recenter(in_ds1, defaultVar, 
                                  (isNodal ? AVT_ZONECENT : AVT_NODECENT));
             }
        }
        vtkDataSet *new_obj = (vtkDataSet *) in_ds1->NewInstance();
        new_obj->ShallowCopy(in_ds1);
        vtkFloatArray *addarr = vtkFloatArray::New();
        addarr->SetName(outvar.c_str());
        addarr->SetNumberOfComponents(nComp);
        int nvals = (isNodal ? in_ds1->GetNumberOfPoints() 
                             : in_ds1->GetNumberOfCells());
        addarr->SetNumberOfTuples(nvals);
        for (j = 0 ; j < nvals ; j++)
        {
            const float *val = dp.GetValue(i, j);
            if (*val != +FLT_MAX)
                addarr->SetTuple(j, dp.GetValue(i, j));
            else
                addarr->SetTuple(j, defaultVar->GetTuple(j));
        }
        if (isNodal)
            new_obj->GetPointData()->AddArray(addarr);
        else
            new_obj->GetCellData()->AddArray(addarr);
        addarr->Delete();

        leaves[i] = new avtDataTree(new_obj, output_list.domains[i],
                                    output_list.labels[i]);
        new_obj->Delete();
        if (deleteDefault)
            defaultVar->Delete();
    }
    avtDataTree_p rv;
    if (output_list.datasets.size() > 0)
        rv = new avtDataTree(output_list.datasets.size(), leaves);
    else
        rv = new avtDataTree();
    delete [] leaves;
    visitTimer->StopTimer(t2, "Constructing VTK dataset");

    visitTimer->StopTimer(t0, "POS CMFE evaluation phase");

    return rv;
}


// ****************************************************************************
//  Method: DesiredPoints constructor
//
//  Programmer: Hank Childs
//  Creation:   October 11, 2005
//
// ****************************************************************************

avtPosCMFEExpression::DesiredPoints::DesiredPoints(bool isN, int nc)
{
    isNodal   = isN;
    nComps    = nc;
    map_to_ds = NULL;
    ds_start  = NULL;
    vals      = NULL;
}


// ****************************************************************************
//  Method: DesiredPoints destructor
//
//  Programmer: Hank Childs
//  Creation:   October 11, 2005
//
// ****************************************************************************

avtPosCMFEExpression::DesiredPoints::~DesiredPoints()
{
    delete [] map_to_ds;
    delete [] ds_start;
    delete [] vals;
    for (int i = 0 ; i < pt_list.size() ; i++)
        delete [] pt_list[i];
}


// ****************************************************************************
//  Method: DesiredPoints::AddDataset
//
//  Purpose:
//      Registers a dataset with the desired points object.
//
//  Programmer: Hank Childs
//  Creation:   October 11, 2005
//
// ****************************************************************************

void
avtPosCMFEExpression::DesiredPoints::AddDataset(vtkDataSet *ds)
{
    int nvals = (isNodal ? ds->GetNumberOfPoints() : ds->GetNumberOfCells());
    float *plist = new float[3*nvals];
    pt_list.push_back(plist);
    pt_list_size.push_back(nvals);

    for (int i = 0 ; i < nvals ; i++)
    {
        float *cur_pt = plist + 3*i;
        if (isNodal)
            ds->GetPoint(i, cur_pt);
        else
        {
            vtkCell *cell = ds->GetCell(i);
            vtkVisItUtility::GetCellCenter(cell, cur_pt);
        }
    }
}


// ****************************************************************************
//  Method: DesiredPoints::Finalize
//
//  Purpose:
//      Gives the desired points object a chance to finish its initialization
//      process.  This gives the object the cue that it will not receive
//      any more "AddDataset" calls and that it can initialize itself.
//
//  Programmer: Hank Childs
//  Creation:   October 11, 2005
//
// ****************************************************************************

void
avtPosCMFEExpression::DesiredPoints::Finalize(void)
{
    int   i, j;
    int   index = 0;

    if (vals != NULL)
        delete [] vals;
    if (ds_start != NULL)
        delete [] ds_start;
    if (map_to_ds != NULL)
        delete [] map_to_ds;

    total_nvals  = 0;
    num_datasets = pt_list_size.size();
    for (i = 0 ; i < num_datasets ; i++)
        total_nvals += pt_list_size[i];

    ds_start = new int[num_datasets];
    ds_start[0] = 0;
    for (i = 1 ; i < num_datasets ; i++)
        ds_start[i] = ds_start[i-1] + pt_list_size[i-1];

    map_to_ds = new int[total_nvals];
    index = 0;
    for (i = 0 ; i < num_datasets ; i++)
        for (j = 0 ; j < pt_list_size[i] ; j++)
        {
            map_to_ds[index] = i;
            index++;
        }

    vals = new float[total_nvals*nComps];
}


// ****************************************************************************
//  Method: DesiredPoints::GetPoint
//
//  Purpose:
//      Gets the next point in the sequence that should be sampled.
//
//  Programmer: Hank Childs
//  Creation:   October 11, 2005
//
// ****************************************************************************

void
avtPosCMFEExpression::DesiredPoints::GetPoint(int p, float *pt) const
{
    if (p < 0 || p >= total_nvals)
    {
        EXCEPTION0(ImproperUseException);
    }
    int ds = map_to_ds[p];
    int start = ds_start[ds];
    int rel_index = p-start;
    float *ptr = pt_list[ds] + 3*rel_index;
    pt[0] = ptr[0];
    pt[1] = ptr[1];
    pt[2] = ptr[2];
}


// ****************************************************************************
//  Method: DesiredPoints::SetValue
//
//  Purpose:
//      Sets the value of the 'p'th point.
//
//  Programmer: Hank Childs
//  Creation:   October 11, 2005
//
// ****************************************************************************

void 
avtPosCMFEExpression::DesiredPoints::SetValue(int p, float *v)
{
    for (int i = 0 ; i < nComps ; i++)
        vals[p*nComps + i] = v[i];
}


// ****************************************************************************
//  Method: DesiredPoints::GetValue
//
//  Purpose:
//      Gets the value for an index, where the index is a convenient indexing
//      scheme when iterating over the final datasets.
//
//  Programmer: Hank Childs
//  Creation:   October 11, 2005
//
// ****************************************************************************

const float *
avtPosCMFEExpression::DesiredPoints::GetValue(int ds_idx, int pt_idx) const
{
    if (ds_idx < 0 || ds_idx >= num_datasets)
    {
        EXCEPTION0(ImproperUseException);
    }

    int true_idx = ds_start[ds_idx] + pt_idx;
    return vals + nComps*true_idx;
}


// ****************************************************************************
//  Method: DesiredPoints::RelocatePointsUsingPartition
//
//  Purpose:
//      Relocates the points to different processors to create a spatial 
//      partition.
//
//  Programmer: Hank Childs
//  Creation:   October 12, 2005
//
// ****************************************************************************

void
avtPosCMFEExpression::DesiredPoints::RelocatePointsUsingPartition(
                                                   SpatialPartition &spat_part)
{
#ifdef PARALLEL
    int   i, j;
    int   nProcs = PAR_Size();

    int t0 = visitTimer->StartTimer();

    //
    // Start off by assessing how much data needs to be sent, and to where.
    //
    vector<int> pt_cts(nProcs, 0);
    for (i = 0 ; i < pt_list_size.size() ; i++)
    {
        const int npts = pt_list_size[i];
        float    *pts  = pt_list[i];
        for (j = 0 ; j < npts ; j++)
        {
            float pt[3];
            pt[0] = *pts++;
            pt[1] = *pts++;
            pt[2] = *pts++;
            int proc = spat_part.GetProcessor(pt);
            pt_cts[proc]++;
        }
    }

    // 
    // Now construct the messages to send to the other processors.
    //
    int *sendcount = new int[nProcs];
    int  total_msg_size = 0;
    for (j = 0 ; j < nProcs ; j++)
    {
        sendcount[j] = 3*sizeof(float)*pt_cts[j];
        total_msg_size += sendcount[j];
    }
    char *big_send_msg = new char[total_msg_size];
    char **sub_ptr = new char*[nProcs];
    sub_ptr[0] = big_send_msg;
    for (i = 1 ; i < nProcs ; i++)
        sub_ptr[i] = sub_ptr[i-1] + sendcount[i-1];
    for (i = 0 ; i < pt_list_size.size() ; i++)
    {
        const int npts = pt_list_size[i];
        float    *pts  = pt_list[i];
        for (j = 0 ; j < npts ; j++)
        {
            float pt[3];
            pt[0] = *pts++;
            pt[1] = *pts++;
            pt[2] = *pts++;
            int proc = spat_part.GetProcessor(pt);
            memcpy(sub_ptr[proc], (void *) pt, 3*sizeof(float));
            sub_ptr[proc] += 3*sizeof(float);
        }
    }
    delete [] sub_ptr;

    int *recvcount = new int[nProcs];
    MPI_Alltoall(sendcount, 1, MPI_INT, recvcount, 1, MPI_INT, MPI_COMM_WORLD);

    char **recvmessages = new char*[nProcs];
    char *big_recv_msg = CreateMessageStrings(recvmessages, recvcount, nProcs);

    int *senddisp  = new int[nProcs];
    int *recvdisp  = new int[nProcs];
    senddisp[0] = 0;
    recvdisp[0] = 0;
    for (j = 1 ; j < nProcs ; j++)
    {
        senddisp[j] = sendcount[j-1] + senddisp[j-1];
        recvdisp[j] = recvcount[j-1] + recvdisp[j-1];
    }

    MPI_Alltoallv(big_send_msg, sendcount, senddisp, MPI_CHAR,
                  big_recv_msg, recvcount, recvdisp, MPI_CHAR,
                  MPI_COMM_WORLD);
    delete [] sendcount;
    delete [] senddisp;
    delete [] big_send_msg;

    //
    // Translate the buffers we just received into the points we should look
    // at.
    //
    int totalRecvd = 0;
    num_return_to_proc.resize(nProcs);
    for (j = 0 ; j < nProcs ; j++)
    {
        totalRecvd += recvcount[j];
        num_return_to_proc[j] = recvcount[j] / (3*sizeof(float));
    }
    int numPts = totalRecvd / (3*sizeof(float));
    float *newPts = new float[3*numPts];  // Deleted in UnRelocatePoints
    memcpy(newPts, big_recv_msg, totalRecvd);
    vector<float *> new_pt_list;
    new_pt_list.push_back(newPts);
    vector<int> new_pt_list_size;
    new_pt_list_size.push_back(numPts);

    delete [] recvmessages;
    delete [] big_recv_msg;
    delete [] recvcount;
    delete [] recvdisp;

    //
    // Now take our relocated points and use them as the new "desired points."
    //
    orig_pt_list = pt_list;
    orig_pt_list_size = pt_list_size;
    pt_list = new_pt_list;
    pt_list_size = new_pt_list_size;

    visitTimer->StopTimer(t0, "Spatial partitioning of desired points.");
#endif
}


// ****************************************************************************
//  Method: DesiredPoints::UnRelocatePoints
//
//  Purpose:
//      Sends the points from before the spatial partition object back to 
//      the processors they came from.
//
//  Programmer: Hank Childs
//  Creation:   October 12, 2005
//
// ****************************************************************************

void
avtPosCMFEExpression::DesiredPoints::UnRelocatePoints(
                                                   SpatialPartition &spat_part)
{
#ifdef PARALLEL
    int   i, j, k;
    int   nProcs = PAR_Size();

    //
    // We need to take the vals for our point list and send them back to the
    // processor they came from.
    //
    int *sendcount = new int[nProcs];
    int  totalSend = 0;
    for (i = 0 ; i < nProcs ; i++)
    {
        sendcount[i] = num_return_to_proc[i]*sizeof(float)*nComps;
        totalSend += sendcount[i];
    }

    int *recvcount = new int[nProcs];
    MPI_Alltoall(sendcount, 1, MPI_INT, recvcount, 1, MPI_INT, MPI_COMM_WORLD);

    char **recvmessages = new char*[nProcs];
    char *big_recv_msg = CreateMessageStrings(recvmessages, recvcount, nProcs);

    int *senddisp  = new int[nProcs];
    int *recvdisp  = new int[nProcs];
    senddisp[0] = 0;
    recvdisp[0] = 0;
    for (j = 1 ; j < nProcs ; j++)
    {
        senddisp[j] = sendcount[j-1] + senddisp[j-1];
        recvdisp[j] = recvcount[j-1] + recvdisp[j-1];
    }

    // Note that the "vals" array is already perfectly encoded to send
    // directly into this call without further processing.
    MPI_Alltoallv((char *) vals, sendcount, senddisp, MPI_CHAR,
                  big_recv_msg, recvcount, recvdisp, MPI_CHAR,
                  MPI_COMM_WORLD);
    delete [] sendcount;
    delete [] senddisp;

    //
    // Now put our point list back in order like it was never modified for
    // parallel reasons.
    //
    delete [] pt_list[0]; // This is the list of points we used after the
                          // relocation.  Delete it now.
    pt_list = orig_pt_list;
    pt_list_size = orig_pt_list_size;
    orig_pt_list.clear();
    orig_pt_list_size.clear();
    Finalize(); // Have it set up internal data structures based on the "new"
                // (i.e. original) points list.

    //
    // Now go through the recently sent messages and get the "vals" sent by
    // the other processors.  Encode them into a new "vals" array.
    //
    int idx = 0;
    for (i = 0 ; i < pt_list_size.size() ; i++)
    {
        const int npts = pt_list_size[i];
        float    *pts  = pt_list[i];
        for (j = 0 ; j < npts ; j++)
        {
            float pt[3];
            pt[0] = *pts++;
            pt[1] = *pts++;
            pt[2] = *pts++;
            int proc = spat_part.GetProcessor(pt);
            float *p = (float *) recvmessages[proc];
            for (k = 0 ; k < nComps ; k++)
                vals[idx++] = *p++;
            recvmessages[proc] += sizeof(float)*nComps;
        }
    }

    delete [] recvmessages;
    delete [] big_recv_msg;
    delete [] recvcount;
    delete [] recvdisp;
#endif
}


// ****************************************************************************
//  Method: FastLookupGrouping constructor
//
//  Programmer: Hank Childs
//  Creation:   October 11, 2005
//
// ****************************************************************************

avtPosCMFEExpression::FastLookupGrouping::FastLookupGrouping(std::string v,
                                                             bool isN)
{
    varname   = v;
    isNodal   = isN;
    itree     = NULL;
    map_to_ds = NULL;
    ds_start  = NULL;
}


// ****************************************************************************
//  Method: FastLookupGrouping destructor
//
//  Programmer: Hank Childs
//  Creation:   October 11, 2005
//
// ****************************************************************************

avtPosCMFEExpression::FastLookupGrouping::~FastLookupGrouping()
{
    ClearAllInputMeshes();
    delete itree;
    delete [] map_to_ds;
    delete [] ds_start;
}


// ****************************************************************************
//  Method: FastLookupGrouping::AddMesh
//
//  Purpose:
//      Gives the fast lookup grouping object another mesh to include in the
//      grouping.
//
//  Programmer: Hank Childs
//  Creation:   October 11, 2005
//
// ****************************************************************************

void
avtPosCMFEExpression::FastLookupGrouping::AddMesh(vtkDataSet *mesh)
{
    mesh->Register(NULL);
    meshes.push_back(mesh);
}


// ****************************************************************************
//  Method: FastLookupGrouping::ClearAllInputMeshes
//
//  Purpose:
//      Clears all the input meshes.  This is typically called from within
//      RelocateDataUsingPartition.
//
//  Programmer: Hank Childs
//  Creation:   November 2, 2005
//
// ****************************************************************************

void
avtPosCMFEExpression::FastLookupGrouping::ClearAllInputMeshes(void)
{
    for (int i = 0 ; i < meshes.size() ; i++)
    {
        meshes[i]->Delete();
    }
    meshes.clear();
}

// ****************************************************************************
//  Method: FastLookupGrouping::Finalize
//
//  Purpose:
//      Gives the fast lookup groupin object a chance to finish its
//      initializtion process.  This gives the object the cue that it will not
//      receive any more "AddMesh" calls and that it can initialize itself.
//
//  Programmer: Hank Childs
//  Creation:   October 11, 2005
//
// ****************************************************************************

void
avtPosCMFEExpression::FastLookupGrouping::Finalize(void)
{
    int   i, j;
    int   index = 0;

    int   t0 = visitTimer->StartTimer();

    nZones = 0;
    for (i = 0 ; i < meshes.size() ; i++)
        nZones += meshes[i]->GetNumberOfCells();

    ds_start = new int[meshes.size()];
    ds_start[0] = 0;
    for (i = 1 ; i < meshes.size() ; i++)
        ds_start[i] = ds_start[i-1] + meshes[i-1]->GetNumberOfCells();

    bool degenerate = false;
    if (nZones == 0)
    {
        degenerate = true;
        nZones = 1;
    }
    itree = new avtIntervalTree(nZones, 3);
    map_to_ds = new int[nZones];
    index = 0;
    for (i = 0 ; i < meshes.size() ; i++)
    {
        int nCells = meshes[i]->GetNumberOfCells();
        for (j = 0 ; j < nCells ; j++)
        {
            vtkCell *cell = meshes[i]->GetCell(j);
            float bounds[6];
            cell->GetBounds(bounds);
            itree->AddDomain(index, bounds);

            map_to_ds[index] = i;
            index++;
        }
    }
    if (degenerate)
    {
        float bounds[6] = { 0, 1, 0, 1, 0, 1 };
        itree->AddDomain(0, bounds);
    }

    int t1 = visitTimer->StartTimer();
    itree->Calculate(true);
    visitTimer->StopTimer(t1, "Initializing interval tree");

    visitTimer->StopTimer(t0, "Total initialization of fast lookup grouping");
}


// ****************************************************************************
//  Method: FastLookupGrouping::GetValue
//
//  Purpose:
//      Evaluates the value at a position.  Does this for the grouping of
//      meshes its been given and does it with fast lookups.
//
//  Programmer: Hank Childs
//  Creation:   October 11, 2005
//
// ****************************************************************************

bool
avtPosCMFEExpression::FastLookupGrouping::GetValue(const float *pt, float *val)
{
    float closestPt[3];
    int subId;
    float pcoords[3];
    float dist2;
    float weights[100]; // MUST BE BIGGER THAN NPTS IN A CELL (ie 8).
    float non_const_pt[3];
    non_const_pt[0] = pt[0];
    non_const_pt[1] = pt[1];
    non_const_pt[2] = pt[2];

    vector<int> list;
    itree->GetDomainsListFromRange(pt, pt, list);
    for (int j = 0 ; j < list.size() ; j++)
    {
        int mesh = map_to_ds[list[j]];
        int index = list[j] - ds_start[mesh];
        vtkCell *cell = meshes[mesh]->GetCell(index);
        int inCell = cell->EvaluatePosition(non_const_pt, closestPt, subId,
                                            pcoords, dist2, weights);
        if (!inCell)
            continue;

        if (isNodal)
        {
            vtkDataArray *arr = meshes[mesh]->GetPointData()
                                                   ->GetArray(varname.c_str());
            if (arr == NULL)
            {
                EXCEPTION0(ImproperUseException);
            }

            int nComponents = arr->GetNumberOfComponents();
            int nPts = cell->GetNumberOfPoints();
            for (int c = 0 ; c < nComponents ; c++)
            {
                val[c] = 0.;
                for (int pt = 0 ; pt < nPts ; pt++)
                {
                    vtkIdType id = cell->GetPointId(pt);
                    val[c] += weights[pt]*arr->GetComponent(id, c);
                }
            }
        }
        else
        {
            vtkDataArray *arr = meshes[mesh]->GetCellData()
                                                   ->GetArray(varname.c_str());
            if (arr == NULL)
            {
                EXCEPTION0(ImproperUseException);
            }

            int nComponents = arr->GetNumberOfComponents();
            for (int c = 0 ; c < nComponents ; c++)
                val[c] = arr->GetComponent(index, c);
        }
        return true;
    }

    return false;
}


// ****************************************************************************
//  Method: FastLookupGrouping::RelocateDataUsingPartition
//
//  Purpose:
//      Relocates the data to different processors to honor the spatial 
//      partition.
//
//  Programmer: Hank Childs
//  Creation:   October 12, 2005
//
// ****************************************************************************

void
avtPosCMFEExpression::FastLookupGrouping::RelocateDataUsingPartition(
                                                   SpatialPartition &spat_part)
{
#ifdef PARALLEL
    int  t0 = visitTimer->StartTimer();

    int  i, j, k;

    int   nProcs = PAR_Size();

    vtkUnstructuredGrid **meshForProcP = new vtkUnstructuredGrid*[nProcs];
    int                  *nCellsForP   = new int[nProcs];
    vtkAppendFilter     **appenders    = new vtkAppendFilter*[nProcs];
    for (i = 0 ; i < nProcs ; i++)
        appenders[i] = vtkAppendFilter::New();

    vector<int> list;
    for (i = 0 ; i < meshes.size() ; i++)
    {
        //
        // Get the mesh from the input for this iteration.  During each 
        // iteration, our goal is, for each cell in the mesh, to identify
        // which processors need this cell to do their sampling and to add
        // that cell to a data structure so we can do a large communication
        // afterwards.
        //
        vtkDataSet *mesh = meshes[i];
        const int nCells = mesh->GetNumberOfCells();

        // 
        // Reset the data structures that contain the cells from mesh i that
        // need to be sent to processor P.
        //
        for (j = 0 ; j < nProcs ; j++)
        {
            meshForProcP[j] = NULL;
            nCellsForP[j]   = 0;
        }

        //
        // For each cell in the input mesh, determine which processors that
        // cell needs to be sent to (typically just one other processor).  Then
        // add the cell to a data structure that will holds the cells.  In the
        // next phase we will use this data structure to construct a large
        // message to each of the other processors containing the cells it
        // needs.
        //
        for (j = 0 ; j < nCells ; j++)
        {
            vtkCell *cell = mesh->GetCell(j);
            spat_part.GetProcessorList(cell, list);
            for (k = 0 ; k < list.size() ; k++)
            {
                if (meshForProcP[list[k]] == NULL)
                {
                    meshForProcP[list[k]] = vtkUnstructuredGrid::New();
                    vtkPoints *pts = vtkVisItUtility::GetPoints(mesh);
                    meshForProcP[list[k]]->SetPoints(pts);
                    meshForProcP[list[k]]->GetPointData()->ShallowCopy(
                                                         mesh->GetPointData());
                    meshForProcP[list[k]]->GetCellData()->CopyAllocate(
                                                          mesh->GetCellData());
                    meshForProcP[list[k]]->Allocate(nCells*9);
                    pts->Delete();
                }
                int cellType = mesh->GetCellType(j);
                vtkIdList *ids = cell->GetPointIds();
                meshForProcP[list[k]]->InsertNextCell(cellType, ids);
                meshForProcP[list[k]]->GetCellData()->CopyData(
                                  mesh->GetCellData(), j, nCellsForP[list[k]]);
                nCellsForP[list[k]]++;
            }
        }

        //
        // The data structures we used for examining mesh 'i' are temporary.
        // Put them in a more permanent location so we can move on to the
        // next iteration.
        //
        for (j = 0 ; j < nProcs ; j++)
            if (meshForProcP[j] != NULL)
            {
                vtkUnstructuredGridRelevantPointsFilter *ugrpf = 
                                vtkUnstructuredGridRelevantPointsFilter::New();
                ugrpf->SetInput(meshForProcP[j]);
                ugrpf->Update();
                appenders[j]->AddInput(ugrpf->GetOutput());
                ugrpf->Delete();
                meshForProcP[j]->Delete();
            }
    }
    delete [] meshForProcP;
    delete [] nCellsForP;

    //
    // We now have, for each processor P, the list of cells from this processor
    // that P will need to do its job.  So construct a big message containing
    // these cell lists and then send it to P.  Of course, while we are busy
    // composing messages to each of the processors, they are busy composing
    // message to us.  So use an 'alltoallV' call that allows us to get the
    // cells that are necessary for *this* processor to do its job.
    //
    int *sendcount = new int[nProcs];
    char **msg_tmp = new char *[nProcs];
    for (j = 0 ; j < nProcs ; j++)
    {
        if (appenders[j]->GetNumberOfInputs() == 0)
        {
            sendcount[j] = 0;
            msg_tmp[j]   = NULL;
            continue;
        }
        appenders[j]->Update();
        vtkUnstructuredGridWriter *wrtr = vtkUnstructuredGridWriter::New();
        wrtr->SetInput(appenders[j]->GetOutput());
        wrtr->SetWriteToOutputString(1);
        wrtr->SetFileTypeToBinary();
        wrtr->Write();
        sendcount[j] = wrtr->GetOutputStringLength();
        msg_tmp[j] = (char *) wrtr->RegisterAndGetOutputString();
        wrtr->Delete();
        appenders[j]->Delete();
    }
    delete [] appenders;

    int total_msg_size = 0;
    for (j = 0 ; j < nProcs ; j++)
        total_msg_size += sendcount[j];

    char *big_send_msg = new char[total_msg_size];
    char *ptr = big_send_msg;
    for (j = 0 ; j < nProcs ; j++)
    {
        if (msg_tmp[j] != NULL)
        {
            memcpy(ptr, msg_tmp[j], sendcount[j]*sizeof(char));
            ptr += sendcount[j]*sizeof(char);
            delete [] msg_tmp[j];
        }
    }
    delete [] msg_tmp;

    int *recvcount = new int[nProcs];
    MPI_Alltoall(sendcount, 1, MPI_INT, recvcount, 1, MPI_INT, MPI_COMM_WORLD);

    char **recvmessages = new char*[nProcs];
    char *big_recv_msg = CreateMessageStrings(recvmessages, recvcount, nProcs);

    int *senddisp  = new int[nProcs];
    int *recvdisp  = new int[nProcs];
    senddisp[0] = 0;
    recvdisp[0] = 0;
    for (j = 1 ; j < nProcs ; j++)
    {
        senddisp[j] = sendcount[j-1] + senddisp[j-1];
        recvdisp[j] = recvcount[j-1] + recvdisp[j-1];
    }

    MPI_Alltoallv(big_send_msg, sendcount, senddisp, MPI_CHAR,
                  big_recv_msg, recvcount, recvdisp, MPI_CHAR,
                  MPI_COMM_WORLD);
    delete [] sendcount;
    delete [] senddisp;
    delete [] big_send_msg;

    ClearAllInputMeshes();
    for (j = 0 ; j < nProcs ; j++)
    {
        if (recvcount[j] == 0)
            continue;

        vtkCharArray *charArray = vtkCharArray::New();
        int iOwnIt = 1;  // 1 means we own it -- you don't delete it.
        charArray->SetArray(recvmessages[j], recvcount[j], iOwnIt);
        vtkUnstructuredGridReader *reader = vtkUnstructuredGridReader::New();
        reader->SetReadFromInputString(1);
        reader->SetInputArray(charArray);
        vtkUnstructuredGrid *ugrid = reader->GetOutput();
        ugrid->Update();
        AddMesh(ugrid);
        ugrid->SetSource(NULL);
        reader->Delete();
        charArray->Delete();
    }
    delete [] recvmessages;
    delete [] big_recv_msg;
    delete [] recvcount;
    delete [] recvdisp;

    visitTimer->StopTimer(t0, "Spatial partitioning of fast lookup group.");
#endif
}


// ****************************************************************************
//  Method: SpatialPartition constructor
//
//  Programmer: Hank Childs
//  Creation:   October 12, 2005
//
// ****************************************************************************

avtPosCMFEExpression::SpatialPartition::SpatialPartition()
{
    itree = NULL;
}


// ****************************************************************************
//  Method: SpatialPartition destructor
//
//  Programmer: Hank Childs
//  Creation:   October 12, 2005
//
// ****************************************************************************

avtPosCMFEExpression::SpatialPartition::~SpatialPartition()
{
    delete itree;
}


// ****************************************************************************
//  Class: Boundary
//
//  Purpose:
//      This class is for setting up a spatial partition.  It contains methods
//      that allow the spatial partitioning routine to not be so cumbersome.
//
//  Programmer: Hank Childs
//  Creation:   January 9, 2006
//
// ****************************************************************************

typedef enum
{
    X_AXIS,
    Y_AXIS, 
    Z_AXIS
} Axis;

const int npivots = 5;
class Boundary
{
   public:
                      Boundary(const float *, int, Axis);
     virtual         ~Boundary() {;};

     float           *GetBoundary() { return bounds; };
     bool             AttemptSplit(Boundary *&, Boundary *&);
     bool             IsDone(void) { return isDone; };
     bool             IsLeaf(void) { return (numProcs == 1); };
     void             AddCell(const float *);
     void             AddPoint(const float *);
     static void      SetIs2D(bool b) { is2D = b; };
     static void      PrepareSplitQuery(Boundary **, int);
     
   protected:
     float            bounds[6];
     float            pivots[npivots];
     int              numCells[npivots+1];
     int              numProcs;
     int              nAttempts;
     Axis             axis;
     bool             isDone;
     static bool      is2D;
};

bool Boundary::is2D = false;


// ****************************************************************************
//  Method: Boundary constructor
//
//  Programmer: Hank Childs
//  Creation:   January 9, 2006
// 
// ****************************************************************************

Boundary::Boundary(const float *b, int n, Axis a)
{
    int  i;

    for (i = 0 ; i < 6 ; i++)
        bounds[i] = b[i];
    numProcs = n;
    axis = a;
    isDone = (numProcs == 1);
    nAttempts = 0;

    //
    // Set up the pivots.
    //
    int index = 0;
    if (axis == Y_AXIS)
        index = 2;
    else if (axis == Z_AXIS)
        index = 4;
    float min = bounds[index];
    float max = bounds[index+1];
    float step = (max-min) / (npivots+1);
    for (i = 0 ; i < npivots ; i++)
    {
        pivots[i] = min + (i+1)*step;
    }
    for (i = 0 ; i < npivots+1 ; i++)
        numCells[i] = 0;
}


// ****************************************************************************
//  Method: Boundary::PrepareSplitQuery
//
//  Purpose:
//      Each Boundary is operating only with the information on this processor.
//      When it comes time to determine if we can split a boundary, the info
//      from each processor needs to be unified.  That is the purpose of this
//      method.  It unifies the information so that Boundaries can later make
//      good decisions regarding whether or not they can split themselves.
//
//  Programmer: Hank Childs
//  Creation:   January 9, 2006
// 
// ****************************************************************************

void
Boundary::PrepareSplitQuery(Boundary **b_list, int listSize)
{
    int   i, j;
    int   idx;

    int  num_vals = listSize*(npivots+1);
    int *in_vals = new int[num_vals];
    idx = 0;
    for (i = 0 ; i < listSize ; i++)
        for (j = 0 ; j < npivots+1 ; j++)
            in_vals[idx++] = b_list[i]->numCells[j];

    int *out_vals = new int[num_vals];
    SumIntArrayAcrossAllProcessors(in_vals, out_vals, num_vals);

    idx = 0;
    for (i = 0 ; i < listSize ; i++)
        for (j = 0 ; j < npivots+1 ; j++)
            b_list[i]->numCells[j] = out_vals[idx++];

    delete [] in_vals;
    delete [] out_vals;
}


// ****************************************************************************
//  Method: Boundary::PrepareSplitQuery
//
//  Purpose:
//      Adds a point to the boundary.
//
//  Programmer: Hank Childs
//  Creation:   January 9, 2006
// 
// ****************************************************************************

void
Boundary::AddPoint(const float *pt)
{
    float p = (axis == X_AXIS ? pt[0] : axis == Y_AXIS ? pt[1] : pt[2]);
    for (int i = 0 ; i < npivots ; i++)
        if (p < pivots[i])
        {
            numCells[i]++;
            return;
        }
    numCells[npivots]++;
}


// ****************************************************************************
//  Method: Boundary::PrepareSplitQuery
//
//  Purpose:
//      Sees if the boundary has found an acceptable pivot to split around.
//
//  Programmer: Hank Childs
//  Creation:   January 9, 2006
// 
// ****************************************************************************

bool
Boundary::AttemptSplit(Boundary *&b1, Boundary *&b2)
{
    int  i;

    int numProcs1 = numProcs/2;
    int numProcs2 = numProcs-numProcs1;
    
    int totalCells = 0;
    for (i = 0 ; i < npivots+1 ; i++)
        totalCells += numCells[i]; 

    if (totalCells == 0)
    {
        // Should never happen...
        debug1 << "Error condition occurred when making boundaries" << endl;
        isDone = true;
        return false;
    }

    int cellsSoFar = 0;
    float amtSeen[npivots];
    for (i = 0 ; i < npivots ; i++)
    {
        cellsSoFar += numCells[i];
        amtSeen[i] = cellsSoFar / ((float) totalCells);
    }

    float proportion = ((float) numProcs1) / ((float) numProcs);
    float closest  = fabs(proportion - amtSeen[0]); // == proportion
    int   closestI = 0;
    for (i = 1 ; i < npivots ; i++)
    {
        float diff = fabs(proportion - amtSeen[i]);
        if (diff < closest)
        {
            closest  = diff;
            closestI = i;
        }
    }

    nAttempts++;
    if (closest < 0.02 || nAttempts > 3)
    {
        float b_tmp[6];
        for (i = 0 ; i < 6 ; i++)
            b_tmp[i] = bounds[i];
        if (axis == X_AXIS)
        {
            b_tmp[1] = pivots[closestI];
            b1 = new Boundary(b_tmp, numProcs1, Y_AXIS);
            b_tmp[0] = pivots[closestI];
            b_tmp[1] = bounds[1];
            b2 = new Boundary(b_tmp, numProcs2, Y_AXIS);
        }
        else if (axis == Y_AXIS)
        {
            Axis next_axis = (is2D ? X_AXIS : Z_AXIS);
            b_tmp[3] = pivots[closestI];
            b1 = new Boundary(b_tmp, numProcs1, next_axis);
            b_tmp[2] = pivots[closestI];
            b_tmp[3] = bounds[3];
            b2 = new Boundary(b_tmp, numProcs2, next_axis);
        }
        else
        {
            b_tmp[5] = pivots[closestI];
            b1 = new Boundary(b_tmp, numProcs1, X_AXIS);
            b_tmp[4] = pivots[closestI];
            b_tmp[5] = bounds[5];
            b2 = new Boundary(b_tmp, numProcs2, X_AXIS);
        }
        isDone = true;
    }
    else
    {
        //
        // Set up the pivots.  We are going to reset the pivot positions to be
        // in between the two closest pivots.
        //
        int firstBigger = -1;
        int amtSeen = 0;
        for (i = 0 ; i < npivots+1 ; i++)
        {
            amtSeen += numCells[i];
            float soFar = ((float) amtSeen) / ((float) totalCells);
            if (soFar > proportion)
            {
                firstBigger = i;
                break;
            }
        }

        float min, max;

        int index = 0;
        if (axis == Y_AXIS)
            index = 2;
        else if (axis == Z_AXIS)
            index = 4;

        if (firstBigger <= 0)
        {
            min = pivots[0] - (pivots[1] - pivots[0]);
            max = pivots[0];
        } 
        else if (firstBigger >= npivots)
        {
            min = pivots[npivots-1];
            max = pivots[npivots-1] + (pivots[1] - pivots[0]);
        }
        else
        {
            min = pivots[firstBigger-1];
            max = pivots[firstBigger];
        }
        float step = (max-min) / (npivots+1);
        for (i = 0 ; i < npivots ; i++)
            pivots[i] = min + (i+1)*step;
        for (i = 0 ; i < npivots+1 ; i++)
            numCells[i] = 0;

        return false;
    }

    return true;
}


// ****************************************************************************
//  Method: SpatialPartition::CreatePartition
//
//  Purpose:
//      Creates a partition that is balanced for both the desired points and
//      the fast lookup grouping.
//
//  Programmer: Hank Childs
//  Creation:   October 12, 2005
//
// ****************************************************************************

void
avtPosCMFEExpression::SpatialPartition::CreatePartition(DesiredPoints &dp,
                                       FastLookupGrouping &flg, double *bounds)
{
    int   i, j, k;

    if (itree != NULL)
        delete itree;

    //
    // Here's the gameplan:
    // We are going to start off with a single "boundary".  Ultimately, we
    // are going to want to have N boundaries, where N is the number of
    // processors.  So we tell this initial boundary that it represents N
    // processors.  Then we tell it to choose some pivots that it thinks
    // might allow itself to split into two boundaries, each with half the 
    // amount of work and each representing half the number of processor. 
    // Now we have two boundaries, and we keep splitting them (across 
    // different axes) until we get N boundaries, where each one represents
    // a single processor.
    //
    // Once we do that, we can construct an interval tree of the boundaries,
    // which represents our spatial partitioning.
    //
    bool is2D = (bounds[4] == bounds[5]);
    Boundary::SetIs2D(is2D);
    int nProcs = PAR_Size();
    Boundary **b_list = new Boundary*[2*nProcs];
    float fbounds[6];
    fbounds[0] = bounds[0];
    fbounds[1] = bounds[1];
    fbounds[2] = bounds[2];
    fbounds[3] = bounds[3];
    fbounds[4] = bounds[4];
    fbounds[5] = bounds[5];
    if (is2D)
    {
        fbounds[4] -= 1.;
        fbounds[5] += 1.;
    }
    b_list[0] = new Boundary(fbounds, nProcs, X_AXIS);
    int listSize = 1;
    int *bin_lookup = new int[2*nProcs];
    bool keepGoing = true;
    while (keepGoing)
    {
        // Figure out how many boundaries need to keep going.
        int nBins = 0;
        for (i = 0 ; i < listSize ; i++)
            if (!(b_list[i]->IsDone()))
            {
                bin_lookup[nBins] = i;
                nBins++;
            }

        // Construct an interval tree out of the boundaries.  We need this
        // because we want to be able to quickly determine which boundaries
        // a point falls in.
        avtIntervalTree it = avtIntervalTree(nBins, 3);
        nBins = 0;
        for (i = 0 ; i < listSize ; i++)
        {
            if (b_list[i]->IsDone())
                continue;
            it.AddDomain(nBins, b_list[i]->GetBoundary());
            nBins++;
        }
        it.Calculate(true);

        // Now add each point to the boundary it falls in.
        const int nPoints = dp.GetNumberOfPoints();
        vector<int> list;
        float pt[3];
        for (i = 0 ; i < nPoints ; i++)
        {
            dp.GetPoint(i, pt);
            it.GetDomainsListFromRange(pt, pt, list);
            for (j = 0 ; j < list.size() ; j++)
            {
                Boundary *b = b_list[bin_lookup[list[j]]];
                b->AddPoint(pt);
            }
        }
        // Now do the cells.  We are using the cell centers, which is a decent
        // approximation.
        vector<vtkDataSet *> meshes = flg.GetMeshes();
        for (i = 0 ; i < meshes.size() ; i++)
        {
            const int ncells = meshes[i]->GetNumberOfCells();
            float bbox[6];
            float pt[3];
            for (j = 0 ; j < ncells ; j++)
            {
                vtkCell *cell = meshes[i]->GetCell(j);
                cell->GetBounds(bbox);
                pt[0] = (bbox[0] + bbox[1]) / 2.;
                pt[1] = (bbox[2] + bbox[3]) / 2.;
                pt[2] = (bbox[4] + bbox[5]) / 2.;
                it.GetDomainsListFromRange(pt, pt, list);
                for (k = 0 ; k < list.size() ; k++)
                {
                    Boundary *b = b_list[bin_lookup[list[k]]];
                    b->AddPoint(pt);
                }
            }
        }

        // See which boundaries found a suitable pivot and can now split.
        Boundary::PrepareSplitQuery(b_list, listSize);
        int numAtStartOfLoop = listSize;
        for (i = 0 ; i < numAtStartOfLoop ; i++)
        {
            if (b_list[i]->IsDone())
                continue;
            Boundary *b1, *b2;
            if (b_list[i]->AttemptSplit(b1, b2))
            {
                b_list[listSize++] = b1;
                b_list[listSize++] = b2;
            }
        }

        // See if there are any boundaries that need more processing.  
        // Obviously, all the boundaries that were just split need more 
        // processing, because they haven't done any yet.
        keepGoing = false;
        for (i = 0 ; i < listSize ; i++)
            if (!(b_list[i]->IsDone()))
                keepGoing = true;
    }

    // Construct an interval tree out of the boundaries.  This interval tree
    // contains the actual spatial partitioning.
    itree = new avtIntervalTree(nProcs, 3);
    int count = 0;
    for (i = 0 ; i < listSize ; i++)
    {
        if (b_list[i]->IsLeaf())
        {
            float *b = b_list[i]->GetBoundary();
            itree->AddDomain(count++, b);
        }
    }
    itree->Calculate(true);

    bool determineBalance = false;
    if (determineBalance)
    {
        count = 0;
        for (i = 0 ; i < listSize ; i++)
        {
            if (b_list[i]->IsLeaf())
            {
                float *b = b_list[i]->GetBoundary();
                debug1 << "Boundary " << count++ << " = " << b[0] << "-" <<b[1]
                       << ", " << b[2] << "-" << b[3] << ", " << b[4] << "-"
                       << b[5] << endl;
            }
        }

        int *cnts = new int[nProcs];
        for (i = 0 ; i < nProcs ; i++)
            cnts[i] = 0;
        const int nPoints = dp.GetNumberOfPoints();
        vector<int> list;
        float pt[3];
        for (i = 0 ; i < nPoints ; i++)
        {
            dp.GetPoint(i, pt);
            itree->GetDomainsListFromRange(pt, pt, list);
            for (j = 0 ; j < list.size() ; j++)
            {
                cnts[list[j]]++;
            }
        }
        int *cnts_out = new int[nProcs];
        SumIntArrayAcrossAllProcessors(cnts, cnts_out, nProcs);
        for (i = 0 ; i < nProcs ; i++)
            debug5 << "Amount for processor " << i << " = " << cnts_out[i] 
                   << endl;
        
        delete [] cnts;
        delete [] cnts_out;
    }

    // Clean up.
    for (i = 0 ; i < listSize ; i++)
        delete b_list[i];
    delete [] b_list;
    delete [] bin_lookup;
}


// ****************************************************************************
//  Method: SpatialPartition::GetProcessor
//
//  Purpose:
//      Gets the processor that contains this point
//
//  Programmer: Hank Childs
//  Creation:   October 12, 2005
//
// ****************************************************************************

int
avtPosCMFEExpression::SpatialPartition::GetProcessor(float *pt)
{
    vector<int> list;
    itree->GetDomainsListFromRange(pt, pt, list);
    if (list.size() <= 0)
    {
        EXCEPTION0(ImproperUseException);
    }

    return list[0];
}


// ****************************************************************************
//  Method: SpatialPartition::GetProcessor
//
//  Purpose:
//      Gets the processor that contains this cell
//
//  Programmer: Hank Childs
//  Creation:   October 12, 2005
//
// ****************************************************************************

int
avtPosCMFEExpression::SpatialPartition::GetProcessor(vtkCell *cell)
{
    float bounds[6];
    cell->GetBounds(bounds);
    float mins[3];
    mins[0] = bounds[0];
    mins[1] = bounds[2];
    mins[2] = bounds[4];
    float maxs[3];
    maxs[0] = bounds[1];
    maxs[1] = bounds[3];
    maxs[2] = bounds[5];

    vector<int> list;
    itree->GetDomainsListFromRange(mins, maxs, list);
    if (list.size() <= 0)
    {
        return -2;
    }
    if (list.size() > 1)
    {
        return -1;
    }

    return list[0];
}


// ****************************************************************************
//  Method: SpatialPartition::GetProcessorList
//
//  Purpose:
//      Gets the processor that contains this cell.  This should be called
//      when a list of processors contain a cell.
//
//  Programmer: Hank Childs
//  Creation:   October 12, 2005
//
// ****************************************************************************

void
avtPosCMFEExpression::SpatialPartition::GetProcessorList(vtkCell *cell,
                                                        std::vector<int> &list)
{
    list.clear();

    float bounds[6];
    cell->GetBounds(bounds);
    float mins[3];
    mins[0] = bounds[0];
    mins[1] = bounds[2];
    mins[2] = bounds[4];
    float maxs[3];
    maxs[0] = bounds[1];
    maxs[1] = bounds[3];
    maxs[2] = bounds[5];

    itree->GetDomainsListFromRange(mins, maxs, list);
}


