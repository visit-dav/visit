/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVertexFilter.cxx,v $
  Language:  C++
  Date:      $Date: 2001/08/30 14:10:58 $
  Version:   $Revision: 1.1 $
  Thanks:    Hank Childs, B Division, Lawrence Livermore Nat'l Laboratory

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

#include "vtkVertexFilter.h"

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>

// **************************************************************************
//  Modifications:
//    Kathleen Bonnell, Wed Mar  6 17:10:03 PST 2002 
//    Replace 'New' method with Macro to match VTK 4.0 API.
// **************************************************************************

vtkStandardNewMacro(vtkVertexFilter);


vtkVertexFilter::vtkVertexFilter()
{
    VertexAtPoints = 1;
}


// ***************************************************************************
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 13:22:36 PST 2001
//    Make onevertex of type vtkIdType to match VTK 4.0 API.
//
//    Eric Brugger, Tue May 14 15:27:24 PDT 2002
//    Modified to work properly with cell centered variables.
//
// ****************************************************************************

void vtkVertexFilter::Execute(void)
{
  int   i, j;

  vtkDataSet  *input  = this->GetInput();
  vtkPolyData *output = this->GetOutput();

  vtkCellData  *inCd = input->GetCellData();
  vtkPointData *inPd = input->GetPointData();

  vtkPointData  *outPD = output->GetPointData();

  int nPts   = input->GetNumberOfPoints();
  int nCells = input->GetNumberOfCells();

  vtkPoints *outPts  = vtkPoints::New();
  int        nOutPts = 0;

  if (VertexAtPoints)
    {
    // We want to put vertices at each of the points, but only if the vertices
    // are incident to a cell.
    int *lookupList = new int[nPts];
    for (i = 0 ; i < nPts ; i++)
      {
      lookupList[i] = 0;
      }

    for (i = 0 ; i < nCells ; i++)
      {
      vtkCell *cell = input->GetCell(i);
      int nPtsForThisCell = cell->GetNumberOfPoints();
      for (j = 0 ; j < nPtsForThisCell ; j++)
        {
        int id = cell->GetPointId(j);
        lookupList[id] = 1;
        }
      }

    nOutPts = 0;
    for (i = 0 ; i < nPts ; i++)
      {
      if (lookupList[i] != 0)
        {
        nOutPts++;
        }
      }

    outPts->SetNumberOfPoints(nOutPts);
    outPD->CopyAllocate(inPd, nOutPts);
    int count = 0;
    for (i = 0 ; i < nPts ; i++)
      {
      if (lookupList[i] != 0)
        {
        float pt[3];
        input->GetPoint(i, pt);
        outPts->SetPoint(count, pt);
        outPD->CopyData(inPd, i, count);
        count++;
        }
      }

      delete [] lookupList;
    }
  else
    {
    // Make an output vertex at each point.
    nOutPts = nCells;
    outPts->SetNumberOfPoints(nOutPts);
    outPD->CopyAllocate(inCd, nOutPts);
    int subId=0;
    float point[3];
    for (i = 0 ; i < nOutPts ; i++)
      {
      vtkCell *cell = input->GetCell(i);
      if (cell->GetNumberOfPoints() <= 27)
        {
        float pcoord[3];
        float weights[27];
        cell->GetParametricCenter(pcoord);
        cell->EvaluateLocation(subId, pcoord, point, weights);
        }
      else
        {
        float pcoord[3];
        float *weights = new float[cell->GetNumberOfPoints()];
        cell->GetParametricCenter(pcoord);
        cell->EvaluateLocation(subId, pcoord, point, weights);
        delete [] weights;
        }
      outPts->SetPoint(i, point);
      outPD->CopyData(inCd, i, i);
      }
    }

  vtkIdType onevertex[1];
  output->Allocate(nOutPts);
  for (i = 0 ; i < nOutPts ; i++)
    {
    onevertex[0] = i;
    output->InsertNextCell(VTK_VERTEX, 1, onevertex);
    }
  output->SetPoints(outPts);
  outPts->Delete();
}

  
void vtkVertexFilter::PrintSelf(ostream &os, vtkIndent indent)
{
   this->Superclass::PrintSelf(os, indent);
   os << indent << "VertexAtPoints: " << this->VertexAtPoints << "\n";
}

