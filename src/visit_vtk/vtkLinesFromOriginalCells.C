#include "vtkLinesFromOriginalCells.h"

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkEdgeTable.h>
#include <vtkExtractEdges.h>
#include <vtkMergePoints.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkVisItUtility.h>
#include <vtkPolyData.h>
#include <vtkUnsignedIntArray.h>
#include <vtkVisItUtility.h>


//------------------------------------------------------------------------------
// Modifications:
//   Kathleen Bonnell, Wed Mar  6 15:14:29 PST 2002 
//   Replace 'New' method with Macro to match VTK 4.0 API.
//------------------------------------------------------------------------------
vtkStandardNewMacro(vtkLinesFromOriginalCells);

// Construct object.

//------------------------------------------------------------------------------
// Modifications:
//   Akira Haddox, Wed May 21 13:43:55 PDT 2003
//   Removed Locator code.
//------------------------------------------------------------------------------
vtkLinesFromOriginalCells::vtkLinesFromOriginalCells()
{
}

vtkLinesFromOriginalCells::~vtkLinesFromOriginalCells()
{
}

// ***************************************************************************
// Generate feature edges for mesh
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 13:22:36 PST 2001
//    Make pts of type vtkIdType to match VTK 4.0 API.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    Removed test for FieldData, as it is no longer a part of the
//    VTK 4.0 API at the CellData level. 
//
//    Kathleen Bonnell, Mon May 20 13:33:03 PDT 2002 
//    avtOriginalCellNumbers now has 2 components. 
//    
//    Jeremy Meredith, Tue Jul  9 14:00:53 PDT 2002
//    Added the code to make use of the 3DCellNumbers array.
//
//    Hank Childs, Tue Jul 23 10:15:40 PDT 2002
//    Clean up memory leak.
//
//    Hank Childs, Thu Sep 12 18:44:01 PDT 2002
//    Made the extractor be an automatic variable, since keeping it around
//    just bloats memory.
//
//    Akira Haddox, Wed May 21 13:44:23 PDT 2003
//    Removed the Locator calls (time consuming relic from vtkExtractLines).
//    Replaced some vtk calls with direct access calls for speed.
//
// ****************************************************************************

void vtkLinesFromOriginalCells::Execute()
{
  vtkPolyData  *input  = this->GetInput();
  vtkCellData  *inCD   = input->GetCellData();
  vtkPolyData  *output = this->GetOutput();
  vtkCellData  *outCD  = output->GetCellData();

  output->SetPoints(vtkVisItUtility::GetPoints(input));
  output->GetPointData()->ShallowCopy(input->GetPointData());

  vtkCellArray *newLines;
  vtkIdList *edgeNeighbors;
  int numCells, cellNum, edgeNum, numEdgePts, numCellEdges;
  int numPts, i, k, pt2, newId;
  vtkIdType pts[2];
  int pt1 = 0, neighbor;
  vtkEdgeTable *edgeTable;
  vtkCell *cell, *edge;
  bool insert;

  vtkDataArray* origCellsArr = inCD->GetArray("avtOriginalCellNumbers");
  if ( (!origCellsArr) || (origCellsArr->GetDataType() != VTK_UNSIGNED_INT)
    || (origCellsArr->GetNumberOfComponents() != 2))
  {
      vtkDebugMacro(<<"No proper match for OriginalCellNumbers found in "
                    "field data. Using vtkExtractEdges.");
      vtkExtractEdges *extractor = vtkExtractEdges::New();
      extractor->SetInput(input);
      extractor->GetOutput()->Update();
      output->ShallowCopy(extractor->GetOutput());
      extractor->Delete();
      return;
  }
  unsigned int* origCellNums =
      ((vtkUnsignedIntArray*)origCellsArr)->GetPointer(0);

  vtkDataArray* cellNums3DArr = inCD->GetArray("avt3DCellNumbers");
  unsigned int* cellNums3D = NULL;
  if (cellNums3DArr && cellNums3DArr->GetDataType() == VTK_UNSIGNED_INT)
  {
      cellNums3D = ((vtkUnsignedIntArray*)cellNums3DArr)->GetPointer(0);
  }

  //  Check input
  //
  input->BuildLinks();
  numPts=input->GetNumberOfPoints();
  numCells=input->GetNumberOfCells();
  if ( numCells < 1 || numPts < 1 )
    {
    vtkErrorMacro(<<"No input data!");
    return;
    }

  // Set up processing
  //
  edgeTable = vtkEdgeTable::New();
  edgeTable->InitEdgeInsertion(numPts);
  newLines = vtkCellArray::New();
  newLines->EstimateSize(numPts*4,2);

  outCD->CopyAllocate(outCD,numCells);

  edgeNeighbors = vtkIdList::New();
  // Loop over all cells, extracting non-visited edges. 
  //

  for (cellNum=0; cellNum < numCells; cellNum++ )
  {
      if ( ! (cellNum % 10000) ) //manage progress reports / early abort
      {
          this->UpdateProgress ((float)cellNum / numCells);
          if ( this->GetAbortExecute() ) 
          {
            break;
          }
      }

      cell = input->GetCell(cellNum);
      numCellEdges = cell->GetNumberOfEdges();
      for (edgeNum=0; edgeNum < numCellEdges; edgeNum++ )
      {
          edge = cell->GetEdge(edgeNum);
          numEdgePts = edge->GetNumberOfPoints();
      
          for ( i=0; i < numEdgePts; i++, pt1=pt2, pts[0]=pts[1] )
          {
              pt2 = edge->PointIds->GetId(i);
              pts[1] = pt2;
              
              if ( i > 0 && edgeTable->IsEdge(pt1,pt2) == -1 )
              {
                  insert = true;
                  input->GetCellEdgeNeighbors(cellNum, pt1, pt2,edgeNeighbors);

                  vtkIdType *neighborIdList = edgeNeighbors->GetPointer(0);
                  for (k = 0; k < edgeNeighbors->GetNumberOfIds(); k++)
                  {
                      neighbor = neighborIdList[k];
                      if (origCellNums[2*cellNum+1] == origCellNums[2*neighbor+1]
                          && (!cellNums3D || cellNums3D[cellNum] != cellNums3D[neighbor]))
                      {
                          // don't insert this edge if any of its neighbors 
                          // which were generated from different 3D cells
                          // belong to the same *original* cell.

                          // NOTE -- the additional cellNums3D logic was
                          // put in because all 3D mesh plots now go through
                          // this filter, and without the extra logic,
                          // mesh lines at corners disappear.
                          // It also corrects some corner mesh lines
                          // disappearing on subdivided meshes (e.g. MIR),
                          // though not all.
                          insert = false;
                          break;
                      }
                  }
                  if (insert)
                  {
                      edgeTable->InsertEdge(pt1, pt2);
                      newId = newLines->InsertNextCell(2,pts);
                      outCD->CopyData(inCD, cellNum, newId);
                  }
              } // if unique edge
          } // for all edge points
      }//for all edges of cell
  }//for all cells

  vtkDebugMacro(<<"Created " << newLines->GetNumberOfCells() << " edges");

  //
  //  Update ourselves.
  //
  edgeTable->Delete();
  edgeNeighbors->Delete();

  output->SetLines(newLines);
  newLines->Delete();

  output->Squeeze();
}

