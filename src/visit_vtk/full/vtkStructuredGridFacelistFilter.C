// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "vtkStructuredGridFacelistFilter.h"

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkIdTypeArray.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
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
//  Method: vtkStructuredGridFacelistFilter::RequestData
//
//  Modifications:
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
//    Kathleen Biagas, Thu Aug 11, 2022
//    Support VTK9: connectivity and offsets now stored in separate arrays.
//
// ****************************************************************************

int
vtkStructuredGridFacelistFilter::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  //
  // Initialize some frequently used values.
  //
  vtkStructuredGrid *input = vtkStructuredGrid::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  int   i, j;

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

  vtkIdTypeArray *connectivity = vtkIdTypeArray::New();
  connectivity->SetNumberOfValues(numOutCells*4);
  vtkIdType *cl = connectivity->GetPointer(0);

  vtkIdTypeArray *offsets = vtkIdTypeArray::New();
  // size of offsets array always numCells + 1
  // last value holds size of connectivity array
  offsets->SetNumberOfValues(numOutCells+1);
  vtkIdType *ol = offsets->GetPointer(0);
  // set up first offset
  *ol++ = 0;
  // subsequent offsets are incremented from previous by num pts in cell
  // set up a holder for the increment
  vtkIdType currentOffset = 0;

  //
  // Left face
  //
  int cellId = 0;
  for (i = 0 ; i < nY-1 ; i++)
  {
    for (j = 0 ; j < nZ-1 ; j++)
    {
      currentOffset += 4;
      *ol++ = currentOffset;
      *cl++ = PointIndex(0, i, j, nX, nY, nZ);
      *cl++ = PointIndex(0, i, j+1, nX, nY, nZ);
      *cl++ = PointIndex(0, i+1, j+1, nX, nY, nZ);
      *cl++ = PointIndex(0, i+1, j, nX, nY, nZ);
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
        currentOffset += 4;
        *ol++ = currentOffset;
        *cl++ = PointIndex(nX-1, i, j, nX, nY, nZ);
        *cl++ = PointIndex(nX-1, i+1, j, nX, nY, nZ);
        *cl++ = PointIndex(nX-1, i+1, j+1, nX, nY, nZ);
        *cl++ = PointIndex(nX-1, i, j+1, nX, nY, nZ);
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
      currentOffset += 4;
      *ol++ = currentOffset;
      *cl++ = PointIndex(i, 0, j, nX, nY, nZ);
      *cl++ = PointIndex(i+1, 0, j, nX, nY, nZ);
      *cl++ = PointIndex(i+1, 0, j+1, nX, nY, nZ);
      *cl++ = PointIndex(i, 0, j+1, nX, nY, nZ);
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
        currentOffset += 4;
        *ol++ = currentOffset;
        *cl++ = PointIndex(i, nY-1, j, nX, nY, nZ);
        *cl++ = PointIndex(i, nY-1, j+1, nX, nY, nZ);
        *cl++ = PointIndex(i+1, nY-1, j+1, nX, nY, nZ);
        *cl++ = PointIndex(i+1, nY-1, j, nX, nY, nZ);
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
      currentOffset += 4;
      *ol++ = currentOffset;
      *cl++ = PointIndex(i, j, 0, nX, nY, nZ);
      *cl++ = PointIndex(i, j+1, 0, nX, nY, nZ);
      *cl++ = PointIndex(i+1, j+1, 0, nX, nY, nZ);
      *cl++ = PointIndex(i+1, j, 0, nX, nY, nZ);
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
        currentOffset += 4;
        *ol++ = currentOffset;
        *cl++ = PointIndex(i, j, nZ-1, nX, nY, nZ);
        *cl++ = PointIndex(i+1, j, nZ-1, nX, nY, nZ);
        *cl++ = PointIndex(i+1, j+1, nZ-1, nX, nY, nZ);
        *cl++ = PointIndex(i, j+1, nZ-1, nX, nY, nZ);
        int cId = CellIndex(i, j, nZ-2, nX, nY, nZ);
        outCellData->CopyData(inCellData, cId, cellId);
        cellId++;
      }
    }
  }
  polys->SetData(offsets, connectivity);
  offsets->Delete();
  connectivity->Delete();

  outCellData->Squeeze();
  output->SetPolys(polys);
  polys->Delete();

  return 1;
}

// ****************************************************************************
//  Method: vtkStructuredGridFacelistFilter::FillInputPortInformation
//
// ****************************************************************************

int
vtkStructuredGridFacelistFilter::FillInputPortInformation(int,
  vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkStructuredGrid");
  return 1;
}

// ****************************************************************************
//  Method: vtkStructuredGridFacelistFilter::PrintSelf
//
// ****************************************************************************

void
vtkStructuredGridFacelistFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkPolyDataAlgorithm::PrintSelf(os,indent);
}
