// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "vtkPolyDataRelevantPointsFilter.h"

#include <vtkCellArray.h>
#include <vtkCellArrayIterator.h>
#include <vtkCellData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>

//-----------------------------------------------------------------------------
// Modifications:
//   Kathleen Bonnell, Wed Mar  6 15:14:29 PST 2002 
//   Replace 'New' method with Macro to match VTK 4.0 API.
//-----------------------------------------------------------------------------

vtkStandardNewMacro(vtkPolyDataRelevantPointsFilter);

// ***************************************************************************
//  Modifications:
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
//    Brad Whitlock, Mon Apr 4 11:34:03 PDT 2005
//    Added support for cells that have more than 1024 points.
//
//    Brad Whitlock, Wed Mar 21 12:29:53 PDT 2012
//    Support for double coordinates.
//
//    Eric Brugger, Wed Jan  9 13:15:08 PST 2013
//    Modified to inherit from vtkPolyDataAlgorithm.
//
//    Brad Whitlock, Thu Jul 23 16:01:46 PDT 2015
//    Support for non-standard memory layout.
//
// ****************************************************************************

int vtkPolyDataRelevantPointsFilter::RequestData(
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

  vtkPoints *inPts     = input->GetPoints();
  vtkIdType numPts     = input->GetNumberOfPoints();
  vtkIdType numCells   = input->GetNumberOfCells();
  
  //
  // Sanity checks
  //
  vtkDebugMacro(<<"Beginning PolyData Relevant Points Filter ");
  if (input == NULL) {
      vtkErrorMacro(<<"Input is NULL");
      return 1;
  }
  if ( (numPts<1) || (inPts == NULL ) ) {
      vtkErrorMacro(<<"No data to Operate On!");
      return 1;
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
  for (vtkIdType i = 0; i < numPts; i++)
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
  for (int i = 0 ; i < 4 ; i++)
    {
    auto cellIter = vtk::TakeSmartPointer(arrays[i]->NewIterator());
    for (cellIter->GoToFirstCell();
         !cellIter->IsDoneWithTraversal();
         cellIter->GoToNextCell())
      {
      vtkIdList *cell = cellIter->GetCurrentCell();
      const vtkIdType npts = cell->GetNumberOfIds();
      for (vtkIdType ptId = 0; ptId < npts; ++ptId)
        {
        // Beware downcast to 32 bit...
        int oldPt = static_cast<int>(cell->GetId(ptId));
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
  //
  vtkPoints *newPts = vtkPoints::New(inPts->GetDataType());
  newPts->SetNumberOfPoints(numNewPts);
  vtkPointData *inputPD = input->GetPointData();
  vtkPointData *outputPD = output->GetPointData();
  bool havePointVars = (inputPD->GetNumberOfArrays() > 0);
  if (havePointVars)
    {
    outputPD->CopyAllocate(inputPD, numNewPts);
    }

  int accessMethod = 0;
  if(inPts->GetData()->HasStandardMemoryLayout())
    {
    if(inPts->GetDataType() == VTK_FLOAT)
      accessMethod = 1;
    else if(inPts->GetDataType() == VTK_DOUBLE)
      accessMethod = 2;
    }

  if(accessMethod == 1)
    {
    const float *in_ptr = (const float *) inPts->GetVoidPointer(0);
    float *out_ptr = (float *) newPts->GetVoidPointer(0);
    for (vtkIdType i = 0 ; i < numNewPts ; i++)
      {
      const float *src = in_ptr + 3*newToOld[i];
      *out_ptr++ = src[0];
      *out_ptr++ = src[1];
      *out_ptr++ = src[2];
      }
    }
  else if(accessMethod == 2)
    {
    const double *in_ptr = (const double *) inPts->GetVoidPointer(0);
    double *out_ptr = (double *) newPts->GetVoidPointer(0);
    for (vtkIdType i = 0 ; i < numNewPts ; i++)
      {
      const double *src = in_ptr + 3*newToOld[i];
      *out_ptr++ = src[0];
      *out_ptr++ = src[1];
      *out_ptr++ = src[2];
      }
    }
  else
    {
    for (vtkIdType i = 0 ; i < numNewPts ; i++)
      {
      newPts->SetPoint(i, inPts->GetPoint(newToOld[i]));
      }
    }

  for (vtkIdType i = 0 ; i < numNewPts ; i++)
    {
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
  int nIdStoreSize = 1024;
  vtkIdType *pts = new vtkIdType[nIdStoreSize];
  const vtkIdType *oldPts = NULL;
  vtkIdType nids = 0;
  input->BuildCells();
  for (vtkIdType i = 0; i < numCells; i++) 
    {
    input->GetCellPoints(i, nids, oldPts);
    if(nids > nIdStoreSize)
      {
      delete [] pts;
      nIdStoreSize = int(nids * 1.25);
      pts = new vtkIdType[nIdStoreSize];
      }
    int cellType = input->GetCellType(i);
    for (vtkIdType j = 0; j < nids; j++)
      {
      pts[j] = oldToNew[oldPts[j]];
      }
    output->InsertNextCell(cellType, nids, pts);
    }
  delete [] pts;
  delete [] oldToNew;
  delete [] newToOld;

  return 1;
}

//-----------------------------------------------------------------------------
void vtkPolyDataRelevantPointsFilter::PrintSelf(ostream& os, vtkIndent indent) 
{
  this->Superclass::PrintSelf(os,indent);
}
