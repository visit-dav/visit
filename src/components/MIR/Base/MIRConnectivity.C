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
//    Jeremy Meredith, Wed Jun 23 15:31:07 PDT 2004
//    Added true voxel cases.  Doing a translation here forced us to do
//    more translations later.  It is both easier and faster to get the
//    shape type correct.
//
//    Jeremy Meredith, Tue Jul 13 17:50:00 PDT 2004
//    The previous fix only applied to rectilinear cases.  I added code to
//    revert to the old method for curvilinear meshes.
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
                        if (dstype == VTK_RECTILINEAR_GRID)
                        {
                            *c++ = 8;
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
                    if (dstype == VTK_RECTILINEAR_GRID)
                    {
                        *c++ = 4;
                        *c++ = yOff + i;
                        *c++ = yOff + i+1;
                        *c++ = yOff1 + i;
                        *c++ = yOff1 + i+1;
                        celltype[cell_idx++] = VTK_PIXEL;
                    }
                    else
                    {
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


