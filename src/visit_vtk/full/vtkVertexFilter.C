// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "vtkVertexFilter.h"

#include <visit-config.h> // For LIB_VERSION_LE

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
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
    VertexAtPoints = true;
}

// ***************************************************************************
//  Method: vtkVertexFilter::RequestData
//
//  Modifications:
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
//    Kathleen Biagas, Thu Aug 30 16:55:32 MST 2012
//    Preserve coordinate type.
//
//    Eric Brugger, Thu Jan 10 12:18:23 PST 2013
//    Modified to inherit from vtkPolyDataAlgorithm.
//
// ****************************************************************************

int
vtkVertexFilter::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  vtkDebugMacro(<<"Executing vtkVertexFilter");

  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  //
  // Initialize some frequently used values.
  //
  vtkDataSet   *input = vtkDataSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  int   i, j;

  vtkCellData  *inCd = input->GetCellData();
  vtkPointData *inPd = input->GetPointData();

  vtkPointData  *outPD = output->GetPointData();

  int nPts   = input->GetNumberOfPoints();
  int nCells = input->GetNumberOfCells();

  vtkPoints *outPts  = vtkVisItUtility::NewPoints(input);
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
          vtkIdType npts;
#if LIB_VERSION_LE(VTK, 8,1,0)
          vtkIdType *pts;
#else
          const vtkIdType *pts;
#endif
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
  return 1;
}
  
// ****************************************************************************
//  Method: vtkVertexFilter::FillInputPortInformation
//
// ****************************************************************************

int
vtkVertexFilter::FillInputPortInformation(int, vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
  return 1;
}

// ****************************************************************************
//  Method: vtkVertexFilter::PrintSelf
//
// ****************************************************************************

void
vtkVertexFilter::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "VertexAtPoints: " << this->VertexAtPoints << "\n";
}
