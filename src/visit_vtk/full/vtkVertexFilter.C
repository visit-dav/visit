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
#include <vtkUnstructuredGrid.h>
#include <vtkVisItUtility.h>

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
//    Kathleen Bonnell, Wed Oct 20 17:10:21 PDT 2004 
//    Use vtkVisItUtility method to compute cell center. 
//
//    Hank Childs, Fri Jun  9 13:13:20 PDT 2006
//    Remove unused variable.
//
//    Hank Childs, Thu Sep  7 14:34:48 PDT 2006
//    Code around VTK slowness for convex point sets ['7311].
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
        double pt[3];
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
    double point[3];

    vtkUnstructuredGrid *ugrid = NULL;
    if (input->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
    {
        ugrid = (vtkUnstructuredGrid *) input;
    }

    for (i = 0 ; i < nOutPts ; i++)
      {
      // Calling GetCellCenter on ConvexPointSet cells takes ~1/2 second,
      // so avoid that if possible.
      if (ugrid != NULL && ugrid->GetCellType(i) == VTK_CONVEX_POINT_SET)
      {
          int npts;
          vtkIdType *pts;
          ugrid->GetCellPoints(i, npts, pts);
          point[0] = 0.;
          point[1] = 0.;
          point[2] = 0.;
          double weight = 1./npts;
          for (j = 0 ; j < npts ; j++)
          {
              double pt2[3];
              ugrid->GetPoint(pts[j], pt2);
              point[0] += weight*pt2[0];
              point[1] += weight*pt2[1];
              point[2] += weight*pt2[2];
          }
      }
      else
        vtkVisItUtility::GetCellCenter(input->GetCell(i), point);
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

