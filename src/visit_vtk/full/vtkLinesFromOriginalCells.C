/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

#include "vtkLinesFromOriginalCells.h"

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkEdgeTable.h>
#include <vtkExtractEdges.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkMergePoints.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkVisItUtility.h>
#include <vtkPolyData.h>
#include <vtkUnsignedIntArray.h>

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
// ****************************************************************************
int vtkLinesFromOriginalCells::RequestData(
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
  vtkPolyData  *input = vtkPolyData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkCellData  *inCD   = input->GetCellData();
  vtkCellData  *outCD  = output->GetCellData();

  vtkPoints *pts2 = vtkVisItUtility::GetPoints(input);
  output->SetPoints(pts2);
  pts2->Delete();
  output->GetPointData()->ShallowCopy(input->GetPointData());

  vtkCellArray *newLines;
  vtkIdList *edgeNeighbors;
  int numCells, cellNum, edgeNum, numEdgePts, numCellEdges;
  int numPts, i, k, pt2, newId;
  vtkIdType pts[2] = { 0, 0 };
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
      extractor->SetInputData(input);
      extractor->Update();
      output->ShallowCopy(extractor->GetOutput());
      extractor->Delete();
      return 1;
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
    return 1;
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

  if (newLines->GetNumberOfCells() > 0)
  {
      output->SetLines(newLines);
      vtkDebugMacro(<<"Created " << newLines->GetNumberOfCells() << " edges");
  }
  else
  {
      // HACK to work-around problem with arb-poly data. The logic above
      // using 3DCellNumbers may not allow any edges to be added, so if that
      // is the case, use the edges filter.
      vtkDebugMacro(<<"Finding unique edges failed. Using vtkExtractEdges.");
      vtkExtractEdges *extractor = vtkExtractEdges::New();
      extractor->SetInputData(input);
      extractor->Update();
      output->ShallowCopy(extractor->GetOutput());
      extractor->Delete();
      vtkDebugMacro(<<"Created " << output->GetNumberOfCells() << " edges");
  }

  //
  //  Update ourselves.
  //
  edgeTable->Delete();
  edgeNeighbors->Delete();
  newLines->Delete();
  output->Squeeze();

  return 1;
}
