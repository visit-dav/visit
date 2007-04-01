// ************************************************************************* //
//                      avtStructuredChunkStreamer.C                         //
// ************************************************************************* //

#include <avtStructuredChunkStreamer.h>

#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>


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
// ****************************************************************************

avtDataTree_p
avtStructuredChunkStreamer::ExecuteDataTree(vtkDataSet *in_ds, int domain,
                                            std::string label)
{
    int ds_type = in_ds->GetDataObjectType();
    bool haveStructured = (ds_type == VTK_RECTILINEAR_GRID ||
                           ds_type == VTK_STRUCTURED_GRID);
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
    GetAssignments(in_ds, dims, designation);
    
    vtkUnstructuredGrid *ugrid = NULL;
    vector<vtkDataSet *> grids;
    avtStructuredMeshChunker::ChunkStructuredMesh(in_ds, designation,
                   grids, ugrid, downstreamGhostType, downstreamOptimizations);

    vtkDataSet *out_ugrid = ProcessOneChunk(ugrid, domain, label, true);

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


