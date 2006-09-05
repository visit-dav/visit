/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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


vtkStandardNewMacro(vtkLineLegend);

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

  this->BarWidth = 0.04;

  this->LastOrigin[0] = 0;
  this->LastOrigin[1] = 0;
  this->LastSize[0] = 0;
  this->LastSize[1] = 0;

  // Construct the line
  vtkLineSource *line = vtkLineSource::New();
  line->SetPoint1(0,0,0);
  line->SetPoint2(10,0,0);
  line->SetResolution(11);

  this->Transform = vtkTransform::New();
  this->TransformFilter = vtkTransformPolyDataFilter::New();
  this->TransformFilter->SetTransform(this->Transform);
  this->TransformFilter->SetInput(line->GetOutput());
  line->Delete();

  this->LineMapper = vtkPolyDataMapper2D::New();
  this->LineMapper->SetInput(this->TransformFilter->GetOutput());
  
  this->LineActor = vtkActor2D::New();
  this->LineActor->SetMapper(this->LineMapper);
}

// Release any graphics resources that are being consumed by this actor.
// The parameter window could be used to determine which graphic
// resources to release.
//-----------------------------------------------------------------------------
void vtkLineLegend::ReleaseGraphicsResources(vtkWindow *win)
{
  this->TitleActor->ReleaseGraphicsResources(win);
  this->LineActor->ReleaseGraphicsResources(win);
}


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

  this->LineActor->Delete();
  this->LineMapper->Delete();
  this->Transform->Delete();
  this->TransformFilter->Delete();
}


//-----------------------------------------------------------------------------
//  Modifications:
//
//    Hank Childs, Fri Jun  9 12:54:36 PDT 2006
//    Removed unused variable.
//
//-----------------------------------------------------------------------------
int 
vtkLineLegend::RenderOverlay(vtkViewport *viewport)
{
  int renderedSomething = 0;
  
  // Everything is built, just have to render
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
  titleOrigin[0] = 0;
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
//-----------------------------------------------------------------------------
void 
vtkLineLegend::BuildLine(vtkViewport *viewport)
{
  this->TransformFilter->GetInput()->Update();
  double *bounds = ((vtkPolyData*)this->TransformFilter->GetInput())->GetBounds();

  //Get position information
  int *x1, *x2, *x3;
  double p1[2], p2[2];
  x1 = this->PositionCoordinate->GetComputedViewportValue(viewport);
  x2 = this->Position2Coordinate->GetComputedViewportValue(viewport);
  x3 = this->TitleActor->GetPositionCoordinate()->
             GetComputedViewportValue(viewport);

  p1[0] = (double)x1[0]; 
  p1[1] = (double)x1[1]; 
  p2[0] = (double)x2[0]; 
  p2[1] = (double)x3[1]; 

  int tempi[2];
  double sf, twr, swr = 0.0;

  if ( (bounds[3]-bounds[2]) == 0.0 )
    {
    swr = 1.0;
    }
  else
    {
    swr = (bounds[1]-bounds[0]) / (bounds[3]-bounds[2]);
    }

  this->TitleMapper->GetSize(viewport,tempi);
  twr = (double)tempi[0]/tempi[1];
  double symbolSize = swr / (swr + twr);

  int size[2];
  size[0] = (int)(symbolSize*(p2[0] - p1[0]));
  size[1] = (int)((p2[1] - p1[1]));

  if ( (bounds[1]-bounds[0]) == 0.0 ) 
    { 
    sf = VTK_LARGE_FLOAT; 
    }
  else 
    { 
    sf = size[0]/(bounds[1]-bounds[0]); 
    }
        
  if ( (bounds[3]-bounds[2]) == 0.0 )
    {
    if ( sf >= VTK_LARGE_FLOAT )          
      {
      sf = 1.0;
      }
    }
  else if ( (size[1]/(bounds[3]-bounds[2])) < sf )
    {
    sf = size[1]/(bounds[3]-bounds[2]);
    }

  double posX = p1[0];
  double posY = p2[1] - 0.1*size[1]; 
  this->Transform->Identity();
  this->Transform->Translate(posX, posY, 0.0);
  this->Transform->Scale(sf, sf, 1);
  this->BuildTime.Modified();
}


//-----------------------------------------------------------------------------
//  Modifications:
//
//    Hank Childs, Fri Jun  9 12:54:36 PDT 2006
//    Removed unused variable.
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

    this->BuildTime.Modified();
    }

  // Everything is built, just have to render

  if (this->Title != NULL && this->TitleOkayToDraw && this->TitleVisibility)
    {
    renderedSomething += this->TitleActor->RenderOpaqueGeometry(viewport);
    }
  renderedSomething += this->LineActor->RenderOpaqueGeometry(viewport);
  renderedSomething = (renderedSomething > 0)?(1):(0);
  return renderedSomething;
}

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
  os << indent << "Position: " << this->PositionCoordinate << "\n";
  this->PositionCoordinate->PrintSelf(os, indent.GetNextIndent());
  os << indent << "Width: " << this->GetWidth() << "\n";
  os << indent << "Height: " << this->GetHeight() << "\n";
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

