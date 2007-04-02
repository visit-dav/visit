/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                           avtPosCMFEAlgorithm.C                           //
// ************************************************************************* //

#include <avtPosCMFEAlgorithm.h>

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
#include <vtkRectilinearGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnstructuredGridRelevantPointsFilter.h>
#include <vtkUnstructuredGridWriter.h>
#include <vtkUnstructuredGridReader.h>

#include <Utility.h>
#include <vtkVisItUtility.h>

#include <avtCommonDataFunctions.h>
#include <avtDatasetExaminer.h>
#include <avtExpressionFilter.h>
#include <avtIntervalTree.h>
#include <avtParallel.h>
#include <avtSILRestrictionTraverser.h>

#include <DebugStream.h>
#include <ExpressionException.h>
#include <InvalidMergeException.h>
#include <TimingsManager.h>


using std::vector;


// ****************************************************************************
//  Method: avtPosCMFEAlgorithm::PerformCMFE
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
avtPosCMFEAlgorithm::PerformCMFE(avtDataTree_p output_mesh,
                                  avtDataTree_p mesh_to_be_sampled,
                                  const std::string &invar,
                                  const std::string &default_var,
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
        const char *defaultName = default_var.c_str();
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
                 defaultVar = avtExpressionFilter::Recenter(in_ds1, defaultVar, 
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
//  Modifications:
//
//    Hank Childs, Sat Mar 18 09:42:29 PST 2006
//    Add some more initializations.
//
// ****************************************************************************

avtPosCMFEAlgorithm::DesiredPoints::DesiredPoints(bool isN, int nc)
{
    isNodal   = isN;
    nComps    = nc;
    map_to_ds = NULL;
    ds_start  = NULL;
    vals      = NULL;
    total_nvals  = 0;
    num_datasets = 0;
    num_rgrids   = 0;
}


// ****************************************************************************
//  Method: DesiredPoints destructor
//
//  Programmer: Hank Childs
//  Creation:   October 11, 2005
//
//  Modifications:
//
//    Hank Childs, Sat Mar 18 09:42:29 PST 2006
//    Added the deletion of rgrid_pts.
//
// ****************************************************************************

avtPosCMFEAlgorithm::DesiredPoints::~DesiredPoints()
{
    int   i;

    delete [] map_to_ds;
    delete [] ds_start;
    delete [] vals;
    for (i = 0 ; i < pt_list.size() ; i++)
        delete [] pt_list[i];
    for (i = 0 ; i < rgrid_pts.size() ; i++)
        delete [] rgrid_pts[i];
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
//  Modifications:
//
//    Hank Childs, Sat Mar 18 09:42:29 PST 2006
//    Optimize for rectilinear grids.
//
//    Kathleen Bonnell, Tue May  2 08:58:01 PDT 2006 
//    Use correct dims index for Y and Z coords tests. 
//
// ****************************************************************************

void
avtPosCMFEAlgorithm::DesiredPoints::AddDataset(vtkDataSet *ds)
{
    int  i;

    int nvals = (isNodal ? ds->GetNumberOfPoints() : ds->GetNumberOfCells());
    if (ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        //
        // Get the rectilinear grid and determine its dimensions.  Be leery
        // of situations where the grid is flat in a dimension.
        //
        vtkRectilinearGrid *rgrid = (vtkRectilinearGrid *) ds;
        int dims[3];
        rgrid->GetDimensions(dims);
        if (!isNodal)
        {
            dims[0] = (dims[0] > 1 ? dims[0]-1 : 1);
            dims[1] = (dims[1] > 1 ? dims[1]-1 : 1);
            dims[2] = (dims[2] > 1 ? dims[2]-1 : 1);
        }

        //
        // Set up the X-coordinates.
        //
        vtkDataArray *x    = rgrid->GetXCoordinates();
        float        *newX = new float[dims[0]];
        for (i = 0 ; i < dims[0] ; i++)
        {
            if (isNodal)
                newX[i] = x->GetTuple1(i);
            else if (dims[0] == 1)
                newX[i] = x->GetTuple1(i);
            else
                newX[i] = (x->GetTuple1(i) + x->GetTuple1(i+1)) / 2.;
        }
        rgrid_pts_size.push_back(dims[0]);
        rgrid_pts.push_back(newX);

        //
        // Set up the Y-coordinates.
        //
        vtkDataArray *y    = rgrid->GetYCoordinates();
        float        *newY = new float[dims[1]];
        for (i = 0 ; i < dims[1] ; i++)
        {
            if (isNodal)
                newY[i] = y->GetTuple1(i);
            else if (dims[1] == 1)
                newY[i] = y->GetTuple1(i);
            else
                newY[i] = (y->GetTuple1(i) + y->GetTuple1(i+1)) / 2.;
        }
        rgrid_pts_size.push_back(dims[1]);
        rgrid_pts.push_back(newY);

        //
        // Set up the Z-coordinates.
        //
        vtkDataArray *z    = rgrid->GetZCoordinates();
        float        *newZ = new float[dims[2]];
        for (i = 0 ; i < dims[2] ; i++)
        {
            if (isNodal)
                newZ[i] = z->GetTuple1(i);
            else if (dims[2] == 1)
                newZ[i] = z->GetTuple1(i);
            else
                newZ[i] = (z->GetTuple1(i) + z->GetTuple1(i+1)) / 2.;
        }
        rgrid_pts_size.push_back(dims[2]);
        rgrid_pts.push_back(newZ);
    }
    else
    {
        float *plist = new float[3*nvals];
        pt_list.push_back(plist);
        pt_list_size.push_back(nvals);
   
        double dcp[3]; 
        for (i = 0 ; i < nvals ; i++)
        {
            float *cur_pt = plist + 3*i;
            if (isNodal)
                ds->GetPoint(i, dcp);
            else
            {
                vtkCell *cell = ds->GetCell(i);
                vtkVisItUtility::GetCellCenter(cell, dcp);
            }
            cur_pt[0] = (float)dcp[0];
            cur_pt[1] = (float)dcp[1];
            cur_pt[2] = (float)dcp[2];
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
//  Modifications:
//
//    Hank Childs, Sat Mar 18 09:42:29 PST 2006
//    Add support for rectilinear grids.
//
// ****************************************************************************

void
avtPosCMFEAlgorithm::DesiredPoints::Finalize(void)
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
    num_rgrids = rgrid_pts.size() / 3;
    int numNonRGrid = pt_list_size.size();
    num_datasets = numNonRGrid + num_rgrids;
    for (i = 0 ; i < numNonRGrid ; i++)
        total_nvals += pt_list_size[i];
    rgrid_start = total_nvals;
    for (i = 0 ; i < num_rgrids ; i++)
    {
        int nvals = rgrid_pts_size[3*i] * rgrid_pts_size[3*i+1]
                  * rgrid_pts_size[3*i+2];
        total_nvals += nvals;
    }

    int *ds_size = new int[num_datasets];
    for (i = 0 ; i < numNonRGrid ; i++)
        ds_size[i] = pt_list_size[i];
    for (i = 0 ; i < num_rgrids ; i++)
        ds_size[i+numNonRGrid] = rgrid_pts_size[3*i] * rgrid_pts_size[3*i+1]
                               * rgrid_pts_size[3*i+2];

    ds_start = new int[num_datasets];
    ds_start[0] = 0;
    for (i = 1 ; i < num_datasets ; i++)
        ds_start[i] = ds_start[i-1] + ds_size[i-1];
    delete [] ds_size;

    map_to_ds = new int[total_nvals];
    index = 0;
    for (i = 0 ; i < numNonRGrid ; i++)
        for (j = 0 ; j < pt_list_size[i] ; j++)
        {
            map_to_ds[index] = i;
            index++;
        }
    for (i = 0 ; i < num_rgrids ; i++)
    {
        int nvals = rgrid_pts_size[3*i] * rgrid_pts_size[3*i+1]
                  * rgrid_pts_size[3*i+2];
        for (j = 0 ; j < nvals ; j++)
        {
            map_to_ds[index] = i+numNonRGrid;
            index++;
        }
    }

    vals = new float[total_nvals*nComps];
}


// ****************************************************************************
//  Method: DesiredPoints::GetRGrid
//
//  Purpose:
//      Gets a rectilinear grid.
//
//  Programmer: Hank Childs
//  Creation:   March 18, 2006
//
// ****************************************************************************

void
avtPosCMFEAlgorithm::DesiredPoints::GetRGrid(int idx, const float *&x,
                   const float *&y, const float *&z, int &nx, int &ny, int &nz)
{
    if (idx < 0 || idx >= num_rgrids)
    {
        EXCEPTION0(ImproperUseException);
    }
    x = rgrid_pts[3*idx];
    y = rgrid_pts[3*idx+1];
    z = rgrid_pts[3*idx+2];
    nx = rgrid_pts_size[3*idx];
    ny = rgrid_pts_size[3*idx+1];
    nz = rgrid_pts_size[3*idx+2];
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
//  Modifications:
//
//    Hank Childs, Sat Mar 18 10:33:41 PST 2006
//    Add support for rectilinear grids.
//
// ****************************************************************************

void
avtPosCMFEAlgorithm::DesiredPoints::GetPoint(int p, float *pt) const
{
    if (p < 0 || p >= total_nvals)
    {
        EXCEPTION0(ImproperUseException);
    }
    int ds = map_to_ds[p];
    int start = ds_start[ds];
    int rel_index = p-start;
    if (p < rgrid_start)
    {
        float *ptr = pt_list[ds] + 3*rel_index;
        pt[0] = ptr[0];
        pt[1] = ptr[1];
        pt[2] = ptr[2];
    }
    else
    {
        int ds_rel_index = ds - rgrid_start;
        int nX = rgrid_pts_size[3*ds_rel_index];
        int xIdx = rel_index % nX;
        pt[0] = rgrid_pts[3*ds_rel_index][xIdx];
        int nY = rgrid_pts_size[3*ds_rel_index+1];
        int yIdx = (rel_index/nX) % nY;
        pt[1] = rgrid_pts[3*ds_rel_index+1][yIdx];
        int zIdx = rel_index/(nX*nY);
        pt[2] = rgrid_pts[3*ds_rel_index+2][zIdx];
    }
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
avtPosCMFEAlgorithm::DesiredPoints::SetValue(int p, float *v)
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
avtPosCMFEAlgorithm::DesiredPoints::GetValue(int ds_idx, int pt_idx) const
{
    if (ds_idx < 0 || ds_idx >= num_datasets)
    {
        EXCEPTION0(ImproperUseException);
    }

    int true_idx = ds_start[ds_idx] + pt_idx;
    return vals + nComps*true_idx;
}


// ****************************************************************************
//  Method: DesiredPoints::GetProcessorsForGrid
//
//  Purpose:
//      Uses the spatial partition to determine which processors a rectilinear
//      grid overlaps with.  Also finds the boundaries of each of those
//      processors.
//
//  Programmer: Hank Childs
//  Creation:   March 19, 2006
//
// ****************************************************************************

void
avtPosCMFEAlgorithm::DesiredPoints::GetProcessorsForGrid(int grid,
                    std::vector<int> &procId, std::vector<float> &procBoundary,
                    SpatialPartition &spat_part)
{
    float *xp = rgrid_pts[3*grid];
    float *yp = rgrid_pts[3*grid+1];
    float *zp = rgrid_pts[3*grid+2];
    int nX = rgrid_pts_size[3*grid];
    int nY = rgrid_pts_size[3*grid+1];
    int nZ = rgrid_pts_size[3*grid+2];
    float bounds[6];
    bounds[0] = xp[0];
    bounds[1] = xp[nX-1];
    bounds[2] = yp[0];
    bounds[3] = yp[nY-1];
    bounds[4] = zp[0];
    bounds[5] = zp[nZ-1];
    spat_part.GetProcessorBoundaries(bounds, procId, procBoundary);
}
         

// ****************************************************************************
//  Method: DesiredPoints::GetSubgridForBoundary
//
//  Purpose:
//      When a rectilinear grid overlaps with a region from the spatial
//      partition, it is likely that parts of the grid are within the region
//      and parts are outside the region.  If we naively send the whole
//      grid to the processor that owns the region, it will have more points
//      than it needs to sample.  The solution is to find the subgrid that
//      lies totally within the boundary.  That is the purpose of this method.
//
//  Notes:      Even though a boundary may overlap a region, it is possible
//              for this method to return "false" for no overlap.  This is 
//              because the grid may overlap the region, but none of its
//              points are actually within the region.
//              As in:
//                  G-------G
//                  |       |
//             R----+-------+----R
//             |    |       |    |
//             |    |       |    |
//             |    |       |    |
//             R----+-------+----R
//                  |       |
//                  G-------G
//
//             If R represents the boundaries of the region and G represents
//             grid points on the grid, then no point in G lies within R,
//             even though they overlap.  Obviously, this case comes up very
//             rarely, as the grid must be as coarse as actual regions, but
//             it does come up, so the caller must prepare for it.
//
//  Programmer: Hank Childs
//  Creation:   March 19, 2006
//
// ****************************************************************************

bool
avtPosCMFEAlgorithm::DesiredPoints::GetSubgridForBoundary(int grid,
                                                 float *bounds, int *extents)
{
    float *xp = rgrid_pts[3*grid];
    float *yp = rgrid_pts[3*grid+1];
    float *zp = rgrid_pts[3*grid+2];
    int nX = rgrid_pts_size[3*grid];
    int nY = rgrid_pts_size[3*grid+1];
    int nZ = rgrid_pts_size[3*grid+2];

    int xStart = 0;
    while (xp[xStart] < bounds[0] && xStart < nX)
        xStart++;
    int xEnd = nX-1;
    while (xp[xEnd] > bounds[1] && xEnd > 0)
        xEnd--;
    if (xEnd < xStart)
        return false;

    int yStart = 0;
    while (yp[yStart] < bounds[2] && yStart < nY)
        yStart++;
    int yEnd = nY-1;
    while (yp[yEnd] > bounds[3] && yEnd > 0)
        yEnd--;
    if (yEnd < yStart)
        return false;

    int zStart = 0;
    while (zp[zStart] < bounds[4] && zStart < nZ)
        zStart++;
    int zEnd = nZ-1;
    while (zp[zEnd] > bounds[5] && zEnd > 0)
        zEnd--;
    if (zEnd < zStart)
        return false;

    extents[0] = xStart;
    extents[1] = xEnd;
    extents[2] = yStart;
    extents[3] = yEnd;
    extents[4] = zStart;
    extents[5] = zEnd;

    return true;
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
//  Modifications:
//
//    Hank Childs, Sat Mar 18 11:03:48 PST 2006
//    Add support for rectilinear grids.
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// ****************************************************************************

void
avtPosCMFEAlgorithm::DesiredPoints::RelocatePointsUsingPartition(
                                                   SpatialPartition &spat_part)
{
#ifdef PARALLEL
    int   i, j, k;
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
    vector<int> grids(nProcs, 0);
    vector<int> total_size(nProcs, 0);
    for (i = 0 ; i < num_rgrids ; i++) 
    {
        vector<int> procId;
        vector<float> procBoundary;
        GetProcessorsForGrid(i, procId, procBoundary, spat_part);
        for (j = 0 ; j < procId.size() ; j++)
        {
            int extents[6];
            float bounds[6];
            for (k = 0 ; k < 6 ; k++)
                bounds[k] = procBoundary[6*j+k];
            bool overlaps = GetSubgridForBoundary(i, bounds, extents);
            if (!overlaps)
                continue;

            grids[procId[j]]++;
            int npts = (extents[1]-extents[0]+1)
                     + (extents[3]-extents[2]+1)
                     + (extents[5]-extents[4]+1);
            total_size[procId[j]] += npts;
        }
    }

    // 
    // Now construct the messages to send to the other processors.
    //

    // 
    // Construct the actual sizes for each message.
    //
    int *sendcount = new int[nProcs];
    int  total_msg_size = 0;
    for (j = 0 ; j < nProcs ; j++)
    {
        sendcount[j] = sizeof(int); // npts for non-rgrids;
        sendcount[j] += 3*sizeof(float)*pt_cts[j];
        sendcount[j] += sizeof(int); // num rgrids;
        sendcount[j] += 3*grids[j]*sizeof(int); // dims for each rgrid
        sendcount[j] += total_size[j]*sizeof(float); // coords for all rgrids
        total_msg_size += sendcount[j];
    }

    //
    // Now allocate the memory and set up the "sub-messages", which allow
    // us to directly access the memory based on which processor a piece
    // of data is going to.
    //
    char *big_send_msg = new char[total_msg_size];
    char **sub_ptr = new char*[nProcs];
    sub_ptr[0] = big_send_msg;
    for (i = 1 ; i < nProcs ; i++)
        sub_ptr[i] = sub_ptr[i-1] + sendcount[i-1];

    //
    // Now add the initial header info ... "how many points I have for your
    // processor".
    //
    for (j = 0 ; j < nProcs ; j++)
    {
        int numFromMeToProcJ = pt_cts[j];
        memcpy(sub_ptr[j], (void *) &numFromMeToProcJ, sizeof(int));
        sub_ptr[j] += sizeof(int);
    }

    //
    // Now add the actual points to the message.
    //
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

    //
    // Now add the initial header info for "how many rgrids I have for your
    // processor".
    //
    for (j = 0 ; j < nProcs ; j++)
    {
        int numGridsFromMeToProcJ = grids[j];
        memcpy(sub_ptr[j], (void *) &numGridsFromMeToProcJ, sizeof(int));
        sub_ptr[j] += sizeof(int);
    }

    //
    // And add the actual information about the rgrid.
    //
    for (i = 0 ; i < num_rgrids ; i++) 
    {
        vector<int> procId;
        vector<float> procBoundary;
        GetProcessorsForGrid(i, procId, procBoundary, spat_part);
        for (j = 0 ; j < procId.size() ; j++)
        {
            int extents[6];
            float bounds[6];
            for (k = 0 ; k < 6 ; k++)
                bounds[k] = procBoundary[6*j+k];
            bool overlaps = GetSubgridForBoundary(i, bounds, extents);
            if (!overlaps)
                continue;
            int proc = procId[j];
            int nX = extents[1] - extents[0] + 1;
            int nY = extents[3] - extents[2] + 1;
            int nZ = extents[5] - extents[4] + 1;
            memcpy(sub_ptr[proc], (void *) &nX, sizeof(int));
            sub_ptr[proc] += sizeof(int);
            memcpy(sub_ptr[proc], (void *) &nY, sizeof(int));
            sub_ptr[proc] += sizeof(int);
            memcpy(sub_ptr[proc], (void *) &nZ, sizeof(int));
            sub_ptr[proc] += sizeof(int);

            memcpy(sub_ptr[proc],rgrid_pts[3*i] + extents[0],sizeof(float)*nX);
            sub_ptr[proc] += sizeof(float)*nX;
            memcpy(sub_ptr[proc],rgrid_pts[3*i+1]+extents[2],sizeof(float)*nY);
            sub_ptr[proc] += sizeof(float)*nY;
            memcpy(sub_ptr[proc],rgrid_pts[3*i+2]+extents[4],sizeof(float)*nZ);
            sub_ptr[proc] += sizeof(float)*nZ;
        }
    }

    int *recvcount = new int[nProcs];
    MPI_Alltoall(sendcount, 1, MPI_INT, recvcount, 1, MPI_INT, VISIT_MPI_COMM);

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
                  VISIT_MPI_COMM);
    delete [] sendcount;
    delete [] senddisp;
    delete [] big_send_msg;

    //
    // Set up the buffers so we can read the information out.
    //
    sub_ptr[0] = big_recv_msg;
    for (i = 1 ; i < nProcs ; i++)
        sub_ptr[i] = sub_ptr[i-1] + recvcount[i-1];

    //
    // Translate the buffers we just received into the points we should look
    // at.
    //
    int numPts = 0;
    pt_list_came_from.clear();
    vector<float *> new_pt_list;
    vector<int> new_pt_list_size;
    for (j = 0 ; j < nProcs ; j++)
    {
        int numFromProcJ = 0;
        memcpy((void *) &numFromProcJ, sub_ptr[j], sizeof(int));
        sub_ptr[j] += sizeof(int);
        if (numFromProcJ == 0)
            continue;
        float *newPts = new float[3*numFromProcJ];
        memcpy(newPts, sub_ptr[j], numFromProcJ * 3 * sizeof(float));
        new_pt_list.push_back(newPts);
        new_pt_list_size.push_back(numFromProcJ);
        pt_list_came_from.push_back(j);
        sub_ptr[j] += numFromProcJ * 3 * sizeof(float);
    }

    //
    // Translate the buffers that correspond to rgrids.
    //
    vector<float *> new_rgrid_pts;
    vector<int>     new_rgrid_pts_size;
    rgrid_came_from.clear();
    for (j = 0 ; j < nProcs ; j++)
    {
        int numGridsFromProcJToMe;
        memcpy((void *) &numGridsFromProcJToMe, sub_ptr[j], sizeof(int));
        sub_ptr[j] += sizeof(int);
        for (k = 0 ; k < numGridsFromProcJToMe ; k++)
        {
            int nX, nY, nZ;
            memcpy((void *) &nX, sub_ptr[j], sizeof(int));
            sub_ptr[j] += sizeof(int);
            memcpy((void *) &nY, sub_ptr[j], sizeof(int));
            sub_ptr[j] += sizeof(int);
            memcpy((void *) &nZ, sub_ptr[j], sizeof(int));
            sub_ptr[j] += sizeof(int);

            float *x = new float[nX];
            memcpy(x, sub_ptr[j], sizeof(float)*nX);
            sub_ptr[j] += sizeof(float)*nX;
            float *y = new float[nY];
            memcpy(y, sub_ptr[j], sizeof(float)*nY);
            sub_ptr[j] += sizeof(float)*nY;
            float *z = new float[nZ];
            memcpy(z, sub_ptr[j], sizeof(float)*nZ);
            sub_ptr[j] += sizeof(float)*nZ;

            new_rgrid_pts.push_back(x);
            new_rgrid_pts.push_back(y);
            new_rgrid_pts.push_back(z);
            new_rgrid_pts_size.push_back(nX);
            new_rgrid_pts_size.push_back(nY);
            new_rgrid_pts_size.push_back(nZ);

            rgrid_came_from.push_back(j);
        }
    }

    //
    // Now take our relocated points and use them as the new "desired points."
    //
    orig_pt_list = pt_list;
    orig_pt_list_size = pt_list_size;
    pt_list = new_pt_list;
    pt_list_size = new_pt_list_size;

    orig_rgrid_pts = rgrid_pts;
    orig_rgrid_pts_size = rgrid_pts_size;
    rgrid_pts = new_rgrid_pts;
    rgrid_pts_size = new_rgrid_pts_size;

    delete [] sub_ptr;
    delete [] recvmessages;
    delete [] big_recv_msg;
    delete [] recvcount;
    delete [] recvdisp;

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
//  Modifications:
//
//    Hank Childs, Sat Mar 18 11:09:51 PST 2006
//    Add support for rectilinear grids.
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// ****************************************************************************

void
avtPosCMFEAlgorithm::DesiredPoints::UnRelocatePoints(
                                                   SpatialPartition &spat_part)
{
#ifdef PARALLEL
    int   i, j, k;
    int   nProcs = PAR_Size();

    //
    // Clean up the current points and restore the "orig" points.
    // Do this first, because it will buy us a little memory in case we're
    // close to going over.
    //
    for (i = 0 ; i < pt_list.size() ; i++)
        delete [] pt_list[i];
    for (i = 0 ; i < rgrid_pts.size() ; i++)
        delete [] rgrid_pts[i];

    //
    // We need to take the vals for our point list and send them back to the
    // processor they came from.
    //
    int *sendcount = new int[nProcs];
    for (i = 0 ; i < nProcs ; i++)
        sendcount[i] = 0;
    for (i = 0 ; i < pt_list_came_from.size() ; i++)
        sendcount[pt_list_came_from[i]]+=pt_list_size[i]*sizeof(float)*nComps;
    for (i = 0 ; i < rgrid_came_from.size() ; i++)
    {
        int npts = rgrid_pts_size[3*i] * rgrid_pts_size[3*i+1]
                 * rgrid_pts_size[3*i+2];
        sendcount[rgrid_came_from[i]] += npts*sizeof(float)*nComps;
    }

    int  totalSend = 0;
    for (i = 0 ; i < nProcs ; i++)
        totalSend += sendcount[i];

    //
    // Set up the message that contains the actual point values.
    //
    char *big_send_msg = new char[totalSend];
    char **sub_ptr = new char*[nProcs];
    sub_ptr[0] = big_send_msg;
    for (i = 1 ; i < nProcs ; i++)
        sub_ptr[i] = sub_ptr[i-1] + sendcount[i-1];

    float *vals_tmp = vals;
    for (i = 0 ; i < pt_list_came_from.size() ; i++)
    {
        int msgGoingTo = pt_list_came_from[i];
        memcpy(sub_ptr[msgGoingTo], vals_tmp, 
               pt_list_size[i]*sizeof(float)*nComps);
        sub_ptr[msgGoingTo] += pt_list_size[i]*sizeof(float)*nComps;
        vals_tmp += pt_list_size[i]*nComps;
    }
    for (i = 0 ; i < rgrid_came_from.size() ; i++)
    {
        int msgGoingTo = rgrid_came_from[i];
        int npts = rgrid_pts_size[3*i] * rgrid_pts_size[3*i+1]
                 * rgrid_pts_size[3*i+2];
        memcpy(sub_ptr[msgGoingTo], vals_tmp, npts*sizeof(float)*nComps);
        sub_ptr[msgGoingTo] += npts*sizeof(float)*nComps;
        vals_tmp += npts*nComps;
    }

    int *recvcount = new int[nProcs];
    MPI_Alltoall(sendcount, 1, MPI_INT, recvcount, 1, MPI_INT, VISIT_MPI_COMM);

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
                  VISIT_MPI_COMM);
    delete [] sendcount;
    delete [] senddisp;

    //
    // Now put our point list back in order like it was never modified for
    // parallel reasons.
    //
    pt_list = orig_pt_list;
    pt_list_size = orig_pt_list_size;
    orig_pt_list.clear();
    orig_pt_list_size.clear();
    rgrid_pts = orig_rgrid_pts;
    rgrid_pts_size = orig_rgrid_pts_size;
    orig_rgrid_pts.clear();
    orig_rgrid_pts_size.clear();
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
    for (i = 0 ; i < num_rgrids ; i++) 
    {
        int realNX = rgrid_pts_size[3*i];
        int realNY = rgrid_pts_size[3*i+1];
        int realNZ = rgrid_pts_size[3*i+2];
        int npts = realNX * realNY * realNZ;
        vector<int> procId;
        vector<float> procBoundary;
        GetProcessorsForGrid(i, procId, procBoundary, spat_part);
        for (j = 0 ; j < procId.size() ; j++)
        {
            int extents[6];
            float bounds[6];
            for (k = 0 ; k < 6 ; k++)
                bounds[k] = procBoundary[6*j+k];
            // Find out how much of the rectilinear grid overlapped with
            // the boundary for processor procId[j].  How much overlap there
            // is is how much data that processor is sending us back.
            bool overlaps = GetSubgridForBoundary(i, bounds, extents);
            if (!overlaps)
                continue;
            for (int z = extents[4] ; z <= extents[5] ; z++)
                for (int y = extents[2] ; y <= extents[3] ; y++)
                    for (int x = extents[0] ; x <= extents[1] ; x++)
                    {
                        int valIDX = z*realNX*realNY + y*realNX + x;
                        float *p = (float *) recvmessages[procId[j]];
                        for (k = 0 ; k < nComps ; k++)
                            vals[idx + nComps*valIDX + k] = *p;
                        recvmessages[procId[j]] += sizeof(float)*nComps;
                    }
        }
        idx += npts*nComps;
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

avtPosCMFEAlgorithm::FastLookupGrouping::FastLookupGrouping(std::string v,
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

avtPosCMFEAlgorithm::FastLookupGrouping::~FastLookupGrouping()
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
avtPosCMFEAlgorithm::FastLookupGrouping::AddMesh(vtkDataSet *mesh)
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
avtPosCMFEAlgorithm::FastLookupGrouping::ClearAllInputMeshes(void)
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
//  Modifications:
//    Kathleen Bonnell, Mon Aug 14 16:40:30 PDT 2006
//    API change for avtIntervalTree.
//
// ****************************************************************************

void
avtPosCMFEAlgorithm::FastLookupGrouping::Finalize(void)
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
            double bounds[6];
            cell->GetBounds(bounds);
            itree->AddElement(index, bounds);

            map_to_ds[index] = i;
            index++;
        }
    }
    if (degenerate)
    {
        double bounds[6] = { 0, 1, 0, 1, 0, 1 };
        itree->AddElement(0, bounds);
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
//      This method is actually a thin layer on top of GetValueUsingList.
//      It calls that method using the last successful list and then, if
//      necessary, using a list that comes from the interval tree.
//
//  Programmer: Hank Childs
//  Creation:   March 18, 2006
//
//  Modifications:
//    Kathleen Bonnell, Mon Aug 14 16:40:30 PDT 2006
//    API change for avtIntervalTree.
//
// ****************************************************************************

bool
avtPosCMFEAlgorithm::FastLookupGrouping::GetValue(const float *pt, float *val)
{
    //
    // Start off by using the list from the previous search.  Searching the
    // interval tree is so costly that this is a worthwhile "guess".
    //
    if (list_from_last_successful_search.size() > 0)
    {
        bool v = GetValueUsingList(list_from_last_successful_search, pt, val);
        if (v)
            return true;
    }

    //
    // OK, we struck out with the list from the last winning search.  So
    // get the correct list from the interval tree.
    //
    vector<int> list;
    double dpt[3] = {pt[0], pt[1] , pt[2]};
    itree->GetElementsListFromRange(dpt, dpt, list);
    bool v = GetValueUsingList(list, pt, val);
    if (v == true)
        list_from_last_successful_search = list;
    else
        list_from_last_successful_search.clear();
    return v;
}


// ****************************************************************************
//  Method: FastLookupGrouping::GetValueUsingList
//
//  Purpose:
//      Evaluates the value at a position.  Does this for the grouping of
//      meshes its been given and does it with fast lookups.
//
//  Programmer: Hank Childs
//  Creation:   October 11, 2005
//
//  Modifications:
//
//    Hank Childs, Sat Mar 18 14:39:50 PST 2006
//    Use the vtkVisItUtility method to see if a cell contains a point
//    (it's faster).
//
// ****************************************************************************

bool
avtPosCMFEAlgorithm::FastLookupGrouping::GetValueUsingList(vector<int> &list,
                                                   const float *pt, float *val)
{
    double closestPt[3];
    int subId;
    double pcoords[3];
    double dist2;
    double weights[100]; // MUST BE BIGGER THAN NPTS IN A CELL (ie 8).
    double non_const_pt[3];
    non_const_pt[0] = pt[0];
    non_const_pt[1] = pt[1];
    non_const_pt[2] = pt[2];

    for (int j = 0 ; j < list.size() ; j++)
    {
        int mesh = map_to_ds[list[j]];
        int index = list[j] - ds_start[mesh];
        vtkCell *cell = meshes[mesh]->GetCell(index);
        bool inCell = vtkVisItUtility::CellContainsPoint(cell, non_const_pt);
        if (!inCell)
            continue;

        if (isNodal)
        {
            // Need the weights.
            cell->EvaluatePosition(non_const_pt, closestPt, subId,
                                   pcoords, dist2, weights);

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
//  Modifications:
//    Kathleen Bonnell, Tue May 16 09:41:46 PDT 2006
//    Changed GetNumberOfInputs to GetTotalNumberOfInputConnections, due to
//    VTK api changes.  Removed call to SetSource(NULL) as it also removes
//    necessary information from the dataset.
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// ****************************************************************************

void
avtPosCMFEAlgorithm::FastLookupGrouping::RelocateDataUsingPartition(
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
        if (appenders[j]->GetTotalNumberOfInputConnections() == 0)
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
    MPI_Alltoall(sendcount, 1, MPI_INT, recvcount, 1, MPI_INT, VISIT_MPI_COMM);

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
                  VISIT_MPI_COMM);
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
        // using SetSource(NULL) on vtkDataSets is no longer a good idea.
        //ugrid->SetSource(NULL);
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

avtPosCMFEAlgorithm::SpatialPartition::SpatialPartition()
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

avtPosCMFEAlgorithm::SpatialPartition::~SpatialPartition()
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
//  Modifications:
//
//    Hank Childs, Sat Mar 18 10:15:23 PST 2006
//    Add support for rectilinear grids.
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
     void             AddRGrid(const float *, const float *, const float *,
                               int, int, int);
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
//  Method: Boundary::AddPoint
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
//  Method: Boundary::AddRGrid
//
//  Purpose:
//      Adds an rgrid to the boundary.
//
//  Programmer: Hank Childs
//  Creation:   March 18, 2006
// 
// ****************************************************************************

void
Boundary::AddRGrid(const float *x, const float *y, const float *z, int nX,
                   int nY, int nZ)
{
    //
    // Start by narrowing the total rgrid down to just the portion that is
    // relevant to this boundary.
    //
    int xStart = 0;
    while (x[xStart] < bounds[0] && xStart < nX)
        xStart++;
    int xEnd = nX-1;
    while (x[xEnd] > bounds[1] && xEnd > 0)
        xEnd--;
    int yStart = 0;
    while (y[yStart] < bounds[2] && yStart < nY)
        yStart++;
    int yEnd = nY-1;
    while (y[yEnd] > bounds[3] && yEnd > 0)
        yEnd--;
    int zStart = 0;
    while (z[zStart] < bounds[4] && zStart < nZ)
        zStart++;
    int zEnd = nZ-1;
    while (z[zEnd] > bounds[5] && zEnd > 0)
        zEnd--;

    const float *arr  = NULL;
    int          arrStart = 0;
    int          arrEnd   = 0;
    int          slab     = 0;

    switch (axis)
    {
      case X_AXIS:
        arr  = x;
        arrStart = xStart;
        arrEnd   = xEnd;
        slab = (yEnd-yStart+1)*(zEnd-zStart+1);
        break;
      case Y_AXIS:
        arr  = y;
        arrStart = yStart;
        arrEnd   = yEnd;
        slab = (xEnd-xStart+1)*(zEnd-zStart+1);
        break;
      case Z_AXIS:
        arr  = z;
        arrStart = zStart;
        arrEnd   = zEnd;
        slab = (xEnd-xStart+1)*(yEnd-yStart+1);
        break;
    }

    int curIdx = arrStart;
    for (int i = 0 ; i < npivots ; i++)
        while (curIdx <= arrEnd && arr[curIdx] < pivots[i])
        {
            curIdx++;
            numCells[i] += slab;
        }
    while (curIdx <= arrEnd)
    {
        curIdx++;
        numCells[npivots] += slab;
    }
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
//  Modification:
//
//    Hank Childs, Fri Mar 10 14:35:32 PST 2006
//    Add fix for parallel engines of 1 processor.
//
//    Hank Childs, Sat Mar 18 10:15:23 PST 2006
//    Add support for rectilinear meshes.
//
//    Kathleen Bonnell, Mon Aug 14 16:40:30 PDT 2006
//    API change for avtIntervalTree.
//
// ****************************************************************************

void
avtPosCMFEAlgorithm::SpatialPartition::CreatePartition(DesiredPoints &dp,
                                       FastLookupGrouping &flg, double *bounds)
{
    int   i, j, k;
    int t0 = visitTimer->StartTimer();

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
    bool keepGoing = (nProcs > 1);
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
            float *b = b_list[i]->GetBoundary();
            double db[6] = {b[0], b[1], b[2], b[3], b[4], b[5]};
            it.AddElement(nBins, db);
            nBins++;
        }
        it.Calculate(true);

        // Now add each point to the boundary it falls in.  Start by doing
        // the points that come from unstructured or structured meshes.
        const int nPoints = dp.GetRGridStart();
        vector<int> list;
        float pt[3];
        for (i = 0 ; i < nPoints ; i++)
        {
            dp.GetPoint(i, pt);
            double dpt[3] = {pt[0], pt[1], pt[2]};
            it.GetElementsListFromRange(dpt, dpt, list);
            for (j = 0 ; j < list.size() ; j++)
            {
                Boundary *b = b_list[bin_lookup[list[j]]];
                b->AddPoint(pt);
            }
        }

        // Now do the points that come from rectlinear meshes.
        int num_rgrid = dp.GetNumberOfRGrids();
        for (i = 0 ; i < num_rgrid ; i++)
        {
            const float *x, *y, *z;
            int          nX, nY, nZ;
            dp.GetRGrid(i, x, y, z, nX, nY, nZ);
            double min[3];
            min[0] = x[0];
            min[1] = y[0];
            min[2] = z[0];
            double max[3];
            max[0] = x[nX-1];
            max[1] = y[nY-1];
            max[2] = z[nY-1];
            it.GetElementsListFromRange(min, max, list);
            for (j = 0 ; j < list.size() ; j++)
            {
                Boundary *b = b_list[bin_lookup[list[j]]];
                b->AddRGrid(x, y, z, nX, nY, nZ);
            }
        }

        // Now do the cells.  We are using the cell centers, which is a decent
        // approximation.
        vector<vtkDataSet *> meshes = flg.GetMeshes();
        for (i = 0 ; i < meshes.size() ; i++)
        {
            const int ncells = meshes[i]->GetNumberOfCells();
            double bbox[6];
            double pt[3];
            for (j = 0 ; j < ncells ; j++)
            {
                vtkCell *cell = meshes[i]->GetCell(j);
                cell->GetBounds(bbox);
                pt[0] = (bbox[0] + bbox[1]) / 2.;
                pt[1] = (bbox[2] + bbox[3]) / 2.;
                pt[2] = (bbox[4] + bbox[5]) / 2.;
                it.GetElementsListFromRange(pt, pt, list);
                float fpt[3] = {pt[0], pt[1], pt[2]};
                for (k = 0 ; k < list.size() ; k++)
                {
                    Boundary *b = b_list[bin_lookup[list[k]]];
                    b->AddPoint(fpt);
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
            double db[6] = {b[0], b[1], b[2], b[3], b[4], b[5]};
            itree->AddElement(count++, db);
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
            double dpt[3] = {(double)pt[0], (double)pt[1], (double)pt[2]};
            itree->GetElementsListFromRange(dpt, dpt, list);
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

    visitTimer->StopTimer(t0, "Creating spatial partition");
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
//  Modifications:
//    Kathleen Bonnell, Mon Aug 14 16:40:30 PDT 2006
//    API change for avtIntervalTree.
//
// ****************************************************************************

int
avtPosCMFEAlgorithm::SpatialPartition::GetProcessor(float *pt)
{
    vector<int> list;

    double dpt[3] = {(double)pt[0], (double)pt[1],(double) pt[2]};
    itree->GetElementsListFromRange(dpt, dpt, list);
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
//  Modifications:
//    Kathleen Bonnell, Mon Aug 14 16:40:30 PDT 2006
//    API change for avtIntervalTree.
//
// ****************************************************************************

int
avtPosCMFEAlgorithm::SpatialPartition::GetProcessor(vtkCell *cell)
{
    double bounds[6];
    cell->GetBounds(bounds);
    double mins[3];
    mins[0] = bounds[0];
    mins[1] = bounds[2];
    mins[2] = bounds[4];
    double maxs[3];
    maxs[0] = bounds[1];
    maxs[1] = bounds[3];
    maxs[2] = bounds[5];

    vector<int> list;
    itree->GetElementsListFromRange(mins, maxs, list);
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
//  Modifications:
//    Kathleen Bonnell, Mon Aug 14 16:40:30 PDT 2006
//    API change for avtIntervalTree.
//
// ****************************************************************************

void
avtPosCMFEAlgorithm::SpatialPartition::GetProcessorList(vtkCell *cell,
                                                        std::vector<int> &list)
{
    list.clear();

    double bounds[6];
    cell->GetBounds(bounds);
    double mins[3];
    mins[0] = bounds[0];
    mins[1] = bounds[2];
    mins[2] = bounds[4];
    double maxs[3];
    maxs[0] = bounds[1];
    maxs[1] = bounds[3];
    maxs[2] = bounds[5];

    itree->GetElementsListFromRange(mins, maxs, list);
}


// ****************************************************************************
//  Method: SpatialPartition::GetProcessorBoundaries
//
//  Purpose:
//      Gets the processor that contains this cell.  This should be called
//      when a list of processors contain a cell.
//
//  Programmer: Hank Childs
//  Creation:   March 19, 2006
//
//  Modifications:
//    Kathleen Bonnell, Mon Aug 14 16:40:30 PDT 2006
//    API change for avtIntervalTree.
//
// ****************************************************************************

void
avtPosCMFEAlgorithm::SpatialPartition::GetProcessorBoundaries(float *bounds,
                                std::vector<int> &list, std::vector<float> &db)
{
    list.clear();

    double mins[3];
    mins[0] = bounds[0];
    mins[1] = bounds[2];
    mins[2] = bounds[4];
    double maxs[3];
    maxs[0] = bounds[1];
    maxs[1] = bounds[3];
    maxs[2] = bounds[5];

    itree->GetElementsListFromRange(mins, maxs, list);

    int numMatches = list.size();
    db.resize(numMatches*6);
    for (int i = 0 ; i < numMatches ; i++)
    {
        double domBounds[6];
        itree->GetElementExtents(list[i], domBounds);
        for (int j = 0 ; j < 6 ; j++)
            db[6*i+j] = domBounds[j];
    }
}


