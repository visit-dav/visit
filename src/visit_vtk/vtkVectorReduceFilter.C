/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVectorReduceFilter.cxx,v $
  Language:  C++
  Date:      $Date: 2001/03/20 14:10:58 $
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

#include "vtkObjectFactory.h"
#include "vtkVectorReduceFilter.h"
#include "vtkFloatArray.h"
#include <vtkDataSet.h>
#include <vtkPolyData.h>

// **************************************************************************
//  Modifications:
//    Kathleen Bonnell, Wed Mar  6 17:10:03 PST 2002 
//    Replace 'New' method with Macro to match VTK 4.0 API.
// **************************************************************************

vtkStandardNewMacro(vtkVectorReduceFilter);


vtkVectorReduceFilter::vtkVectorReduceFilter()
{
  stride = 10;
  numEls = -1;
}


void vtkVectorReduceFilter::SetStride(int s)
{
  numEls = -1;
  stride = s;
}


void vtkVectorReduceFilter::SetNumberOfElements(int n)
{
  stride = -1;
  numEls = n;
}

// ****************************************************************************
// Modifications:
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkVectors has been deprecated in VTK 4.0, use 
//    vtkDataArray and vtkFloatArray instead.
//
//    Hank Childs, Wed Apr  9 09:36:07 PDT 2003
//    Made each output vertex also have a VTK_VERTEX in the cell list.
//
// ****************************************************************************

void vtkVectorReduceFilter::Execute(void)
{
  vtkDataSet *input  = this->GetInput();
  vtkPolyData *output = this->GetOutput();

  vtkCellData *inCd = input->GetCellData();
  vtkPointData *inPd = input->GetPointData();

  vtkDataArray *inCvecs = inCd->GetVectors();
  vtkDataArray *inPvecs = inPd->GetVectors();

  int npts = input->GetNumberOfPoints();
  int ncells = input->GetNumberOfCells();

  if (inPvecs == NULL && inCvecs == NULL)
    {
    vtkErrorMacro(<<"No vectors to reduce");
    return;
    }

  // Determine what the stride is.
  if (stride <= 0 && numEls <= 0)
    {
    vtkErrorMacro(<<"Invalid stride");
    return;
    }

  float actingStride = stride;
  if (actingStride <= 0)
    {
    int totalVecs = 0;
    if (inPvecs != NULL)
    {
        totalVecs += npts;
    }
    if (inCvecs != NULL)
    {
        totalVecs += ncells;
    }
    actingStride = ceil(((float) totalVecs) / ((float) numEls));
    }

  vtkPoints *outpts = vtkPoints::New();
  vtkFloatArray *outVecs = vtkFloatArray::New();
  outVecs->SetNumberOfComponents(3);

  float nextToTake = 0.;
  if (inPvecs != NULL)
    {
    for (int i = 0 ; i < npts ; i++)
      {
      if (i >= nextToTake)
        {
        nextToTake += actingStride;

        float pt[3];
        input->GetPoint(i, pt);
        outpts->InsertNextPoint(pt);

        float v[3];
        inPvecs->GetTuple(i, v);
        outVecs->InsertNextTuple(v);
        }
      }
    }

  nextToTake = 0.;
  if (inCvecs != NULL)
    {
    for (int i = 0 ; i < ncells ; i++)
      {
      if (i >= nextToTake)
        {
        nextToTake += actingStride;

        vtkCell *cell = input->GetCell(i);
        float pt[3];
        cell->GetParametricCenter(pt);
        outpts->InsertNextPoint(pt);

        float v[3];
        inCvecs->GetTuple(i, v);
        outVecs->InsertNextTuple(v);
        }
      }
    }

  int nOutPts = outpts->GetNumberOfPoints();
  output->SetPoints(outpts);
  outpts->Delete();
  output->GetPointData()->SetVectors(outVecs);
  outVecs->Delete();

  output->Allocate(nOutPts);
  vtkIdType onevertex[1];
  for (int i = 0 ; i < nOutPts ; i++)
    {
    onevertex[0] = i;
    output->InsertNextCell(VTK_VERTEX, 1, onevertex);
    }
}

  
void vtkVectorReduceFilter::PrintSelf(ostream &os, vtkIndent indent)
{
   this->Superclass::PrintSelf(os, indent);
   os << indent << "Stride: " << this->stride << "\n";
   os << indent << "Target number of vectors: " << this->numEls << "\n";
}

