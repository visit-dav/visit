/* =======================================================================================
   Copyright 2014-2015 Texas Advanced Computing Center, The University of Texas at Austin
   All rights reserved.

   Licensed under the BSD 3-Clause License, (the "License"); you may not use this file
   except in compliance with the License.
   A copy of the License is included with this software in the file LICENSE.
   If your copy does not contain the License, you may obtain a copy of the License at:

       http://opensource.org/licenses/BSD-3-Clause

   Unless required by applicable law or agreed to in writing, software distributed under
   the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
   KIND, either express or implied.
   See the License for the specific language governing permissions and limitations under
   limitations under the License.

   pvOSPRay is derived from VTK/ParaView Los Alamos National Laboratory Modules (PVLANL)
   Copyright (c) 2007, Los Alamos National Security, LLC
   ======================================================================================= */

// .NAME vtkOSPRayTestSource - produce triangles to benchmark OSPRay with

#include "vtkOSPRayTestSource.h"
#include "vtkObjectFactory.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkIdTypeArray.h"
#include "vtkCellArray.h"

vtkStandardNewMacro(vtkOSPRayTestSource);

//----------------------------------------------------------------------------
vtkOSPRayTestSource::vtkOSPRayTestSource()
{
  this->SetNumberOfInputPorts(0);
  this->Resolution = 100;

  //Give it some geometric coherence
  this->DriftFactor = 0.1;
  //Give it some memory coherence
  this->SlidingWindow = 0.01;
}

//----------------------------------------------------------------------------
vtkOSPRayTestSource::~vtkOSPRayTestSource()
{
}

//----------------------------------------------------------------------------
void vtkOSPRayTestSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Resolution: " << this->Resolution << endl;
  os << indent << "DriftFactor: " << this->DriftFactor << endl;
  os << indent << "SlidingWindow: " << this->SlidingWindow << endl;
}

//----------------------------------------------------------------------------
int vtkOSPRayTestSource::RequestInformation(
  vtkInformation *vtkNotUsed(info),
  vtkInformationVector **vtkNotUsed(inputV),
  vtkInformationVector *output)
{
  return 1;
}

//----------------------------------------------------------------------------
int vtkOSPRayTestSource::RequestData(vtkInformation *vtkNotUsed(info),
                                    vtkInformationVector **vtkNotUsed(inputV),
                                    vtkInformationVector *output)
{
  vtkInformation *outInfo = output->GetInformationObject(0);
  int Rank = 0;
  if (outInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER()))
    {
    Rank =
      outInfo->Get(
        vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER());
    }
  int Processors = 1;
  if (outInfo->Has(
        vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES()))
    {
    Processors =
      outInfo->Get(
        vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES());
    }


  vtkPolyData *outPD =
    vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));
  if (!outPD)
    {
    return VTK_ERROR;
    }

  outPD->Initialize();
  outPD->Allocate();

  vtkIdType myStart = this->Resolution/Processors * Rank;
  vtkIdType myEnd = this->Resolution/Processors * (Rank+1);

  vtkIdType indices[3];
  vtkIdType minIndex = this->Resolution;
  vtkIdType maxIndex = 0;
  for (vtkIdType i = 0; i < this->Resolution; i++)
    {
    double offset;
    indices[0] = -1;
    indices[1] = -2;
    indices[2] = -3;

    for (vtkIdType c = 0; c < 3; c++)
      {
      offset = vtkMath::Random()*this->SlidingWindow*this->Resolution -
        (this->SlidingWindow*this->Resolution/2.0);
      indices[c] = ((vtkIdType)((double)i+c + offset));
      if (indices[c] < 0 || indices[c] >= this->Resolution)
        {
        indices[c] = ((vtkIdType)((double)i+c - offset));
        }

      if (indices[0] == indices[1] ||
          indices[0] == indices[2] ||
          indices[2] == indices[1])
        {
        c--;
        }
      }

    if (i >= myStart && i < myEnd)
      {
      //remember index range for this slice so we can readjust
      for (int c = 0; c < 3; c++)
        {
        if (indices[c] < minIndex)
          {
          minIndex = indices[c];
          }
        if (indices[c] > maxIndex)
          {
          maxIndex = indices[c];
          }
        }

      outPD->InsertNextCell(VTK_TRIANGLE, 3, indices);
      }
    if (i % (this->Resolution/10) == 0)
      {
      double frac = (double)i/this->Resolution * 0.33;
      this->UpdateProgress(frac);
      }
    }


  //shift indices to 0, because each processor only produces local points
  vtkCellArray *polys = outPD->GetPolys();
  polys->InitTraversal();
  vtkIdType npts;
  vtkIdType *thePts;
  vtkIdType i = 0;
  vtkIdType nCells = polys->GetNumberOfCells();
  while(polys->GetNextCell(npts, thePts))
    {
    for (vtkIdType c = 0; c < npts; c++)
      {
      thePts[c] = thePts[c] - minIndex;
      }
    i++;
    if (i % (nCells/10) == 0)
      {
      double frac = (double)i/nCells * 0.33 + 0.33;
      this->UpdateProgress(frac);
      }
    }

  vtkPoints *pts = vtkPoints::New();
  double X = vtkMath::Random();
  double Y = vtkMath::Random();
  double Z = vtkMath::Random();
  for (i = 0; (i < this->Resolution || i <= maxIndex); i++)
    {
    X = X+vtkMath::Random() * this->DriftFactor - this->DriftFactor*0.5;
    Y = Y+vtkMath::Random() * this->DriftFactor - this->DriftFactor*0.5;
    Z = Z+vtkMath::Random() * this->DriftFactor - this->DriftFactor*0.5;
    if (i >= minIndex && i <= maxIndex)
      {
      pts->InsertNextPoint(X, Y, Z);
      }

    if (i % (this->Resolution/10) == 0)
      {
      double frac = (double)i/this->Resolution * 0.33 + 0.66;
      this->UpdateProgress(frac);
      }
    }
  outPD->SetPoints(pts);
  pts->Delete();


  return 1;
}
