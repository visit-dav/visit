/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItAxisActor2D.cxx,v $
  Language:  C++
  Date:      $Date: 2000/06/08 09:11:05 $
  Version:   $Revision: 1.13 $
  Thanks:    Thanks to Kitware & RPI/SCOREC who supported the development
             of this class.

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
#include <math.h>
#include "vtkVisItAxisActor2D.h"
#include <vtkCellArray.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkTextProperty.h>
#include <vtkProperty2D.h>
#include <vtkViewport.h>
#include <snprintf.h>

//------------------------------------------------------------------------------
// Modifications:
//   Kathleen Bonnell, Wed Mar  6 13:48:48 PST 2002
//   Replace 'New' method with macro.
// -----------------------------------------------------------------------------
vtkStandardNewMacro(vtkVisItAxisActor2D);


// **********************************************************************
// Instantiate this object.
//
//  Modifications:
//    Kathleen Bonnell, Wed Nov  7 16:07:00 PST 2001
//    No longer allocate memory for labels here, handle it dynamically.
//
//    Eric Brugger, Fri Nov  1 15:59:20 PST 2002
//    Added support for specifying the tick locations precisely.
//
//    Eric Brugger, Tue Jun  3 11:45:37 PDT 2003 
//    Change the default tick offset to 4.
//
//    Eric Brugger, Tue Nov 25 11:44:40 PST 2003
//    Added the ability to specify the axis orientation angle.
//
// **********************************************************************
vtkVisItAxisActor2D::vtkVisItAxisActor2D()
{
  this->Point1Coordinate = vtkCoordinate::New();
  this->Point1Coordinate->SetCoordinateSystemToNormalizedViewport();
  this->Point1Coordinate->SetValue(0.0, 0.0);

  this->Point2Coordinate = vtkCoordinate::New();
  this->Point2Coordinate->SetCoordinateSystemToNormalizedViewport();
  this->Point2Coordinate->SetValue(0.75, 0.0);

  this->UseOrientationAngle = 0;
  this->OrientationAngle = 0.; 
  
  this->NumberOfLabels = 5;

  this->Title = NULL;
  this->AdjustLabels = 1;
  this->MajorTickLabelScale = 1.0;
  this->MajorTickMinimum = 0.0;
  this->MajorTickMaximum = 1.0;
  this->MajorTickSpacing = 1.0;
  this->MinorTickSpacing = 0.1;
  this->MinorTicksVisible = 1;
  this->TitleAtEnd   = 0;
  this->LabelFontHeight = 0.02;
  this->TitleFontHeight = 0.02;
  this->TickLength = 5;
  this->TickOffset = 4;
  this->TickLocation = 1; // outside
  this->Range[0] = 0.0;
  this->Range[1] = 1.0;

  this->Bold = 1;
  this->Italic = 1;
  this->Shadow = 1;
  this->FontFamily = VTK_ARIAL;
  this->LabelFormat = new char[8]; 
  SNPRINTF(this->LabelFormat,8, "%s","%-#6.3f");

  this->TitleMapper = vtkTextMapper::New();
  this->TitleActor = vtkActor2D::New();
  this->TitleActor->SetMapper(this->TitleMapper);
  
  // to avoid deleting/rebuilding create once up front
  this->NumberOfLabelsBuilt = 0;
  this->LabelMappers = NULL; 
  this->LabelActors = NULL; 

  this->Axis = vtkPolyData::New();
  this->AxisMapper = vtkPolyDataMapper2D::New();
  this->AxisMapper->SetInput(this->Axis);
  this->AxisActor = vtkActor2D::New();
  this->AxisActor->SetMapper(this->AxisMapper);
  
  this->AxisVisibility = 1;
  this->TickVisibility = 1;
  this->LabelVisibility = 1;
  this->TitleVisibility = 1;
  
  this->LastPoint1[0] = this->LastPoint1[1] = 0;
  this->LastPoint2[0] = this->LastPoint2[1] = 0;

  this->DrawGridlines = 0;
  this->GridlineXLength = 1.;  
  this->GridlineYLength = 1.;  
}

vtkVisItAxisActor2D::~vtkVisItAxisActor2D()
{
  if (this->Point1Coordinate)
    {
    this->Point1Coordinate->Delete();
    this->Point1Coordinate = NULL;
    }
  
  if (this->Point2Coordinate)
    {
    this->Point2Coordinate->Delete();
    this->Point2Coordinate = NULL;
    }
  
  if (this->LabelFormat) 
    {
    delete [] this->LabelFormat;
    this->LabelFormat = NULL;
    }
 
  if (this->TitleMapper)
    {
    this->TitleMapper->Delete();
    this->TitleMapper = NULL;
    }
  if (this->TitleActor)
    {
    this->TitleActor->Delete();
    this->TitleActor = NULL;
    }

  if (this->Title)
    {
    delete [] this->Title;
    this->Title = NULL;
    }

  if (this->LabelMappers)
    {
    for (int i=0; i < this->NumberOfLabelsBuilt; i++)
      {
      this->LabelMappers[i]->Delete();
      this->LabelActors[i]->Delete();
      }
    this->NumberOfLabelsBuilt = 0;
    delete [] this->LabelMappers;
    delete [] this->LabelActors;
    this->LabelMappers = NULL;
    this->LabelActors = NULL;
    }
 
  if (this->Axis)
    {
    this->Axis->Delete();
    this->Axis = NULL;
    }
  if (this->AxisMapper)
    {
    this->AxisMapper->Delete();
    this->AxisMapper = NULL;
    }
  if (this->AxisActor)
    {
    this->AxisActor->Delete();
    this->AxisActor = NULL;
    }
}

// ********************************************************************
// Build the axis, ticks, title, and labels and render.
//
// Modifications:
//   Kathleen Bonnell, Thu Jul 18 13:24:03 PDT 2002
//   Added check for Gridline visibility for rendering the
//   AxisActor. 
//
// ********************************************************************

int vtkVisItAxisActor2D::RenderOpaqueGeometry(vtkViewport *viewport)
{
  int i, renderedSomething=0;

  this->BuildAxis(viewport);
  
  // Everything is built, just have to render
#ifndef NO_ANNOTATIONS
  if ( this->Title != NULL && this->Title[0] != 0 && this->TitleVisibility )
    {
    renderedSomething += this->TitleActor->RenderOpaqueGeometry(viewport);
    }
#endif

  if ( this->AxisVisibility || this->TickVisibility || this->DrawGridlines)
    {
    renderedSomething += this->AxisActor->RenderOpaqueGeometry(viewport);
    }
  
#ifndef NO_ANNOTATIONS
  if ( this->LabelVisibility )
    {
    for (i=0; i<this->NumberOfLabelsBuilt; i++)
      {
      renderedSomething += this->LabelActors[i]->RenderOpaqueGeometry(viewport);
      }
    }
#endif

  return renderedSomething;
}

// Render the axis, ticks, title, and labels.
//
int vtkVisItAxisActor2D::RenderOverlay(vtkViewport *viewport)
{
  int i, renderedSomething=0;

  // Everything is built, just have to render
  if ( this->Title != NULL && this->Title[0] != 0 && this->TitleVisibility )
    {
    renderedSomething += this->TitleActor->RenderOverlay(viewport);
    }

  if ( this->AxisVisibility || this->TickVisibility )
    {
    renderedSomething += this->AxisActor->RenderOverlay(viewport);
    }
  
  if ( this->LabelVisibility )
    {
    for (i=0; i<this->NumberOfLabelsBuilt; i++)
      {
      renderedSomething += this->LabelActors[i]->RenderOverlay(viewport);
      }
    }

  return renderedSomething;
}

// Release any graphics resources that are being consumed by this actor.
// The parameter window could be used to determine which graphic
// resources to release.
void vtkVisItAxisActor2D::ReleaseGraphicsResources(vtkWindow *win)
{
  this->TitleActor->ReleaseGraphicsResources(win);
  for (int i=0; i < this->NumberOfLabelsBuilt; i++)
    {
    this->LabelActors[i]->ReleaseGraphicsResources(win);
    }
  this->AxisActor->ReleaseGraphicsResources(win);
}

// ********************************************************************
// Modifications:
//   Kathleen Bonnell, Wed Mar  6 13:48:48 PST 2002
//   Call superclass's method in the new vtk way
//
//   Eric Brugger, Tue Nov 25 11:44:40 PST 2003
//   Added the ability to specify the axis orientation angle.
//
// ********************************************************************
void vtkVisItAxisActor2D::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Title: " << (this->Title ? this->Title : "(none)") << "\n";
  os << indent << "Number Of Labels: " << this->NumberOfLabels << "\n";
  os << indent << "Number Of Labels Built: " 
     << this->NumberOfLabelsBuilt << "\n";
  os << indent << "Range: (" << this->Range[0] 
     << ", " << this->Range[1] << ")\n";

  os << indent << "Font Family: ";
  if ( this->FontFamily == VTK_ARIAL )
    {
    os << "Arial\n";
    }
  else if ( this->FontFamily == VTK_COURIER )
    {
    os << "Courier\n";
    }
  else
    {
    os << "Times\n";
    }

  os << indent << "Bold: " << (this->Bold ? "On\n" : "Off\n");
  os << indent << "Italic: " << (this->Italic ? "On\n" : "Off\n");
  os << indent << "Shadow: " << (this->Shadow ? "On\n" : "Off\n");
  os << indent << "Label Format: " << this->LabelFormat << "\n";
  os << indent << "Label Font Height: " << this->LabelFontHeight << "\n";
  os << indent << "Title Font Height: " << this->TitleFontHeight << "\n";
  os << indent << "Tick Length: " << this->TickLength << "\n";
  os << indent << "Tick Offset: " << this->TickOffset << "\n";
  
  os << indent << "Adjust Labels: " 
     << (this->AdjustLabels ? "On\n" : "Off\n");
  
  os << indent << "Axis Visibility: " 
     << (this->AxisVisibility ? "On\n" : "Off\n");
  
  os << indent << "Tick Visibility: " 
     << (this->TickVisibility ? "On\n" : "Off\n");
  
  os << indent << "Label Visibility: " 
     << (this->LabelVisibility ? "On\n" : "Off\n");
  
  os << indent << "Title Visibility: " 
     << (this->TitleVisibility ? "On\n" : "Off\n");
  
  os << indent << "Point1 Coordinate: " << this->Point1Coordinate << "\n";
  this->Point1Coordinate->PrintSelf(os, indent.GetNextIndent());
  
  os << indent << "Point2 Coordinate: " << this->Point2Coordinate << "\n";
  this->Point2Coordinate->PrintSelf(os, indent.GetNextIndent());

  os << indent << "Use Orientation Angle: "
     << (this->UseOrientationAngle ? "On\n" : "Off\n");

  os << indent << "Orientation Angle: " << this->OrientationAngle << "\n";
}


// ****************************************************************************
//
// Modifications:
//
//   Hank Childs, Fri Aug 10 17:30:40 PDT 2001
//   Account for negative ranges.
//
//   Kathleen Bonnell, Wed Nov  7 16:19:16 PST 2001 
//   Call SetNumberOfLabelsBuilt, so that memory for labels can be
//   allocated dynamically. 
//
//   Kathleen Bonnell, Thu Nov 15 13:20:44 PST 2001 
//   Make ptIds of type vtkIdType to match VTK 4.0 API. 
//   
//   Kathleen Bonnell, Thu Jul 18 10:33:07 PDT 2002   
//   Ensure that gridlines are drawn when enabled, even if ticks are disabled. 
//   Also, only insert points for ticks or gridlines if they are visible.
//   
//   Eric Brugger, Fri Nov  1 15:59:20 PST 2002
//   Added support for specifying the tick locations precisely.
//
//   Eric Brugger, Tue Jun  3 11:45:37 PDT 2003 
//   Multiply the text height by a fudge factor to account for the fact that
//   the height of a digit is considerably less than the height returned.
//
//   Eric Brugger, Tue Nov 25 11:44:40 PST 2003
//   Added the ability to specify the axis orientation angle.
//
//   Eric Brugger, Mon Jul 26 16:11:37 PDT 2004
//   Correct a bug with a misplaced closing parenthesis.
//
// ****************************************************************************

void vtkVisItAxisActor2D::BuildAxis(vtkViewport *viewport)
{
  int i, *x;
  vtkIdType ptIds[2];
  float p1[3], p2[3], offset;
  int numLabels, labelCount = 0;
  float outRange[2], deltaX, deltaY, xTick[3];
  float theta, val;
  int *size, stringSize[2], maxLabelStringSize[2];
  char string[64];
  float  proportion[VTK_MAX_LABELS];
  float  ticksize[VTK_MAX_LABELS];
  float  sin_theta, cos_theta;

  if ( this->GetMTime() < this->BuildTime &&
  viewport->GetMTime() < this->BuildTime )
    {
    return; //already built
    }

  // Check to see whether we have to rebuild everything
  if ( this->GetMTime() < this->BuildTime &&
  viewport->GetMTime() > this->BuildTime )
    { //viewport change may not require rebuild
    int *lastPoint1=this->Point1Coordinate->GetComputedViewportValue(viewport);
    int *lastPoint2=this->Point2Coordinate->GetComputedViewportValue(viewport);
    if ( lastPoint1[0] == this->LastPoint1[0] &&
    lastPoint1[1] == this->LastPoint1[1] &&
    lastPoint2[0] == this->LastPoint2[0] &&
    lastPoint2[1] == this->LastPoint2[1] )
      {
      return;
      }
    }

  vtkDebugMacro(<<"Rebuilding axis");

  // Initialize and get important info
  this->Axis->Initialize();
  this->AxisActor->SetProperty(this->GetProperty());
  this->TitleActor->SetProperty(this->GetProperty());
  
  size = viewport->GetSize();
  
  // Compute the location of tick marks and labels
  if ( this->AdjustLabels )
    {
    this->AdjustLabelsComputeRange(this->Range, outRange, this->NumberOfLabels,
                                   numLabels, proportion, ticksize,
                                   this->MinorTicksVisible,
                                   this->DrawGridlines);
    }
  else
    {
    this->SpecifiedComputeRange(this->Range, outRange,
                                this->MajorTickMinimum, this->MajorTickMaximum,
                                this->MajorTickSpacing, this->MinorTickSpacing,
                                numLabels, proportion, ticksize,
                                this->MinorTicksVisible, this->DrawGridlines);
    }

  // Generate the axis and tick marks.
  // We'll do our computation in viewport coordinates. First determine the
  // location of the endpoints.
  x = this->Point1Coordinate->GetComputedViewportValue(viewport);
  p1[0] = (float)x[0]; p1[1] = (float)x[1]; p1[2] = 0.0;
  this->LastPoint1[0] = x[0]; this->LastPoint1[1] = x[1];
  x = this->Point2Coordinate->GetComputedViewportValue(viewport);
  p2[0] = (float)x[0]; p2[1] = (float)x[1]; p2[2] = 0.0;
  this->LastPoint2[0] = x[0]; this->LastPoint2[1] = x[1];

  vtkPoints *pts = vtkPoints::New();
  vtkCellArray *lines = vtkCellArray::New();
  this->Axis->SetPoints(pts);
  this->Axis->SetLines(lines);
  pts->Delete();
  lines->Delete();

  // generate point along axis (as well as tick points)
  deltaX = p2[0] - p1[0];
  deltaY = p2[1] - p1[1];
  
  // Determine the axis orientation angle.
  if (UseOrientationAngle)
    {
    theta = OrientationAngle;
    }
  else
    {
    // Suns throw an exception for atan2 when deltaX == deltaY == 0.
    theta = (deltaX != 0. || deltaY != 0. ? atan2(deltaY, deltaX) : 0.);
    }

  // Compute these for later.
  sin_theta = sin(theta);
  cos_theta = cos(theta);
  xTick[2] = 0.0;
  float temp[2];
  for (i = 0 ; i < numLabels ; i++)
    {
    if (ticksize[i] == 2.0) // gridlines, draw them inside the viewport.
      {
      temp[0] = p1[0] + proportion[i]*(p2[0] - p1[0]);
      temp[1] = p1[1] + proportion[i]*(p2[1] - p1[1]);
      xTick[0] = temp[0] - this->GridlineXLength*sin_theta;
      xTick[1] = temp[1] + this->GridlineYLength*cos_theta;
      pts->InsertNextPoint(xTick);
      // this ordering is important for proper label positioning
      xTick[0] = temp[0];
      xTick[1] = temp[1];
      pts->InsertNextPoint(xTick);
      }
    else if (this->TickVisibility)
      {
      if (this->TickLocation == 1) // outside
        {   
        xTick[0] = p1[0] + proportion[i]*(p2[0] - p1[0]);
        xTick[1] = p1[1] + proportion[i]*(p2[1] - p1[1]);
        pts->InsertNextPoint(xTick);
        xTick[0] = xTick[0] + 2*ticksize[i]*this->TickLength*sin_theta;
        xTick[1] = xTick[1] - 2*ticksize[i]*this->TickLength*cos_theta;
        pts->InsertNextPoint(xTick);
        }
      else if (this->TickLocation == 0) // inside
        {
        temp[0] = p1[0] + proportion[i]*(p2[0] - p1[0]);
        temp[1] = p1[1] + proportion[i]*(p2[1] - p1[1]);
        xTick[0] = temp[0] - 2*ticksize[i]*this->TickLength*sin_theta;
        xTick[1] = temp[1] + 2*ticksize[i]*this->TickLength*cos_theta;
        pts->InsertNextPoint(xTick);
        // this ordering is important for proper label positioning
        xTick[0] = temp[0];
        xTick[1] = temp[1];
        pts->InsertNextPoint(xTick);
        }
      else  // both sides
        {
        temp[0] = p1[0] + proportion[i]*(p2[0] - p1[0]);
        temp[1] = p1[1] + proportion[i]*(p2[1] - p1[1]);
        xTick[0] = temp[0] - 2*ticksize[i]*this->TickLength*sin_theta;
        xTick[1] = temp[1] + 2*ticksize[i]*this->TickLength*cos_theta;
        pts->InsertNextPoint(xTick);
        xTick[0] = temp[0] + 2*ticksize[i]*this->TickLength*sin_theta;
        xTick[1] = temp[1] - 2*ticksize[i]*this->TickLength*cos_theta;
        pts->InsertNextPoint(xTick);
        }
      }
    }

  //create points and lines if ticks or gridlines are enabled.
  if ( this->TickVisibility || this->DrawGridlines) 
    {
    int nlines = pts->GetNumberOfPoints()/2;
    for (i=0; i < nlines; i++)
      {
      ptIds[0] = 2*i;
      ptIds[1] = 2*i + 1;
      lines->InsertNextCell(2, ptIds);
      }
    }
  
  // Build the labels
  maxLabelStringSize[0] = 0;
  maxLabelStringSize[1] = 0;
  if ( this->LabelVisibility )
    {
    // determine actual number of labels we need to build
    for ( i = 0; i < numLabels; i++)
     {
      if (ticksize[i] == 1.0)
        {
        labelCount++;
        }
     }
    if (labelCount <= 200)
      {
      this->SetNumberOfLabelsBuilt(labelCount);
      }
    else
      {
      vtkWarningMacro(<<"Expected number of labels > 200, limiting to 200");
      this->SetNumberOfLabelsBuilt(200);
      }

    labelCount = 0;
    for ( i=0; i < numLabels && labelCount < 200; i++)
      {
      if (ticksize[i] != 1.0)
        {
        continue;  // minor tick or gridline, should not be labeled.
        }

      val = proportion[i]*(outRange[1]-outRange[0]) + outRange[0];
      if ((fabs(val) < 0.01) &&
          (fabs(outRange[1]-outRange[0]) > 1))
        {
        // We just happened to fall at something near zero and the range is
        // large, so set it to zero to avoid ugliness.
        val = 0.;  
        }

      SNPRINTF(string,64,this->LabelFormat, val*this->MajorTickLabelScale);

      this->LabelMappers[labelCount]->SetInput(string);
      if (fabs(val) < 0.01)
      {
          // 
          // Ensure that -0.0 is never a label
          // The maximum number of digits that we allow past the decimal is 5.
          // 
          if (strcmp(string, "-0") == 0) 
              this->LabelMappers[labelCount]->SetInput("0");
          else if (strcmp(string, "-0.0") == 0) 
              this->LabelMappers[labelCount]->SetInput("0.0");
          else if (strcmp(string, "-0.00") == 0) 
              this->LabelMappers[labelCount]->SetInput("0.00");
          else if (strcmp(string, "-0.000") == 0) 
              this->LabelMappers[labelCount]->SetInput("0.000");
          else if (strcmp(string, "-0.0000") == 0)
              this->LabelMappers[labelCount]->SetInput("0.0000");
          else if (strcmp(string, "-0.00000") == 0)
              this->LabelMappers[labelCount]->SetInput("0.00000");
      }

      vtkTextProperty *tprop = this->LabelMappers[labelCount]->GetTextProperty();
      tprop->SetBold(this->Bold);
      tprop->SetItalic(this->Italic);
      tprop->SetShadow(this->Shadow);
      tprop->SetFontFamily(this->FontFamily);
      tprop->SetColor(this->GetProperty()->GetColor());
      tprop->SetFontSize((int)(this->LabelFontHeight*size[1]));
      labelCount++;
      }
     
    for (i = 0, labelCount = 0; i < numLabels; i++)
      {
      if (ticksize[i] != 1.0)
        {
        continue;
        }
      pts->GetPoint(2*i+1, xTick);
      this->LabelMappers[labelCount]->GetSize(viewport, stringSize);
      // Fudge factor, the height of the digits varies roughly from
      // 0.61 to 0.68 of the font height.  Use 0.68 to be conservative.
      // This centers the labels properly on the tick marks.
      stringSize[1] = 0.68 * stringSize[1];
      this->SetOffsetPosition(xTick, theta, stringSize[0], stringSize[1],
                              this->TickOffset, 
                              this->LabelActors[labelCount++], 0);
      maxLabelStringSize[0] = stringSize[0] > maxLabelStringSize[0] ?
                              stringSize[0] : maxLabelStringSize[0];
      maxLabelStringSize[1] = stringSize[1] > maxLabelStringSize[1] ?
                              stringSize[1] : maxLabelStringSize[1];
      }
    }// if labels visible

  // Now build the title
  if ( this->Title != NULL && this->Title[0] != 0 && this->TitleVisibility )
    {
    this->TitleMapper->SetInput(this->Title);
    vtkTextProperty *titleTprop = this->TitleMapper->GetTextProperty();
    titleTprop->SetBold(this->Bold);
    titleTprop->SetItalic(this->Italic);
    titleTprop->SetShadow(this->Shadow);
    titleTprop->SetFontFamily(this->FontFamily);
    titleTprop->SetColor(this->GetProperty()->GetColor());
    titleTprop->SetFontSize((int)(this->TitleFontHeight*size[1]));

    if ( this->TitleAtEnd )
      {
      xTick[0] = p2[0] + cos_theta*(this->TickLength+this->TickOffset);
      xTick[1] = p2[1] + sin_theta*(this->TickLength+this->TickOffset);
      }
    else
      {
      xTick[0] = p1[0] + (p2[0] - p1[0]) / 2.0;
      xTick[1] = p1[1] + (p2[1] - p1[1]) / 2.0;
      xTick[0] = xTick[0] + (this->TickLength+this->TickOffset)*sin(theta);
      xTick[1] = xTick[1] - (this->TickLength+this->TickOffset)*cos(theta);
      }
    
    offset = 0.0;
    if ( this->LabelVisibility)
      {
      offset = this->ComputeStringOffset(maxLabelStringSize[0],
                                         maxLabelStringSize[1], theta);
      }

    this->TitleMapper->GetSize(viewport, stringSize);
    this->SetOffsetPosition(xTick, theta, stringSize[0], stringSize[1], 
                            static_cast<int>(offset), this->TitleActor, 
                            this->TitleAtEnd);
    } //if title visible

  if ( this->AxisVisibility )
    {
    ptIds[0] = pts->InsertNextPoint(p1); //first axis point
    ptIds[1] = pts->InsertNextPoint(p2); //last axis point
    lines->InsertNextCell(2, ptIds);
    }

  this->BuildTime.Modified();
}

// ********************************************************************
//  Purpose:  Calculates tick locations based on the location of a
//            major tick mark and minor and major tick mark spacing.
//
//  Programmer:  Eric Brugger
//  Creation:    November 1, 2002
// ********************************************************************
  
void vtkVisItAxisActor2D::SpecifiedComputeRange(float inRange[2],
                                               float outRange[2],
                                               double majorMinimum,
                                               double majorMaximum,
                                               double majorSpacing,
                                               double minorSpacing,
                                             int &numTicks, float *proportion,
                                             float *ticksize, int minorVisible,
                                             int drawGrids)
{
  double minor_tick = 0.5;
  double major_tick = 1.;
  float grid_tick = 2.;
  double sortedRange[2], range;
  double majorStart, majorEnd, minorStart, minorEnd, location;

  outRange[0] = inRange[0];
  outRange[1] = inRange[1];

  sortedRange[0] = (double)(inRange[0] < inRange[1] ? inRange[0] : inRange[1]);
  sortedRange[1] = (double)(inRange[0] > inRange[1] ? inRange[0] : inRange[1]);

  range = sortedRange[1] - sortedRange[0];

  majorStart = majorMinimum+
               ceil((sortedRange[0]-majorMinimum) / majorSpacing) *
               majorSpacing;
  majorStart = majorStart > majorMinimum ? majorStart : majorMinimum;
  majorEnd   = majorMaximum < sortedRange[1] ? majorMaximum : sortedRange[1];
  majorEnd   = majorEnd + majorSpacing / 1e6;
  minorStart = majorMinimum+
               ceil((sortedRange[0]-majorMinimum) / minorSpacing) *
               minorSpacing;
  minorStart = minorStart > majorMinimum ? minorStart : majorMinimum;
  minorEnd   = majorMaximum < sortedRange[1] ? majorMaximum : sortedRange[1];
  minorEnd   = minorEnd + minorSpacing / 1e6;
  
  // Create all of the minor ticks
  numTicks = 0;
  if (minorVisible)
    {
    location = minorStart;
    while (location < minorEnd && numTicks < VTK_MAX_LABELS)
      {
      proportion[numTicks] = (float)((location - sortedRange[0]) / range);
      ticksize[numTicks] = (float)minor_tick;
      numTicks++;
      location += minorSpacing;
      }
    }
  // Create all of the major ticks.
  location = majorStart;
  while (location < majorEnd && numTicks < VTK_MAX_LABELS)
    {
    proportion[numTicks] = (float)((location - sortedRange[0]) / range);
    ticksize[numTicks] = (float)major_tick;
    numTicks++;
    if (drawGrids)
      {
      proportion[numTicks] = (float)((location - sortedRange[0]) / range);
      ticksize[numTicks] = grid_tick;
      numTicks++;
      }
    location += majorSpacing;
    }
  if (sortedRange[0] != (double)inRange[0])
    {
    // We must reverse all of the proportions.
    int  j;
    for (j = 0 ; j < numTicks ; j++)
      {
      proportion[j] = 1. - proportion[j];
      }
    }
}

// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Fri Sep 27 13:46:14 PDT 2002
//    Put in a tolerance to stop numerical precision errors from creating 
//    jumpy behavior.
//
// ****************************************************************************

inline double ffix(double value)
{
  int ivalue = (int)(value);
  double v = (value - ivalue);
  if (v > 0.9999)
  {
    ivalue++;
  }
  return (double) ivalue;
}

inline double fsign(double value, double sign)
{
  value = fabs(value);
  if (sign < 0.)
    {
    value *= -1.;
    }
  return value;
}

// *******************************************************************
// Method: vtkVisItAxisActor2D::AdjustLabelsComputeRange
//
// Purpose: 
//   
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Hank Childs
// Creation:   The Epoch
//
// Modifications:
//   Brad Whitlock, Wed Nov 8 18:15:43 PST 2000
//   Made the code use log10 instead of log10f when using a GNU
//   compiler. GNU compilers do not seem to have log10f defined.
//
//   Eric Brugger, Fri Nov 10 07:56:47 PST 2000
//   I modified the routine to use doubles for all internal
//   calculations.  This avoids numeric problems when the ranges are
//   large or small.  Also the above comment is not quite correct, it
//   was AIX that didn't define log10f, the GNU compiler uses the AIX
//   supplied math header files and libraries.
//
//   Kathleen Bonnell, Fri Jul  6 14:48:53 PDT 2001
//   Added support for drawing gridlines.
//
//   Akira Haddox, Wed Jul 16 16:45:48 PDT 2003
//   Added special case for when range is too small.
//
// *******************************************************************

void vtkVisItAxisActor2D::AdjustLabelsComputeRange(float inRange[2], 
                                             float outRange[2], 
                                             int vtkNotUsed(inNumTicks),
                                             int &numTicks, float *proportion, 
                                             float *ticksize, int minorVisible,
                                             int drawGrids)
{
  double minor_tick = 0.5;
  double major_tick = 1.;
  float grid_tick = 2.;
  double sortedRange[2], range;
  double fxt, fnt, frac;
  double div, major, minor;
  double majorStart, minorStart, location;

  outRange[0] = inRange[0];
  outRange[1] = inRange[1];

  sortedRange[0] = (double)(inRange[0] < inRange[1] ? inRange[0] : inRange[1]);
  sortedRange[1] = (double)(inRange[0] > inRange[1] ? inRange[0] : inRange[1]);

  range = sortedRange[1] - sortedRange[0];

  // Find the integral points.
  double pow10 = log10(range);

  // Build in numerical tolerance
  if (pow10 != 0.)
    {
    double eps = 10.0e-10;
    pow10 = fsign((fabs(pow10) + eps), pow10);
    }

  // ffix move you in the wrong direction if pow10 is negative.
  if (pow10 < 0.)
    {
    pow10 = pow10 - 1.;
    }

  fxt = pow(10., ffix(pow10));
    
  // Find the number of integral points in the interval.
  fnt  = range/fxt;
  fnt  = ffix(fnt);
  frac = fnt;
  numTicks = (frac <= 0.5 ? (int)ffix(fnt) : ((int)ffix(fnt) + 1));

  div = 1.;
  if (numTicks < 5)
    {
    div = 2.;
    }
  if (numTicks <= 2)
    {
    div = 5.;
    }

  // If there aren't enough major tick points in this decade, use the next
  // decade.
  major = fxt;
  if (div != 1.)
    {
    major /= div;
    }
  minor = (fxt/div) / 10.;

  // When we get too close, we lose the tickmarks. Run some special case code.
  if (minor == 0)
    {
    numTicks = 3;
    ticksize[0] = (float)major_tick;
    proportion[0] = 0.0;
    ticksize[1] = (float)major_tick;
    proportion[1] = 0.5;
    ticksize[2] = (float)major_tick;
    proportion[2] = 1.0;
    return;
    }

  // Figure out the first major and minor tick locations, relative to the
  // start of the axis.
  if (sortedRange[0] < 0.)
    {
    majorStart = major*(ffix(sortedRange[0]*(1./major)) + 0.);
    minorStart = minor*(ffix(sortedRange[0]*(1./minor)) + 0.);
    }
  else
    {
    majorStart = major*(ffix(sortedRange[0]*(1./major)) + 1.);
    minorStart = minor*(ffix(sortedRange[0]*(1./minor)) + 1.);
    }

  // Create all of the minor ticks
  numTicks = 0;
  if (minorVisible)
    {
    location = minorStart;
    while (location < sortedRange[1] && numTicks < VTK_MAX_LABELS)
      {
      proportion[numTicks] = (float)((location - sortedRange[0]) / range);
      ticksize[numTicks] = (float)minor_tick;
      numTicks++;
      location += minor;
      }
    }
  // Create all of the major ticks.
  location = majorStart;
  while (location < sortedRange[1] && numTicks < VTK_MAX_LABELS)
    {
    proportion[numTicks] = (float)((location - sortedRange[0]) / range);
    ticksize[numTicks] = (float)major_tick;
    numTicks++;
    if (drawGrids)
      {
      proportion[numTicks] = (float)((location - sortedRange[0]) / range);
      ticksize[numTicks] = grid_tick;
      numTicks++;
      }
    location += major;
    }
  if (sortedRange[0] != (double)inRange[0])
    {
    // We must reverse all of the proportions.
    int  j;
    for (j = 0 ; j < numTicks ; j++)
      {
      proportion[j] = 1. - proportion[j];
      }
    }
}

// Posiion text with respect to a point (xTick) where the angle of the line
// from the point to the center of the text is given by theta. The offset
// is the spacing between ticks and labels.
void vtkVisItAxisActor2D::SetOffsetPosition(float xTick[3], float theta, 
                                       int stringWidth, int stringHeight, 
                                       int offset, vtkActor2D *actor,
                                       int titleAtEnd)
{
  float x, y, center[2];
  int pos[2];
   
  if ( titleAtEnd )
    {
    pos[0] = (int)xTick[0];
    pos[1] = (int)xTick[1];
    }
  else
    {
    x = stringWidth/2.0 + offset;
    y = stringHeight/2.0 + offset;

    center[0] = xTick[0] + x*sin(theta);
    center[1] = xTick[1] - y*cos(theta);
    
    pos[0] = (int)(center[0] - stringWidth/2.0);
    pos[1] = (int)(center[1] - stringHeight/2.0);
    }
  
  actor->SetPosition(pos[0], pos[1]);
}

float vtkVisItAxisActor2D::ComputeStringOffset(float width, float height,
                                          float theta)
{
  float f1 = height*cos(theta);
  float f2 = width*sin(theta);
  return (1.2 * sqrt(f1*f1 + f2*f2));
}

// ********************************************************************
// Modifications:
//   Kathleen Bonnell, Wed Mar  6 13:48:48 PST 2002
//   Call superclass's method in the new vtk way
//
//   Eric Brugger, Tue Nov 25 11:44:40 PST 2003
//   Added the ability to specify the axis orientation angle.
//
// ********************************************************************

void vtkVisItAxisActor2D::ShallowCopy(vtkProp *prop)
{
  vtkVisItAxisActor2D *a = vtkVisItAxisActor2D::SafeDownCast(prop);
  if ( a != NULL )
    {
    this->SetPoint1(a->GetPoint1());
    this->SetPoint2(a->GetPoint2());
    this->SetUseOrientationAngle(a->GetUseOrientationAngle());
    this->SetOrientationAngle(a->GetOrientationAngle());
    this->SetRange(a->GetRange());
    this->SetNumberOfLabels(a->GetNumberOfLabels());
    this->SetLabelFormat(a->GetLabelFormat());
    this->SetAdjustLabels(a->GetAdjustLabels());
    this->SetTitle(a->GetTitle());
    this->SetBold(a->GetBold());
    this->SetItalic(a->GetItalic());
    this->SetShadow(a->GetShadow());
    this->SetFontFamily(a->GetFontFamily());
    this->SetTickLength(a->GetTickLength());
    this->SetTickOffset(a->GetTickOffset());
    this->SetAxisVisibility(a->GetAxisVisibility());
    this->SetTickVisibility(a->GetTickVisibility());
    this->SetLabelVisibility(a->GetLabelVisibility());
    this->SetTitleVisibility(a->GetTitleVisibility());
    this->SetLabelFontHeight(a->GetLabelFontHeight());
    this->SetTitleFontHeight(a->GetTitleFontHeight());
    }

  // Now do superclass
  this->Superclass::ShallowCopy(prop);
}

// ********************************************************************
//  Purpose:  Allocates memory (if necessary) for number of labels
//            being requested.
//
//  Programmer:  Kathleen Bonnell
//  Creation:    November 7, 2001
// ********************************************************************
  
void 
vtkVisItAxisActor2D::SetNumberOfLabelsBuilt(const int numLabels)
{    
  if (this->NumberOfLabelsBuilt == numLabels)
    {
    return;
    }

  if (this->LabelMappers != NULL )
    {
    for (int i=0; i < this->NumberOfLabelsBuilt; i++)
      {
      this->LabelMappers[i]->Delete();
      this->LabelActors[i]->Delete();
      }
    delete [] this->LabelMappers;
    delete [] this->LabelActors;
    }

  this->LabelMappers = new vtkTextMapper * [numLabels];
  this->LabelActors = new vtkActor2D * [numLabels];
  for ( int i=0; i < numLabels; i++)
    {
    this->LabelMappers[i] = vtkTextMapper::New();
    this->LabelActors[i] = vtkActor2D::New();
    this->LabelActors[i]->SetMapper(this->LabelMappers[i]);
    }
  this->NumberOfLabelsBuilt = numLabels;
}

//------------------------------------------------------------------------------
// Take into account the MTimes of Point1Coordinate and Point2Coordinate.
// -----------------------------------------------------------------------------
unsigned long 
vtkVisItAxisActor2D::GetMTime()
{
  unsigned long mTime = this->Superclass::GetMTime();

  unsigned long time;
  time = this->Point1Coordinate->GetMTime();
  mTime = (time > mTime ? time : mTime);
  time = this->Point2Coordinate->GetMTime();
  mTime = (time > mTime ? time : mTime);

  return mTime;
}

