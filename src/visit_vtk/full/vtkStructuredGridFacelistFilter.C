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

inline int
CellIndex(int x, int y, int z, int nX, int nY, int nZ)
{
    return z*(nY-1)*(nX-1) + y*(nX-1) + x;
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
  int   numOutCells = 2*(nX-1)*(nY-1) + 2*(nX-1)*(nZ-1) + 2*(nY-1)*(nZ-1);
  
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
