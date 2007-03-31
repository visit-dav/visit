#include "MIRConnectivity.h"

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <TimingsManager.h>
#include <vtkCellArray.h>
#include <vtkCellType.h>
#include <vtkDataSet.h>
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
            connectivity = new int[9*ncells];
            int *c = connectivity;
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
                        *c++ = 8;
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
        else
        {
            ncells = nx*ny;
            int cell_idx = 0;
            cellindex = new int[ncells];
            celltype = new int[ncells];
            connectivity = new int[5*ncells];
            int *c = connectivity;
            for (int j = 0 ; j < ny ; j++)
            {
                int yOff  = j*(nx+1);
                int yOff1 = (j+1)*(nx+1);
                for (int i = 0 ; i < nx ; i++)
                {
                    cellindex[cell_idx] = (c - connectivity);
                    *c++ = 4;
                    *c++ = yOff + i;
                    *c++ = yOff + i+1;
                    *c++ = yOff1 + i+1;
                    *c++ = yOff1 + i;
                    celltype[cell_idx++] = VTK_QUAD;
                }
            }
        }
    }
    else if (dstype == VTK_UNSTRUCTURED_GRID)
    {
        vtkUnstructuredGrid *ug = (vtkUnstructuredGrid *) ds;
        vtkCellArray *ca = ug->GetCells();
        ncells = ca->GetNumberOfCells();
        int buff_size = ca->GetSize();
        connectivity = new int[buff_size];
        vtkIdType *ptr = ca->GetPointer();
        memcpy(connectivity, ptr, buff_size*sizeof(int));
        debug5 << "Setting up connectivity array for " << ncells
               << " (unstructured grid)." << endl;

        celltype = new int[ncells];
        for (int i = 0 ; i < ncells ; i++)
        {
            celltype[i] = ug->GetCellType(i);
        }

        int c = 0;
        cellindex = new int[ncells];
        for (int j = 0 ; j < ncells ; j++)
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


