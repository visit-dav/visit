// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "vtkLineoutFilter.h"

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCellDataToPointData.h>
#include <vtkIdTypeArray.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkLineSource.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkUnsignedCharArray.h>
#include <vtkVisItProbeFilter.h>


vtkStandardNewMacro(vtkLineoutFilter);


// ****************************************************************************
//  Constructor:
//
//  Modifications:
//    Kathleen Bonnell, Fri Jul 12 17:19:40 PDT 2002
//    Removed YScale, no longer needed.
//
//    Kathleen Bonnell, Fri Mar 28 12:09:01 PDT 2008
//    Removed cd2pd, use VisIt version of vtkProbeFilter.
//
// ****************************************************************************

vtkLineoutFilter::vtkLineoutFilter()
{
  this->Point1[0] = this->Point1[1] = this->Point1[2] = 0.; 
  this->Point2[0] = this->Point2[1] = this->Point2[2] = 1.; 
  this->NumberOfSamplePoints = 50;
  this->LineSource = vtkLineSource::New();
  this->Probe = vtkVisItProbeFilter::New();
}

// ****************************************************************************
//  Destructor:
//
//  Modifications:
//    Kathleen Bonnell, Fri Mar 28 12:09:01 PDT 2008
//    Removed cd2pd.
//
// ****************************************************************************

vtkLineoutFilter::~vtkLineoutFilter()
{
  if (this->LineSource != NULL)
    {
    this->LineSource->Delete();
    this->LineSource = NULL;
    }
  if (this->Probe != NULL)
    {
    this->Probe->Delete();
    this->Probe = NULL;
    }
}

// ****************************************************************************
//  Method: vtkLineoutFilter::RequestData.
//
//  Modifications:
//    Kathleen Bonnell, Tue Jun  4 09:17:56 PDT 2002
//    Copy point data is happening from wrong source. Use
//    probe->GetOutput->GetPointData().
//
//    Kathleen Bonnell, Fri Jul 12 17:19:40 PDT 2002
//    Removed YScale, no longer needed.
//
//    Kathleen Bonnell, Tue Dec 23 10:18:06 PST 2003 
//    Slight rework to consider ghost levels. 
//
//    Hank Childs, Fri Aug 27 15:15:20 PDT 2004
//    Rename ghost data array.
//
//    Hank Childs, Sun Mar 13 09:19:30 PST 2005
//    Fix memory leak.
//
//    Kathleen Bonnell, Fri Mar 28 12:09:01 PDT 2008
//    Removed cd2pd.
//
//    Eric Brugger, Thu Jan 10 09:51:08 PST 2013
//    Modified to inherit from vtkPolyDataAlgorithm.
//
// ****************************************************************************

int
vtkLineoutFilter::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  vtkDebugMacro(<<"Executing vtkLineoutFilter");

  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  //
  // Initialize some frequently used values.
  //
  vtkDataSet      *inDS = vtkDataSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *outPolys = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  this->LineSource->SetPoint1(this->Point1);
  this->LineSource->SetPoint2(this->Point2);
  this->LineSource->SetResolution(this->NumberOfSamplePoints);

  if (inDS->GetPointData()->GetScalars() == NULL)
    {
    if (inDS->GetCellData()->GetScalars() == NULL)
      {
       vtkErrorMacro(<<"No Scalars to probe!");
       return 1;
      }
    this->Probe->SetCellData(1);
    }
  this->Probe->SetSource(inDS);
  this->Probe->SetInputConnection(this->LineSource->GetOutputPort());
  this->Probe->Update();
  
  //
  // Since the input to this->Probe was polydata, we know that
  // the output is also polydata.
  //
  vtkPolyData *probeOut = vtkPolyData::New();
  probeOut->CopyStructure((vtkPolyData*)this->Probe->GetOutput());
  vtkIdTypeArray *validPoints = this->Probe->GetValidPoints();

  if (validPoints == NULL || validPoints->GetNumberOfTuples() == 0)
    {
        probeOut->Delete();
        vtkDebugMacro(<<"Probe did not find any valid points");
        return 1;
    }

  vtkPoints *inPts = probeOut->GetPoints();

  vtkIdType i, index, numPoints = validPoints->GetNumberOfTuples();

  vtkPoints *outPts = vtkPoints::New(inPts->GetDataType());
  outPolys->SetPoints(outPts);
  outPts->Delete();

  vtkCellArray *verts = vtkCellArray::New();
  outPolys->SetVerts(verts);
  verts->Delete();

  double newPoint[3] = { 0., 0., 0. };

  vtkDataArray *scalars = this->Probe->GetOutput()->GetPointData()->GetScalars();
  if (scalars == NULL)
  {
      probeOut->Delete();
      vtkErrorMacro(<<"Probe did not return point data scalars");
      return 1;
  }

  unsigned char* ghosts = 0;
  vtkDataArray *gl = 
      this->Probe->GetOutput()->GetPointData()->GetArray("avtGhostZones");

  int updateLevel = outInfo->Get(
    vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS());
  if (gl && gl->GetDataType() == VTK_UNSIGNED_CHAR && 
      gl->GetNumberOfComponents() == 1)
    {
        ghosts = ((vtkUnsignedCharArray*)gl)->GetPointer(0);
    }
  //
  //  Distance needs to be calculated for each new point.
  //
  double currentPoint[3];
  vtkIdTypeArray *nonGhostValidPoints =  vtkIdTypeArray::New();
  for (i = 0; i < numPoints; i++)
    {
    index = validPoints->GetValue(i);
    bool ghost = (ghosts && ghosts[index] > updateLevel);
    if (!ghost)
      {
      inPts->GetPoint(index, currentPoint);
      newPoint[0] = sqrt(vtkMath::Distance2BetweenPoints(this->Point1, currentPoint));
      newPoint[1] = scalars->GetTuple1(index); 
      outPts->InsertNextPoint(newPoint); 
      nonGhostValidPoints->InsertNextValue(index);
      }
    }

  numPoints = outPts->GetNumberOfPoints();
  //
  //  Create vertex cells.
  //
  verts->InsertNextCell(numPoints);
  for (i = 0; i < numPoints; i++)
    {
    verts->InsertCellPoint(i);
    }

  //
  // Copy point data.
  // Our new cells have no corrspondence with input cells, so no cell
  // data is copied.
  //
  vtkPointData *inPD = this->Probe->GetOutput()->GetPointData();
  vtkPointData *outPD = outPolys->GetPointData();
  outPD->CopyAllocate(inPD, numPoints);
  for (i = 0; i < numPoints; i++)
    {
    outPD->CopyData(inPD, nonGhostValidPoints->GetValue(i), i);
    }

  //
  // Clean up;
  //
  probeOut->Delete();
  nonGhostValidPoints->Delete();

  return 1;
} 

// ****************************************************************************
//  Method: vtkLineoutFilter::FillInputPortInformation
//
// ****************************************************************************

int
vtkLineoutFilter::FillInputPortInformation(int, vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
  return 1;
}

// ****************************************************************************
//  Method: vtkLineoutFilter::PrintSelf
//
//  Modifications:
//    Kathleen Bonnell, Fri Jul 12 17:19:40 PDT 2002
//    Removed YScale, no longer needed.
//
// ****************************************************************************

void 
vtkLineoutFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Point1: (" << this->Point1[0] << ", " 
                              << this->Point1[1] << ", "
                              << this->Point1[2] << ")\n"; 
  os << indent << "Point2: (" << this->Point1[0] << ", " 
                              << this->Point1[1] << ", "
                              << this->Point1[2] << ")\n"; 
  os << indent << "Number of Sample points: " 
               << this->NumberOfSamplePoints << "\n"; 
}
