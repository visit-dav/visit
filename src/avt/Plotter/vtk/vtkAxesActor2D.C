/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <vtkObjectFactory.h>
#include <vtkProperty2D.h>
#include "vtkAxesActor2D.h"

vtkStandardNewMacro(vtkAxesActor2D);


// **********************************************************************
// Instantiate this object.
//
//  Modifications:
//
// **********************************************************************

vtkAxesActor2D::vtkAxesActor2D()
{
  this->XAxis = vtkVisItAxisActor2D::New();
  this->XAxis->SetTickVisibility(1);
  this->XAxis->SetLabelVisibility(1);
  this->XAxis->SetTitleVisibility(1);
  this->XAxis->SetFontFamilyToCourier();
  this->XAxis->SetLabelFontHeight(0.02);
  this->XAxis->SetTitleFontHeight(0.02);
  this->XAxis->SetShadow(0);
  this->XAxis->SetAdjustLabels(1);
  this->XAxis->GetPoint1Coordinate()->SetCoordinateSystemToNormalizedViewport();
  this->XAxis->GetPoint2Coordinate()->SetCoordinateSystemToNormalizedViewport();
  this->XAxis->PickableOff();
  this->XAxis->SetUseSeparateColors(1);
  this->XAxis->SetGridlineXLength(0);
  this->XAxis->SetUseOrientationAngle(1);
  this->XAxis->SetOrientationAngle(0.);

  this->YAxis = vtkVisItAxisActor2D::New();
  this->YAxis->SetTickVisibility(1);
  this->YAxis->SetLabelVisibility(1);
  this->YAxis->SetTitleVisibility(1);
  this->YAxis->SetFontFamilyToCourier();
  this->YAxis->SetLabelFontHeight(0.02);
  this->YAxis->SetTitleFontHeight(0.02);
  this->YAxis->SetShadow(0);
  this->YAxis->SetAdjustLabels(1);
  this->YAxis->GetPoint1Coordinate()->SetCoordinateSystemToNormalizedViewport();
  this->YAxis->GetPoint2Coordinate()->SetCoordinateSystemToNormalizedViewport();
  this->YAxis->PickableOff();
  this->YAxis->SetUseSeparateColors(1);
  this->YAxis->SetGridlineYLength(0);
  this->YAxis->SetUseOrientationAngle(1);
  this->YAxis->SetOrientationAngle(-1.5707963);
}

vtkAxesActor2D::~vtkAxesActor2D()
{
  if (this->XAxis)
    {
    this->XAxis->Delete();
    this->XAxis = NULL;
    }
  
  if (this->YAxis)
    {
    this->YAxis->Delete();
    this->YAxis = NULL;
    }
}


void 
vtkAxesActor2D::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "XAxis: \n";
  this->XAxis->PrintSelf(os, indent.GetNextIndent());
  os << indent << "YAxis: \n";
  this->YAxis->PrintSelf(os, indent.GetNextIndent());
}


// ****************************************************************************
// Render the axis, ticks, title, and labels.
// ****************************************************************************

int 
vtkAxesActor2D::RenderOverlay(vtkViewport *viewport)
{
  int renderedSomething=0;

  renderedSomething += this->XAxis->RenderOverlay(viewport);
  renderedSomething += this->YAxis->RenderOverlay(viewport);

  return renderedSomething;
}


// ****************************************************************************
// Render the axis, ticks, title, and labels.
// ****************************************************************************

int 
vtkAxesActor2D::RenderOpaqueGeometry(vtkViewport *viewport)
{
  int renderedSomething=0;

  if (this->XAxis->GetDrawGridlines() || this->YAxis->GetDrawGridlines())
    {
    int *x = this->XAxis->GetPoint2Coordinate()->GetComputedViewportValue(viewport);
    int *y = this->YAxis->GetPoint1Coordinate()->GetComputedViewportValue(viewport);
    this->XAxis->SetGridlineYLength(abs(x[1] - y[1]));
    this->YAxis->SetGridlineXLength(abs(x[0] - y[0]));
    }
  renderedSomething += this->XAxis->RenderOpaqueGeometry(viewport);
  renderedSomething += this->YAxis->RenderOpaqueGeometry(viewport);

  return renderedSomething;
}


// ****************************************************************************
// Release any graphics resources that are being consumed by this actor.
// The parameter window could be used to determine which graphic
// resources to release.
// ****************************************************************************

void 
vtkAxesActor2D::ReleaseGraphicsResources(vtkWindow *win)
{
  this->XAxis->ReleaseGraphicsResources(win);
  this->YAxis->ReleaseGraphicsResources(win);
}


// ****************************************************************************
// Set the viewport. The viewport controls the coordinate values.
// ****************************************************************************

void 
vtkAxesActor2D::SetCoordinateValuesFromViewport(double vl, double vb, 
                                                double vr, double vt)
{
  this->XAxis->GetPoint1Coordinate()->SetValue(vl, vb);
  this->XAxis->GetPoint2Coordinate()->SetValue(vr, vb);

  //
  // Make coordinates for y-axis backwards so the labels will appear on
  // the left side and out of the viewport.
  //
  this->YAxis->GetPoint1Coordinate()->SetValue(vl, vt);
  this->YAxis->GetPoint2Coordinate()->SetValue(vl, vb);
}


// ****************************************************************************
// Sets the line width
// ****************************************************************************

void 
vtkAxesActor2D::SetLineWidth(int width)
{
  this->XAxis->GetProperty()->SetLineWidth(width);
  this->YAxis->GetProperty()->SetLineWidth(width);
}


// ****************************************************************************
// Set the color for the axes
// ****************************************************************************

void 
vtkAxesActor2D::SetColor(double r, double g, double b)
{
  this->XAxis->GetProperty()->SetColor(r, g, b);
  this->YAxis->GetProperty()->SetColor(r, g, b);
}


// ****************************************************************************
// Set the log scaling
// ****************************************************************************

void 
vtkAxesActor2D::SetXLogScaling(int logScale)
{
  this->XAxis->SetLogScale(logScale);
}

void 
vtkAxesActor2D::SetYLogScaling(int logScale)
{
  this->YAxis->SetLogScale(logScale);
}

// ****************************************************************************
// LABEL CONTROLS
// ****************************************************************************

// ****************************************************************************
// Set the label range of the axes.
// ****************************************************************************

void 
vtkAxesActor2D::SetXRange(double minX, double maxX)
{
  this->XAxis->SetRange(minX, maxX);
}

void
vtkAxesActor2D::SetYRange(double minY, double maxY)
{
  //
  // We put the y-axis in reverse so that its labels would appear on the
  // correct side of the viewport.  Must propogate kludge by sending
  // range in backwards.
  //
  this->YAxis->SetRange(maxY, minY);
}


// ****************************************************************************
// Set the visibility of labels
// ****************************************************************************

void 
vtkAxesActor2D::SetXLabelVisibility(int vis)
{
  this->XAxis->SetLabelVisibility(vis);
}

void 
vtkAxesActor2D::SetYLabelVisibility(int vis)
{
  this->YAxis->SetLabelVisibility(vis);
}


// ****************************************************************************
// Set the major tick label scale
// ****************************************************************************

void 
vtkAxesActor2D::SetXLabelScale(double labelScale)
{
  this->XAxis->SetMajorTickLabelScale(labelScale);
}

void 
vtkAxesActor2D::SetYLabelScale(double labelScale)
{
  this->YAxis->SetMajorTickLabelScale(labelScale);
}


// ****************************************************************************
// Set the label format string
// ****************************************************************************

void 
vtkAxesActor2D::SetXLabelFormatString(char *format)
{
  this->XAxis->SetLabelFormat(format);
}

void 
vtkAxesActor2D::SetYLabelFormatString(char *format)
{
  this->YAxis->SetLabelFormat(format);
}


// ****************************************************************************
// Set the label format string for log scaling
// ****************************************************************************

void 
vtkAxesActor2D::SetXLogLabelFormatString(char *format)
{
  this->XAxis->SetLogLabelFormat(format);
}

void 
vtkAxesActor2D::SetYLogLabelFormatString(char *format)
{
  this->YAxis->SetLogLabelFormat(format);
}


// ****************************************************************************
// Set the label font height
// ****************************************************************************

void 
vtkAxesActor2D::SetXLabelFontHeight(double height)
{
  this->XAxis->SetLabelFontHeight(height);
}

void 
vtkAxesActor2D::SetYLabelFontHeight(double height)
{
  this->YAxis->SetLabelFontHeight(height);
}


// ****************************************************************************
// Set the flag specifying if labels are auto adjusted
// ****************************************************************************

void 
vtkAxesActor2D::SetAutoAdjustLabels(int autoAdjust)
{
  this->XAxis->SetAdjustLabels(autoAdjust);
  this->YAxis->SetAdjustLabels(autoAdjust);
}


// ****************************************************************************
// Retrieve the text property for labels
// ****************************************************************************

vtkTextProperty *
vtkAxesActor2D::GetXLabelTextProperty()
{
  return this->XAxis->GetLabelTextProperty();
}

vtkTextProperty *
vtkAxesActor2D::GetYLabelTextProperty()
{
  return this->YAxis->GetLabelTextProperty();
}


// ****************************************************************************
// TITLE CONTROLS
// ****************************************************************************


// ****************************************************************************
// Set the title 
// ****************************************************************************

void 
vtkAxesActor2D::SetXTitle(char *title)
{
  this->XAxis->SetTitle(title);
}

void 
vtkAxesActor2D::SetYTitle(char *title)
{
  this->YAxis->SetTitle(title);
}


// ****************************************************************************
// Set the title font height
// ****************************************************************************

void 
vtkAxesActor2D::SetXTitleFontHeight(double height)
{
  this->XAxis->SetTitleFontHeight(height);
}

void 
vtkAxesActor2D::SetYTitleFontHeight(double height)
{
  this->YAxis->SetTitleFontHeight(height);
}


// ****************************************************************************
// Set the visibility of title
// ****************************************************************************

void 
vtkAxesActor2D::SetXTitleVisibility(int vis)
{
  this->XAxis->SetTitleVisibility(vis);
}

void 
vtkAxesActor2D::SetYTitleVisibility(int vis)
{
  this->YAxis->SetTitleVisibility(vis);
}


// ****************************************************************************
// Retrieve the text property for titles
// ****************************************************************************

vtkTextProperty *
vtkAxesActor2D::GetXTitleTextProperty()
{
  return this->XAxis->GetTitleTextProperty();
}

vtkTextProperty *
vtkAxesActor2D::GetYTitleTextProperty()
{
  return this->YAxis->GetTitleTextProperty();
}


// ****************************************************************************
// TICK CONTROLS
// ****************************************************************************


// ****************************************************************************
// Set the location for the tick marks.
// ****************************************************************************

void 
vtkAxesActor2D::SetTickLocation(int loc)
{
  this->XAxis->SetTickLocation(loc);
  this->YAxis->SetTickLocation(loc);
}


// ****************************************************************************
// Set the visibility of ticks.
// ****************************************************************************

void 
vtkAxesActor2D::SetXMinorTickVisibility(int vis)
{
  this->XAxis->SetMinorTicksVisible(vis);
}

void 
vtkAxesActor2D::SetXMajorTickVisibility(int vis)
{
  this->XAxis->SetTickVisibility(vis);
}

void 
vtkAxesActor2D::SetYMinorTickVisibility(int vis)
{
  this->YAxis->SetMinorTicksVisible(vis);
}

void 
vtkAxesActor2D::SetYMajorTickVisibility(int vis)
{
  this->YAxis->SetTickVisibility(vis);
}


// ****************************************************************************
// Sets the minimum value for Major Ticks
// ****************************************************************************

void 
vtkAxesActor2D::SetXMajorTickMinimum(double minV)
{
  this->XAxis->SetMajorTickMinimum(minV);
}

void 
vtkAxesActor2D::SetYMajorTickMinimum(double minV)
{
  this->YAxis->SetMajorTickMinimum(minV);
}


// ****************************************************************************
// Sets the maximum value for Major Ticks
// ****************************************************************************

void 
vtkAxesActor2D::SetXMajorTickMaximum(double maxV)
{
  this->XAxis->SetMajorTickMaximum(maxV);
}

void 
vtkAxesActor2D::SetYMajorTickMaximum(double maxV)
{
  this->YAxis->SetMajorTickMaximum(maxV);
}


// ****************************************************************************
// Sets the spacing for Major Ticks
// ****************************************************************************

void 
vtkAxesActor2D::SetXMajorTickSpacing(double spacing)
{
  this->XAxis->SetMajorTickSpacing(spacing);
}

void 
vtkAxesActor2D::SetYMajorTickSpacing(double spacing)
{
  this->YAxis->SetMajorTickSpacing(spacing);
}


// ****************************************************************************
// Sets the spacing for Minor Ticks
// ****************************************************************************

void 
vtkAxesActor2D::SetXMinorTickSpacing(double spacing)
{
  this->XAxis->SetMinorTickSpacing(spacing);
}

void 
vtkAxesActor2D::SetYMinorTickSpacing(double spacing)
{
  this->YAxis->SetMinorTickSpacing(spacing);
}


// ****************************************************************************
// GRID CONTROLS
// ****************************************************************************


// ****************************************************************************
// Set the visibility of gridlines
// ****************************************************************************

void 
vtkAxesActor2D::SetXGridVisibility(int vis)
{
  this->XAxis->SetDrawGridlines(vis);
}

void 
vtkAxesActor2D::SetYGridVisibility(int vis)
{
  this->YAxis->SetDrawGridlines(vis);
}


// ****************************************************************************
// Take into account the MTimes of the different axes
//
// ****************************************************************************

unsigned long 
vtkAxesActor2D::GetMTime()
{
  unsigned long mTime = this->Superclass::GetMTime();

  unsigned long time;
  time = this->XAxis->GetMTime();
  mTime = (time > mTime ? time : mTime);
  time = this->YAxis->GetMTime();
  mTime = (time > mTime ? time : mTime);

  return mTime;
}
