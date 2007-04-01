#include <vtkRectilinearGridFacelistFilter.h>

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkUnsignedCharArray.h>


using  std::vector;


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

// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Wed Aug 25 16:28:52 PDT 2004
//    Account for degenerate meshes.
//
// ****************************************************************************

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

    if (nX > 1)
       cellYBase = (nX-1);
    else
       cellYBase = 1;

    if (nY > 1 && nX > 1)
       cellZBase = (nY-1)*(nX-1);
    else if (nX > 1)
       cellZBase = nX-1;
    else if (nY > 1)
       cellZBase = nY-1;
    else
       cellZBase = 1;
}


// ****************************************************************************
//
//  Hank Childs, Thu Aug 15 21:13:43 PDT 2002
//  Fixed bug where cell data was being copied incorrectly.
//
//  Hank Childs, Wed Oct 15 19:24:56 PDT 2003
//  Added logic for consolidating faces.
//
//  Hank Childs, Sun Nov  9 09:44:04 PST 2003
//  Made logic for consolidating faces when ghost zones are involved a little
//  more sophisticated.  Also re-ordered how some of the quads in the normal
//  execution were stored, to make the consolidation logic easier.  The new
//  ordering is also more consistent.
//
//  Hank Childs, Fri Jan 30 08:31:44 PST 2004
//  Use pointer arithmetic to construct poly data output.
//
//  Hank Childs, Sun Feb  1 22:02:51 PST 2004
//  Do a better job of estimating the number of cells in the 2D case.
//
//  Hank Childs, Wed Aug 25 16:30:30 PDT 2004
//  Do a better job of handling degenerate meshes.
//
//  Hank Childs, Fri Aug 27 15:15:20 PDT 2004
//  Rename ghost data array.
//
//  Brad Whitlock, Fri Oct 1 17:03:53 PST 2004
//  Pass field data through to the output.
//
//  Hank Childs, Wed Nov 10 11:30:03 PST 2004
//  Correct problem where we are over-allocating number of points for 2D case.
//
//  Hank Childs, Sun Mar 13 11:09:16 PST 2005
//  Fix memory leak.
//
// ****************************************************************************

void vtkRectilinearGridFacelistFilter::Execute()
{
  int   i, j;

  //
  // Set up some objects that we will be using throughout the process.
  //
  vtkRectilinearGrid *input        = GetInput();
  vtkPolyData        *output       = vtkPolyData::New();
  vtkCellData        *inCellData   = input->GetCellData();
  vtkPointData       *inPointData  = input->GetPointData();
  vtkCellData        *outCellData  = output->GetCellData();
  vtkPointData       *outPointData = output->GetPointData();

  //
  // If there are no ghost zones and we want to consolidate faces, this is a
  // very easy problem.  Just call the routine that will do this simply.
  //
  if (ForceFaceConsolidation)
  {
     if (inCellData->GetArray("avtGhostZones") == NULL)
     {
        ConsolidateFacesWithoutGhostZones();
        output->Delete();
        return;
     }
  }  

  //
  // If we are doing face compaction, we will come in after the fact and
  // compact some of the faces.  These variables are used for telling the
  // face compaction routine about how the faces are laid out.
  //
  vector<int> faceStart;
  vector<int> rowSize;
  vector<int> columnSize;

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
    npts = nY*nZ;
  }
  else if (nY <= 1)
  {
    npts = nX*nZ;
  }
  else if (nZ <= 1)
  {
    npts = nX*nY;
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
  outCellData->CopyAllocate(inCellData);

  vtkCellArray *polys = vtkCellArray::New();
  vtkIdTypeArray *list = vtkIdTypeArray::New();
  list->SetNumberOfValues(numOutCells*(4+1));
  vtkIdType *nl = list->GetPointer(0);
  
  //
  // Left face
  //
  int cellId = 0;
  faceStart.push_back(cellId);
  columnSize.push_back(nZ-1);
  rowSize.push_back(nY-1);
  for (j = 0 ; j < nZ-1 ; j++)
  {
    for (i = 0 ; i < nY-1 ; i++)
    {
      *nl++ = 4;
      *nl++ = indexer.GetLeftFacePoint(i, j);
      *nl++ = indexer.GetLeftFacePoint(i, j+1);
      *nl++ = indexer.GetLeftFacePoint(i+1, j+1);
      *nl++ = indexer.GetLeftFacePoint(i+1, j);
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
    faceStart.push_back(cellId);
    rowSize.push_back(nZ-1);
    columnSize.push_back(nY-1);
    for (i = 0 ; i < nY-1 ; i++)
    {
      for (j = 0 ; j < nZ-1 ; j++) 
      {
        *nl++ = 4;
        *nl++ = indexer.GetRightFacePoint(i, j);
        *nl++ = indexer.GetRightFacePoint(i+1, j);
        *nl++ = indexer.GetRightFacePoint(i+1, j+1);
        *nl++ = indexer.GetRightFacePoint(i, j+1);
        int cId = indexer.GetCellIndex(nX-2, i, j);
        outCellData->CopyData(inCellData, cId, cellId);
        cellId++;
      }
    }
  }
  
  //
  // Bottom face
  //
  faceStart.push_back(cellId);
  rowSize.push_back(nZ-1);
  columnSize.push_back(nX-1);
  for (i = 0 ; i < nX-1 ; i++)
  {
    for (j = 0 ; j < nZ-1 ; j++)
    {
      *nl++ = 4;
      *nl++ = indexer.GetBottomFacePoint(i, j);
      *nl++ = indexer.GetBottomFacePoint(i+1, j);
      *nl++ = indexer.GetBottomFacePoint(i+1, j+1);
      *nl++ = indexer.GetBottomFacePoint(i, j+1);
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
    faceStart.push_back(cellId);
    rowSize.push_back(nX-1);
    columnSize.push_back(nZ-1);
    for (j = 0 ; j < nZ-1 ; j++)
    {
      for (i = 0 ; i < nX-1 ; i++)
      {
        *nl++ = 4;
        *nl++ = indexer.GetTopFacePoint(i, j);
        *nl++ = indexer.GetTopFacePoint(i, j+1);
        *nl++ = indexer.GetTopFacePoint(i+1, j+1);
        *nl++ = indexer.GetTopFacePoint(i+1, j);
        int cId = indexer.GetCellIndex(i, nY-2, j);
        outCellData->CopyData(inCellData, cId, cellId);
        cellId++;
      }
    }
  }
  
  //
  // Back face
  //
  faceStart.push_back(cellId);
  rowSize.push_back(nX-1);
  columnSize.push_back(nY-1);
  for (j = 0 ; j < nY-1 ; j++)
  {
    for (i = 0 ; i < nX-1 ; i++)
    {
      *nl++ = 4;
      *nl++ = indexer.GetFrontFacePoint(i, j);
      *nl++ = indexer.GetFrontFacePoint(i, j+1);
      *nl++ = indexer.GetFrontFacePoint(i+1, j+1);
      *nl++ = indexer.GetFrontFacePoint(i+1, j);
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
    faceStart.push_back(cellId);
    rowSize.push_back(nY-1);
    columnSize.push_back(nX-1);
    for (i = 0 ; i < nX-1 ; i++)
    {
      for (j = 0 ; j < nY-1 ; j++)
      {
        *nl++ = 4;
        *nl++ = indexer.GetBackFacePoint(i, j);
        *nl++ = indexer.GetBackFacePoint(i+1, j);
        *nl++ = indexer.GetBackFacePoint(i+1, j+1);
        *nl++ = indexer.GetBackFacePoint(i, j+1);
        int cId = indexer.GetCellIndex(i, j, nZ-2);
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

  if (ForceFaceConsolidation)
  {
     //
     // We only get to this spot if we have ghost zones -- which makes 
     // consolidating faces a harder problem.  Use a sub-routine to do that.
     //
     vtkPolyData *new_output = ConsolidateFacesWithGhostZones(output, 
                                               faceStart, rowSize, columnSize);
     output->Delete();
     output = new_output;
  }

  GetOutput()->GetFieldData()->ShallowCopy(GetInput()->GetFieldData());
  GetOutput()->ShallowCopy(output);
  output->Delete();
}


// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Fri Aug 27 15:15:20 PDT 2004
//    Rename ghost data array.
//
// ****************************************************************************

vtkPolyData *
vtkRectilinearGridFacelistFilter::ConsolidateFacesWithGhostZones(
                                 vtkPolyData *pd, vector<int> &sideStart,
                                 vector<int> &rowSize, vector<int> &columnSize)
{
  //
  // The output will have identical point information to our input.  So copy
  // that over now.
  //
  vtkPolyData *cpd = vtkPolyData::New();
  cpd->SetPoints(pd->GetPoints());
  cpd->GetPointData()->ShallowCopy(pd->GetPointData());

  //
  // Set up some useful vars for later.
  //
  vtkCellData *inCellData  = pd->GetCellData();
  vtkCellData *outCellData = cpd->GetCellData();
  vtkUnsignedCharArray *gza = (vtkUnsignedCharArray *) 
                                 inCellData->GetArray("avtGhostZones");

  //
  // We will be modifying the cells.  So set up some of the data structures.
  //
  outCellData->CopyAllocate(inCellData);
  int cellGuess = pd->GetNumberOfCells();
  vtkCellArray *polys = vtkCellArray::New();
  polys->Allocate(cellGuess*(4+1));
  
  //
  // These for loops will walk through the cells and try to identify 
  // neighboring cells that can be compacted.  Although the algorithm appears
  // to be quite slow (there are 5 nested for loops), it should actually run
  // in about O(nfaces) time.
  //
  int nOutputCells = 0;
  int nSides = sideStart.size();
  for (int i = 0 ; i < nSides ; i++)
  {
    int nEntries = rowSize[i]*columnSize[i];
    int startFace = sideStart[i];
    vector<bool> faceUsed(nEntries, false);
    for (int k = 0 ; k < columnSize[i] ; k++)
    {
      for (int j = 0 ; j < rowSize[i] ; j++)
      {
        int face = k*rowSize[i] + j;
        if (faceUsed[face])
           continue;
        unsigned char gz_standard = gza->GetValue(startFace+face);

        //
        // Find out how far we can go along the row with the same ghost
        // zone value.
        //
        int lastRowMatch = j;
        int l, m;
        for (l = j+1 ; l < rowSize[i] ; l++)
        {
           int face = k*rowSize[i] + l;
           if (faceUsed[face])
             break;
           unsigned char gz_current = gza->GetValue(startFace+face);
           if (gz_current != gz_standard)
             break;
           lastRowMatch = l;
        }

        //
        // Now we know we can go from k - lastRowMatch with the same ghost zone
        // value.  Now see how far we can down in columns.
        //
        int lastColumnMatch = k;
        for (m = k+1 ; m < columnSize[i] ; m++)
        {
          bool all_matches = true;
          for (l = j ; l <= lastRowMatch ; l++)
          {
            int face = m*rowSize[i] + l;
            if (faceUsed[face])
            {
              all_matches = false;
              break;
            }
            unsigned char gz_current = gza->GetValue(startFace+face);
            if (gz_current != gz_standard)
            {
              all_matches = false;
              break;
            }
          }
            
          if (all_matches)
            lastColumnMatch = m;
          else
            break;
        }

        for (l = j ; l <= lastRowMatch ; l++)
          for (m = k ; m <= lastColumnMatch ; m++)
          {
            int face = m*rowSize[i] + l;
            faceUsed[face] = true;
          }

        //
        // We know now that we have a face that hasn't been used yet --
        // face (j, k).  In addition, we know that we can form a bigger quad
        // with face (j, k) as one corner and 
        // face (lastRowMatch, lastColumnMatch) as the opposite corner.
        //
        int quad_index[4];
        quad_index[0] = startFace + k*rowSize[i]+j;  // bottom-left
        quad_index[1] = startFace + lastColumnMatch*rowSize[i]+j; // top-left
        quad_index[2] = startFace 
                       + lastColumnMatch*rowSize[i]+lastRowMatch; // top-right
        quad_index[3] = startFace + k*rowSize[i]+lastRowMatch; // bottom-right

        //
        // We've constructed quad_index so that we want the l'th point from
        // quad 'l' to make our new, bigger quad.  Note that this heavily
        // depends on the quads in "Execute" being constructed in a consistent
        // way.
        //
        vtkIdType   quad[4];
        for (l = 0 ; l < 4 ; l++)
        {
          quad[l] = pd->GetCell(quad_index[l])->GetPointId(l);
        }
        polys->InsertNextCell(4, quad);

        //
        // Copy over the new cell data, too.  Just copy the cell data from
        // the original face -> (j, k).
        //
        outCellData->CopyData(inCellData, quad_index[0], nOutputCells++);
      }
    }
  }

  cpd->SetPolys(polys);
  polys->Squeeze();
  cpd->GetCellData()->Squeeze();
  polys->Delete();
  
  return cpd;
}


// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Sun Nov  9 12:37:15 PST 2003
//    Modified this routine to not handle ghost zones at all (since it wasn't
//    doing a very good job in the first place).  Also renamed the routine
//    to make it clear what its purpose was.
//
// ****************************************************************************

void vtkRectilinearGridFacelistFilter::ConsolidateFacesWithoutGhostZones(void)
{
  int  i;

  vtkRectilinearGrid *input        = GetInput();
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
}

void vtkRectilinearGridFacelistFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
