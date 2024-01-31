// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "MIRConnectivity.h"

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <TimingsManager.h>
#include <vtkCellArray.h>
#include <vtkCellType.h>
#include <vtkDataSet.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>


// ****************************************************************************
//  Method: MIRConnectivity constructor
//
//  Programmer: Hank Childs
//  Creation:   October 7, 2002
//
//  Modifications:
//    Kathleen Biagas, Thu Aug 11 2022
//    For VTK9: add offsets, to keep in line with storage in vtkCellArray class:
//    separate connectivity and offsets arrays.
//
// ****************************************************************************

MIRConnectivity::MIRConnectivity()
{
    connectivity = NULL;
    celltype = NULL;
    ncells = 0;
#if LIB_VERSION_LE(VTK, 8,1,0)
    cellindex = NULL;
#else
    offsets = NULL;
#endif

}

// ****************************************************************************
//  Method: MIRConnectivity destructor
//
//  Programmer: Hank Childs
//  Creation:   October 7, 2002
//
//  Modifications:
//    Kathleen Biagas, Thu Aug 11 2022
//    For VTK9: add offsets, to keep in line with storage in vtkCellArray class:
//    separate connectivity and offsets arrays.
//
// ****************************************************************************

MIRConnectivity::~MIRConnectivity()
{
#if LIB_VERSION_LE(VTK, 8,1,0)
    if (cellindex != NULL)
    {
        delete [] cellindex;
    }
#else
    if (offsets != NULL)
    {
        delete [] offsets;
    }
#endif
    if (connectivity != NULL)
    {
        delete [] connectivity;
    }
    if (celltype != NULL)
    {
        delete [] celltype;
    }
}

// ****************************************************************************
//  Method: MIRConnectivity::SetUpConnectivity
//
//  Purpose:
//      Sets up the connectivity from a VTK dataset.
//
//  Programmer: Hank Childs
//  Creation:   October 7, 2002
//
//  Modifications:
//    Brad Whitlock, Tue Sep 23 09:41:14 PDT 2003
//    Made it build on Windows.
//
//    Jeremy Meredith, Wed Jun 23 15:31:07 PDT 2004
//    Added true voxel cases.  Doing a translation here forced us to do
//    more translations later.  It is both easier and faster to get the
//    shape type correct.
//
//    Jeremy Meredith, Tue Jul 13 17:50:00 PDT 2004
//    The previous fix only applied to rectilinear cases.  I added code to
//    revert to the old method for curvilinear meshes.
//
//    Mark C. Miller, Fri Oct 19 15:45:06 PDT 2018
//    Handle VTK_POLY_DATA in same block as VTK_UNSTRUCTURED_GRID
//
//    Mark C. Miller, Mon Jun 13 22:50:55 PDT 2022
//    Handle topologically 2D surface (of quads) potentially embedded in 3
//    space. The basic trick is to identify the two dimensions of the input
//    mesh more than a single node thick and treat those two dimensions as
//    a basic 2D mesh.

//    Kathleen Biagas, Thu Aug 11 2022
//    For VTK9: Add offsets, to keep in line with storage in vtkCellArray class:
//    separate connectivity and offsets arrays.
//
// ****************************************************************************

void
MIRConnectivity::SetUpConnectivity(vtkDataSet *ds)
{
    int timerHandle = visitTimer->StartTimer();

    int dstype = ds->GetDataObjectType();

    if (dstype == VTK_RECTILINEAR_GRID || dstype == VTK_STRUCTURED_GRID)
    {
        int dims[3];
        if (dstype == VTK_RECTILINEAR_GRID)
        {
            ((vtkRectilinearGrid*)ds)->GetDimensions(dims);
        }
        else
        {
            ((vtkStructuredGrid*)ds)->GetDimensions(dims);
        }

        int nx = dims[0]-1;
        int ny = dims[1]-1;
        int nz = dims[2]-1;
        debug5 << "Setting up connectivity array for " << dims[0] << ", "
               << dims[1] << ", " << dims[2] << endl;
        if (nx > 0 && ny > 0 && nz > 0) // fully 3D case
        {
            ncells = nx*ny*nz;
            int cell_idx = 0;
            celltype = new int[ncells];
            connectivity = new vtkIdType[9*ncells];
            vtkIdType *c = connectivity;
#if LIB_VERSION_LE(VTK, 8,1,0)
            cellindex = new int[ncells];
#else
            // offsets always nCells +1 in size
            offsets = new vtkIdType[ncells+1];
            vtkIdType *o = offsets;
            // First offset is always 0, so set it here, outside of the loop.
            *o++ = 0;
            // Subsequent offsets are incremented by the number of ids in
            // the current cell, so set up a holder for that increment.
            vtkIdType currentOffset = 0;
            // The last entry in offsets will hold the size of connectivity
#endif
            for (int k = 0 ; k < nz ; k++)
            {
                int zOff  = k*(nx+1)*(ny+1);
                int zOff1 = (k+1)*(nx+1)*(ny+1);
                for (int j = 0 ; j < ny ; j++)
                {
                    int yOff  = j*(nx+1);
                    int yOff1 = (j+1)*(nx+1);
                    for (int i = 0 ; i < nx ; i++)
                    {
#if LIB_VERSION_LE(VTK,8,1,0)
                        cellindex[cell_idx] = (c - connectivity);
#endif
                        if (dstype == VTK_RECTILINEAR_GRID)
                        {
#if LIB_VERSION_LE(VTK,8,1,0)
                            *c++ = 8;
#else
                            currentOffset += 8;
                            *o++ = currentOffset;
#endif
                            *c++ = zOff + yOff + i;
                            *c++ = zOff + yOff + i+1;
                            *c++ = zOff + yOff1 + i;
                            *c++ = zOff + yOff1 + i+1;
                            *c++ = zOff1 + yOff + i;
                            *c++ = zOff1 + yOff + i+1;
                            *c++ = zOff1 + yOff1 + i;
                            *c++ = zOff1 + yOff1 + i+1;
                            celltype[cell_idx++] = VTK_VOXEL;
                        }
                        else
                        {
#if LIB_VERSION_LE(VTK,8,1,0)
                            *c++ = 8;
#else
                            currentOffset += 8;
                            *o++ = currentOffset;
#endif
                            *c++ = zOff + yOff + i;
                            *c++ = zOff + yOff + i+1;
                            *c++ = zOff + yOff1 + i+1;
                            *c++ = zOff + yOff1 + i;
                            *c++ = zOff1 + yOff + i;
                            *c++ = zOff1 + yOff + i+1;
                            *c++ = zOff1 + yOff1 + i+1;
                            *c++ = zOff1 + yOff1 + i;
                            celltype[cell_idx++] = VTK_HEXAHEDRON;
                        }
                    }
                }
            }
        }
        else // 2D (potentially 2D topological but 3D spatial)
        {
            // Identify dimensions to treat as 'x' and 'y' here
            if (nz == 0) // no-op case
            {
                         // treat input x dimension like 'x' here
                (void)0; // treat input y dimension like 'y' here
            }
            else if (ny == 0)
            {
                         // treat input x dimension like 'x' here
                ny = nz; // treat input z dimension like 'y' here
            }
            else if (nx == 0)
            {
                nx = ny; // treat input y dimension like 'x' here
                ny = nz; // treat input z dimension like 'y' here
            }

            ncells = nx*ny;
            int cell_idx = 0;
            celltype = new int[ncells];
            connectivity = new vtkIdType[5*ncells];
            vtkIdType *c = connectivity;
#if LIB_VERSION_LE(VTK,8,1,0)
            cellindex = new int[ncells];
#else
            // offsets always nCells +1 in size
            offsets = new vtkIdType[ncells+1];
            vtkIdType *o = offsets;
            // First offset is always 0, so set it here, outside of the loop.
            *o++ = 0;
            // Subsequent offsets are incremented by the number of ids in
            // the current cell, so set up a holder for that increment.
            vtkIdType currentOffset = 0;
            // The last entry in offsets will hold the size of connectivity
#endif
            for (int j = 0 ; j < ny ; j++)
            {
                int yOff  = j*(nx+1);
                int yOff1 = (j+1)*(nx+1);
                for (int i = 0 ; i < nx ; i++)
                {
#if LIB_VERSION_LE(VTK,8,1,0)
                    cellindex[cell_idx] = (c - connectivity);
#endif
                    if (dstype == VTK_RECTILINEAR_GRID)
                    {
#if LIB_VERSION_LE(VTK,8,1,0)
                        *c++ = 4;
#else
                        currentOffset+=4;
                        *o++ = currentOffset;
#endif
                        *c++ = yOff + i;
                        *c++ = yOff + i+1;
                        *c++ = yOff1 + i;
                        *c++ = yOff1 + i+1;
                        celltype[cell_idx++] = VTK_PIXEL;
                    }
                    else
                    {
#if LIB_VERSION_LE(VTK,8,1,0)
                        *c++ = 4;
#else
                        currentOffset+=4;
                        *o++ = currentOffset;
#endif
                        *c++ = yOff + i;
                        *c++ = yOff + i+1;
                        *c++ = yOff1 + i+1;
                        *c++ = yOff1 + i;
                        celltype[cell_idx++] = VTK_QUAD;
                    }
                }
            }
        }
    }
    else if (dstype == VTK_UNSTRUCTURED_GRID || dstype == VTK_POLY_DATA)
    {
        vtkUnstructuredGrid *ug = (vtkUnstructuredGrid *) ds;
        vtkPolyData *pd = (vtkPolyData *) ds;
        vtkCellArray *ca = dstype==VTK_POLY_DATA?pd->GetPolys():ug->GetCells();
        ncells = ca->GetNumberOfCells();

        if (dstype == VTK_POLY_DATA)
        {
            debug5 << "Setting up connectivity array for " << ncells
                   << " (polydata)." << endl;
        }
        else
        {
            debug5 << "Setting up connectivity array for " << ncells
                   << " (unstructured grid)." << endl;
        }

#if LIB_VERSION_LE(VTK,8,1,0)
        int buff_size = ca->GetSize();
        connectivity = new vtkIdType[buff_size];
        vtkIdType *ptr = ca->GetPointer();
        memcpy(connectivity, ptr, buff_size*sizeof(vtkIdType));
#else
        // connectivity
        vtkDataArray *conn_array = ca->GetConnectivityArray();
        vtkIdType csize = conn_array->GetNumberOfTuples();
        connectivity = new vtkIdType[csize];
        for (vtkIdType i = 0 ; i < conn_array->GetNumberOfTuples(); ++i)
                connectivity[i] = vtkIdType(conn_array->GetTuple1(i));

        // offsets
        vtkDataArray *off_array = ca->GetOffsetsArray();
        vtkIdType osize = off_array->GetNumberOfTuples();
        offsets = new vtkIdType[osize];
        for (vtkIdType i = 0 ; i < off_array->GetNumberOfTuples(); ++i)
            offsets[i] = vtkIdType(off_array->GetTuple1(i));
#endif

        // celltypes
        celltype = new int[ncells];
        for (vtkIdType i = 0 ; i < ncells ; i++)
        {
            celltype[i] = dstype==VTK_POLY_DATA?pd->GetCellType(i):ug->GetCellType(i);
        }
#if LIB_VERSION_LE(VTK,8,1,0)
        int c = 0;
        cellindex = new int[ncells];
        for (int j = 0 ; j < ncells ; j++)
        {
            cellindex[j] = c;
            c += connectivity[c] + 1;
        }
#endif
    }
    else
    {
        EXCEPTION0(ImproperUseException);
    }

    visitTimer->StopTimer(timerHandle, "MIR: Setting up connectivity array");
    visitTimer->DumpTimings();
}


