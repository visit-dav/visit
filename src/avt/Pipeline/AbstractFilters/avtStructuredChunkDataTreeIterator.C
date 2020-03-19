// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                   avtStructuredChunkDataTreeIterator.C                    //
// ************************************************************************* //

#include <avtStructuredChunkDataTreeIterator.h>

#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>

#include <TimingsManager.h>

#include <vector>


// ****************************************************************************
//  Method: avtStructuredChunkDataTreeIterator constructor
//
//  Programmer: Hank Childs
//  Creation:   April 27, 2005
//
// ****************************************************************************

avtStructuredChunkDataTreeIterator::avtStructuredChunkDataTreeIterator()
{
    downstreamRectilinearMeshOptimizations = false;
    downstreamCurvilinearMeshOptimizations = false;
    downstreamGhostType = NO_GHOST_DATA;
    chunkedStructuredMeshes = false;
}


// ****************************************************************************
//  Method: avtStructuredChunkDataTreeIterator destructor
//
//  Programmer: Hank Childs
//  Creation:   April 27, 2005
//
// ****************************************************************************

avtStructuredChunkDataTreeIterator::~avtStructuredChunkDataTreeIterator()
{
}


// ****************************************************************************
//  Method: avtStructuredChunkDataTreeIterator::ExecuteDataTree
//
//  Programmer: Hank Childs
//  Creation:   April 27, 2005
//
//  Modifications:
//
//    Hank Childs, Fri Jun  9 13:25:31 PDT 2006
//    Comment out currently unused variable to remove compiler warning.
//
//    Eric Brugger, Wed Aug 20 16:33:03 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

avtDataTree_p
avtStructuredChunkDataTreeIterator::ExecuteDataTree(avtDataRepresentation *in_dr)
{
    //
    // Get the VTK data set, the domain number, and the label.
    //
    vtkDataSet *in_ds = in_dr->GetDataVTK();
    int domain = in_dr->GetDomain();
    std::string label = in_dr->GetLabel();

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
        avtDataRepresentation *out = ProcessOneChunk(in_dr, false);
        avtDataTree_p rv = new avtDataTree(1, out);
        if (out != NULL)
            delete out;
        return rv;
    }

    int ncells = in_ds->GetNumberOfCells();
    std::vector<avtStructuredMeshChunker::ZoneDesignation> designation(ncells);
    // cell-dims
    dims[0] -= 1;
    dims[1] -= 1;
    dims[2] -= 1;
    int t0 = visitTimer->StartTimer();
    GetAssignments(in_ds, dims, designation);
    visitTimer->StopTimer(t0, "Structured Chunk DataTreeIterator: Getting assignments");
    
    //
    // FIX_ME: ChunkStructuredMesh should return an array of avtDataReps
    //
    vtkUnstructuredGrid *ugrid = NULL;
    std::vector<vtkDataSet *> grids;

    int t1 = visitTimer->StartTimer();
    avtStructuredMeshChunker::ChunkStructuredMesh(in_ds, designation,
                   grids, ugrid, downstreamGhostType, downstreamOptimizations);
    visitTimer->StopTimer(t1, "Identifying grids");

    int t2 = visitTimer->StartTimer();

    avtDataRepresentation ugrid_dr(ugrid, domain, label);

    avtDataRepresentation *out_ugrid_dr = ProcessOneChunk(&ugrid_dr, true);
    visitTimer->StopTimer(t2, 
                      "Structured Chunk DataTreeIterator: Processing ugrid leftovers");
    //
    // Create a data tree that has all of the structured meshes, as well
    // as the single unstructured mesh.
    //
    avtDataRepresentation **out_dr = new avtDataRepresentation*[grids.size()+1];
    for (size_t i = 0 ; i < grids.size() ; i++)
        out_dr[i] = new avtDataRepresentation(grids[i], domain, label);
    out_dr[grids.size()] = out_ugrid_dr;
    avtDataTree_p rv = new avtDataTree((int)grids.size()+1, out_dr);
    for (size_t i = 0 ; i < grids.size() ; i++)
        delete out_dr[i];
    delete [] out_dr;
    if (out_ugrid_dr != NULL)
        delete out_ugrid_dr;
    if (ugrid != NULL)
        ugrid->Delete();
    chunkedStructuredMeshes = true;

    return rv;
}


// ****************************************************************************
//  Method: avtStructuredChunkDataTreeIterator::ModifyContract
//
//  Purpose:
//      Inspect the input specification and determine what its requirements
//      are.
//
//  Programmer: Hank Childs
//  Creation:   March 27, 2005
//
// ****************************************************************************

avtContract_p
avtStructuredChunkDataTreeIterator::ModifyContract(avtContract_p spec)
{
    downstreamRectilinearMeshOptimizations =
                                   spec->GetHaveRectilinearMeshOptimizations();
    downstreamCurvilinearMeshOptimizations =
                                   spec->GetHaveCurvilinearMeshOptimizations();
    downstreamGhostType =
                       spec->GetDataRequest()->GetDesiredGhostDataType();

    return spec;
}


// ****************************************************************************
//  Method: avtStructuredChunkDataTreeIterator::PreExecute
//
//  Purpose:
//      Initialize chunkedStructuredMesh.
//
//  Programmer: Hank Childs
//  Creation:   March 27, 2005
//
// ****************************************************************************

void
avtStructuredChunkDataTreeIterator::PreExecute(void)
{
    avtSIMODataTreeIterator::PreExecute();
    chunkedStructuredMeshes = false;
}


// ****************************************************************************
//  Method: avtStructuredChunkDataTreeIterator::PostExecute
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
avtStructuredChunkDataTreeIterator::PostExecute(void)
{
    avtSIMODataTreeIterator::PostExecute();

    if (chunkedStructuredMeshes && downstreamGhostType != NO_GHOST_DATA)
    {
        GetOutput()->GetInfo().GetAttributes().
                                     SetContainsGhostZones(AVT_CREATED_GHOSTS);
    }
}


