/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkDataSetRemoveGhostCells.C,v $
  Language:  C++
  Date:      $Date: 2001/05/01 20:49:40 $
  Version:   $Revision: 1.1 $


Copyright (c) 1993-2001 Ken Martin, Will Schroeder, Bill Lorensen 
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither name of Ken Martin, Will Schroeder, or Bill Lorensen nor the names
   of any contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include "vtkDataSetRemoveGhostCells.h"

#include <vtkCellData.h>
#include <vtkExtractGrid.h>
#include <vtkExtractRectilinearGrid.h>
#include <vtkFloatArray.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnstructuredGrid.h>

//------------------------------------------------------------------------------
// Modifications:
//   Kathleen Bonnell, Wed Mar  6 13:48:48 PST 2002
//   Use vtk Macro instead of old New method.
//------------------------------------------------------------------------------
vtkStandardNewMacro(vtkDataSetRemoveGhostCells);

// Construct with ghost level = 1.
vtkDataSetRemoveGhostCells::vtkDataSetRemoveGhostCells()
{
  this->GhostLevel = 1;
}


void vtkDataSetRemoveGhostCells::Execute()
{
  vtkDataSet *input = this->GetInput();

  switch(input->GetDataObjectType())
  {

      case VTK_RECTILINEAR_GRID :
          RectilinearGridExecute();
          break;
      case VTK_POLY_DATA :
          PolyDataExecute();
          break;

      case VTK_UNSTRUCTURED_GRID :
          UnstructuredGridExecute();
          break;

      case VTK_STRUCTURED_GRID :
          StructuredGridExecute();
          break;

      default: 
          vtkDebugMacro(<<"vtkDataSetRemoveGhostCells not set up to "
                          "operate on this data type" );
          vtkDataSet *output = this->GetOutput();
          output->DeepCopy(input); 
          break;
  }
}

// ***************************************************************************
//  Modifications:
//
//    Kathleen Bonnell, Fri Aug 31 08:50:30 PDT 2001
//    Removed check of field data.  No longer necessary with new vtk.
//
//    Hank Childs, Fri Oct 19 08:33:28 PDT 2001
//    Fix memory leak.
//
//    Hank Childs, Sat Jun 29 16:27:41 PDT 2002
//    Copy over field data.
//
//    Kathleen Bonnell, Wed Jul 10 16:02:56 PDT 2002 
//    Reworked code to utilize vtk RemoveGhostCells method. 
//
// ****************************************************************************

void vtkDataSetRemoveGhostCells::UnstructuredGridExecute()
{
  vtkDebugMacro(<< "Executing remove ghost cells filter for unstructured grid");
 
  vtkUnstructuredGrid *input  = (vtkUnstructuredGrid*)this->GetInput();
  vtkUnstructuredGrid *output = (vtkUnstructuredGrid*)this->GetOutput();
 
  output->ShallowCopy(input);
  output->RemoveGhostCells(this->GhostLevel);
  output->GetCellData()->RemoveArray("vtkGhostLevels");
}


// ***************************************************************************
//  Modifications:
//
//    Kathleen Bonnell, Fri Aug 31 08:50:30 PDT 2001
//    Removed check of field data, as it is no longer necessary with VTK 4.0 
//    Made pts of type vtkIdType to match VTK 4.0 API.
//
//    Hank Childs, Sat Jun 29 16:27:41 PDT 2002
//    Copy over field data.
//
//    Kathleen Bonnell, Wed Jul 10 16:02:56 PDT 2002 
//    Reworked code to utilize vtk RemoveGhostCells method. 
//
//    Hank Childs, Fri Sep 13 20:19:52 PDT 2002
//    Reverted back to removing ghost zones by hand since the VTK
//    implementation is buggy.
//
//    Hank Childs, Sun Jun 27 09:49:22 PDT 2004
//    Account for ghost nodes as well as ghost zones.
//
//    Jeremy Meredith, Wed Jul 21 17:37:09 PDT 2004
//    I added a new value for ghost nodes that has new meaning.  Specifically
//    *all* nodes had to be nonzero in a cell for that cell to be removed.
//    With my changes, if *any* node for a cell is "2", then that cell is
//    removed.  This is consistent with the concept of IBLANKing for nodal
//    data, because when a nodal value is meant to be hidden, you must remove
//    any cells that contain that node.  There are other uses for ghost
//    nodes, in particular for removing the polygons between domains.  See
//    http://www.nas.nasa.gov/FAST/RND-93-010.walatka-clucas/htmldocs/chp5.file_io.html
//    for a discussion of this -- the relevant pieces have been pasted into
//    avtOVERFLOWFileFormat::GetMesh.
//
//    Hank Childs, Wed Aug 11 15:33:31 PDT 2004
//    Allow for the presence of both ghost zones and ghost nodes.  This can
//    happen when ghost zones are created by the database and then ghost nodes
//    are created by the reflect filter, for example.  Also remove ghost nodes
//    array when we are done with it, since it just slows down processing.
//
// ***************************************************************************

void vtkDataSetRemoveGhostCells::PolyDataExecute()
{
  vtkPolyData *input   = (vtkPolyData*) this->GetInput();
  vtkPolyData *output  = (vtkPolyData*) this->GetOutput();

  vtkUnsignedCharArray *ghost_zones = (vtkUnsignedCharArray *)
                              input->GetCellData()->GetArray("vtkGhostLevels");

  vtkUnsignedCharArray *ghost_nodes = (vtkUnsignedCharArray *)
                              input->GetPointData()->GetArray("vtkGhostNodes");

  if ((ghost_zones == NULL) && (ghost_nodes == NULL))
    {
    output->ShallowCopy(input);
    return;
    }

  output->SetPoints(input->GetPoints());
  output->GetPointData()->PassData(input->GetPointData());
  output->GetFieldData()->PassData(input->GetFieldData());

  vtkCellData *inCD  = input->GetCellData();
  vtkCellData *outCD = output->GetCellData();
  outCD->CopyAllocate(inCD);
 
  bool usingGhostZones = (ghost_zones != NULL);
  bool usingGhostNodes = (ghost_nodes != NULL);
  unsigned char *zone_ptr = NULL;
  unsigned char *node_ptr = NULL;
  if (usingGhostZones)
    zone_ptr = ghost_zones->GetPointer(0);
  if (usingGhostNodes)
    node_ptr = ghost_nodes->GetPointer(0);

  int nCells = input->GetNumberOfCells();
  output->Allocate(nCells);
  input->BuildCells();
  vtkIdType npts;
  vtkIdType *pts;
  int cell = 0;
  for (int i = 0 ; i < nCells ; i++)
    {
    if (usingGhostZones)
      {
      if (zone_ptr[i] > 0)
        {
        continue;
        }
      }

    input->GetCellPoints(i, npts, pts);

    if (usingGhostNodes)
      {
      bool haveOneLevel0Node = false;
      bool haveOneLevel2Node = false;
      for (int j = 0 ; j < npts ; j++)
        {
        if (node_ptr[pts[j]] == 0)
            haveOneLevel0Node = true;
        if (node_ptr[pts[j]] == 2)
            haveOneLevel2Node = true;
        }
      if (!haveOneLevel0Node || haveOneLevel2Node)
        continue;
      }

    output->InsertNextCell(input->GetCellType(i), npts, pts);
    outCD->CopyData(inCD, i, cell++);
    }
  output->GetCellData()->RemoveArray("vtkGhostLevels");
  output->GetPointData()->RemoveArray("vtkGhostNodes");
  output->Squeeze();
}


// ***************************************************************************
//  Modifications:
//
//    Kathleen Bonnell, Fri Aug 31 08:50:30 PDT 2001
//    VTK 4.0 compatibility issues: Removed check of field data.  
//    Use vtkDataArray for rectilinear grid coordinates instead of vtkScalars,
//    use correct access methods for this array.
//
//    Hank Childs, Sat Jun 29 16:27:41 PDT 2002
//    Copy over field data and set the dimensions.
//
//    Kathleen Bonnell, Wed Jul 10 16:02:56 PDT 2002 
//    Reworked code to utilize vtk ExtractRectilinearGrid filter. 
//    Use new avtRealDims array.
//
//    Hank Childs, Sun Nov  9 13:19:45 PST 2003
//    Tell output that it doesn't have ghost zones, even if we couldn't handle
//    it correctly.  This makes the life of the wireframe filter much easier.
//
// ***************************************************************************

void vtkDataSetRemoveGhostCells::RectilinearGridExecute()
{
  vtkRectilinearGrid *input  = (vtkRectilinearGrid*)this->GetInput();
  vtkRectilinearGrid *output  = (vtkRectilinearGrid*)this->GetOutput();
 
  vtkDataArray *realDims = input->GetFieldData()->GetArray("avtRealDims");
 
  if (!realDims || (realDims->GetDataType() != VTK_INT)
    || (realDims->GetNumberOfComponents() != 1))
    {
    vtkErrorMacro(<<"No proper match for avtRealDims found in field data.");
    output->ShallowCopy(input);
    output->GetCellData()->RemoveArray("vtkGhostLevels");
    return;
    }

  vtkDebugMacro(<< "Executing remove ghost cells filter for rectilinear grid");

  int i, voi[6];
  for (i = 0; i < 6; i++)
    {
    voi[i] = (int) realDims->GetComponent(i, 0);
    }
 
  vtkExtractRectilinearGrid *extractor = vtkExtractRectilinearGrid::New();
  extractor->SetInput(input);
  extractor->SetVOI(voi);
  extractor->GetOutput()->Update();
 
  output->ShallowCopy(extractor->GetOutput());
  extractor->Delete();

  output->GetFieldData()->RemoveArray("avtRealDims");
  output->GetCellData()->RemoveArray("vtkGhostLevels");
}


// ***************************************************************************
//  Modifications:
//
//    Kathleen Bonnell, Fri Aug 31 08:50:30 PDT 2001
//    Removed check of field data.  No longer necessary with new vtk.
//
//    Hank Childs, Sun Apr 14 10:12:30 PDT 2002
//    Removed SetOutput call, due to a weird bug with VTK 4.0.  It wasn't
//    needed in the first place, anyway.
//
//    Hank Childs, Sat Jun 29 16:27:41 PDT 2002
//    Copy over field data and set the dimensions.
//
//    Kathleen Bonnell, Wed Jul 10 16:02:56 PDT 2002 
//    Reworked code to properly use vtkExtractGrid filter. 
//    Use new avtRealDims array.
//
// ***************************************************************************

void vtkDataSetRemoveGhostCells::StructuredGridExecute()
{
  vtkStructuredGrid *input  = (vtkStructuredGrid*)this->GetInput();
  vtkStructuredGrid *output = (vtkStructuredGrid*)this->GetOutput();
 
  vtkDataArray *realDims = input->GetFieldData()->GetArray("avtRealDims");
  if (!realDims || (realDims->GetDataType() != VTK_INT)
    || (realDims->GetNumberOfComponents() != 1))
    {
    vtkErrorMacro(<<"No proper match for avtRealDims found in field data.");
    output->ShallowCopy(input);
    return;
    }

  vtkDebugMacro(<< "Executing remove ghost cells filter for structured grid");
 
   int i, voi[6];
   for (i = 0; i < 6; i++)
     {
     voi[i] = (int) realDims->GetComponent(i, 0);
     }
 
  vtkExtractGrid *extractor = vtkExtractGrid::New();
  extractor->SetInput(input);
  extractor->SetVOI(voi);
  extractor->GetOutput()->Update();
 
  output->ShallowCopy(extractor->GetOutput());
  extractor->Delete();
 
  output->GetFieldData()->RemoveArray("avtRealDims");
  output->GetCellData()->RemoveArray("vtkGhostLevels");
}

// *******************************************************************
// Modifications:
//   Kathleen Bonnell, Wed Mar  6 13:48:48 PST 2002
//   Call superclass's method the new vtk way.
// ******************************************************************
void vtkDataSetRemoveGhostCells::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Ghost Level: " << this->GhostLevel << "\n";;
}
