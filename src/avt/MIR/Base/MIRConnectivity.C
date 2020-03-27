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
// ****************************************************************************

MIRConnectivity::MIRConnectivity()
{
    cellindex = NULL;
    conn_offsets = NULL;
    connectivity = NULL;
    celltype = NULL;
    ncells = 0;
}

// ****************************************************************************
//  Method: MIRConnectivity destructor
//
//  Programmer: Hank Childs
//  Creation:   October 7, 2002
//
// ****************************************************************************

MIRConnectivity::~MIRConnectivity()
{
    if (cellindex != NULL)
    {
        delete [] cellindex;
    }
    if (conn_offsets != NULL)
    {
        delete [] conn_offsets;
    }
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
        if (nz > 0)
        {
            ncells = nx*ny*nz;
            int cell_idx = 0;
            cellindex = new int[ncells];
            celltype = new int[ncells];
            conn_offsets = new vtkIdType[ncells+1];
            // last entry in the offsets array needs to be the size of the connectivity array.
            conn_offsets[ncells] = 8*ncells;
            connectivity = new vtkIdType[8*ncells];
            vtkIdType *o = conn_offsets;
            vtkIdType *c = connectivity;
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
                        cellindex[cell_idx] = (c - connectivity);
                        if (dstype == VTK_RECTILINEAR_GRID)
                        {
                            *o++ = 8;
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
                            *o++ = 8;
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
        else
        {
            ncells = nx*ny;
            int cell_idx = 0;
            cellindex = new int[ncells];
            celltype = new int[ncells];
            conn_offsets = new vtkIdType[ncells+1];
            // last entry in the offsets array needs to be the size of the connectivity array.
            conn_offsets[ncells] = 4*ncells;
            connectivity = new vtkIdType[4*ncells];
            vtkIdType *o = conn_offsets;
            vtkIdType *c = connectivity;
            for (int j = 0 ; j < ny ; j++)
            {
                int yOff  = j*(nx+1);
                int yOff1 = (j+1)*(nx+1);
                for (int i = 0 ; i < nx ; i++)
                {
                    cellindex[cell_idx] = (c - connectivity);
                    if (dstype == VTK_RECTILINEAR_GRID)
                    {
                        *o++ = 4;
                        *c++ = yOff + i;
                        *c++ = yOff + i+1;
                        *c++ = yOff1 + i;
                        *c++ = yOff1 + i+1;
                        celltype[cell_idx++] = VTK_PIXEL;
                    }
                    else
                    {
                        *o++ = 4;
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
        vtkIdType conn_size = ca->GetNumberOfConnectivityIds();
        conn_offsets = new vtkIdType[ncells+1];
        // last entry in the offsets array needs to be the size of the connectivity array.
        conn_offsets[ncells] = conn_size;
        connectivity = new vtkIdType[conn_size];
        vtkDataArray *off_array = ca->GetOffsetsArray();
        vtkDataArray *conn_array = ca->GetConnectivityArray();
        debug5 << "Setting up connectivity array for " << ncells
               << " (unstructured grid)." << endl;

        for (vtkIdType i = 0 ; i < conn_size ; i++)
            connectivity[i] = vtkIdType(conn_array->GetTuple1(i));

        celltype = new int[ncells];
        for (vtkIdType i = 0 ; i < ncells ; i++)
        {
            conn_offsets[i] = vtkIdType(off_array->GetTuple1(i));
            celltype[i] = dstype==VTK_POLY_DATA?pd->GetCellType(i):ug->GetCellType(i);
        }

        int c = 0;
        cellindex = new int[ncells];
        for (vtkIdType j = 0 ; j < ncells ; j++)
        {
            cellindex[j] = c;
            c += connectivity[c] + 1;
        }
    }
    else
    {
        EXCEPTION0(ImproperUseException);
    }

    visitTimer->StopTimer(timerHandle, "MIR: Setting up connectivity array");
    visitTimer->DumpTimings();
}


