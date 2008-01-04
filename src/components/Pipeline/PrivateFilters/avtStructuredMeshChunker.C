/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                          avtStructuredMeshChunker.C                       //
// ************************************************************************* //

#include <avtStructuredMeshChunker.h>

#include <vtkCellData.h>
#include <vtkCellType.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnstructuredGrid.h>

#include <avtSweepPlanePartitionStrategy.h>
#include <avtMultiResolutionPartitionStrategy.h>

#include <ImproperUseException.h>
#include <TimingsManager.h>


using std::vector;


// ****************************************************************************
//  Method: ChunkStructuredMesh
//
//  Purpose:
//      Determines which mesh type we have and calls the appropriate 
//      sub-routine for it.
//
//  Programmer: Hank Childs
//  Creation:   March 19, 2005
//
// ****************************************************************************

void
avtStructuredMeshChunker::ChunkStructuredMesh(vtkDataSet *sgrid,
          vector<ZoneDesignation> &designation, vector<vtkDataSet *> &outGrids,
          vtkUnstructuredGrid *&outUgrid, avtGhostDataType ghost_type,
          bool haveOptimizations)
{
    int dstype = sgrid->GetDataObjectType();
    if (dstype == VTK_RECTILINEAR_GRID)
    {
        vtkRectilinearGrid *rgrid = (vtkRectilinearGrid *) sgrid;
        avtStructuredMeshChunker::ChunkRectilinearMesh(rgrid, designation,
                            outGrids, outUgrid, ghost_type, haveOptimizations);
    }
    else if (dstype == VTK_STRUCTURED_GRID)
    {
        vtkStructuredGrid *cgrid = (vtkStructuredGrid *) sgrid;
        avtStructuredMeshChunker::ChunkCurvilinearMesh(cgrid, designation,
                            outGrids, outUgrid, ghost_type, haveOptimizations);
    }
    else
    {
        EXCEPTION0(ImproperUseException);
    }
}


// ****************************************************************************
//  Method: ChunkRectilinearMesh
//
//  Purpose:
//      Chunks a rectilinear mesh into sub-rectilinear meshes based on the
//      input zone designation.
//
//  Programmer: Hank Childs
//  Creation:   September 18, 2004
//
//  Modifications:
//
//    Jeremy Meredith, Tue Apr 19 12:10:35 PDT 2005
//    xlC was choking on multiple uses of the same variable name that g++
//    accepted.  I disambiguated them for it.
//
//    Hank Childs, Sun Apr  3 13:20:48 PDT 2005
//    Allow for full grids to be copies of the input.  Note: change was made
//    before above changes, but merged after.
//
// ****************************************************************************

void
avtStructuredMeshChunker::ChunkRectilinearMesh(vtkRectilinearGrid *rgrid,
      vector<ZoneDesignation> &designation, 
      vector<vtkDataSet *> &outGrids, vtkUnstructuredGrid *&outUgrid, 
      avtGhostDataType ghost_type, bool haveOptimizations)
{
    int   i, j, k;

    int dims[3];
    rgrid->GetDimensions(dims);

    //
    // Let a subroutine set up the chunking.
    //
    vector<MeshDescription>  outDescr;
    ZoneDesignation *d_plus = SplitIntoSubgrids(dims, rgrid, designation,
                                                ghost_type, outDescr, 
                                                outUgrid, haveOptimizations);
    
    //
    // Now create the output subgrids.
    //
    int nGrids = outDescr.size();
    vtkPointData *origPD = rgrid->GetPointData();
    vtkCellData *origCD = rgrid->GetCellData();

    vtkDataArray *origX = rgrid->GetXCoordinates();
    vtkDataArray *origY = rgrid->GetYCoordinates();
    vtkDataArray *origZ = rgrid->GetZCoordinates();
    for (int gridI = 0 ; gridI < nGrids ; gridI++)
    {
        MeshDescription &desc = outDescr[gridI];

        // See if we can take the grid whole-sale.
        if (desc.start_index[0] == 0 && desc.start_index[1] == 0 &&
            desc.start_index[2] == 0 && desc.index_size[0] == dims[0] &&
            desc.index_size[1] == dims[1] && desc.index_size[2] == dims[2])
        {
            rgrid->Register(NULL);
            outGrids.push_back(rgrid);
            continue;
        }

        vtkRectilinearGrid *grid = vtkRectilinearGrid::New();
        grid->SetDimensions(desc.index_size);

        vtkFloatArray *newX = vtkFloatArray::New();
        int numX = desc.index_size[0];
        int startX = desc.start_index[0];
        newX->SetNumberOfTuples(numX);
        for (i = 0 ; i < numX ; i++)
        {
            newX->SetValue(i, origX->GetTuple1(startX+i));
        }
        grid->SetXCoordinates(newX);
        newX->Delete();
        
        vtkFloatArray *newY = vtkFloatArray::New();
        int numY = desc.index_size[1];
        int startY = desc.start_index[1];
        newY->SetNumberOfTuples(numY);
        for (i = 0 ; i < numY ; i++)
        {
            newY->SetValue(i, origY->GetTuple1(startY+i));
        }
        grid->SetYCoordinates(newY);
        newY->Delete();
        
        vtkFloatArray *newZ = vtkFloatArray::New();
        int numZ = desc.index_size[2];
        int startZ = desc.start_index[2];
        newZ->SetNumberOfTuples(numZ);
        for (i = 0 ; i < numZ ; i++)
        {
            newZ->SetValue(i, origZ->GetTuple1(startZ+i));
        }
        grid->SetZCoordinates(newZ);
        newZ->Delete();
        
        //
        // Get some temporary variables ready to copy over the data.
        //
        int npts = desc.NumPoints();
        int ncells = desc.NumCells();
        vtkPointData *newPD = grid->GetPointData();
        bool copyPointData = (origPD->GetNumberOfArrays() > 0);
        if (copyPointData)
            newPD->CopyAllocate(origPD, npts);
        vtkCellData *newCD = grid->GetCellData();
        bool copyCellData = (origCD->GetNumberOfArrays() > 0);
        if (copyCellData)
            newCD->CopyAllocate(origCD, ncells);

        //
        // Copy over the point data.
        //
        if (copyPointData)
        {
            for (k = 0 ; k < desc.index_size[2] ; k++)
            {
                int oldKBase = (k+desc.start_index[2])*(dims[1]*dims[0]);
                int newKBase = (k)*(desc.index_size[1]*desc.index_size[0]);
                for (j = 0 ; j < desc.index_size[1] ; j++)
                {
                    int oldJBase = (j+desc.start_index[1])*(dims[0]);
                    int newJBase = (j)*(desc.index_size[0]);
                    for (i = 0 ; i < desc.index_size[0] ; i++)
                    {
                        int oldIBase = (i+desc.start_index[0]);
                        int newIBase = (i);
    
                        int oldIndex = oldKBase + oldJBase + oldIBase;
                        int newIndex = newKBase + newJBase + newIBase;
     
                        newPD->CopyData(origPD, oldIndex, newIndex);
                    }
                }
            }
        }

        //
        // Copy over the cell data.
        //
        if (copyCellData)
        {
            int cell_dim[3];
            cell_dim[0] = (dims[0] > 1 ? dims[0]-1 : 1);
            cell_dim[1] = (dims[1] > 1 ? dims[1]-1 : 1);
            cell_dim[2] = (dims[2] > 1 ? dims[2]-1 : 1);
            for (k = 0 ; k < desc.index_size[2]-1 ; k++)
            {
                int oldKBase=(k+desc.start_index[2])*(cell_dim[1]*cell_dim[0]);
                int newKBase = (k)*((desc.index_size[1]-1)*(desc.index_size[0]-1));
                for (j = 0 ; j < desc.index_size[1]-1 ; j++)
                {
                    int oldJBase = (j+desc.start_index[1])*(cell_dim[0]);
                    int newJBase = (j)*(desc.index_size[0]-1);
                    for (i = 0 ; i < desc.index_size[0]-1 ; i++)
                    {
                        int oldIBase = (i+desc.start_index[0]);
                        int newIBase = (i);
    
                        int oldIndex = oldKBase + oldJBase + oldIBase;
                        int newIndex = newKBase + newJBase + newIBase;
     
                        newCD->CopyData(origCD, oldIndex, newIndex);
                    }
                }
            }
        }
 
        //
        // Let a convenience routine take care of creating the ghost data.
        //
        CreateGhostData(desc, dims, ghost_type, d_plus, grid->GetPointData(),
                        grid->GetCellData());
        outGrids.push_back(grid);
    }

    delete [] d_plus;
}


// ****************************************************************************
//  Method: ChunkCurvilinearMesh
//
//  Purpose:
//      Chunks a curvilinear mesh into sub-curvilinear meshes based on the
//      input zone designation.
//
//  Programmer: Hank Childs
//  Creation:   September 18, 2004
//
//  Modifications:
//
//    Jeremy Meredith, Tue Apr 19 12:10:35 PDT 2005
//    xlC was choking on multiple uses of the same variable name that g++
//    accepted.  I disambiguated them for it.
//
//    Hank Childs, Sun Apr  3 13:20:48 PDT 2005
//    Allow for full grids to be copies of the input.  Note: change was made
//    before above changes, but merged after.
//
// ****************************************************************************

void
avtStructuredMeshChunker::ChunkCurvilinearMesh(vtkStructuredGrid *sgrid,
      vector<ZoneDesignation> &designation, 
      vector<vtkDataSet *> &outGrids, vtkUnstructuredGrid *&outUgrid,
      avtGhostDataType ghost_type, bool haveOptimizations)
{
    int   i, j, k;

    int dims[3];
    sgrid->GetDimensions(dims);

    //
    // Let a subroutine set up the chunking.
    //
    vector<MeshDescription>  outDescr;
    ZoneDesignation *d_plus = SplitIntoSubgrids(dims, sgrid, designation,
                                                ghost_type, outDescr, 
                                                outUgrid, haveOptimizations);
    
    //
    // Now create the output subgrids.
    //
    vtkPointData *origPD = sgrid->GetPointData();
    vtkCellData *origCD = sgrid->GetCellData();
    float *orig_pts = (float *) sgrid->GetPoints()->GetVoidPointer(0);
    int nGrids = outDescr.size();
    for (int gridI = 0 ; gridI < nGrids ; gridI++)
    {
        MeshDescription &desc = outDescr[gridI];

        // See if we can take the grid whole-sale.
        if (desc.start_index[0] == 0 && desc.start_index[1] == 0 &&
            desc.start_index[2] == 0 && desc.index_size[0] == dims[0] &&
            desc.index_size[1] == dims[1] && desc.index_size[2] == dims[2])
        {
            sgrid->Register(NULL);
            outGrids.push_back(sgrid);
            continue;
        }

        vtkStructuredGrid *grid = vtkStructuredGrid::New();
        grid->SetDimensions(desc.index_size);

        vtkPoints *pts = vtkPoints::New();
        grid->SetPoints(pts);
        pts->Delete();
        int npts = desc.NumPoints();
        int ncells = desc.NumCells();
        pts->SetNumberOfPoints(npts);
        float *new_pts = (float *) pts->GetVoidPointer(0);

        vtkPointData *newPD = grid->GetPointData();
        bool copyPointData = (origPD->GetNumberOfArrays() > 0);
        if (copyPointData)
            newPD->CopyAllocate(origPD, npts);
        vtkCellData *newCD = grid->GetCellData();
        bool copyCellData = (origCD->GetNumberOfArrays() > 0);
        if (copyCellData)
            newCD->CopyAllocate(origCD, ncells);

        //
        // Copy over the point data and the points themselves.
        //
        for (k = 0 ; k < desc.index_size[2] ; k++)
        {
            int oldKBase = (k+desc.start_index[2])*(dims[1]*dims[0]);
            int newKBase = (k)*(desc.index_size[1]*desc.index_size[0]);
            for (j = 0 ; j < desc.index_size[1] ; j++)
            {
                int oldJBase = (j+desc.start_index[1])*(dims[0]);
                int newJBase = (j)*(desc.index_size[0]);
                for (i = 0 ; i < desc.index_size[0] ; i++)
                {
                    int oldIBase = (i+desc.start_index[0]);
                    int newIBase = (i);

                    int oldIndex = oldKBase + oldJBase + oldIBase;
                    int newIndex = newKBase + newJBase + newIBase;
 
                    new_pts[3*newIndex]   = orig_pts[3*newIndex];
                    new_pts[3*newIndex+1] = orig_pts[3*newIndex+1];
                    new_pts[3*newIndex+2] = orig_pts[3*newIndex+2];

                    if (copyPointData)
                        newPD->CopyData(origPD, oldIndex, newIndex);
                }
            }
        }

        //
        // Copy over the cell data.
        //
        if (copyCellData)
        {
            int cell_dim[3];
            cell_dim[0] = (dims[0] > 1 ? dims[0]-1 : 1);
            cell_dim[1] = (dims[1] > 1 ? dims[1]-1 : 1);
            cell_dim[2] = (dims[2] > 1 ? dims[2]-1 : 1);
            for (k = 0 ; k < desc.index_size[2]-1 ; k++)
            {
                int oldKBase=(k+desc.start_index[2])*(cell_dim[1]*cell_dim[0]);
                int newKBase = (k)*((desc.index_size[1]-1)*(desc.index_size[0]-1));
                for (j = 0 ; j < desc.index_size[1]-1 ; j++)
                {
                    int oldJBase = (j+desc.start_index[1])*(cell_dim[0]);
                    int newJBase = (j)*(desc.index_size[0]-1);
                    for (i = 0 ; i < desc.index_size[0]-1 ; i++)
                    {
                        int oldIBase = (i+desc.start_index[0]);
                        int newIBase = (i);
    
                        int oldIndex = oldKBase + oldJBase + oldIBase;
                        int newIndex = newKBase + newJBase + newIBase;
     
                        newCD->CopyData(origCD, oldIndex, newIndex);
                    }
                }
            }
        }
 
        //
        // Let a convenience routine take care of creating the ghost data.
        //
        CreateGhostData(desc, dims, ghost_type, d_plus, grid->GetPointData(),
                        grid->GetCellData());
        outGrids.push_back(grid);
    }

    delete [] d_plus;
}


// ****************************************************************************
//  Method: avtStructuredMeshChunker::NodeIsGhost
//
//  Purpose:
//      Determines if a node is a ghost node based on whether the surrounding
//      zones are retained, discarded, etc.
//
//  Programmer: Hank Childs
//  Creation:   November 13, 2004
//
// ****************************************************************************

bool
avtStructuredMeshChunker::NodeIsGhost(int i, int j, int k, const int *dims,
                                      ZoneDesignation *d_plus, bool onEdge)
{
    if (!onEdge)
    {
        if (i == 0 || i == dims[0]-1)
            return false;
        if (j == 0 || j == dims[1]-1)
            return false;
        if (k == 0 || k == dims[2]-1)
            return false;
    }

    int cell_dims[3];
    cell_dims[0] = (dims[0] > 1 ? dims[0]-1 : 1);
    cell_dims[1] = (dims[1] > 1 ? dims[1]-1 : 1);
    cell_dims[2] = (dims[2] > 1 ? dims[2]-1 : 1);

    int minI = (i-1 < 0 ? 0 : i-1);
    int minJ = (j-1 < 0 ? 0 : j-1);
    int minK = (k-1 < 0 ? 0 : k-1);
    int maxI = (i < dims[0]-1 ? i : dims[0]-1);
    int maxJ = (j < dims[1]-1 ? j : dims[1]-1);
    int maxK = (k < dims[2]-1 ? k : dims[2]-1);
    for (int I = minI ; I <= maxI ; I++)
        for (int J = minJ ; J <= maxJ ; J++)
            for (int K = minK ; K <= maxK ; K++)
            {
                int idx = K*(cell_dims[0]*cell_dims[1]) + J*cell_dims[0] + I;
                if (d_plus[idx] == DISCARD)
                {
                    return false;
                }
            }

    return true;
}


// ****************************************************************************
//  Method: avtStructuredMeshChunker::CreateGhostData
//
//  Purpose:
//      Creates ghost data -- ghost zones or ghost nodes -- for the grid.
//
//  Programmer: Hank Childs
//  Creation:   September 18, 2004
//
// ****************************************************************************

void
avtStructuredMeshChunker::CreateGhostData(MeshDescription &desc, 
                    const int *whole_dims, avtGhostDataType ghost_type,
                    ZoneDesignation *d_plus, vtkPointData *pd, vtkCellData *cd)
{
    int  i, j, k;

    if (ghost_type == GHOST_NODE_DATA)
    {
        vtkUnsignedCharArray *arr = (vtkUnsignedCharArray *) 
                                                 pd->GetArray("avtGhostNodes");
        if (arr == NULL)
        {
            arr = vtkUnsignedCharArray::New();
            arr->SetName("avtGhostNodes");
            int npts = desc.NumPoints();
            arr->SetNumberOfTuples(npts);
            unsigned char *vals = arr->GetPointer(0);
            for (i = 0 ; i < npts ; i++)
                vals[i] = 0;
            pd->AddArray(arr);
            arr->Delete();
        }

        unsigned char *vals = arr->GetPointer(0);
        int dims[3];
        dims[0] = desc.index_size[0];
        dims[1] = desc.index_size[1];
        dims[2] = desc.index_size[2];

        // i = 0 face.
        for (k = 0 ; k < dims[2] ; k++)
        {
            int kBase = k*dims[1]*dims[0];
            for (j = 0 ; j < dims[1] ; j++)
            {
                int idx = kBase + j*dims[0] + 0;
                bool onEdge = (k == 0 || k == dims[2]-1 
                            || j == 0 || j == dims[1]-1);
                if (NodeIsGhost(desc.start_index[0], desc.start_index[1]+j, 
                         desc.start_index[2]+k, whole_dims, d_plus, onEdge))
                    avtGhostData::AddGhostNodeType(vals[idx], DUPLICATED_NODE);
            }
        }

        // i = dims[0]-1 face.
        for (k = 0 ; k < dims[2] ; k++)
        {
            int kBase = k*dims[1]*dims[0];
            for (j = 0 ; j < dims[1] ; j++)
            {
                int idx = kBase + j*dims[0] + dims[0]-1;
                bool onEdge = (k == 0 || k == dims[2]-1 
                            || j == 0 || j == dims[1]-1);
                if (NodeIsGhost(desc.start_index[0]+dims[0]-1, 
                                desc.start_index[1]+j, 
                                desc.start_index[2]+k, whole_dims,
                                d_plus, onEdge))
                    avtGhostData::AddGhostNodeType(vals[idx], DUPLICATED_NODE);
            }
        }

        // j = 0 face.
        int jBase = 0*dims[0];
        for (k = 0 ; k < dims[2] ; k++)
        {
            int kBase = k*dims[1]*dims[0];
            for (i = 0 ; i < dims[0] ; i++)
            {
                int idx = kBase + jBase + i;
                bool onEdge = (k == 0 || k == dims[2]-1 
                            || i == 0 || i == dims[0]-1);
                if (NodeIsGhost(desc.start_index[0]+i,
                                desc.start_index[1],
                                desc.start_index[2]+k, whole_dims, 
                                d_plus, onEdge))
                    avtGhostData::AddGhostNodeType(vals[idx], DUPLICATED_NODE);
            }
        }

        // j = dims[1]-1 face.
        jBase = (dims[1]-1)*dims[0];
        for (k = 0 ; k < dims[2] ; k++)
        {
            int kBase = k*dims[1]*dims[0];
            for (i = 0 ; i < dims[0] ; i++)
            {
                int idx = kBase + jBase + i;
                bool onEdge = (k == 0 || k == dims[2]-1 
                            || i == 0 || i == dims[0]-1);
                if (NodeIsGhost(desc.start_index[0]+i,
                                desc.start_index[1]+dims[1]-1,
                                desc.start_index[2]+k, whole_dims,
                                d_plus, onEdge))
                    avtGhostData::AddGhostNodeType(vals[idx], DUPLICATED_NODE);
            }
        }

        // k = 0 face.
        int kBase = 0*dims[1]*dims[0];
        for (j = 0 ; j < dims[1] ; j++)
        {
            int jBase = j*dims[0];
            for (i = 0 ; i < dims[0] ; i++)
            {
                int idx = kBase + jBase + i;
                bool onEdge = (j == 0 || j == dims[1]-1 
                            || i == 0 || i == dims[0]-1);
                if (NodeIsGhost(desc.start_index[0]+i,
                                desc.start_index[1]+j,
                                desc.start_index[2], whole_dims,
                                d_plus, onEdge))
                    avtGhostData::AddGhostNodeType(vals[idx], DUPLICATED_NODE);
            }
        }

        // k = dims[2]-1 face.
        kBase = (dims[2]-1)*dims[1]*dims[0];
        for (j = 0 ; j < dims[1] ; j++)
        {
            int jBase = j*dims[0];
            for (i = 0 ; i < dims[0] ; i++)
            {
                int idx = kBase + jBase + i;
                bool onEdge = (j == 0 || j == dims[1]-1 
                            || i == 0 || i == dims[0]-1);
                if (NodeIsGhost(desc.start_index[0]+i,
                                desc.start_index[1]+j,
                                desc.start_index[2]+dims[2]-1, whole_dims,
                                d_plus, onEdge))
                    avtGhostData::AddGhostNodeType(vals[idx], DUPLICATED_NODE);
            }
        }
    }
    else if (ghost_type == GHOST_ZONE_DATA)
    {
        vtkUnsignedCharArray *arr = (vtkUnsignedCharArray *) 
                                                 cd->GetArray("avtGhostZones");
        if (arr == NULL)
        {
            arr = vtkUnsignedCharArray::New();
            arr->SetName("avtGhostZones");
            int ncells = desc.NumCells();
            arr->SetNumberOfTuples(ncells);
            unsigned char *vals = arr->GetPointer(0);
            for (i = 0 ; i < ncells ; i++)
                vals[i] = 0;
            cd->AddArray(arr);
            arr->Delete();
        }

        unsigned char *vals = arr->GetPointer(0);
        int dims[3];
        dims[0] = desc.index_size[0];
        dims[0] = (dims[0] > 1 ? dims[0]-1 : 1);
        dims[1] = desc.index_size[1];
        dims[1] = (dims[1] > 1 ? dims[1]-1 : 1);
        dims[2] = desc.index_size[2];
        dims[2] = (dims[2] > 1 ? dims[2]-1 : 1);

        if (desc.lowIIsGhost)
        {
            for (k = 0 ; k < dims[2] ; k++)
            {
                int kBase = k*dims[1]*dims[0];
                for (j = 0 ; j < dims[1] ; j++)
                {
                    int idx = kBase + j*dims[0] + 0;
                    avtGhostData::AddGhostZoneType(vals[idx], 
                                          DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
                }
            }
        }
        if (desc.hiIIsGhost)
        {
            for (k = 0 ; k < dims[2] ; k++)
            {
                int kBase = k*dims[1]*dims[0];
                for (j = 0 ; j < dims[1] ; j++)
                {
                    int idx = kBase + j*dims[0] + dims[0]-1;
                    avtGhostData::AddGhostZoneType(vals[idx], 
                                          DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
                }
            }
        }

        if (desc.lowJIsGhost)
        {
            int jBase = 0*dims[0];
            for (k = 0 ; k < dims[2] ; k++)
            {
                int kBase = k*dims[1]*dims[0];
                for (i = 0 ; i < dims[0] ; i++)
                {
                    int idx = kBase + jBase + i;
                    avtGhostData::AddGhostZoneType(vals[idx], 
                                          DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
                }
            }
        }
        if (desc.hiJIsGhost)
        {
            int jBase = (dims[1]-1)*dims[0];
            for (k = 0 ; k < dims[2] ; k++)
            {
                int kBase = k*dims[1]*dims[0];
                for (i = 0 ; i < dims[0] ; i++)
                {
                    int idx = kBase + jBase + i;
                    avtGhostData::AddGhostZoneType(vals[idx], 
                                          DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
                }
            }
        }

        if (desc.lowKIsGhost)
        {
            int kBase = 0*dims[1]*dims[0];
            for (j = 0 ; j < dims[1] ; j++)
            {
                int jBase = j*dims[0];
                for (i = 0 ; i < dims[0] ; i++)
                {
                    int idx = kBase + jBase + i;
                    avtGhostData::AddGhostZoneType(vals[idx], 
                                          DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
                }
            }
        }
        if (desc.hiKIsGhost)
        {
            int kBase = (dims[2]-1)*dims[1]*dims[0];
            for (j = 0 ; j < dims[1] ; j++)
            {
                int jBase = j*dims[0];
                for (i = 0 ; i < dims[0] ; i++)
                {
                    int idx = kBase + jBase + i;
                    avtGhostData::AddGhostZoneType(vals[idx], 
                                          DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
                }
            }
        }
    }
}


// ****************************************************************************
//  Method: avtStructuredMeshChunker::SplitIntoSubgrids
//
//  Purpose:
//      Splits the dataset into subgrids.
//
//  Programmer: Hank Childs
//  Creation:   September 18, 2004
//
// ****************************************************************************

avtStructuredMeshChunker::ZoneDesignation *
avtStructuredMeshChunker::SplitIntoSubgrids(const int *dims, vtkDataSet *in_ds,
                vector<ZoneDesignation> &designations, avtGhostDataType data,
                vector<MeshDescription> &outGrids, vtkUnstructuredGrid *&ugrid,
                bool haveOptimizations)
{
    int  i;

    int cell_dims[3];
    cell_dims[0] = (dims[0] > 1 ? dims[0]-1 : 1);
    cell_dims[1] = (dims[1] > 1 ? dims[1]-1 : 1);
    cell_dims[2] = (dims[2] > 1 ? dims[2]-1 : 1);
    int ncells = cell_dims[2]*cell_dims[1]*cell_dims[0];

    ZoneDesignation *d_plus = new ZoneDesignation[ncells];
    for (i = 0 ; i < ncells ; i++)
        d_plus[i] = designations[i];

    //
    // Tell the partition strategy how small the boxes can be.
    //
    avtMultiResolutionPartitionStrategy p;
    if (haveOptimizations)
        p.SetMinimumSize(256);
    else
        p.SetMinimumSize(2048);

    // 
    // Make the "partition".
    //
    vector<int> boxes;
    int t0 = visitTimer->StartTimer();
    p.ConstructPartition(cell_dims, d_plus, boxes);
    char str[1024];
    sprintf(str, "Constructing %d grid.", boxes.size());
    visitTimer->StopTimer(t0, str);

    //
    // Put them into our internal structure.
    //
    if ((boxes.size() % 6) != 0)
        EXCEPTION0(ImproperUseException);
    int nBoxes = boxes.size() / 6;
    for (i = 0 ; i < nBoxes ; i++)
    {
        // Now mark everything as being "in the grid".
        MeshDescription descr;
        descr.start_index[0] = boxes[6*i];
        descr.start_index[1] = boxes[6*i+2];
        descr.start_index[2] = boxes[6*i+4];
        // +2 --> +1 gives pt index
        //    --> +1 because we use boxes[6*i+1] (range is inclusive)
        descr.index_size[0] = boxes[6*i+1]-boxes[6*i]+2;
        descr.index_size[1] = boxes[6*i+3]-boxes[6*i+2]+2;  
        descr.index_size[2] = boxes[6*i+5]-boxes[6*i+4]+2;
        outGrids.push_back(descr);

        const int box[6] = { boxes[6*i], boxes[6*i+1], boxes[6*i+2],
                             boxes[6*i+3], boxes[6*i+4], boxes[6*i+5] };

        for (int kk = box[4] ; kk <= box[5] ; kk++)
        {
            int kkBase = kk*cell_dims[1]*cell_dims[0];
            for (int jj = box[2] ; jj <= box[3] ; jj++)
            {
                int jjBase = jj*cell_dims[0];
                for (int ii = box[0] ; ii <= box[1] ; ii++)
                {
                    int iiBase = ii;
                    int idx = kkBase + jjBase + iiBase;
                    d_plus[idx] = PUT_IN_GRID;
                }
            }
        }
    }

    //
    // Account for ghost zones.
    //
    if (data == GHOST_ZONE_DATA)
        ModifyGridsForGhostZones(outGrids, cell_dims, d_plus);

    //
    // Create an unstructured grid of what is left.
    //
    int t1 = visitTimer->StartTimer();
    CreateUnstructuredGrid(in_ds, d_plus, data, ugrid, dims);
    char str2[1024];
    sprintf(str2, "Creating a ugrid of size %d", ugrid->GetNumberOfCells());
    visitTimer->StopTimer(t1, str2);

    return d_plus;
}


// ****************************************************************************
//  Function: AddPoints
//
//  Purpose:
//      Adds the points associated with an individual cell.
//
//  Programmer: Hank Childs
//  Creation:   November 13, 2004
//
// ****************************************************************************

static void
AddPoints(bool *pt_list, int cell, const int *dims)
{
    int cell_dims[3];
    cell_dims[0] = (dims[0]-1 < 1 ? 1 : dims[0]-1);
    cell_dims[1] = (dims[1]-1 < 1 ? 1 : dims[1]-1);
    cell_dims[2] = (dims[2]-1 < 1 ? 1 : dims[2]-1);

    int I = cell % cell_dims[0];
    int J = (cell/cell_dims[0]) % cell_dims[1];
    int K = cell / (cell_dims[0]*cell_dims[1]);

    pt_list[K*dims[0]*dims[1] + J*dims[0] + I] = true;
    pt_list[K*dims[0]*dims[1] + J*dims[0] + I+1] = true;
    pt_list[K*dims[0]*dims[1] + (J+1)*dims[0] + I] = true;
    pt_list[K*dims[0]*dims[1] + (J+1)*dims[0] + I+1] = true;
    if (cell_dims[2] > 1)
    {
        pt_list[(K+1)*dims[0]*dims[1] + J*dims[0] + I] = true;
        pt_list[(K+1)*dims[0]*dims[1] + J*dims[0] + I+1] = true;
        pt_list[(K+1)*dims[0]*dims[1] + (J+1)*dims[0] + I] = true;
        pt_list[(K+1)*dims[0]*dims[1] + (J+1)*dims[0] + I+1] = true;
    }
}


// ****************************************************************************
//  Function: GetUnstructuredCellList
//
//  Purpose:
//      Gets the cell list for the unstructured grid we are going to build.
//
//  Programmer: Hank Childs
//  Creation:   November 13, 2004
//
// ****************************************************************************

void
avtStructuredMeshChunker::GetUnstructuredCellList(
                   avtStructuredMeshChunker::ZoneDesignation *d_plus,
                   const int *dims, avtGhostDataType ghost_type,
                   vector<int> &real_zones, vector<int> &ghost_zones)
{
    int   i, j, k, ii, jj, kk;

    real_zones.clear();
    ghost_zones.clear();

    int cell_dims[3];
    cell_dims[0] = (dims[0]-1 < 1 ? 1 : dims[0]-1);
    cell_dims[1] = (dims[1]-1 < 1 ? 1 : dims[1]-1);
    cell_dims[2] = (dims[2]-1 < 1 ? 1 : dims[2]-1);

    //
    // Go through the zones and determine if there are any that have status
    // "RETAIN" or "TO_BE_PROCESSED".  If so, then collect these for our
    // unstructured mesh.  Also, if we have ghost zones, then collect that
    // information for later.
    //
    vector<int> ghost_zones_with_repeats;
    for (k = 0 ; k < cell_dims[2] ; k++)
    {
        for (j = 0 ; j < cell_dims[1] ; j++)
        {
            for (i = 0 ; i < cell_dims[0] ; i++)
            {
                int idx = k*cell_dims[0]*cell_dims[1] + j*cell_dims[0] + i;
                if (d_plus[idx] == RETAIN || d_plus[idx] == TO_BE_PROCESSED)
                {
                    real_zones.push_back(idx);
                    if (ghost_type == GHOST_ZONE_DATA)
                    {
                        int lowI = (i-1 < 0 ? 0 : i-1);
                        int lowJ = (j-1 < 0 ? 0 : j-1);
                        int lowK = (k-1 < 0 ? 0 : k-1);
                        int hiI = (i+1 >= cell_dims[0] ? cell_dims[0]-1 : i+1);
                        int hiJ = (j+1 >= cell_dims[1] ? cell_dims[1]-1 : j+1);
                        int hiK = (k+1 >= cell_dims[2] ? cell_dims[2]-1 : k+1);
                        for (ii = lowI ; ii <= hiI ; ii++)
                            for (jj = lowJ ; jj <= hiJ ; jj++)
                                for (kk = lowK ; kk <= hiK ; kk++)
                                {
                                    int idx = kk*cell_dims[0]*cell_dims[1] +
                                              jj*cell_dims[0] + ii;
                                    if (d_plus[idx] == PUT_IN_GRID)
                                       ghost_zones_with_repeats.push_back(idx);
                                }
                     }
                }
            }
        }
    }

    if (ghost_zones_with_repeats.size() > 0)
    {
        int ncells = cell_dims[0]*cell_dims[1]*cell_dims[2];
        bool *have_zone = new bool[ncells];
        for (i = 0 ; i < ncells ; i++)
            have_zone[i] = false;

        int nentries = ghost_zones_with_repeats.size();
        for (i = 0 ; i < nentries ; i++)
            have_zone[ghost_zones_with_repeats[i]] = true;

        for (i = 0 ; i < ncells ; i++)
            if (have_zone[i])
                ghost_zones.push_back(i);
        delete [] have_zone;
    }
}


// ****************************************************************************
//  Method: avtStructuredMeshChunker::CreateUnstructuredGrid
//
//  Purpose:
//      Although most of the cells we want are going to be put in subgrids,
//      some of them will not fit into a structured setting.  Those cells
//      will be put in one large unstructured grid.  This method creates that
//      grid.
//
//  Programmer: Hank Childs
//  Creation:   September 19, 2004
//
// ****************************************************************************

void
avtStructuredMeshChunker::CreateUnstructuredGrid(vtkDataSet *in_ds, 
        ZoneDesignation *designations, avtGhostDataType ghost_type,
        vtkUnstructuredGrid *&ugrid, const int *dims)
{
    int   i;

    //
    // Now take all of the zones that were not used and put them in an
    // unstructured grid.
    //
    int nNewPts = 0;
    int nOrigPts = dims[0]*dims[1]*dims[2];
    bool *needPt = new bool[nOrigPts];
    for (i = 0 ; i < nOrigPts ; i++)
         needPt[i] = false;

    vector<int> real_zones;
    vector<int> ghost_zones;
    GetUnstructuredCellList(designations, dims, ghost_type, real_zones,
                            ghost_zones);
    int nReal = real_zones.size();
    int nGhost = ghost_zones.size();
    int ncells = nReal + nGhost;
    for (i = 0 ; i < ncells ; i++)
    {
        int id = (i < nReal ? real_zones[i] : ghost_zones[i-nReal]);
        AddPoints(needPt, id, dims);
    }

    ugrid = vtkUnstructuredGrid::New();

    for (i = 0 ; i < nOrigPts ; i++)
        if (needPt[i])
            nNewPts++;

    vtkPoints *pts = vtkPoints::New();
    pts->SetNumberOfPoints(nNewPts);
    ugrid->SetPoints(pts);
    pts->Delete();
    float *pts_ptr = (float *) pts->GetVoidPointer(0);

    vtkPointData *newPD = ugrid->GetPointData();
    vtkPointData *origPD = in_ds->GetPointData();
    newPD->CopyAllocate(origPD, nNewPts);

    int *newPtIndex = new int[nOrigPts];
    int nextIndex = 0;
    if (in_ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        vtkRectilinearGrid *rgrid = (vtkRectilinearGrid *) in_ds;
        float *x = (float *) rgrid->GetXCoordinates()->GetVoidPointer(0);
        float *y = (float *) rgrid->GetYCoordinates()->GetVoidPointer(0);
        float *z = (float *) rgrid->GetZCoordinates()->GetVoidPointer(0);
        for (i = 0 ; i < nOrigPts ; i++)
        {
            if (needPt[i])
            {
                int I = (i%dims[0]);
                int J = (i/dims[0])%dims[1];
                int K = i/(dims[0]*dims[1]);
                pts_ptr[3*nextIndex] = x[I];
                pts_ptr[3*nextIndex+1] = y[J];
                pts_ptr[3*nextIndex+2] = z[K];
                newPD->CopyData(origPD, i, nextIndex);
                newPtIndex[i] = nextIndex;
                nextIndex++;
            }
            else
                newPtIndex[i] = -1;
        }
    }
    else
    {
        vtkStructuredGrid *sgrid = (vtkStructuredGrid *) in_ds;
        float *orig_pts = (float *) sgrid->GetPoints()->GetVoidPointer(0);
        for (i = 0 ; i < nOrigPts ; i++)
        {
            if (needPt[i])
            {
                pts_ptr[3*nextIndex] = orig_pts[3*nextIndex];
                pts_ptr[3*nextIndex+1] = orig_pts[3*nextIndex+1];
                pts_ptr[3*nextIndex+2] = orig_pts[3*nextIndex+2];
                newPD->CopyData(origPD, i, nextIndex);
                newPtIndex[i] = nextIndex;
                nextIndex++;
            }
            else
                newPtIndex[i] = -1;
        }
    }

    vtkCellData *newCD = ugrid->GetCellData();
    vtkCellData *origCD = in_ds->GetCellData();
    newCD->CopyAllocate(origCD, ncells);

    bool is2D = (dims[2] <= 1);
    ugrid->Allocate(is2D ? 5*ncells : 9*ncells);
    int cell_dims[3];
    cell_dims[0] = (dims[0]-1 < 1 ? 1 : dims[0]-1);
    cell_dims[1] = (dims[1]-1 < 1 ? 1 : dims[1]-1);
    cell_dims[2] = (dims[2]-1 < 1 ? 1 : dims[2]-1);
    for (i = 0 ; i < ncells ; i++)
    {
        int id = (i < nReal ? real_zones[i] : ghost_zones[i-nReal]);
        int I = id % cell_dims[0];
        int J = (id/cell_dims[0]) % cell_dims[1];
        int K = id/(cell_dims[0]*cell_dims[1]);
        vtkIdType conn[8];
        if (is2D)
        {
            conn[0] = newPtIndex[J*dims[0] + I];
            conn[1] = newPtIndex[J*dims[0] + (I+1)];
            conn[2] = newPtIndex[(J+1)*dims[0] + (I+1)];
            conn[3] = newPtIndex[(J+1)*dims[0] + I];
            ugrid->InsertNextCell(VTK_QUAD, 4, conn);
        }
        else
        {
            conn[0] = newPtIndex[K*dims[0]*dims[1] + J*dims[0] + I];
            conn[1] = newPtIndex[K*dims[0]*dims[1] + J*dims[0] + (I+1)];
            conn[2] = newPtIndex[K*dims[0]*dims[1] + (J+1)*dims[0] + (I+1)];
            conn[3] = newPtIndex[K*dims[0]*dims[1] + (J+1)*dims[0] + I];
            conn[4] = newPtIndex[(K+1)*dims[0]*dims[1] + J*dims[0] + I];
            conn[5] = newPtIndex[(K+1)*dims[0]*dims[1] + J*dims[0] + (I+1)];
            conn[6] = newPtIndex[(K+1)*dims[0]*dims[1] + (J+1)*dims[0] +(I+1)];
            conn[7] = newPtIndex[(K+1)*dims[0]*dims[1] + (J+1)*dims[0] + I];
            ugrid->InsertNextCell(VTK_HEXAHEDRON, 8, conn);
        }
        newCD->CopyData(origCD, id, i);
    }

    if (ghost_type == GHOST_ZONE_DATA)
    {
        vtkDataArray *arr = newCD->GetArray("avtGhostZones");
        if (arr == NULL)
        {
            arr = vtkUnsignedCharArray::New();
            arr->SetName("avtGhostZones");
            arr->SetNumberOfTuples(ncells);
            unsigned char *vals = (unsigned char *) arr->GetVoidPointer(0);
            for (i = 0 ; i < ncells ; i++)
                vals[i] = 0;
            newCD->AddArray(arr);
            arr->Delete();
        }

        unsigned char *vals = (unsigned char *) arr->GetVoidPointer(0);
        for (i = nReal ; i < ncells ; i++)
            avtGhostData::AddGhostZoneType(vals[i], 
                                          DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
    }
    else
    {
        vtkUnsignedCharArray *arr = (vtkUnsignedCharArray *) 
                                              newPD->GetArray("avtGhostNodes");
        if (arr == NULL)
        {
            arr = vtkUnsignedCharArray::New();
            arr->SetName("avtGhostNodes");
            int npts = nNewPts;
            arr->SetNumberOfTuples(npts);
            unsigned char *vals = arr->GetPointer(0);
            for (i = 0 ; i < npts ; i++)
                vals[i] = 0;
            newPD->AddArray(arr);
            arr->Delete();
        }
        unsigned char *vals = arr->GetPointer(0);

        for (i = 0 ; i < nReal ; i++)
        {
            // For each zone, look to see if there is an external face.  If
            // so, look to see if that face borders a retained zone in a
            // sub-grid.  If so, mark the incident nodes as ghost.
            int id = real_zones[i];
            int I = id % cell_dims[0];
            int J = (id/cell_dims[0]) % cell_dims[1];
            int K = id/(cell_dims[0]*cell_dims[1]);
            int left = I;
            int right = I+1;
            int bottom = J;
            int top = J+1;
            int back = K;
            int front = K+1;
            int K_ind = dims[0]*dims[1];
            int J_ind = dims[0];
            int I_ind = 1;
            int back_bottom_left_raw = back*K_ind + bottom*J_ind + left*I_ind;
            int front_bottom_left_raw =front*K_ind + bottom*J_ind + left*I_ind;
            int back_top_left_raw = back*K_ind + top*J_ind + left*I_ind;
            int front_top_left_raw = front*K_ind + top*J_ind + left*I_ind;
            int back_bottom_right_raw = back*K_ind + bottom*J_ind +right*I_ind;
            int front_bottom_right_raw=front*K_ind + bottom*J_ind +right*I_ind;
            int back_top_right_raw = back*K_ind + top*J_ind + right*I_ind;
            int front_top_right_raw = front*K_ind + top*J_ind + right*I_ind;

            int back_bottom_left = newPtIndex[back_bottom_left_raw];
            int front_bottom_left = newPtIndex[front_bottom_left_raw];
            int back_top_left = newPtIndex[back_top_left_raw];
            int front_top_left = newPtIndex[front_top_left_raw];
            int back_bottom_right = newPtIndex[back_bottom_right_raw];
            int front_bottom_right = newPtIndex[front_bottom_right_raw];
            int back_top_right = newPtIndex[back_top_right_raw];
            int front_top_right = newPtIndex[front_top_right_raw];

            int left_zone = id-1;
            if (I-1 >= 0 && designations[left_zone] == PUT_IN_GRID)
            {
                avtGhostData::AddGhostNodeType(vals[back_bottom_left], 
                                                              DUPLICATED_NODE);
                avtGhostData::AddGhostNodeType(vals[back_top_left], 
                                                              DUPLICATED_NODE);
                avtGhostData::AddGhostNodeType(vals[front_bottom_left], 
                                                              DUPLICATED_NODE);
                avtGhostData::AddGhostNodeType(vals[front_top_left], 
                                                              DUPLICATED_NODE);
            }
            int right_zone = id+1;
            if (I+1 <= cell_dims[0]-1 && designations[right_zone] == PUT_IN_GRID)
            {
                avtGhostData::AddGhostNodeType(vals[back_bottom_right], 
                                                              DUPLICATED_NODE);
                avtGhostData::AddGhostNodeType(vals[back_top_right], 
                                                              DUPLICATED_NODE);
                avtGhostData::AddGhostNodeType(vals[front_bottom_right], 
                                                              DUPLICATED_NODE);
                avtGhostData::AddGhostNodeType(vals[front_top_right], 
                                                              DUPLICATED_NODE);
            }
            int bottom_zone = id-cell_dims[0];
            if (J-1 >= 0 && designations[bottom_zone] == PUT_IN_GRID)
            {
                avtGhostData::AddGhostNodeType(vals[back_bottom_right], 
                                                              DUPLICATED_NODE);
                avtGhostData::AddGhostNodeType(vals[front_bottom_right], 
                                                              DUPLICATED_NODE);
                avtGhostData::AddGhostNodeType(vals[back_bottom_left], 
                                                              DUPLICATED_NODE);
                avtGhostData::AddGhostNodeType(vals[front_bottom_left], 
                                                              DUPLICATED_NODE);
            }
            int top_zone = id+cell_dims[0];
            if (J+1 <= cell_dims[1]-1 && designations[top_zone] == PUT_IN_GRID)
            {
                avtGhostData::AddGhostNodeType(vals[back_top_right], 
                                                              DUPLICATED_NODE);
                avtGhostData::AddGhostNodeType(vals[front_top_right], 
                                                              DUPLICATED_NODE);
                avtGhostData::AddGhostNodeType(vals[back_top_left], 
                                                              DUPLICATED_NODE);
                avtGhostData::AddGhostNodeType(vals[front_top_left], 
                                                              DUPLICATED_NODE);
            }
            int back_zone = id-cell_dims[0]*cell_dims[1];
            if (K-1 >= 0 && designations[back_zone] == PUT_IN_GRID)
            {
                avtGhostData::AddGhostNodeType(vals[back_bottom_right], 
                                                              DUPLICATED_NODE);
                avtGhostData::AddGhostNodeType(vals[back_bottom_left], 
                                                              DUPLICATED_NODE);
                avtGhostData::AddGhostNodeType(vals[back_top_right], 
                                                              DUPLICATED_NODE);
                avtGhostData::AddGhostNodeType(vals[back_top_left], 
                                                              DUPLICATED_NODE);
            }
            int front_zone = id+cell_dims[0]*cell_dims[1];
            if (K+1 <= cell_dims[2]-1 && designations[front_zone] == PUT_IN_GRID)
            {
                avtGhostData::AddGhostNodeType(vals[front_bottom_right], 
                                                              DUPLICATED_NODE);
                avtGhostData::AddGhostNodeType(vals[front_bottom_left], 
                                                              DUPLICATED_NODE);
                avtGhostData::AddGhostNodeType(vals[front_top_right], 
                                                              DUPLICATED_NODE);
                avtGhostData::AddGhostNodeType(vals[front_top_left], 
                                                              DUPLICATED_NODE);
            }
        }

    }

    delete [] needPt;
    delete [] newPtIndex;
}


// ****************************************************************************
//  Method: avtStructuredMeshChunker::ModifyGridsForGhostZones
//
//  Purpose:
//      If we have ghost zones, then we will need to extend each grid to
//      accomodate this.  Do this here.
//
//  Programmer: Hank Childs
//  Creation:   November 13, 2004
//
// ****************************************************************************

void
avtStructuredMeshChunker::ModifyGridsForGhostZones(
                                      std::vector<MeshDescription> &grids, 
                                      const int *cell_dims, ZoneDesignation *d)
{
    int  i, j, k;

    for (int g = 0 ; g < grids.size() ; g++)
    {
        MeshDescription &grid = grids[g];

        if (grid.start_index[0] > 0)
        {
            int plane = grid.start_index[0]-1;
            const int jStart = grid.start_index[1];
            const int jEnd = grid.start_index[1] + grid.index_size[1];
            const int kStart = grid.start_index[2];
            const int kEnd = grid.start_index[2] + grid.index_size[2];
            bool allUsed = true;
            for (j = jStart ; j < jEnd && allUsed ; j++)
                for (k = kStart ; k < kEnd && allUsed ; k++)
                {
                    int idx = k*cell_dims[0]*cell_dims[1] + j*cell_dims[0]
                            + plane;
                    if (d[idx] == DISCARD)
                        allUsed = false;
                }
         
            if (allUsed)
            {
                // We can extend the grid by 1.
                grid.start_index[0] -= 1;
                grid.index_size[0] += 1;
            }
            else
            {
                // We have to declare the lowI portion of the grid as ghost.
                for (j = jStart ; j < jEnd ; j++)
                    for (k = kStart ; k < kEnd ; k++)
                    {
                        int idx = k*cell_dims[0]*cell_dims[1] + j*cell_dims[0]
                                + plane+1;
                        if (d[idx] == PUT_IN_GRID)
                            d[idx] = RETAIN;
                    }
            }
            grid.lowIIsGhost = true;
        }
        if (grid.start_index[1] > 0)
        {
            int plane = grid.start_index[1]-1;
            const int iStart = grid.start_index[0];
            const int iEnd = grid.start_index[0] + grid.index_size[0];
            const int kStart = grid.start_index[2];
            const int kEnd = grid.start_index[2] + grid.index_size[2];
            bool allUsed = true;
            for (i = iStart ; i < iEnd && allUsed ; i++)
                for (k = kStart ; k < kEnd && allUsed ; k++)
                {
                    int idx = k*cell_dims[0]*cell_dims[1] + plane*cell_dims[0]
                            + i;
                    if (d[idx] == DISCARD)
                        allUsed = false;
                }
         
            if (allUsed)
            {
                // We can extend the grid by 1.
                grid.start_index[1] -= 1;
                grid.index_size[1] += 1;
            }
            else
            {
                // We have to declare the lowJ portion of the grid as ghost.
                for (i = iStart ; i < iEnd ; i++)
                    for (k = kStart ; k < kEnd ; k++)
                    {
                        int idx = k*cell_dims[0]*cell_dims[1] 
                                + (plane+1)*cell_dims[0] + i;
                        if (d[idx] == PUT_IN_GRID)
                            d[idx] = RETAIN;
                    }
            }
            grid.lowJIsGhost = true;
        }
        if (grid.start_index[2] > 0)
        {
            int plane = grid.start_index[2]-1;
            const int iStart = grid.start_index[0];
            const int iEnd = grid.start_index[0] + grid.index_size[0];
            const int jStart = grid.start_index[1];
            const int jEnd = grid.start_index[1] + grid.index_size[1];
            bool allUsed = true;
            for (i = iStart ; i < iEnd && allUsed ; i++)
                for (j = jStart ; j < jEnd && allUsed ; j++)
                {
                    int idx = plane*cell_dims[0]*cell_dims[1] + j*cell_dims[0]
                            + i;
                    if (d[idx] == DISCARD)
                        allUsed = false;
                }
         
            if (allUsed)
            {
                // We can extend the grid by 1.
                grid.start_index[2] -= 1;
                grid.index_size[2] += 1;
            }
            else
            {
                // We have to declare the lowK portion of the grid as ghost.
                for (i = iStart ; i < iEnd ; i++)
                    for (j = jStart ; j < jEnd ; j++)
                    {
                        int idx = (plane+1)*cell_dims[0]*cell_dims[1] 
                                + j*cell_dims[0] + i;
                        if (d[idx] == PUT_IN_GRID)
                            d[idx] = RETAIN;
                    }
            }
            grid.lowKIsGhost = true;
        }

        //
        // Note that when we are doing comparisons, the index size is number
        // of nodes, not number of zones.
        //
        if (grid.start_index[0]+(grid.index_size[0]-1) < cell_dims[0])
        {
            int plane = grid.start_index[0]+grid.index_size[0]-1;
            const int jStart = grid.start_index[1];
            const int jEnd = grid.start_index[1] + grid.index_size[1];
            const int kStart = grid.start_index[2];
            const int kEnd = grid.start_index[2] + grid.index_size[2];
            bool allUsed = true;
            for (j = jStart ; j < jEnd && allUsed ; j++)
                for (k = kStart ; k < kEnd && allUsed ; k++)
                {
                    int idx = k*cell_dims[0]*cell_dims[1] + j*cell_dims[0]
                            + plane;
                    if (d[idx] == DISCARD)
                        allUsed = false;
                }
         
            if (allUsed)
            {
                // We can extend the grid by 1.
                grid.index_size[0] += 1;
            }
            else
            {
                // We have to declare the hiI portion of the grid as ghost.
                for (j = jStart ; j < jEnd ; j++)
                    for (k = kStart ; k < kEnd ; k++)
                    {
                        int idx = k*cell_dims[0]*cell_dims[1] + j*cell_dims[0]
                                + plane-1;
                        if (d[idx] == PUT_IN_GRID)
                            d[idx] = RETAIN;
                    }
            }
            grid.hiIIsGhost = true;
        }
        if (grid.start_index[1]+(grid.index_size[1]-1) < cell_dims[1])
        {
            int plane = grid.start_index[1]+grid.index_size[1]-1;
            const int iStart = grid.start_index[0];
            const int iEnd = grid.start_index[0] + grid.index_size[0];
            const int kStart = grid.start_index[2];
            const int kEnd = grid.start_index[2] + grid.index_size[2];
            bool allUsed = true;
            for (i = iStart ; i < iEnd && allUsed ; i++)
                for (k = kStart ; k < kEnd && allUsed ; k++)
                {
                    int idx = k*cell_dims[0]*cell_dims[1] + plane*cell_dims[0]
                            + i;
                    if (d[idx] == DISCARD)
                        allUsed = false;
                }
         
            if (allUsed)
            {
                // We can extend the grid by 1.
                grid.index_size[1] += 1;
            }
            else
            {
                // We have to declare the hiI portion of the grid as ghost.
                for (i = iStart ; i < iEnd ; i++)
                    for (k = kStart ; k < kEnd ; k++)
                    {
                        int idx = k*cell_dims[0]*cell_dims[1] 
                                + (plane-1)*cell_dims[0] + i;
                        if (d[idx] == PUT_IN_GRID)
                            d[idx] = RETAIN;
                    }
            }
            grid.hiJIsGhost = true;
        }
        if (grid.start_index[2]+(grid.index_size[2]-1) < cell_dims[2])
        {
            int plane = grid.start_index[2]+grid.index_size[2]-1;
            const int iStart = grid.start_index[0];
            const int iEnd = grid.start_index[0] + grid.index_size[0];
            const int jStart = grid.start_index[1];
            const int jEnd = grid.start_index[1] + grid.index_size[1];
            bool allUsed = true;
            for (i = iStart ; i < iEnd && allUsed ; i++)
                for (j = jStart ; j < jEnd && allUsed ; j++)
                {
                    int idx = plane*cell_dims[0]*cell_dims[1] + j*cell_dims[0]
                            + i;
                    if (d[idx] == DISCARD)
                        allUsed = false;
                }
         
            if (allUsed)
            {
                // We can extend the grid by 1.
                grid.index_size[2] += 1;
            }
            else
            {
                // We have to declare the hiI portion of the grid as ghost.
                for (i = iStart ; i < iEnd ; i++)
                    for (j = jStart ; j < jEnd ; j++)
                    {
                        int idx = (plane-1)*cell_dims[0]*cell_dims[1] 
                                + j*cell_dims[0] + i;
                        if (d[idx] == PUT_IN_GRID)
                            d[idx] = RETAIN;
                    }
            }
            grid.hiKIsGhost = true;
        }
    }
}


// ****************************************************************************
//  Method: MeshDescription constructor
//
//  Programmer: Hank Childs
//  Creation:   November 13, 2004
//
// ****************************************************************************

avtStructuredMeshChunker::MeshDescription::MeshDescription()
{
    lowIIsGhost = false;
    hiIIsGhost = false;
    lowJIsGhost = false;
    hiJIsGhost = false;
    lowKIsGhost = false;
    hiKIsGhost = false;
    start_index[0] = 0;
    start_index[1] = 0;
    start_index[2] = 0;
    index_size[0] = 0;
    index_size[1] = 0;
    index_size[2] = 0;
}


