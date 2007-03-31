#include <vtkRectilinearGridFacelistFilter.h>

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDataSetRemoveGhostCells.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>


//------------------------------------------------------------------------------
vtkRectilinearGridFacelistFilter* vtkRectilinearGridFacelistFilter::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkRectilinearGridFacelistFilter");
  if(ret)
    {
    return (vtkRectilinearGridFacelistFilter*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkRectilinearGridFacelistFilter;
}

vtkRectilinearGridFacelistFilter::vtkRectilinearGridFacelistFilter()
{
    ForceFaceConsolidation = 0;
}


//
// The rectilinear grid must create a set of points and it does so by walking
// through the front face, back face, bottom face, top face, left face, right
// face (in that order without repeating points on the edges).  Indexing those 
// points later can be tiresome, so the logic is contained here in the
// "SpecializedIndexer".
//
class SpecializedIndexer
{
  public:
                      SpecializedIndexer(int, int, int);

   inline int         GetFrontFacePoint(int x, int y)
                           { return frontOffset + x*nY+y; };
   inline int         GetBackFacePoint(int x, int y)
                           { return backOffset + x*nY+y; };
   inline int         GetBottomFacePoint(int x, int z)
                           {
                             if (z == 0) return GetFrontFacePoint(x, 0);
                             if (z == nZ-1) return GetBackFacePoint(x, 0);
                             return bottomOffset + x*(nZ-2)+z-1;
                           };
   inline int         GetTopFacePoint(int x, int z)
                           {
                             if (z == 0) return GetFrontFacePoint(x, nY-1);
                             if (z == nZ-1) return GetBackFacePoint(x, nY-1);
                             return topOffset + x*(nZ-2)+z-1; 
                           };
   inline int         GetLeftFacePoint(int y, int z)
                           { 
                             if (z == 0) return GetFrontFacePoint(0, y);
                             if (z == nZ-1) return GetBackFacePoint(0, y);
                             if (y == 0) return GetBottomFacePoint(0, z);
                             if (y == nY-1) return GetTopFacePoint(0, z);
                             return leftOffset + (y-1)*(nZ-2)+z-1;
                           };
   inline int         GetRightFacePoint(int y, int z)
                           { 
                             if (z == 0) return GetFrontFacePoint(nX-1, y);
                             if (z == nZ-1) return GetBackFacePoint(nX-1, y);
                             if (y == 0) return GetBottomFacePoint(nX-1, z);
                             if (y == nY-1) return GetTopFacePoint(nX-1, z);
                             return rightOffset + (y-1)*(nZ-2)+z-1;
                           };
   inline int         GetCellIndex(int x, int y, int z)
                           { return z*cellZBase + y*cellYBase + x; };
 

  protected:
   int                nX, nY, nZ;
   int                frontOffset;
   int                backOffset;
   int                bottomOffset;
   int                topOffset;
   int                leftOffset;
   int                rightOffset;
   int                cellZBase;
   int                cellYBase;
};

SpecializedIndexer::SpecializedIndexer(int x, int y, int z)
{
    nX = x;
    nY = y;
    nZ = z;

    frontOffset = 0;
    backOffset = nX*nY;
    bottomOffset = 2*nX*nY;
    topOffset = 2*nX*nY + nX*(nZ-2);
    leftOffset = 2*nX*nY + 2*nX*(nZ-2);
    rightOffset = 2*nX*nY + 2*nX*(nZ-2) + (nY-2)*(nZ-2);

    cellZBase = (nY-1)*(nX-1);
    cellYBase = (nX-1);
}


// ****************************************************************************
//
//  Hank Childs, Thu Aug 15 21:13:43 PDT 2002
//  Fixed bug where cell data was being copied incorrectly.
//
//  Hank Childs, Wed Oct 15 19:24:56 PDT 2003
//  Added logic for consolidating faces.
//
// ****************************************************************************

void vtkRectilinearGridFacelistFilter::Execute()
{
  if (ForceFaceConsolidation)
    {
        ConsolidationExecute();
        return;
    }

  int   i, j;
  vtkIdType   quad[4];

  //
  // Set up some objects that we will be using throughout the process.
  //
  vtkRectilinearGrid *input        = GetInput();
  vtkPolyData        *output       = GetOutput();
  vtkCellData        *inCellData   = input->GetCellData();
  vtkPointData       *inPointData  = input->GetPointData();
  vtkCellData        *outCellData  = output->GetCellData();
  vtkPointData       *outPointData = output->GetPointData();

  //
  // Get the information about X, Y, and Z from the rectilinear grid.
  //
  vtkDataArray *xc = input->GetXCoordinates();
  int nX = xc->GetNumberOfTuples();
  float *x = new float[nX];
  for (i = 0 ; i < nX ; i++)
  {
    x[i] = xc->GetTuple1(i);
  }
  vtkDataArray *yc = input->GetYCoordinates();
  int nY = yc->GetNumberOfTuples();
  float *y = new float[nY];
  for (i = 0 ; i < nY ; i++)
  {
    y[i] = yc->GetTuple1(i);
  }
  vtkDataArray *zc = input->GetZCoordinates();
  int nZ = zc->GetNumberOfTuples();
  float *z = new float[nZ];
  for (i = 0 ; i < nZ ; i++)
  {
    z[i] = zc->GetTuple1(i);
  }

  //
  // Now create the points.  Do this so that the total number of points is
  // minimal -- this requires sharing points along edges and corners and leads
  // to a nasty indexing scheme.  Also be wary of 2D issues.
  //
  vtkPoints *pts = vtkPoints::New();
  int npts = 0;
  if (nX <= 1)
  {
    npts = 2*nY*nZ;
  }
  else if (nY <= 1)
  {
    npts = 2*nX*nZ;
  }
  else if (nZ <= 1)
  {
    npts = 2*nX*nY;
  }
  else
  {
    npts = 2*nX*nY + 2*(nZ-2)*nX + 2*(nZ-2)*(nY-2);
  }
  pts->SetNumberOfPoints(npts);
  float *p = (float *) pts->GetVoidPointer(0);

  //
  // We will be copying the point data as we go so we need to set this up.
  //
  outPointData->CopyAllocate(input->GetPointData());
  int pointId = 0;

  // Front face.
  for (i = 0 ; i < nX ; i++)
  {
    for (j = 0 ; j < nY ; j++)
    {
      p[0] = x[i];
      p[1] = y[j];
      p[2] = z[0];
      p += 3;
      outPointData->CopyData(inPointData, j*nX + i, pointId++);
    }
  }

  // Back face
  if (nZ > 1)
  {
    for (i = 0 ; i < nX ; i++)
    {
      for (j = 0 ; j < nY ; j++)
      {
        p[0] = x[i];
        p[1] = y[j];
        p[2] = z[nZ-1];
        p += 3;
        outPointData->CopyData(inPointData, (nZ-1)*nX*nY + j*nX + i,pointId++);
      }
    }
  }

  // Bottom face
  for (i = 0 ; i < nX ; i++)
  {
    for (j = 1 ; j < nZ-1 ; j++)
    {
      p[0] = x[i];
      p[1] = y[0];
      p[2] = z[j];
      p += 3;
      outPointData->CopyData(inPointData, j*nX*nY + i, pointId++);
    }
  }

  // Top face
  if (nY > 1)
  {
    for (i = 0 ; i < nX ; i++)
    {
      for (j = 1 ; j < nZ-1 ; j++)
      {
        p[0] = x[i];
        p[1] = y[nY-1];
        p[2] = z[j];
        p += 3;
        outPointData->CopyData(inPointData, j*nX*nY + (nY-1)*nX + i,pointId++);
      }
    }
  }

  // Left face
  for (i = 1 ; i < nY-1 ; i++)
  {
    for (j = 1 ; j < nZ-1 ; j++)
    {
      p[0] = x[0];
      p[1] = y[i];
      p[2] = z[j];
      p += 3;
      outPointData->CopyData(inPointData, j*nX*nY + i*nX, pointId++);
    }
  }

  // Right face
  if (nX > 1)
  {
    for (i = 1 ; i < nY-1 ; i++)
    {
      for (j = 1 ; j < nZ-1 ; j++)
      {
        p[0] = x[nX-1];
        p[1] = y[i];
        p[2] = z[j];
        p += 3;
        outPointData->CopyData(inPointData, j*nX*nY + i*nX + nX-1, pointId++);
      }
    }
  }
  delete [] x;
  delete [] y;
  delete [] z;
  output->SetPoints(pts);
  pts->Delete();

  //
  // Our indexing of points can be nasty, so use the indexer set up 
  // specifically for this class and scheme.
  //
  SpecializedIndexer indexer(nX, nY, nZ);
  
  //
  // Have the cell data allocate memory.
  //
  int   numOutCells = 2*(nX-1)*(nY-1) + 2*(nX-1)*(nZ-1) + 2*(nY-1)*(nZ-1);
  outCellData->CopyAllocate(inCellData);

  vtkCellArray *polys = vtkCellArray::New();
  polys->Allocate(numOutCells*(4+1));
  
  //
  // Left face
  //
  int cellId = 0;
  for (i = 0 ; i < nY-1 ; i++)
  {
    for (j = 0 ; j < nZ-1 ; j++)
    {
      quad[0] = indexer.GetLeftFacePoint(i, j);
      quad[1] = indexer.GetLeftFacePoint(i, j+1);
      quad[2] = indexer.GetLeftFacePoint(i+1, j+1);
      quad[3] = indexer.GetLeftFacePoint(i+1, j);
      polys->InsertNextCell(4, quad);
      int cId = indexer.GetCellIndex(0, i, j);
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
        quad[0] = indexer.GetRightFacePoint(i, j);
        quad[1] = indexer.GetRightFacePoint(i+1, j);
        quad[2] = indexer.GetRightFacePoint(i+1, j+1);
        quad[3] = indexer.GetRightFacePoint(i, j+1);
        polys->InsertNextCell(4, quad);
        int cId = indexer.GetCellIndex(nX-2, i, j);
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
      quad[0] = indexer.GetBottomFacePoint(i, j);
      quad[1] = indexer.GetBottomFacePoint(i+1, j);
      quad[2] = indexer.GetBottomFacePoint(i+1, j+1);
      quad[3] = indexer.GetBottomFacePoint(i, j+1);
      polys->InsertNextCell(4, quad);
      int cId = indexer.GetCellIndex(i, 0, j);
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
        quad[0] = indexer.GetTopFacePoint(i, j);
        quad[1] = indexer.GetTopFacePoint(i, j+1);
        quad[2] = indexer.GetTopFacePoint(i+1, j+1);
        quad[3] = indexer.GetTopFacePoint(i+1, j);
        polys->InsertNextCell(4, quad);
        int cId = indexer.GetCellIndex(i, nY-2, j);
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
      quad[0] = indexer.GetFrontFacePoint(i, j);
      quad[1] = indexer.GetFrontFacePoint(i, j+1);
      quad[2] = indexer.GetFrontFacePoint(i+1, j+1);
      quad[3] = indexer.GetFrontFacePoint(i+1, j);
      polys->InsertNextCell(4, quad);
      int cId = indexer.GetCellIndex(i, j, 0);
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
        quad[0] = indexer.GetBackFacePoint(i, j);
        quad[1] = indexer.GetBackFacePoint(i+1, j);
        quad[2] = indexer.GetBackFacePoint(i+1, j+1);
        quad[3] = indexer.GetBackFacePoint(i, j+1);
        polys->InsertNextCell(4, quad);
        int cId = indexer.GetCellIndex(i, j, nZ-2);
        outCellData->CopyData(inCellData, cId, cellId);
        cellId++;
      }
    }
  }
  
  outCellData->Squeeze();
  output->SetPolys(polys);
  polys->Delete();
}


// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Tue Nov  4 13:34:38 PST 2003
//    Do a better job handling ghost zones.
//
// ****************************************************************************

void vtkRectilinearGridFacelistFilter::ConsolidationExecute(void)
{
  int  i;
  vtkDataSetRemoveGhostCells *ghost_remover = NULL;

  vtkRectilinearGrid *input        = GetInput();

  if (input->GetCellData()->GetArray("vtkGhostLevels") &&
      input->GetFieldData()->GetArray("avtRealDims"))
  {
      ghost_remover = vtkDataSetRemoveGhostCells::New();
      ghost_remover->SetInput(input);
      input = (vtkRectilinearGrid *) ghost_remover->GetOutput();
  }

  vtkPolyData        *output       = GetOutput();
  vtkCellData        *inCellData   = input->GetCellData();
  vtkPointData       *inPointData  = input->GetPointData();
  vtkCellData        *outCellData  = output->GetCellData();
  vtkPointData       *outPointData = output->GetPointData();

  int numOutCells;
  int numOutPoints;
  int (*quads)[4];
  int *ptIds;

  int nX = input->GetXCoordinates()->GetNumberOfTuples();
  int nY = input->GetYCoordinates()->GetNumberOfTuples();
  int nZ = input->GetZCoordinates()->GetNumberOfTuples();
  if (nZ > 1)
  {
      static int quads3[6][4] = { { 0, 1, 2, 3 }, { 0, 4, 5, 1 }, 
                                  { 1, 5, 6, 2 }, { 2, 6, 7, 3 },
                                  { 3, 7, 4, 0 }, { 4, 7, 6, 5 } };
      static int ptIds3[8];
      ptIds3[0] = 0;
      ptIds3[1] = nX-1;
      ptIds3[2] = nX*nY-1;
      ptIds3[3] = (nY-1)*nX;
      ptIds3[4] = (nX*nY)*(nZ-1);
      ptIds3[5] = nX-1 + (nX*nY)*(nZ-1);
      ptIds3[6] = nX*nY-1 + (nX*nY)*(nZ-1);
      ptIds3[7] = (nY-1)*nX + (nX*nY)*(nZ-1);

      numOutCells = 6;
      numOutPoints = 8;
      quads = quads3;
      ptIds = ptIds3;
  }
  else
  {
      static int quads2[1][4] = { { 0, 1, 2, 3 } };
      static int ptIds2[4];
      ptIds2[0] = 0;
      ptIds2[1] = nX-1;
      ptIds2[2] = nX*nY-1;
      ptIds2[3] = (nY-1)*nX;

      numOutCells  = 1;
      numOutPoints = 4;
      quads = quads2;
      ptIds = ptIds2;
  }

  vtkCellArray *polys = vtkCellArray::New();
  polys->Allocate(numOutCells*(4+1));

  for (i = 0 ; i < numOutCells ; i++)
      polys->InsertNextCell(4, quads[i]);
  
  outCellData->CopyAllocate(inCellData, numOutCells);
  for (i = 0 ; i < numOutCells ; i++)
      outCellData->CopyData(inCellData, 0, i);

  outPointData->CopyAllocate(inPointData, numOutPoints);
  vtkPoints *pts = vtkPoints::New();
  pts->SetNumberOfPoints(numOutPoints);
  for (i = 0 ; i < numOutPoints ; i++)
  {
      outPointData->CopyData(inPointData, ptIds[i], i);
      float pt[3];
      input->GetPoint(ptIds[i], pt);
      pts->SetPoint(i, pt);
  }

  output->SetPolys(polys);
  polys->Delete();
  output->SetPoints(pts);
  pts->Delete();
  if (output->GetCellData()->GetArray("vtkGhostLevels") != NULL)
      output->GetCellData()->RemoveArray("vtkGhostLevels");
  if (ghost_remover != NULL)
      ghost_remover->Delete();
}

void vtkRectilinearGridFacelistFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
