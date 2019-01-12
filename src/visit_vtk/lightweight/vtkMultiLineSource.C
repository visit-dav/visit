/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkLineSource.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// Modifications:
//   Matt Larsen, Fri Jul 7 07:30:37 PST 2016
//   Changed lineSource to a multiLineSouce to support pick highlights
//   Added shift method to move lines from original points by a supplied vector
     
#include "vtkMultiLineSource.h"

#include "vtkCellArray.h"
#include "vtkFloatArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkLine.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"

#include <math.h>

vtkStandardNewMacro(vtkMultiLineSource);
vtkCxxSetObjectMacro(vtkMultiLineSource,Points,vtkPoints);

// ----------------------------------------------------------------------
vtkMultiLineSource::vtkMultiLineSource()
{
  this->Points = vtkPoints::New();
  this->OriginalPoints = vtkPoints::New();
  this->SetNumberOfInputPorts( 0 );
}

// ----------------------------------------------------------------------
vtkMultiLineSource::~vtkMultiLineSource()
{
  this->Points->Delete();
  this->OriginalPoints->Delete();
}

// ----------------------------------------------------------------------
void vtkMultiLineSource::AddLine(double p0[3], double p1[3])
{ 
   this->Points->InsertNextPoint(p0);
   this->Points->InsertNextPoint(p1);
   this->OriginalPoints->InsertNextPoint(p0);
   this->OriginalPoints->InsertNextPoint(p1);
}

// ----------------------------------------------------------------------
void vtkMultiLineSource::Shift3d(const double vec[3], const double &shiftFactor)
{
    const int numPointsOrig = OriginalPoints->GetNumberOfPoints();
    const int numPoints = Points->GetNumberOfPoints();

    if(numPointsOrig != numPoints)
    {
        vtkWarningMacro( <<"MultiLineSource points not equal");
        return;
    }

    for(int i = 0; i < numPoints; ++i)
    {
        double p[3];
        OriginalPoints->GetPoint(i,p);
        p[0] = p[0] + vec[0] *shiftFactor;
        p[1] = p[1] + vec[1] *shiftFactor;
        p[2] = p[2] + vec[2] *shiftFactor;
        Points->SetPoint(i,p);
    }
}

// ----------------------------------------------------------------------
void vtkMultiLineSource::Shift2d(const double &shiftFactor)
{
    const int numPointsOrig = OriginalPoints->GetNumberOfPoints();
    const int numPoints = Points->GetNumberOfPoints();

    if(numPointsOrig != numPoints)
    {
        vtkWarningMacro( <<"MultiLineSource points not equal");
        return;
    }

    for(int i = 0; i < numPoints; ++i)
    {
        double p[3];
        OriginalPoints->GetPoint(i,p);
        p[0] = p[0];
        p[1] = p[1];
        p[2] = p[2] + shiftFactor;
        Points->SetPoint(i,p);
    }
}
// ----------------------------------------------------------------------
int vtkMultiLineSource::RequestInformation(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  // get the info object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  return 1;
}

// ----------------------------------------------------------------------
int vtkMultiLineSource::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{ 
    const int numPointsOrig = OriginalPoints->GetNumberOfPoints();
    const int numPoints = Points->GetNumberOfPoints();

    //Sanity Check
    if(numPointsOrig != numPoints)
    {
    vtkWarningMacro( <<"MultiLineSource points not equal");
    return 0;
    }
    if(numPoints < 2 && (numPoints % 2 != 0))
    {
    vtkWarningMacro( <<"MultiLineSource must have at least 2 points and"
                 <<"a multiple of 2");
    return 0;
    }

    // get the info object
    vtkInformation *outInfo = outputVector->GetInformationObject(0);

    // get the ouptut
    vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

    vtkCellArray *lines = vtkCellArray::New();
    const int numLines = numPoints / 2;
    lines->Allocate( lines->EstimateSize(numLines, 2));
    
    for(int i = 0; i < numLines; ++i)
    {
        vtkLine * line  = vtkLine::New();
        line->GetPointIds()->SetId(0,i*2+0);
        line->GetPointIds()->SetId(1,i*2+1);
        lines->InsertNextCell(line);
    }

    output->SetPoints(this->Points);
    output->SetLines(lines);
    lines->Delete();
 
  return 1;
}

// ----------------------------------------------------------------------
void vtkMultiLineSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent
     << "Points: ";
  if ( this->Points )
    {
    this->Points->PrintSelf( os, indent );
    }
  else
    {
    os << "(none)" << endl;
    }
  os << indent
     << "Original Points: ";
  if ( this->Points )
    {
    this->OriginalPoints->PrintSelf( os, indent );
    }
  else
    {
    os << "(none)" << endl;
    }
}
