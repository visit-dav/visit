/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkPolyDataRelevantPointsFilter.C,v $
  Language:  C++
  Date:      $Date: 2000/05/17 16:05:12 $
  Version:   $Revision: 1.54 $


Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen 
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
ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include "vtkPolyDataRelevantPointsFilter.h"
#include "vtkObjectFactory.h"
#include <vtkPolyData.h>

//-----------------------------------------------------------------------------
// Modifications:
//   Kathleen Bonnell, Wed Mar  6 15:14:29 PST 2002 
//   Replace 'New' method with Macro to match VTK 4.0 API.
//-----------------------------------------------------------------------------

vtkStandardNewMacro(vtkPolyDataRelevantPointsFilter);


// ***************************************************************************
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 13:22:36 PST 2001
//    Make cells of type vtkIdType to match VTK 4.0 API.
//
//    Hank Childs, Tue Aug 13 17:47:20 PDT 2002
//    Re-wrote to increase performance.
//
//    Hank Childs, Mon Sep  2 18:02:46 PDT 2002
//    Used a different set of VTK calls to avoid odd bug when transitioning
//    between quads and triangles.
//
//    Hank Childs, Wed Oct 16 14:16:56 PDT 2002
//    Fix silly bug that let too many points through.
//
// ****************************************************************************

void vtkPolyDataRelevantPointsFilter::Execute()
{
  int   i, j, k;

  //
  // Initialize some frequently used values.
  //
  vtkPolyData  *input  = this->GetInput();
  vtkPolyData  *output = this->GetOutput();
  vtkPoints *inPts  = input->GetPoints();
  int numPts        = input->GetNumberOfPoints();
  int numCells      = input->GetNumberOfCells();
  
  //
  // Sanity checks
  //
  vtkDebugMacro(<<"Beginning PolyData Relevant Points Filter ");
  if (input == NULL) {
      vtkErrorMacro(<<"Input is NULL");
      return;
  }
  if ( (numPts<1) || (inPts == NULL ) ) {
      vtkErrorMacro(<<"No data to Operate On!");
      return;
  }
 
  //
  // The cells will be largely unaffected, so pass them straight through.
  //
  output->Allocate(numCells);
  vtkCellData  *inputCD  = input->GetCellData();
  vtkCellData  *outputCD = output->GetCellData();
  outputCD->PassData(inputCD);
  
  //
  // First set up some of the constructs that will be used to create a mapping
  // between the old point indices and the new point indices.
  //
  int numNewPts = 0;
  int *oldToNew = new int[numPts];
  int *newToOld = new int[numPts];
  for (i = 0; i < numPts; i++)
    {
    oldToNew[i] = -1;
    }
  vtkCellArray *arrays[4];
  arrays[0] = input->GetVerts();
  arrays[1] = input->GetLines();
  arrays[2] = input->GetPolys();
  arrays[3] = input->GetStrips();

  //
  // Walk through all of the poly data types and mark the points we see.
  // Create a mapping as we go of old points to new points and another one of
  // new points to old points.
  //
  for (i = 0 ; i < 4 ; i++)
    {
    int ncells = arrays[i]->GetNumberOfCells();
    vtkIdType *ptr = arrays[i]->GetPointer();
    for (j = 0 ; j < ncells ; j++)
      {
      int npts = *ptr++;
      for (k = 0 ; k < npts ; k++)
        {
        int oldPt = *ptr++;
        if (oldToNew[oldPt] == -1)
          {
          newToOld[numNewPts] = oldPt;
          oldToNew[oldPt] = numNewPts;
          numNewPts++;
          }
        }
      }
    }

  //
  // Now create a new vtkPoints construct that reflects the mapping we created.
  // This is a bit ugly (re: pointer arithmetic) in the interest of 
  // performance.
  //
  vtkPoints *newPts = vtkPoints::New();
  newPts->SetNumberOfPoints(numNewPts);
  vtkPointData *inputPD = input->GetPointData();
  vtkPointData *outputPD = output->GetPointData();
  bool havePointVars = (inputPD->GetNumberOfArrays() > 0);
  if (havePointVars)
    {
    outputPD->CopyAllocate(inputPD, numNewPts);
    }
  float *in_ptr = (float *) inPts->GetVoidPointer(0);
  float *out_ptr = (float *) newPts->GetVoidPointer(0);
  int out_index = 0;
  for (i = 0 ; i < numNewPts ; i++)
    {
    int in_index  = 3*newToOld[i];
    out_ptr[out_index++] = in_ptr[in_index++];
    out_ptr[out_index++] = in_ptr[in_index++];
    out_ptr[out_index++] = in_ptr[in_index];
    if (havePointVars)
      {
      outputPD->CopyData(inputPD, newToOld[i], i);
      }
    }
  output->SetPoints(newPts);
  newPts->Delete();

  //
  // Now work through cells, changing associated point id to coincide
  // with the new ones as specified in the pointmap.
  //
  vtkIdType pts[1024]; // If we ever see a cell with more than 1024 pts, this
                       // may (will) lead to a problem.
  vtkIdType *oldPts = NULL;
  int nids = 0;
  input->BuildCells();
  for (i = 0; i < numCells; i++) 
    {
    input->GetCellPoints(i, nids, oldPts);
    int cellType = input->GetCellType(i);
    for (j = 0; j < nids; j++)
      {
      pts[j] = oldToNew[oldPts[j]];
      }
    output->InsertNextCell(cellType, nids, pts);
    }
  delete [] oldToNew;
  delete [] newToOld;
}

//-----------------------------------------------------------------------------
void vtkPolyDataRelevantPointsFilter::PrintSelf(ostream& os, vtkIndent indent) 
{
  this->Superclass::PrintSelf(os,indent);
}
