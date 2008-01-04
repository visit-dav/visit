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

#include "vtkStructuredGridFacelistFilter.h"

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkStructuredGrid.h>


//------------------------------------------------------------------------------
vtkStructuredGridFacelistFilter* vtkStructuredGridFacelistFilter::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkStructuredGridFacelistFilter");
  if(ret)
    {
    return (vtkStructuredGridFacelistFilter*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkStructuredGridFacelistFilter;
}

inline int
PointIndex(int x, int y, int z, int nX, int nY, int nZ)
{
    return z*nY*nX + y*nX + x;
}

// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Wed Aug 25 16:28:52 PDT 2004
//    Account for degenerate meshes.
//
// ****************************************************************************

inline int
CellIndex(int x, int y, int z, int nX, int nY, int nZ)
{
    int cellYBase = 1;
    if (nX > 1)
       cellYBase = (nX-1);
    else
       cellYBase = 1;

    int cellZBase = 1;
    if (nY > 1 && nX > 1)
       cellZBase = (nY-1)*(nX-1);
    else if (nY > 1)
       cellZBase = (nY-1);
    else if (nX > 1)
       cellZBase = (nX-1);
    else
       cellZBase = 1;

    return z*cellZBase + y*cellYBase + x;
}

// ***************************************************************************
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 13:22:36 PST 2001
//    Make quad of type vtkIdType to match VTK 4.0 API.
//
//    Jeremy Meredith, Mon Jun 24 13:58:42 PDT 2002
//    Changed the ordering of 3 of the 6 sides so they are always ccw.
//    Relabeled the comments for the six faces to match conventions.
//
//    Hank Childs, Thu Jul 25 18:00:48 PDT 2002
//    Handle meshes where one dimension is '1' better.
//
//    Hank Childs, Fri Jan 30 08:31:44 PST 2004
//    Use pointer arithmetic to construct poly data output.
//
//    Hank Childs, Sun Feb  1 22:02:51 PST 2004
//    Do a better job of estimating the number of cells in the 2D case.
//
//    Hank Childs, Mon Aug  9 07:24:52 PDT 2004
//    Do a better job of handling degenerate meshes with dimensions 1xJxK.
//
//    Hank Childs, Wed Aug 25 16:32:08 PDT 2004
//    Do a better job of assessing the output cell count.
//
// ****************************************************************************

void vtkStructuredGridFacelistFilter::Execute()
{
  int   i, j;

  vtkStructuredGrid *input       = GetInput();
  vtkPolyData       *output      = GetOutput();
  vtkCellData       *inCellData  = input->GetCellData();
  vtkCellData       *outCellData = output->GetCellData();

  int   dims[3];
  input->GetDimensions(dims);
  int   nX = dims[0];
  int   nY = dims[1];
  int   nZ = dims[2];
  int   numOutCells = 0;
  if (nX > 1)
     numOutCells += 2*(nY-1)*(nZ-1);
  else
     numOutCells += (nY-1)*(nZ-1);
  if (nY > 1)
     numOutCells += 2*(nX-1)*(nZ-1);
  else
     numOutCells += (nX-1)*(nZ-1);
  if (nZ > 1)
     numOutCells += 2*(nX-1)*(nY-1);
  else
     numOutCells += (nX-1)*(nY-1);

  //
  // Copy over the points and the point data.
  //
  output->SetPoints(input->GetPoints());
  output->GetPointData()->PassData(input->GetPointData());

  //
  // Have the cell data allocate memory.
  //
  outCellData->CopyAllocate(inCellData);

  vtkCellArray *polys = vtkCellArray::New();
  vtkIdTypeArray *list = vtkIdTypeArray::New();
  list->SetNumberOfValues(numOutCells*(4+1));
  vtkIdType *nl = list->GetPointer(0);
  
  
  //
  // Left face
  //
  int cellId = 0;
  for (i = 0 ; i < nY-1 ; i++)
  {
    for (j = 0 ; j < nZ-1 ; j++)
    {
      *nl++ = 4;
      *nl++ = PointIndex(0, i, j, nX, nY, nZ);
      *nl++ = PointIndex(0, i, j+1, nX, nY, nZ);
      *nl++ = PointIndex(0, i+1, j+1, nX, nY, nZ);
      *nl++ = PointIndex(0, i+1, j, nX, nY, nZ);
      int cId = CellIndex(0, i, j, nX, nY, nZ);
      outCellData->CopyData(inCellData, cId, cellId);
      cellId++;
    }
  }
  
  //
  // Right face
  //
  if (nX > 1)
    {
    for (i = 0 ; i < nY-1 ; i++)
    {
      for (j = 0 ; j < nZ-1 ; j++)
      {
        *nl++ = 4;
        *nl++ = PointIndex(nX-1, i, j, nX, nY, nZ);
        *nl++ = PointIndex(nX-1, i+1, j, nX, nY, nZ);
        *nl++ = PointIndex(nX-1, i+1, j+1, nX, nY, nZ);
        *nl++ = PointIndex(nX-1, i, j+1, nX, nY, nZ);
        int cId = CellIndex(nX-2, i, j, nX, nY, nZ);
        outCellData->CopyData(inCellData, cId, cellId);
        cellId++;
      }
    }
  }
  
  //
  // Bottom face
  //
  for (i = 0 ; i < nX-1 ; i++)
  {
    for (j = 0 ; j < nZ-1 ; j++)
    {
      *nl++ = 4;
      *nl++ = PointIndex(i, 0, j, nX, nY, nZ);
      *nl++ = PointIndex(i+1, 0, j, nX, nY, nZ);
      *nl++ = PointIndex(i+1, 0, j+1, nX, nY, nZ);
      *nl++ = PointIndex(i, 0, j+1, nX, nY, nZ);
      int cId = CellIndex(i, 0, j, nX, nY, nZ);
      outCellData->CopyData(inCellData, cId, cellId);
      cellId++;
    }
  }
  
  //
  // Top face
  //
  if (nY > 1)
  {
    for (i = 0 ; i < nX-1 ; i++)
    {
      for (j = 0 ; j < nZ-1 ; j++)
      {
        *nl++ = 4;
        *nl++ = PointIndex(i, nY-1, j, nX, nY, nZ);
        *nl++ = PointIndex(i, nY-1, j+1, nX, nY, nZ);
        *nl++ = PointIndex(i+1, nY-1, j+1, nX, nY, nZ);
        *nl++ = PointIndex(i+1, nY-1, j, nX, nY, nZ);
        int cId = CellIndex(i, nY-2, j, nX, nY, nZ);
        outCellData->CopyData(inCellData, cId, cellId);
        cellId++;
      }
    }
  }
  
  //
  // Back face
  //
  for (i = 0 ; i < nX-1 ; i++)
  {
    for (j = 0 ; j < nY-1 ; j++)
    {
      *nl++ = 4;
      *nl++ = PointIndex(i, j, 0, nX, nY, nZ);
      *nl++ = PointIndex(i, j+1, 0, nX, nY, nZ);
      *nl++ = PointIndex(i+1, j+1, 0, nX, nY, nZ);
      *nl++ = PointIndex(i+1, j, 0, nX, nY, nZ);
      int cId = CellIndex(i, j, 0, nX, nY, nZ);
      outCellData->CopyData(inCellData, cId, cellId);
      cellId++;
    }
  }
  
  //
  // Front face
  //
  if (nZ > 1)
  {
    for (i = 0 ; i < nX-1 ; i++)
    {
      for (j = 0 ; j < nY-1 ; j++)
      {
        *nl++ = 4;
        *nl++ = PointIndex(i, j, nZ-1, nX, nY, nZ);
        *nl++ = PointIndex(i+1, j, nZ-1, nX, nY, nZ);
        *nl++ = PointIndex(i+1, j+1, nZ-1, nX, nY, nZ);
        *nl++ = PointIndex(i, j+1, nZ-1, nX, nY, nZ);
        int cId = CellIndex(i, j, nZ-2, nX, nY, nZ);
        outCellData->CopyData(inCellData, cId, cellId);
        cellId++;
      }
    }
  }
  
  polys->SetCells(numOutCells, list);
  list->Delete();

  outCellData->Squeeze();
  output->SetPolys(polys);
  polys->Delete();
}


void vtkStructuredGridFacelistFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkStructuredGridToPolyDataFilter::PrintSelf(os,indent);
}
