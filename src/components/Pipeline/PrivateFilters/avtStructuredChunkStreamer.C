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
//                      avtStructuredChunkStreamer.C                         //
// ************************************************************************* //

#include <avtStructuredChunkStreamer.h>

#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>

#include <TimingsManager.h>


// ****************************************************************************
//  Method: avtStructuredChunkStreamer constructor
//
//  Programmer: Hank Childs
//  Creation:   April 27, 2005
//
// ****************************************************************************

avtStructuredChunkStreamer::avtStructuredChunkStreamer()
{
    downstreamRectilinearMeshOptimizations = false;
    downstreamCurvilinearMeshOptimizations = false;
    downstreamGhostType = NO_GHOST_DATA;
    chunkedStructuredMeshes = false;
}


// ****************************************************************************
//  Method: avtStructuredChunkStreamer destructor
//
//  Programmer: Hank Childs
//  Creation:   April 27, 2005
//
// ****************************************************************************

avtStructuredChunkStreamer::~avtStructuredChunkStreamer()
{
}


// ****************************************************************************
//  Method: avtStructuredChunkStreamer::ExecuteDataTree
//
//  Programmer: Hank Childs
//  Creation:   April 27, 2005
//
//  Modifications:
//
//    Hank Childs, Fri Jun  9 13:25:31 PDT 2006
//    Comment out currently unused variable to remove compiler warning.
//
// ****************************************************************************

avtDataTree_p
avtStructuredChunkStreamer::ExecuteDataTree(vtkDataSet *in_ds, int domain,
                                            std::string label)
{
    int ds_type = in_ds->GetDataObjectType();
    // bool haveStructured = (ds_type == VTK_RECTILINEAR_GRID ||
    //                        ds_type == VTK_STRUCTURED_GRID); 
    bool downstreamOptimizations = false;
    int dims[3] = { 0, 0, 0 };
    if (ds_type == VTK_RECTILINEAR_GRID)
    {
        vtkRectilinearGrid *rgrid = (vtkRectilinearGrid *) in_ds;
        rgrid->GetDimensions(dims);
        downstreamOptimizations = downstreamRectilinearMeshOptimizations;
    }
    else if (ds_type == VTK_STRUCTURED_GRID)
    {
        vtkStructuredGrid *sgrid = (vtkStructuredGrid *) in_ds;
        sgrid->GetDimensions(dims);
        downstreamOptimizations = downstreamCurvilinearMeshOptimizations;
    }

    //bool canChunk = haveStructured;
    bool canChunk = false;  // Turn off for now.
    if (dims[0] <= 1 || dims[1] <= 1 || dims[2] <= 1)
        canChunk = false;

    if (!canChunk)
    {
        vtkDataSet *out = ProcessOneChunk(in_ds, domain, label, false);
        avtDataTree_p rv = new avtDataTree(1, &out, domain, label);
        if (out != NULL)
            out->Delete();
        return rv;
    }

    int ncells = in_ds->GetNumberOfCells();
    vector<avtStructuredMeshChunker::ZoneDesignation> designation(ncells);
    // cell-dims
    dims[0] -= 1;
    dims[1] -= 1;
    dims[2] -= 1;
    int t0 = visitTimer->StartTimer();
    GetAssignments(in_ds, dims, designation);
    visitTimer->StopTimer(t0, "Structured Chunk Streamer: Getting assignments");
    
    vtkUnstructuredGrid *ugrid = NULL;
    vector<vtkDataSet *> grids;

    int t1 = visitTimer->StartTimer();
    avtStructuredMeshChunker::ChunkStructuredMesh(in_ds, designation,
                   grids, ugrid, downstreamGhostType, downstreamOptimizations);
    visitTimer->StopTimer(t1, "Identifying grids");

    int t2 = visitTimer->StartTimer();
    vtkDataSet *out_ugrid = ProcessOneChunk(ugrid, domain, label, true);
    visitTimer->StopTimer(t2, 
                      "Structured Chunk Streamer: Processing ugrid leftovers");
    //
    // Create a data tree that has all of the structured meshes, as well
    // as the single unstructured mesh.
    //
    vtkDataSet **out_ds = new vtkDataSet*[grids.size()+1];
    for (int i = 0 ; i < grids.size() ; i++)
        out_ds[i] = grids[i];
    out_ds[grids.size()] = out_ugrid;
    avtDataTree_p rv = new avtDataTree(grids.size()+1, out_ds, domain, label);
    delete [] out_ds;
    if (out_ugrid != NULL)
        out_ugrid->Delete();
    if (ugrid != NULL)
        ugrid->Delete();
    chunkedStructuredMeshes = true;

    return rv;
}


// ****************************************************************************
//  Method: avtStructuredChunkStreamer::PerformRestriction
//
//  Purpose:
//      Inspect the input specification and determine what its requirements
//      are.
//
//  Programmer: Hank Childs
//  Creation:   March 27, 2005
//
// ****************************************************************************

avtPipelineSpecification_p
avtStructuredChunkStreamer::PerformRestriction(avtPipelineSpecification_p spec)
{
    downstreamRectilinearMeshOptimizations =
                                   spec->GetHaveRectilinearMeshOptimizations();
    downstreamCurvilinearMeshOptimizations =
                                   spec->GetHaveCurvilinearMeshOptimizations();
    downstreamGhostType =
                       spec->GetDataSpecification()->GetDesiredGhostDataType();

    return spec;
}


// ****************************************************************************
//  Method: avtStructuredChunkStreamer::PreExecute
//
//  Purpose:
//      Initialize chunkedStructuredMesh.
//
//  Programmer: Hank Childs
//  Creation:   March 27, 2005
//
// ****************************************************************************

void
avtStructuredChunkStreamer::PreExecute(void)
{
    avtDataTreeStreamer::PreExecute();
    chunkedStructuredMeshes = false;
}


// ****************************************************************************
//  Method: avtStructuredChunkStreamer::PostExecute
//
//  Purpose:
//      If we chunked a structured mesh, indicate that we may now have ghost
//      data.
//
//  Programmer: Hank Childs
//  Creation:   March 27, 2005
//
// ****************************************************************************

void
avtStructuredChunkStreamer::PostExecute(void)
{
    avtDataTreeStreamer::PostExecute();

    if (chunkedStructuredMeshes && downstreamGhostType != NO_GHOST_DATA)
    {
        GetOutput()->GetInfo().GetAttributes().
                                     SetContainsGhostZones(AVT_CREATED_GHOSTS);
    }
}


