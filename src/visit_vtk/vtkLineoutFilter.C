#include "vtkLineoutFilter.h"

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCellDataToPointData.h>
#include <vtkIdTypeArray.h>
#include <vtkLineSource.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkProbeFilter.h>


vtkStandardNewMacro(vtkLineoutFilter);


//======================================================================
// Constructor
//
// Modifications:
//   Kathleen Bonnell, Fri Jul 12 17:19:40 PDT 2002
//   Removed YScale, no longer needed.
//  
//======================================================================
vtkLineoutFilter::vtkLineoutFilter()
{
  this->Point1[0] = this->Point1[1] = this->Point1[2] = 0.; 
  this->Point2[0] = this->Point2[1] = this->Point2[2] = 1.; 
  this->NumberOfSamplePoints = 50;
  this->LineSource = vtkLineSource::New();
  this->Probe = vtkProbeFilter::New();
  this->cd2pd = vtkCellDataToPointData::New();
}

//======================================================================
// Destructor
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
  if (this->cd2pd != NULL)
    {
    this->cd2pd->Delete();
    this->cd2pd = NULL;
    }
}

//======================================================================
// Standard Execute method.
//
// Modifications:
//   Kathleen Bonnell, Tue Jun  4 09:17:56 PDT 2002
//   Copy point data is happening from wrong source. Use
//   probe->GetOutput->GetPointData().
//
//   Kathleen Bonnell, Fri Jul 12 17:19:40 PDT 2002
//   Removed YScale, no longer needed.
//  
//======================================================================
void
vtkLineoutFilter::Execute()
{
  vtkDebugMacro(<<"Executing vtkLineoutFilter");
  vtkDataSet *inDS  = this->GetInput();

  this->LineSource->SetPoint1(this->Point1);
  this->LineSource->SetPoint2(this->Point2);
  this->LineSource->SetResolution(this->NumberOfSamplePoints);

  if (inDS->GetPointData()->GetScalars() == NULL)
    {
    if (inDS->GetCellData()->GetScalars() == NULL)
    {
       vtkErrorMacro(<<"No Scalars to probe!");
       return;
    }
    vtkDebugMacro(<<"Converting CellData To PointData");
    this->cd2pd->SetInput(inDS);
    this->Probe->SetSource(this->cd2pd->GetOutput());
    }
  else
    {
    this->Probe->SetSource(inDS);
    }
  this->Probe->SetInput(this->LineSource->GetOutput());
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
        vtkDebugMacro(<<"Probe did not find any valid points");
        return;
    }

  vtkPolyData *outPolys = this->GetOutput(); 
  vtkPoints *inPts = probeOut->GetPoints();

  vtkIdType i, index, numPoints = validPoints->GetNumberOfTuples();

  vtkPoints *outPts = vtkPoints::New();
  outPts->SetNumberOfPoints(numPoints);
  outPolys->SetPoints(outPts);
  outPts->Delete();

  vtkCellArray *verts = vtkCellArray::New();
  verts->InsertNextCell(numPoints);
  outPolys->SetVerts(verts);
  verts->Delete();

  float newPoint[3] = { 0., 0., 0. };

  vtkDataArray *scalars = this->Probe->GetOutput()->GetPointData()->GetScalars();
  if (scalars == NULL)
  {
      vtkErrorMacro(<<"Probe did not return point data scalars");
      return;
  }

  //
  //  Distance needs to be calculated for each new point.
  //
  float currentPoint[3];
  for (i = 0; i < numPoints; i++)
    {
    index = validPoints->GetValue(i);
    inPts->GetPoint(index, currentPoint);
    newPoint[0] = sqrt(vtkMath::Distance2BetweenPoints(this->Point1, currentPoint));
    newPoint[1] = scalars->GetTuple1(index); 
    outPts->SetPoint(i, newPoint); 
    }

  //
  //  Create vertex cells.
  //
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
    outPD->CopyData(inPD, validPoints->GetValue(i), i);
    }

  //
  // Clean up;
  //
  probeOut->Delete();
} 

//======================================================================
// Modifications:
//   Kathleen Bonnell, Fri Jul 12 17:19:40 PDT 2002
//   Removed YScale, no longer needed.
//======================================================================
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



      

