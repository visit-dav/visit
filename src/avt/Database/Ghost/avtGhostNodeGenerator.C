// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtGhostNodeGenerator.C                         //
// ************************************************************************* //

#include <avtGhostNodeGenerator.h>

#ifdef PARALLEL
#include <mpi.h>
#endif 

#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkStructuredGrid.h>
#include <vtkUnsignedCharArray.h>

#include <DebugStream.h>
#include <TimingsManager.h>

#include <avtDatasetCollection.h>
#include <avtParallel.h>

#include <vector>


// ****************************************************************************
//  Method: avtGhostNodeGenerator constructor
//
//  Programmer: Eric Brugger
//  Creation:   May 28, 2020
//
//  Modifications:
//
// ****************************************************************************

avtGhostNodeGenerator::avtGhostNodeGenerator()
{
}


// ****************************************************************************
//  Method: avtGhostNodeGenerator destructor
//
//  Programmer: Eric Brugger
//  Creation:   May 28, 2020
//
//  Modifications:
//
// ****************************************************************************

avtGhostNodeGenerator::~avtGhostNodeGenerator()
{
}


// ****************************************************************************
//  Method: avtGhostNodeGenerator::CreateGhosts
//
//  Purpose:
//    Create the ghost nodes.
//
//  Programmer: Eric Brugger
//  Creation:   May 28, 2020
//
//  Modifications:
//
//    Mark C. Miller, Mon Jun 13 17:48:54 PDT 2022
//    Handle a structured grid (of quads) representing a toplogically 2D
//    surface in 3 space. Basically, this involves setting some params that
//    cause adjustments or wholesale skips in the loops over x, y and/or z
//    faces.
// ****************************************************************************

bool
avtGhostNodeGenerator::CreateGhosts(avtDatasetCollection &ds)
{
    int nChunks = ds.GetNDomains();

    //
    // Check that the datasets meet the criteria for creating ghost nodes.
    //
    if (!IsValid(ds))
        return false;

#ifdef PARALLEL
    //
    // Determine the number of chunks per rank.
    //
    int iProc  = PAR_Rank();
    int nProcs = PAR_Size();
    int *chunksPerProc = new int[nProcs];
    MPI_Allgather(&nChunks, 1, MPI_INT, chunksPerProc, 1, MPI_INT, VISIT_MPI_COMM);
    int nChunksTotal = 0;
    for (int i = 0; i < nProcs; i++)
    {
        debug2 << "ChunksPerProc[" << i << "]=" << chunksPerProc[i] << endl;
        nChunksTotal += chunksPerProc[i];
    }
    debug2 << "nChunksTotal=" << nChunksTotal << endl;
#endif
    
    //
    // Create the list of extents of each block.
    //
    double *dsExtents = new double [nChunks*6];
    for (int i = 0; i < nChunks; i++)
    {
        vtkDataSet *d = ds.GetDataset(i, 0);
        if (d == NULL)
        {
            dsExtents[i*6+0] = DBL_MAX;
            dsExtents[i*6+1] = -DBL_MAX;
            dsExtents[i*6+2] = DBL_MAX;
            dsExtents[i*6+3] = -DBL_MAX;
            dsExtents[i*6+4] = DBL_MAX;
            dsExtents[i*6+5] = -DBL_MAX;
            continue;
        }
        vtkStructuredGrid *sgrid = (vtkStructuredGrid *) d;
        vtkPoints *points = sgrid->GetPoints();

        int ndims[3];
        sgrid->GetDimensions(ndims);

        int nxyz = ndims[0] * ndims[1] * ndims[2];
        double coord[3];
        points->GetPoint(0, coord);
        double xmin = coord[0];
        double xmax = coord[0];
        double ymin = coord[1];
        double ymax = coord[1];
        double zmin = coord[2];
        double zmax = coord[2];
        for (int j = 1; j < nxyz; j++)
        {
            points->GetPoint(j, coord);
            xmin = xmin < coord[0] ? xmin : coord[0];
            xmax = xmax > coord[0] ? xmax : coord[0];
            ymin = ymin < coord[1] ? ymin : coord[1];
            ymax = ymax > coord[1] ? ymax : coord[1];
            zmin = zmin < coord[2] ? zmin : coord[2];
            zmax = zmax > coord[2] ? zmax : coord[2];
        }
        dsExtents[i*6+0] = xmin;
        dsExtents[i*6+1] = xmax;
        dsExtents[i*6+2] = ymin;
        dsExtents[i*6+3] = ymax;
        dsExtents[i*6+4] = zmin;
        dsExtents[i*6+5] = zmax;
    }

#ifdef PARALLEL
    //
    // Send the block extents to all the processors.
    //
    double *dsExtentsTotal = new double[nChunksTotal*6];
    int *offsets = new int[nProcs];
    offsets[0] = 0;
    for (int i = 1; i < nProcs; i++)
        offsets[i] = offsets[i-1] + chunksPerProc[i-1] * 6;
    int *recvcounts = new int[nProcs];
    for (int i = 0; i < nProcs; i++)
        recvcounts[i] = chunksPerProc[i] * 6;
    MPI_Allgatherv(dsExtents, nChunks*6, MPI_DOUBLE, dsExtentsTotal,
                   recvcounts, offsets, MPI_DOUBLE, VISIT_MPI_COMM);
#endif

    //
    // Create the list of external faces for each dataset.
    //
    int *nFaces = new int[nChunks];
    double **faceExtents = new double*[nChunks];
    bool **faceExternal = new bool*[nChunks];
    for (int i = 0; i < nChunks; i++)
    {
        vtkDataSet *d = ds.GetDataset(i, 0);
        if (d == NULL)
        {
            nFaces[i] = 0;
            faceExternal[i] = new bool[0];
            faceExtents[i] = new double[0];
            continue;
        }
        vtkStructuredGrid *sgrid = (vtkStructuredGrid *) d;
        vtkPoints *points = sgrid->GetPoints();

        int ndims[3];
        sgrid->GetDimensions(ndims);
        int nx = ndims[0];
        int ny = ndims[1];
        int nz = ndims[2];

        int nx2 = ndims[0] - 1;
        int ny2 = ndims[1] - 1;
        int nz2 = ndims[2] - 1;

        //
        // Params to manage adjustments in looping logic over faces.
        // For common cases, these defaults are obeyed...all X, Y
        // and Z loops are done and no changes are made to
        // incrementing logic (e.g. zero is added to increment).
        //
        bool doX, doY, doZ; doX = doY = doZ = true;
        int nxD, nyD, nzD; nxD = nyD = nzD = 0;

        //
        // Handle possibility of this being a structured grid of quads
        // but defining a surface in 3 space. There are three options
        // depending on which of the 3 (logical) dimensions is set to
        // a size of 1 node thick.
        //
        if (ndims[2] == 1)      // z is one node thick
        {
            nFaces[i] = nx2 * ny2;
            doX = doY = false; // do only Z faces
            nzD = 1;           // adjust z-inc of outer loop
        }
        else if (ndims[1] == 1) // y is one node tick
        {
            nFaces[i] = nx2 * nz2;
            doX = doZ = false; // do only Y faces
            nyD = 1;           // adjust y-inc of outer loop
        }
        else if (ndims[0] == 1) // x is one node thick
        {
            nFaces[i] = ny2 * nz2;
            doY = doZ = false; // do only X faces
            nxD = 1;           // adjust x-inc of outer loop
        }
        else
        {
            nFaces[i] = nx2 * ny2 * 2 + nx2 * nz2 * 2 + ny2 * nz2 * 2;
        }

        faceExternal[i] = new bool[nFaces[i]];
        bool *external = faceExternal[i];
        for (int j = 0; j < nFaces[i]; j++)
            external[j] = true;

        faceExtents[i] = new double[nFaces[i]*6];
        double *extents = faceExtents[i];
        int iface = 0;

        // Do the x faces.
        for (int ix = 0; ix < nx && doX; ix += nx-1+nxD)
        {
            for (int iy = 0; iy < ny2; iy++)
            {
                for (int iz = 0; iz < nz2; iz++)
                {
                    int ndx[4];
                    ndx[0] = iz     * nx * ny + iy     * nx + ix;
                    ndx[1] = (iz+1) * nx * ny + iy     * nx + ix;
                    ndx[2] = (iz+1) * nx * ny + (iy+1) * nx + ix;
                    ndx[3] = iz     * nx * ny + (iy+1) * nx + ix;
                    double coord[3];
                    points->GetPoint(ndx[0], coord);
                    double xmin = coord[0];
                    double xmax = coord[0];
                    double ymin = coord[1];
                    double ymax = coord[1];
                    double zmin = coord[2];
                    double zmax = coord[2];
                    for (int in = 1; in < 4; in++)
                    {
                        points->GetPoint(ndx[in], coord);
                        xmin = xmin < coord[0] ? xmin : coord[0];
                        xmax = xmax > coord[0] ? xmax : coord[0];
                        ymin = ymin < coord[1] ? ymin : coord[1];
                        ymax = ymax > coord[1] ? ymax : coord[1];
                        zmin = zmin < coord[2] ? zmin : coord[2];
                        zmax = zmax > coord[2] ? zmax : coord[2];
                    }
                    extents[iface*6+0] = xmin;
                    extents[iface*6+1] = xmax;
                    extents[iface*6+2] = ymin;
                    extents[iface*6+3] = ymax;
                    extents[iface*6+4] = zmin;
                    extents[iface*6+5] = zmax;
                    iface++;
                }
            }
        }

        // Do the y faces.
        for (int iy = 0; iy < ny && doY; iy += ny-1+nyD)
        {
            for (int ix = 0; ix < nx2; ix++)
            {
                for (int iz = 0; iz < nz2; iz++)
                {
                    int ndx[4];
                    ndx[0] = iz     * nx * ny + iy * nx + ix;
                    ndx[1] = (iz+1) * nx * ny + iy * nx + ix;
                    ndx[2] = (iz+1) * nx * ny + iy * nx + (ix+1);
                    ndx[3] = iz     * nx * ny + iy * nx + (ix+1);
              
                    double coord[3];
                    points->GetPoint(ndx[0], coord);
                    double xmin = coord[0];
                    double xmax = coord[0];
                    double ymin = coord[1];
                    double ymax = coord[1];
                    double zmin = coord[2];
                    double zmax = coord[2];
                    for (int in = 1; in < 4; in++)
                    {
                        points->GetPoint(ndx[in], coord);
                        xmin = xmin < coord[0] ? xmin : coord[0];
                        xmax = xmax > coord[0] ? xmax : coord[0];
                        ymin = ymin < coord[1] ? ymin : coord[1];
                        ymax = ymax > coord[1] ? ymax : coord[1];
                        zmin = zmin < coord[2] ? zmin : coord[2];
                        zmax = zmax > coord[2] ? zmax : coord[2];
                    }
                    extents[iface*6+0] = xmin;
                    extents[iface*6+1] = xmax;
                    extents[iface*6+2] = ymin;
                    extents[iface*6+3] = ymax;
                    extents[iface*6+4] = zmin;
                    extents[iface*6+5] = zmax;
                    iface++;
                }
            }
        }

        // Do the z faces.
        for (int iz = 0; iz < nz && doZ; iz += (nz-1+nzD))
        {
            for (int ix = 0; ix < nx2; ix++)
            {
                for (int iy = 0; iy < ny2; iy++)
                {
                    int ndx[4];
                    ndx[0] = iz * nx * ny + iy     * nx + ix;
                    ndx[1] = iz * nx * ny + (iy+1) * nx + ix;
                    ndx[2] = iz * nx * ny + (iy+1) * nx + (ix+1);
                    ndx[3] = iz * nx * ny + iy     * nx + (ix+1);
                    double coord[3];
                    points->GetPoint(ndx[0], coord);
                    double xmin = coord[0];
                    double xmax = coord[0];
                    double ymin = coord[1];
                    double ymax = coord[1];
                    double zmin = coord[2];
                    double zmax = coord[2];
                    for (int in = 1; in < 4; in++)
                    {
                        points->GetPoint(ndx[in], coord);
                        xmin = xmin < coord[0] ? xmin : coord[0];
                        xmax = xmax > coord[0] ? xmax : coord[0];
                        ymin = ymin < coord[1] ? ymin : coord[1];
                        ymax = ymax > coord[1] ? ymax : coord[1];
                        zmin = zmin < coord[2] ? zmin : coord[2];
                        zmax = zmax > coord[2] ? zmax : coord[2];
                    }
                    extents[iface*6+0] = xmin;
                    extents[iface*6+1] = xmax;
                    extents[iface*6+2] = ymin;
                    extents[iface*6+3] = ymax;
                    extents[iface*6+4] = zmin;
                    extents[iface*6+5] = zmax;
                    iface++;
                }
            }
        }

        assert(iface == nFaces[i]);
    }

#ifdef PARALLEL
    //
    // Determine the number of faces per block.
    //
    int *nFacesTotal = new int[nChunksTotal];
    offsets[0] = 0;
    for (int i = 1; i < nProcs; i++)
        offsets[i] = offsets[i-1] + chunksPerProc[i-1];
    MPI_Allgatherv(nFaces, nChunks, MPI_INT, nFacesTotal, chunksPerProc,
                   offsets, MPI_INT, VISIT_MPI_COMM);
#endif

    //
    // Match faces against the local blocks.
    //
    for (int i = 0; i < nChunks; i++)
    {
        for (int j = i + 1; j < nChunks; j++)
        {
            //
            // Skip this block if there is no overlap in extents.
            //
            bool xoverlap = !(dsExtents[i*6+1] < dsExtents[j*6+0] ||
                              dsExtents[i*6+0] > dsExtents[j*6+1]);
            bool yoverlap = !(dsExtents[i*6+3] < dsExtents[j*6+2] ||
                              dsExtents[i*6+2] > dsExtents[j*6+3]);
            bool zoverlap = !(dsExtents[i*6+5] < dsExtents[j*6+4] ||
                              dsExtents[i*6+4] > dsExtents[j*6+5]);
            if (!xoverlap || !yoverlap || !zoverlap)
            {
                debug4 << "Skipping block " << i << " vs " << j << endl;
                continue;
            }
            double *extents1 = faceExtents[i];
            double *extents2 = faceExtents[j];
            bool *external1 = faceExternal[i];
            bool *external2 = faceExternal[j];
            for (int iface1 = 0; iface1 < nFaces[i]; iface1++)
            {
                //
                // Check if the face is within the bound of the block.
                //
                if (extents1[iface1*6+0] < dsExtents[j*6+0] || 
                    extents1[iface1*6+1] > dsExtents[j*6+1] ||
                    extents1[iface1*6+2] < dsExtents[j*6+2] ||
                    extents1[iface1*6+3] > dsExtents[j*6+3] ||
                    extents1[iface1*6+4] < dsExtents[j*6+4] ||
                    extents1[iface1*6+5] > dsExtents[j*6+5])
                {
                    continue;
                }
                for (int iface2 = 0; iface2 < nFaces[j]; iface2++)
                {
                    if (external2[iface2] &&
                        extents1[iface1*6+0] == extents2[iface2*6+0] &&
                        extents1[iface1*6+1] == extents2[iface2*6+1] &&
                        extents1[iface1*6+2] == extents2[iface2*6+2] &&
                        extents1[iface1*6+3] == extents2[iface2*6+3] &&
                        extents1[iface1*6+4] == extents2[iface2*6+4] &&
                        extents1[iface1*6+5] == extents2[iface2*6+5])
                    {
                        external1[iface1] = false;
                        external2[iface2] = false;
                        break;
                    }
                }
            }
        }
    }

#ifdef PARALLEL
    //
    // Broadcast all the faces to all the processors.
    // 
    int nGlobalFaces = 0;
    for (int i = 0; i < nChunksTotal; i++)
        nGlobalFaces += nFacesTotal[i];
    double *faceExtentsTotal = new double[nGlobalFaces*6];
    int nLocalFaces = 0;
    for (int i = 0; i < nChunks; i++)
        nLocalFaces += nFaces[i];
    double *faceExtentsLocal = new double[nLocalFaces*6];
    int iface = 0;
    for (int i = 0; i < nChunks; i++)
    {
        double *extents = faceExtents[i];
        for (int j = 0; j < nFaces[i]*6; j++)
        {
            faceExtentsLocal[iface] = extents[j];
            iface++;
        }
    }

    assert(iface == nLocalFaces * 6);

    debug2 << "nGlobalFaces=" << nGlobalFaces << endl;
    debug2 << "nLocalFaces=" << nLocalFaces << endl;

    iface = 0;
    for (int i = 0; i < nProcs; i++)
    {
        recvcounts[i] = 0;
        for (int j = 0; j < chunksPerProc[i]; j++)
        {
            recvcounts[i] += nFacesTotal[iface] * 6;
            iface++;
        }
    }
    offsets[0] = 0;
    for (int i = 1; i < nProcs; i++)
    {
        offsets[i] = offsets[i-1] + recvcounts[i-1];
    }
    MPI_Allgatherv(faceExtentsLocal, nLocalFaces*6, MPI_DOUBLE,
                   faceExtentsTotal, recvcounts, offsets, MPI_DOUBLE,
                   VISIT_MPI_COMM);

    //
    // Match faces against the other processor blocks.
    //
    int chunk_start = 0, chunk_end = 0;
    for (int i = 0; i < iProc; i++)
        chunk_start += chunksPerProc[i];
    chunk_end = chunk_start + chunksPerProc[iProc]; 
    for (int i = 0; i < nChunks; i++)
    {
        for (int iFaceExtentsTotal = 0, j = 0;
             j < nChunksTotal; iFaceExtentsTotal += nFacesTotal[j]*6, j++)
        {
            //
            // Skip this block if it belongs to this processor.
            //
            if (j >= chunk_start && j < chunk_end)
            {
                continue;
            }

            //
            // Skip this block if there is no overlap in extents.
            //
            bool xoverlap = !(dsExtents[i*6+1] < dsExtentsTotal[j*6+0] ||
                              dsExtents[i*6+0] > dsExtentsTotal[j*6+1]);
            bool yoverlap = !(dsExtents[i*6+3] < dsExtentsTotal[j*6+2] ||
                              dsExtents[i*6+2] > dsExtentsTotal[j*6+3]);
            bool zoverlap = !(dsExtents[i*6+5] < dsExtentsTotal[j*6+4] ||
                              dsExtents[i*6+4] > dsExtentsTotal[j*6+5]);
            if (!xoverlap || !yoverlap || !zoverlap)
            {
                debug4 << "Skipping block " << i << " vs " << j << endl;
                continue;
            }
            double *extents1 = faceExtents[i];
            double *extents2 = &faceExtentsTotal[iFaceExtentsTotal];
            bool *external1 = faceExternal[i];
            for (int iface1 = 0; iface1 < nFaces[i]; iface1++)
            {
                //
                // Check if the face is within the bound of the block.
                //
                if (extents1[iface1*6+0] < dsExtentsTotal[j*6+0] || 
                    extents1[iface1*6+1] > dsExtentsTotal[j*6+1] ||
                    extents1[iface1*6+2] < dsExtentsTotal[j*6+2] ||
                    extents1[iface1*6+3] > dsExtentsTotal[j*6+3] ||
                    extents1[iface1*6+4] < dsExtentsTotal[j*6+4] ||
                    extents1[iface1*6+5] > dsExtentsTotal[j*6+5])
                {
                    continue;
                }
                for (int iface2 = 0; iface2 < nFacesTotal[j]; iface2++)
                {
                    if (extents1[iface1*6+0] == extents2[iface2*6+0] &&
                        extents1[iface1*6+1] == extents2[iface2*6+1] &&
                        extents1[iface1*6+2] == extents2[iface2*6+2] &&
                        extents1[iface1*6+3] == extents2[iface2*6+3] &&
                        extents1[iface1*6+4] == extents2[iface2*6+4] &&
                        extents1[iface1*6+5] == extents2[iface2*6+5])
                    {
                        external1[iface1] = false;
                        break;
                    }
                }
            }
        }
    }

    //
    // Clean up parallel related memory.
    //
    delete [] chunksPerProc;
    delete [] dsExtentsTotal;
    delete [] offsets;
    delete [] recvcounts;
    delete [] nFacesTotal;
    delete [] faceExtentsTotal;
    delete [] faceExtentsLocal;
#endif

    //
    // Add the ghost node information based on the external faces.
    //
    for (int i = 0; i < nChunks; i++)
    {
        vtkDataSet *d = ds.GetDataset(i, 0);
        if (d == NULL)
        {
            continue;
        }
        vtkStructuredGrid *sgrid = (vtkStructuredGrid *) d;

        int ndims[3];
        sgrid->GetDimensions(ndims);
        int nx = ndims[0];
        int ny = ndims[1];
        int nz = ndims[2];
        int nvals = nx * ny * nz;

        int nx2 = ndims[0] - 1;
        int ny2 = ndims[1] - 1;
        int nz2 = ndims[2] - 1;

        //
        // Params to manage adjustments in looping logic over faces.
        // For common cases, these defaults are obeyed...all X, Y
        // and Z loops are done and no changes are made to
        // incrementing logic (e.g. zero is added to increment).
        //
        bool doX, doY, doZ; doX = doY = doZ = true;
        int nxD, nyD, nzD; nxD = nyD = nzD = 0;

        //
        // Handle possibility of this being a structured grid of quads
        // but defining a surface in 3 space. There are three options
        // depending on which of the 3 (logical) dimensions is set to
        // a size of 1 node thick.
        //
        if (ndims[2] == 1)      // z is one node thick
        {
            nFaces[i] = nx2 * ny2;
            doX = doY = false; // do only Z faces
            nzD = 1;           // adjust z-inc of outer loop
        }
        else if (ndims[1] == 1) // y is one node tick
        {
            nFaces[i] = nx2 * nz2;
            doX = doZ = false; // do only Y faces
            nyD = 1;           // adjust y-inc of outer loop
        }
        else if (ndims[0] == 1) // x is one node thick
        {
            nFaces[i] = ny2 * nz2;
            doY = doZ = false; // do only X faces
            nxD = 1;           // adjust x-inc of outer loop
        }

        //
        // Create the ghost nodes array, initializing it to no ghost
        // nodes.
        //
        vtkUnsignedCharArray *ghostNodes = vtkUnsignedCharArray::New();
        ghostNodes->SetName("avtGhostNodes");
        ghostNodes->SetNumberOfTuples(nvals);
        for (int j = 0; j < nvals; j++)
        {
            ghostNodes->SetValue(j, 0);
        }

        //
        // Loop over all the faces, setting the ghost flag for all the
        // nodes of any ghost faces. We must traverse the faces in the
        // same order that we did above.
        //
        int iface = 0;
        bool *external = faceExternal[i];

        // Do the x faces.
        for (int ix = 0; ix < nx && doX; ix += nx-1+nxD)
        {
            for (int iy = 0; iy < ny2; iy++)
            {
                for (int iz = 0; iz < nz2; iz++)
                {
                    if (!external[iface])
                    {
                        int ndx[4];
                        ndx[0] = iz     * nx * ny + iy     * nx + ix;
                        ndx[1] = (iz+1) * nx * ny + iy     * nx + ix;
                        ndx[2] = (iz+1) * nx * ny + (iy+1) * nx + ix;
                        ndx[3] = iz     * nx * ny + (iy+1) * nx + ix;
                        for (int in = 0; in < 4; in++)
                        {
                            ghostNodes->SetValue(ndx[in], 1);
                        }
                    }
                    iface++;
                }
            }
        }

        // Do the y faces.
        for (int iy = 0; iy < ny && doY; iy += ny-1+nyD)
        {
            for (int ix = 0; ix < nx2; ix++)
            {
                for (int iz = 0; iz < nz2; iz++)
                {
                    if (!external[iface])
                    {
                        int ndx[4];
                        ndx[0] = iz     * nx * ny + iy * nx + ix;
                        ndx[1] = (iz+1) * nx * ny + iy * nx + ix;
                        ndx[2] = (iz+1) * nx * ny + iy * nx + (ix+1);
                        ndx[3] = iz     * nx * ny + iy * nx + (ix+1);
                        for (int in = 0; in < 4; in++)
                        {
                            ghostNodes->SetValue(ndx[in], 1);
                        }
                    }
                    iface++;
                }
            }
        }

        // Do the z faces.
        for (int iz = 0; iz < nz && doZ; iz += nz-1+nzD)
        {
            for (int ix = 0; ix < nx2; ix++)
            {
                for (int iy = 0; iy < ny2; iy++)
                {
                    if (!external[iface])
                    {
                        int ndx[4];
                        ndx[0] = iz * nx * ny + iy     * nx + ix;
                        ndx[1] = iz * nx * ny + (iy+1) * nx + ix;
                        ndx[2] = iz * nx * ny + (iy+1) * nx + (ix+1);
                        ndx[3] = iz * nx * ny + iy     * nx + (ix+1);
                        for (int in = 0; in < 4; in++)
                        {
                            ghostNodes->SetValue(ndx[in], 1);
                        }
                    }
                    iface++;
                }
            }
        }

        assert(iface == nFaces[i]);

        //
        // Add the ghost data to the dataset.
        //
        vtkDataSet *d2 = d->NewInstance();
        d2->ShallowCopy(d);
        d2->GetPointData()->AddArray(ghostNodes);
        ds.SetDataset(i, 0, d2);
        d2->Delete();
        ghostNodes->Delete();
    }

    //
    // Free memory from calculating shared faces.
    //
    delete [] dsExtents;
    delete [] nFaces;
    for (int i = 0; i < nChunks; i++)
        delete [] faceExtents[i];
    delete [] faceExtents;
    for (int i = 0; i < nChunks; i++)
        delete [] faceExternal[i];
    delete [] faceExternal;

    return true;
}


// ****************************************************************************
//  Method: avtGhostNodeGenerator::IsValid
//
//  Purpose:
//    Check if the datasets meet the criteria to create ghost nodes with
//    this class.
//
//  Programmer: Eric Brugger
//  Creation:   May 28, 2020
//
//  Modifications:
//
// ****************************************************************************

bool
avtGhostNodeGenerator::IsValid(avtDatasetCollection &ds)
{
    int nChunks = ds.GetNDomains();

    //
    // Check that we only have structured grids and that the total
    // number of faces to consider is less than 20 million.
    //
    int valid = 1;
    long long nFaces = 0;
    for (int i = 0; i < nChunks; i++)
    {
        vtkDataSet *d = ds.GetDataset(i, 0);
        if (d == NULL)
        {
            continue;
        }
        if (d->GetDataObjectType() != VTK_STRUCTURED_GRID)
        {
            valid = 0;
            break;
        }
        vtkStructuredGrid *sgrid = (vtkStructuredGrid *) d;

        int ndims[3];
        sgrid->GetDimensions(ndims);

        int nx2 = ndims[0] - 1;
        int ny2 = ndims[1] - 1;
        int nz2 = ndims[2] - 1;

        nFaces += nx2 * ny2 * 2 + nx2 * nz2 * 2 + ny2 * nz2 * 2;
    }

#ifdef PARALLEL
    int valid2;
    MPI_Allreduce(&valid, &valid2, 1, MPI_INT, MPI_MIN, VISIT_MPI_COMM);
    valid = valid2;
#endif

    if (valid == 0)
    {
        debug1 << "Skipping ghost node creation since we don't have all "
                  "structured grids." << endl;
        return false;
    }

#ifdef PARALLEL
    long long nFaces2;
    MPI_Allreduce(&nFaces, &nFaces2, 1, MPI_LONG_LONG, MPI_SUM, VISIT_MPI_COMM);
    nFaces = nFaces2;
#endif

    if (nFaces > 20000000)
    {
        debug1 << "Skipping ghost node creation since the total number of "
                  "faces to consider is greater than 20 million." << endl;
        return false;
    }

    debug1 << "Performing ghost node creation." << endl;

    return true;
}
