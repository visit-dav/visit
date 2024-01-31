// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "vtkLinesFromOriginalCells.h"

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkEdgeTable.h>
#include <vtkGenericCell.h>
#include <vtkPolyData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
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
  this->UseOriginalCells = false;
}

vtkLinesFromOriginalCells::~vtkLinesFromOriginalCells()
{
}

// ***************************************************************************
// Generate feature edges for mesh
//
//  Modifications:
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
//    Hank Childs, Thu Mar 10 09:48:47 PST 2005
//    Fix memory leak.
//
//    Eric Brugger, Wed Jan  9 11:44:42 PST 2013
//    Modified to inherit from vtkPolyDataAlgorithm.
//
//    Kathleen Biagas, Fri Jan 25 16:04:46 PST 2013
//    Call Update on the filter, not the data object.
//
//    Kathleen Biagas, Wed Sep 10 14:36:03 PDT 2014
//    Make a final attempt with vtkExtractEdges if other logic yields no cells.
//
//    Kathleen Biagas, Thu Mar 14 14:38:25 PDT 2019
//    Add VTK_LINE cells if they don't duplicate cell edges.
//
//    Kathleen Biagas, Wed Apr  3 17:46:48 PDT 2019
//    Modified Input to be vtkDataSet, and also add VTK_POLY_LINE, VTK_VERTEX
//    and VTK_POLY_VERTEX cells.
//
// ****************************************************************************
int vtkLinesFromOriginalCells::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the input info object
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);

  // get the input 
  vtkDataSet *input = vtkDataSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkIdType numCells, numPts; 
  //  Check input
  //
  numPts=input->GetNumberOfPoints();
  if ( (numCells=input->GetNumberOfCells()) < 1 || numPts < 1 )
  {
    return 1;
  }

  vtkDebugMacro(<<"Executing edge extractor");

  // get the output info object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the output 
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkCellData *inCD = input->GetCellData();
  vtkCellData *outCD = output->GetCellData();

  vtkPoints *pts2 = vtkVisItUtility::GetPoints(input);
  output->SetPoints(pts2);
  pts2->Delete();
  output->GetPointData()->ShallowCopy(input->GetPointData());

  vtkCellArray *newLines, *newVerts;
  vtkIdType newId;
  int edgeNum, numEdgePts, numCellEdges;
  int abort = 0;
  vtkIdType pts[2];
  vtkEdgeTable *edgeTable;
  vtkGenericCell *cell;
  vtkCell *edge;

  // Set up processing
  //
  edgeTable = vtkEdgeTable::New();
  edgeTable->InitEdgeInsertion(numPts);
  newLines = vtkCellArray::New();
  newLines->EstimateSize(numPts*4,2);
  newVerts = vtkCellArray::New();
  newVerts->EstimateSize(numPts,1);

  outCD->CopyAllocate(inCD,numCells);

  cell = vtkGenericCell::New();

  // Set up for OriginalCell usage, if requested
  unsigned int* origCellNums = NULL;
  unsigned int* cellNums3D = NULL;
  vtkPolyData *polys = NULL;
  if(this->UseOriginalCells && input->GetDataObjectType() == VTK_POLY_DATA)
  {
    vtkDataArray* origCellsArr = inCD->GetArray("avtOriginalCellNumbers");
    if ( (origCellsArr) && (origCellsArr->GetDataType() == VTK_UNSIGNED_INT)
      && (origCellsArr->GetNumberOfComponents() == 2))
    {
      origCellNums = ((vtkUnsignedIntArray*)origCellsArr)->GetPointer(0);
    }

    vtkDataArray* cellNums3DArr = inCD->GetArray("avt3DCellNumbers");
    if (cellNums3DArr && cellNums3DArr->GetDataType() == VTK_UNSIGNED_INT)
    {
      cellNums3D = ((vtkUnsignedIntArray*)cellNums3DArr)->GetPointer(0);
    }
    polys = vtkPolyData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
    polys->BuildLinks();  
  }

  // Loop over all cells, extracting non-visited edges.
  //
  vtkIdType tenth = numCells/10 + 1;

  vtkIdList *edgeNeighbors = vtkIdList::New();
  for (vtkIdType cellNum=0; cellNum < numCells && !abort; cellNum++ )
  {
    if ( ! (cellNum % tenth) ) //manage progress reports / early abort
    {
      this->UpdateProgress (static_cast<double>(cellNum) / numCells);
      abort = this->GetAbortExecute();
    }

    input->GetCell(cellNum,cell);
    numCellEdges = cell->GetNumberOfEdges();
    for (edgeNum=0; edgeNum < numCellEdges; edgeNum++ )
    {
      edge = cell->GetEdge(edgeNum);
      numEdgePts = edge->GetNumberOfPoints();
      pts[0] = edge->PointIds->GetId(0);
      for ( vtkIdType i=1; i < numEdgePts; i++, pts[0]=pts[1] )
      {
        pts[1] = edge->PointIds->GetId(i);
        if ( edgeTable->IsEdge(pts[0],pts[1]) == -1 )
        {
          bool insert = true;
          if(this->UseOriginalCells && polys)
          {
            polys->GetCellEdgeNeighbors(cellNum, pts[0], pts[1],edgeNeighbors);
            vtkIdType *neighborIdList = edgeNeighbors->GetPointer(0);
            for (vtkIdType k = 0; k < edgeNeighbors->GetNumberOfIds(); k++)
            {
              vtkIdType neighbor = neighborIdList[k];
              if (origCellNums)
              {
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
            }
          }
          if(insert)
          {
            edgeTable->InsertEdge(pts[0], pts[1]);
            newId = newLines->InsertNextCell(2,pts);
            outCD->CopyData(inCD, cellNum, newId);
          }
        }
      }
    }//for all edges of cell

    // Lines have 0 edges, so won't be processed by above logic.
    if(cell->GetCellType() == VTK_LINE)
    {
      // should these always be inserted?
      pts[0] = cell->GetPointId(0);
      pts[1] = cell->GetPointId(1);
      if ( edgeTable->IsEdge(pts[0],pts[1]) == -1 )
      {
        edgeTable->InsertEdge(pts[0], pts[1]);
        newId = newLines->InsertNextCell(2,pts);
        outCD->CopyData(inCD, cellNum, newId);
      }
    }
    else if(cell->GetCellType() == VTK_POLY_LINE)
    {
      vtkIdType numCellPts = cell->GetNumberOfPoints();
      pts[0] = cell->GetPointId(0);
      for (vtkIdType i = 1; i < numCellPts; ++i, pts[0] = pts[1])
      {
        pts[1] = cell->GetPointId(i);
        if ( edgeTable->IsEdge(pts[0], pts[1]) == -1 )
        {
          edgeTable->InsertEdge(pts[0], pts[1]);
          newId = newLines->InsertNextCell(2,pts);
          outCD->CopyData(inCD, cellNum, newId);
        }
      }
    }
    else if(cell->GetCellType() == VTK_VERTEX)
    {
      pts[0] = cell->GetPointId(0);
      newVerts->InsertNextCell(1, pts);
    }
    else if(cell->GetCellType() == VTK_POLY_VERTEX)
    {
      for (vtkIdType i = 0; i < cell->GetNumberOfPoints(); ++i) 
      {
        pts[0] = cell->GetPointId(i);
        newVerts->InsertNextCell(1, pts);
      }
    }

  }//for all cells

  vtkDebugMacro(<<"Created " << newLines->GetNumberOfCells() << " edges");

  //  Update ourselves.
  //
  edgeNeighbors->Delete();
  edgeTable->Delete();
  cell->Delete();

  output->SetLines(newLines);
  newLines->Delete();

  output->SetVerts(newVerts);
  newVerts->Delete();

  output->Squeeze();

  return 1;
}


//----------------------------------------------------------------------------
int vtkLinesFromOriginalCells::FillInputPortInformation(int, vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
  return 1;
}

//----------------------------------------------------------------------------
void vtkLinesFromOriginalCells::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Requesting use of OriginalCells: " << this->UseOriginalCells << "\n";
}
