/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

//=========================================================================
//
//  Class:     vtkLineLegend
//  
//  Purpose:
//    Derived type of vtkActor2D. 
//    Creates a legend that indicates to the viewer the correspondence between
//    color value and data value. 
//
//=========================================================================
#include "vtkLineLegend.h"

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkObjectFactory.h>
#include <vtkLineSource.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProperty2D.h>
#include <vtkTextProperty.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkViewport.h>
#include <vtkWindow.h>
#include <limits.h>
#include <float.h>

#define LEFT_OFFSET 0.045

vtkStandardNewMacro(vtkLineLegend);

// Modifications:
//   Brad Whitlock, Thu Mar 22 00:21:44 PDT 2007
//   Added bounding box.
//
//-----------------------------------------------------------------------------
vtkLineLegend::vtkLineLegend()  
{
  this->Position2Coordinate = vtkCoordinate::New();
  this->Position2Coordinate->SetCoordinateSystemToNormalizedViewport();
  this->Position2Coordinate->SetValue(0.05, 0.8);
  this->Position2Coordinate->SetReferenceCoordinate(this->PositionCoordinate);
  
  this->PositionCoordinate->SetCoordinateSystemToNormalizedViewport();
  this->PositionCoordinate->SetValue(0.82, 0.1);

  this->Bold = 0;
  this->Italic = 0;
  this->Shadow = 0;
  this->FontFamily = VTK_ARIAL;
  this->FontHeight = 0.015;
  
  this->Title = NULL;
  this->TitleMapper = vtkTextMapper::New();
  this->TitleMapper->GetTextProperty()->SetJustificationToLeft();
  this->TitleActor = vtkActor2D::New();
  this->TitleActor->SetMapper(this->TitleMapper);
  this->TitleActor->GetPositionCoordinate()->
                    SetReferenceCoordinate(this->PositionCoordinate);
  this->TitleVisibility = 1;
  this->TitleOkayToDraw = 1;

  this->BoundingBox = vtkPolyData::New();
  this->BoundingBoxMapper = vtkPolyDataMapper2D::New();
  this->BoundingBoxMapper->SetInput(this->BoundingBox);
  this->BoundingBoxActor = vtkActor2D::New();
  this->BoundingBoxActor->SetMapper(this->BoundingBoxMapper);
  this->BoundingBoxActor->GetPositionCoordinate()->
    SetReferenceCoordinate(this->PositionCoordinate);
  this->BoundingBoxColor[0] = 0.8;
  this->BoundingBoxColor[1] = 0.8;
  this->BoundingBoxColor[2] = 0.8;
  this->BoundingBoxColor[3] = 1.;
  this->BoundingBoxVisibility = 0;

  this->BarWidth = 0.04;

  this->LastOrigin[0] = 0;
  this->LastOrigin[1] = 0;
  this->LastSize[0] = 0;
  this->LastSize[1] = 0;

  this->Line = vtkPolyData::New();
  this->LineMapper = vtkPolyDataMapper2D::New();
  this->LineMapper->SetInput(this->Line);
  this->LineActor = vtkActor2D::New();
  this->LineActor->SetMapper(this->LineMapper);
  this->LineActor->GetPositionCoordinate()->
    SetReferenceCoordinate(this->PositionCoordinate);
}

// Release any graphics resources that are being consumed by this actor.
// The parameter window could be used to determine which graphic
// resources to release.
//
// Modifications:
//   Brad Whitlock, Thu Mar 22 00:23:08 PDT 2007
//   Added bounding box.
//
//-----------------------------------------------------------------------------
void vtkLineLegend::ReleaseGraphicsResources(vtkWindow *win)
{
  this->TitleActor->ReleaseGraphicsResources(win);
  this->LineActor->ReleaseGraphicsResources(win);
  this->BoundingBoxActor->ReleaseGraphicsResources(win);
}


// Modifications:
//   Brad Whitlock, Thu Mar 22 00:23:22 PDT 2007
//   Added bounding box.
//
//-----------------------------------------------------------------------------
vtkLineLegend::~vtkLineLegend()
{
  this->Position2Coordinate->Delete();
  this->Position2Coordinate = NULL;
  
  if (this->Title) 
    {
    delete [] this->Title;
    this->Title = NULL;
    }

  this->TitleMapper->Delete();
  this->TitleActor->Delete();

  this->BoundingBox->Delete();
  this->BoundingBoxMapper->Delete();
  this->BoundingBoxActor->Delete();

  this->Line->Delete();
  this->LineActor->Delete();
  this->LineMapper->Delete();
}


//-----------------------------------------------------------------------------
//  Modifications:
//    Hank Childs, Fri Jun  9 12:54:36 PDT 2006
//    Removed unused variable.
//
//    Brad Whitlock, Thu Mar 22 00:24:03 PDT 2007
//    Added bounding box.
//
//-----------------------------------------------------------------------------
int 
vtkLineLegend::RenderOverlay(vtkViewport *viewport)
{
  int renderedSomething = 0;
  
  // Everything is built, just have to render
  if (this->BoundingBoxVisibility)
    {
    this->BoundingBoxActor->RenderOverlay(viewport);
    }
  if (this->Title != NULL && this->TitleOkayToDraw && this->TitleVisibility)
    {
    renderedSomething += this->TitleActor->RenderOverlay(viewport);
    }
  renderedSomething += this->LineActor->RenderOverlay(viewport);

  renderedSomething = (renderedSomething > 0)?(1):(0);

  return renderedSomething;
}


// Build the title for this actor 
//-----------------------------------------------------------------------------
void 
vtkLineLegend::BuildTitle(vtkViewport *viewport)
{
  double titleOrigin[3] = { 0., 0., 0. };
  this->TitleMapper->SetInput(this->Title);
  int *viewSize = viewport->GetSize();

  if (0 == viewSize[0]  && 0 == viewSize[1] )
    {
    this->TitleOkayToDraw = 0;
    return;
    }
 
  //
  // Set the font properties.
  //
  int fontSize = (int)(FontHeight * viewSize[1]); 
  vtkTextProperty *tprop = this->TitleMapper->GetTextProperty();
  tprop->SetFontSize(fontSize);
  tprop->SetBold(this->Bold);
  tprop->SetItalic(this->Italic);
  tprop->SetShadow(this->Shadow);
  tprop->SetFontFamily(this->FontFamily);
  tprop->SetColor(this->GetProperty()->GetColor());

  //
  // Set the title position in reference to the legend origin as a
  // percentage of the viewport.
  //
  int tsizePixels[2];
  this->TitleMapper->GetSize(viewport, tsizePixels); 
  titleOrigin[0] = 0.;
  int legURy = LastOrigin[1] + LastSize[1];
  int distFromOrigin = (legURy-tsizePixels[1]-LastOrigin[1]);
  titleOrigin[1] = (double)(distFromOrigin) /(double)viewSize[1] ; 
  this->TitleActor->SetProperty(this->GetProperty());

  this->TitleActor->GetPositionCoordinate()->
                    SetCoordinateSystemToNormalizedViewport();
  this->TitleActor->GetPositionCoordinate()->SetValue(titleOrigin);
  this->TitleOkayToDraw = 1;
}

// Build the line for this actor 
//-----------------------------------------------------------------------------
//  Modifications:
//
//    Hank Childs, Fri Jun  9 12:54:36 PDT 2006
//    Removed unused variable.
//
//    Brad Whitlock, Thu Mar 22 01:33:35 PDT 2007
//    Rewrote.
//
//-----------------------------------------------------------------------------
void 
vtkLineLegend::BuildLine(vtkViewport *viewport)
{
  int *viewSize = viewport->GetSize();

  //
  // Build bounding box object
  //
  vtkPoints *pts = vtkPoints::New();
  pts->SetNumberOfPoints(2);
  vtkCellArray *lines = vtkCellArray::New();
  lines->Allocate(2); 

  this->Line->Initialize();
  this->Line->SetPoints(pts);
  this->Line->SetLines(lines);
  pts->Delete(); lines->Delete();

  //
  // generate points for bounding box
  //
  int *LL = this->GetPositionCoordinate()->
                GetComputedViewportValue(viewport);
  int *UR = this->GetPosition2Coordinate()->
                GetComputedViewportValue(viewport);
  double width = UR[0] - LL[0];
  double height = UR[1] - LL[1];
  double maxX = UR[0];

  // Need to account for the widest text to accurately calculate the width.
  if(this->TitleVisibility)
    {
        int *titleLL = this->TitleActor->GetPositionCoordinate()->
            GetComputedViewportValue(viewport);
        maxX = titleLL[0] + 0.06 * viewSize[0];
    }

  width = maxX - LL[0];

  const double border = 4;
  double pt[3];
  pt[0] = 0. - LEFT_OFFSET * viewSize[0];
  pt[1] = 0.;
  pt[2] = 0.;
  pts->SetPoint(0, pt);

  pt[0] = width;
  pt[1] = 0.;
  pts->SetPoint(1, pt);

  //
  // Polygon
  //
  vtkIdType ptIds[2] = {0,1};
  lines->InsertNextCell(2,ptIds);
}

// ****************************************************************************
// Method: vtkLineLegend::BuildBoundingBox
//
// Purpose: 
//   Builds the bounding box.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 21 16:24:05 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void vtkLineLegend::BuildBoundingBox(vtkViewport *viewport)
{
  int *viewSize = viewport->GetSize();

  //
  // Build bounding box object
  //
  vtkPoints *pts = vtkPoints::New();
  pts->SetNumberOfPoints(4);
  vtkCellArray *polys = vtkCellArray::New();
  polys->Allocate(4); 
  vtkUnsignedCharArray *colors = vtkUnsignedCharArray::New();
  colors->SetNumberOfComponents(4);
  colors->SetNumberOfTuples(1);

  this->BoundingBoxActor->SetProperty(this->GetProperty());
  this->BoundingBox->Initialize();
  this->BoundingBox->SetPoints(pts);
  this->BoundingBox->SetPolys(polys);
  this->BoundingBox->GetCellData()->SetScalars(colors);
  pts->Delete(); polys->Delete(); colors->Delete(); 

  //
  // generate points for bounding box
  //
  int *LL = this->GetPositionCoordinate()->
                GetComputedViewportValue(viewport);
  int *UR = this->GetPosition2Coordinate()->
                GetComputedViewportValue(viewport);
  double width = UR[0] - LL[0];
  double height = UR[1] - LL[1];
  double maxX = UR[0];

  // Need to account for the widest text to accurately calculate the width.
  if(this->TitleVisibility)
    {
        int *titleLL = this->TitleActor->GetPositionCoordinate()->
            GetComputedViewportValue(viewport);
        int rightX = titleLL[0] + 
            this->TitleMapper->GetWidth(viewport);
        if(rightX > maxX)
            maxX = rightX;
    }

  width = maxX - LL[0];

  double leftOffset = LEFT_OFFSET * viewSize[0];
  const double border = 4;
  double pt[3];
  pt[0] = 0. - leftOffset - border;
  pt[1] = 0. - border;
  pt[2] = 0.;
  pts->SetPoint(0, pt);

  pt[0] = width;
  pt[1] = 0. - border;
  pts->SetPoint(1, pt);

  pt[0] = width;
  pt[1] = height;
  pts->SetPoint(2, pt);

  pt[0] = 0.- leftOffset - border;
  pt[1] = height;
  pts->SetPoint(3, pt);

  //
  // Polygon
  //
  vtkIdType ptIds[4] = {0,1,2,3};
  polys->InsertNextCell(4,ptIds);

  //
  // Color
  //
  unsigned char *rgba = colors->GetPointer(0);
  rgba[0] = (unsigned char)(int(this->BoundingBoxColor[0] * 255.));
  rgba[1] = (unsigned char)(int(this->BoundingBoxColor[1] * 255.));
  rgba[2] = (unsigned char)(int(this->BoundingBoxColor[2] * 255.));
  rgba[3] = (unsigned char)(int(this->BoundingBoxColor[3] * 255.));
} // BuildBoundingBox

//-----------------------------------------------------------------------------
//  Modifications:
//
//    Hank Childs, Fri Jun  9 12:54:36 PDT 2006
//    Removed unused variable.
//
//    Brad Whitlock, Thu Mar 22 00:25:54 PDT 2007
//    Added bounding box.
//
//-----------------------------------------------------------------------------
int 
vtkLineLegend::RenderOpaqueGeometry(vtkViewport *viewport)
{
  int renderedSomething = 0;
 
  // Check to see whether we have to rebuild everything
  if ( viewport->GetMTime() > this->BuildTime || 
       ( viewport->GetVTKWindow() && 
         viewport->GetVTKWindow()->GetMTime() > this->BuildTime ) )
    {
    // if the viewport has changed we may - or may not need
    // to rebuild, it depends on if the projected coords change
    int *barOrigin, *barUR;
    int size[2];
    barOrigin = this->PositionCoordinate->GetComputedViewportValue(viewport);
    barUR = this->Position2Coordinate->GetComputedViewportValue(viewport);
    size[0] =  barUR[0] - barOrigin[0];
    size[1] =  barUR[1] - barOrigin[1];

    if (this->LastSize[0] != size[0] || this->LastSize[1] != size[1] ||
        this->LastOrigin[0] != barOrigin[0] || 
        this->LastOrigin[1] != barOrigin[1])
      {
      this->Modified();
      }
    }
  
  // Check to see whether we have to rebuild everything
  if ( this->GetMTime() > this->BuildTime )
    {
    vtkDebugMacro(<<"Rebuilding sub-objects");

    int *legOrigin, *legUR;
    legOrigin = this->PositionCoordinate-> GetComputedViewportValue(viewport);
    legUR = this->Position2Coordinate->GetComputedViewportValue(viewport);
    this->LastOrigin[0] = legOrigin[0];
    this->LastOrigin[1] = legOrigin[1];
    this->LastSize[0] = legUR[0] - legOrigin[0];
    this->LastSize[1] = legUR[1] - legOrigin[1];
    if ( this->TitleVisibility )
      {
      if ( this->Title != NULL )
        this->BuildTitle(viewport); 
      else
        this->TitleOkayToDraw = 0; 
      }

    this->BuildLine(viewport); 
    if( this->BoundingBoxVisibility )
      this->BuildBoundingBox(viewport);

    this->BuildTime.Modified();
    }

  // Everything is built, just have to render
  if ( this->BoundingBoxVisibility )
    {
    renderedSomething += this->BoundingBoxActor->RenderOpaqueGeometry(viewport);
    }
  if (this->Title != NULL && this->TitleOkayToDraw && this->TitleVisibility)
    {
    renderedSomething += this->TitleActor->RenderOpaqueGeometry(viewport);
    }
  renderedSomething += this->LineActor->RenderOpaqueGeometry(viewport);
  renderedSomething = (renderedSomething > 0)?(1):(0);
  return renderedSomething;
}

// Modifications:
//   Brad Whitlock, Thu Mar 22 00:27:17 PDT 2007
//   Added bounding box.
//
//----------------------------------------------------------------------------
void 
vtkLineLegend::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Title: " << (this->Title ? this->Title : "(none)") << "\n";

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

  os << indent << "FontHeight: " << this->FontHeight << "\n";
  os << indent << "Bold: " << (this->Bold ? "On\n" : "Off\n");
  os << indent << "Italic: " << (this->Italic ? "On\n" : "Off\n");
  os << indent << "Shadow: " << (this->Shadow ? "On\n" : "Off\n");

  os << indent << "Bar Width: " << this->BarWidth << "\n";
  os << indent << "Title Visibility: " 
     << (this->TitleVisibility ? "On\n" : "Off\n");
  os << indent << "BoundingBox Visibility: " 
     << (this->BoundingBoxVisibility ? "On\n" : "Off\n");

  os << indent << "Position: " << this->PositionCoordinate << "\n";
  this->PositionCoordinate->PrintSelf(os, indent.GetNextIndent());
  os << indent << "Width: " << this->GetWidth() << "\n";
  os << indent << "Height: " << this->GetHeight() << "\n";
  os << indent << "BoundingBoxColor: "
     << this->BoundingBoxColor[0] << ", "
     << this->BoundingBoxColor[1] << ", "
     << this->BoundingBoxColor[2] << ", "
     << this->BoundingBoxColor[3] << "\n";
}


//----------------------------------------------------------------------------
void 
vtkLineLegend::SetPosition(double x[2]) 
{
  this->SetPosition(x[0],x[1]);
} 

//----------------------------------------------------------------------------
void 
vtkLineLegend::SetPosition(double x, double y) 
{ 
  this->PositionCoordinate->SetCoordinateSystemToNormalizedViewport(); 
  this->PositionCoordinate->SetValue(x,y); 
} 

//----------------------------------------------------------------------------
vtkCoordinate *
vtkLineLegend::GetPosition2Coordinate() 
{ 
  return this->Position2Coordinate; 
} 

//----------------------------------------------------------------------------
void 
vtkLineLegend::SetPosition2(double x[2]) 
{
  this->SetPosition2(x[0],x[1]);
} 

//----------------------------------------------------------------------------
void 
vtkLineLegend::SetPosition2(double x, double y) 
{ 
  this->Position2Coordinate->SetCoordinateSystemToNormalizedViewport(); 
  this->Position2Coordinate->SetValue(x,y); 
} 

//----------------------------------------------------------------------------
double *
vtkLineLegend::GetPosition2() 
{ 
  return this->Position2Coordinate->GetValue(); 
}

// Modifications:
//   Brad Whitlock, Thu Mar 22 00:27:49 PDT 2007
//   Added bounding box.
//
//----------------------------------------------------------------------------
void 
vtkLineLegend::ShallowCopy(vtkProp *prop)
{
  vtkLineLegend *a = vtkLineLegend::SafeDownCast(prop);
  if ( a != NULL )
    {
    this->SetBold(a->GetBold());
    this->SetItalic(a->GetItalic());
    this->SetShadow(a->GetShadow());
    this->SetFontFamily(a->GetFontFamily());
    this->SetFontHeight(a->GetFontHeight());
    this->SetTitle(a->GetTitle());

    this->SetBarWidth(a->GetBarWidth());

    this->SetTitleVisibility(a->GetTitleVisibility());
    this->SetBoundingBoxVisibility(a->GetBoundingBoxVisibility());

    this->GetPositionCoordinate()->SetCoordinateSystem(
      a->GetPositionCoordinate()->GetCoordinateSystem());    
    this->GetPositionCoordinate()->SetValue(
      a->GetPositionCoordinate()->GetValue());

    this->GetPosition2Coordinate()->SetCoordinateSystem(
      a->GetPosition2Coordinate()->GetCoordinateSystem());    
    this->GetPosition2Coordinate()->SetValue(
      a->GetPosition2Coordinate()->GetValue());
    }

  // Now do superclass
  this->Superclass::ShallowCopy(prop);
}

