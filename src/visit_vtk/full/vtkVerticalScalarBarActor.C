/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

//=========================================================================
//
//  Class:     vtkVerticalScalarBarActor
//  
//  Purpose:
//    Derived type of vtkActor2D. 
//    Creates a legend that indicates to the viewer the correspondence between
//    color value and data value. 
//
//=========================================================================
#include "vtkVerticalScalarBarActor.h"

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkProperty2D.h>
#include <vtkTextProperty.h>
#include <vtkViewport.h>
#include <vtkWindow.h>
#include <limits.h>
#include <float.h>

#define DefaultNumLabels 5

//------------------------------------------------------------------------------
//  Modifications:
//    Kathleen Bonnell, Wed Mar  6 17:10:03 PST 2002 
//    Replace 'New' method with Macro to match VTK 4.0 API.
//------------------------------------------------------------------------------

vtkStandardNewMacro(vtkVerticalScalarBarActor);

//------------------------------------------------------------------------------
// Instantiate this object
//
//  Modifications:
//    Kathleen Bonnell, Mon May 19 13:42:19 PDT 2003   
//    Initialize new member 'ReverseOrder' 
//
//    Eric Brugger, Mon Jul 14 11:59:40 PDT 2003
//    I deleted TitleFraction, LabelFraction and FontSize.  I added BarWidth
//    and FontHeight.  I changed the default range format.
//
//    Brad Whitlock, Wed Mar 21 16:02:00 PST 2007
//    Added BoundingBox.
//
//------------------------------------------------------------------------------
vtkVerticalScalarBarActor::vtkVerticalScalarBarActor() : definedLabels(), labelColorMap()
{
  this->LookupTable = NULL;
  this->Position2Coordinate = vtkCoordinate::New();
  this->Position2Coordinate->SetCoordinateSystemToNormalizedViewport();
  this->Position2Coordinate->SetValue(0.05, 0.8);
  this->Position2Coordinate->SetReferenceCoordinate(this->PositionCoordinate);
  
  this->PositionCoordinate->SetCoordinateSystemToNormalizedViewport();
  this->PositionCoordinate->SetValue(0.82, 0.1);
  
  this->MaximumNumberOfColors = 64;
  this->NumberOfLabels = DefaultNumLabels;
  this->NumberOfLabelsBuilt = 0;
  this->Title = NULL;
  this->AltTitle = NULL;
  this->BarWidth = 0.04;

  this->Bold = 0;
  this->Italic = 0;
  this->Shadow = 0;
  this->FontFamily = VTK_ARIAL;
  this->FontHeight = 0.015;
  this->LabelFormat = new char[10]; 
  sprintf(this->LabelFormat,"%s","%# -9.4g");
  this->RangeFormat = new char[30]; 
  sprintf(this->RangeFormat, "%s", "Max: %# -9.4g\nMin: %# -9.4g");

  this->TitleMapper = vtkTextMapper::New();
  this->TitleMapper->GetTextProperty()->SetJustificationToLeft();
  this->TitleActor = vtkActor2D::New();
  this->TitleActor->SetMapper(this->TitleMapper);
  this->TitleActor->GetPositionCoordinate()->
    SetReferenceCoordinate(this->PositionCoordinate);

  this->varRange = new double [2];
  this->range = new double [2];
  varRange[0] = varRange[1] = range[0] = range[1] = FLT_MAX;  
  this->RangeMapper = vtkTextMapper::New();
  this->RangeMapper->GetTextProperty()->SetJustificationToLeft();
  this->RangeActor = vtkActor2D::New();
  this->RangeActor->SetMapper(this->RangeMapper);
  this->RangeActor->GetPositionCoordinate()->
    SetReferenceCoordinate(this->PositionCoordinate);
  this->RangeActor->GetPositionCoordinate()->
                    SetCoordinateSystemToNormalizedViewport();
  this->RangeActor->GetPositionCoordinate()->SetValue(0, 0);

  // to avoid deleting/rebuilding create once up front
  // and set unchangeable properties
  this->LabelMappers = new vtkTextMapper * [VTK_MAX_NUMLABELS];
  this->LabelActors  = new vtkActor2D    * [VTK_MAX_NUMLABELS];
  for (int i = 0; i < VTK_MAX_NUMLABELS; i++)
    {
    this->LabelMappers[i] = vtkTextMapper::New();
    this->LabelMappers[i]->GetTextProperty()->SetJustificationToLeft();
    this->LabelMappers[i]->GetTextProperty()->SetVerticalJustificationToCentered();
    this->LabelActors[i]  = vtkActor2D::New();
    this->LabelActors[i]->SetMapper(this->LabelMappers[i]);
    this->LabelActors[i]->GetPositionCoordinate()->
        SetReferenceCoordinate(this->PositionCoordinate);
    }

  this->ColorBar = vtkPolyData::New();
  this->ColorBarMapper = vtkPolyDataMapper2D::New();
  this->ColorBarMapper->SetInput(this->ColorBar);
  this->ColorBarActor = vtkActor2D::New();
  this->ColorBarActor->SetMapper(this->ColorBarMapper);
  this->ColorBarActor->GetPositionCoordinate()->
    SetReferenceCoordinate(this->PositionCoordinate);

  this->Tics = vtkPolyData::New();
  this->TicsMapper = vtkPolyDataMapper2D::New();
  this->TicsMapper->SetInput(this->Tics);
  this->TicsActor = vtkActor2D::New();
  this->TicsActor->SetMapper(this->TicsMapper);
  this->TicsActor->GetPositionCoordinate()->
    SetReferenceCoordinate(this->PositionCoordinate);

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

  this->LastOrigin[0] = 0;
  this->LastOrigin[1] = 0;
  this->LastSize[0] = 0;
  this->LastSize[1] = 0;
  this->TitleVisibility = 1;
  this->LabelVisibility = 1;
  this->RangeVisibility = 1;
  this->ColorBarVisibility = 1;
  this->BoundingBoxVisibility = 0;
  this->TitleOkayToDraw = 1;
  this->LabelOkayToDraw = 1;
  this->UseDefinedLabels = 0;
  this->definedLabels = stringVector();

  this->SkewFactor = 1.;
  this->UseSkewScaling = 0;
  this->UseLogScaling = 0;
  this->ReverseOrder = 0;
}


// Release any graphics resources that are being consumed by this actor.
// The parameter window could be used to determine which graphic
// resources to release.
void vtkVerticalScalarBarActor::ReleaseGraphicsResources(vtkWindow *win)
{
  this->TitleActor->ReleaseGraphicsResources(win);
  for (int i=0; i < VTK_MAX_NUMLABELS; i++)
    {
    this->LabelActors[i]->ReleaseGraphicsResources(win);
    }
 
  this->ColorBarActor->ReleaseGraphicsResources(win);
  this->RangeActor->ReleaseGraphicsResources(win);
  this->TicsActor->ReleaseGraphicsResources(win);
  this->BoundingBoxActor->ReleaseGraphicsResources(win);
}


// ****************************************************************************
//  Modifications:
//    Hank Childs, Fri Jan 25 10:50:27 PST 2002
//    Fixed memory leak.
//
//    Brad Whitlock, Wed Mar 21 16:01:44 PST 2007
//    Added BoundingBox.
//
// ****************************************************************************

vtkVerticalScalarBarActor::~vtkVerticalScalarBarActor()
{
  this->Position2Coordinate->Delete();
  this->Position2Coordinate = NULL;
  
  if (this->LabelFormat) 
    {
    delete [] this->LabelFormat;
    this->LabelFormat = NULL;
    }

  if (this->RangeFormat) 
    {
    delete [] this->RangeFormat;
    this->RangeFormat = NULL;
    }

  if (this->Title) 
    {
    delete [] this->Title;
    this->Title = NULL;
    }

  if (this->AltTitle) 
    {
    delete [] this->AltTitle;
    this->AltTitle = NULL;
    }

  this->TitleMapper->Delete();
  this->TitleActor->Delete();

  this->RangeMapper->Delete();
  this->RangeActor->Delete();

  if (this->LabelMappers != NULL )
    {
    for (int i=0; i < VTK_MAX_NUMLABELS; i++)
      {
      this->LabelMappers[i]->Delete();
      this->LabelActors[i]->Delete();
      }
    delete [] this->LabelMappers;
    delete [] this->LabelActors;
    }

  this->ColorBar->Delete();
  this->ColorBarMapper->Delete();
  this->ColorBarActor->Delete();

  delete [] this->varRange;
  delete [] this->range;

  this->Tics->Delete();
  this->TicsMapper->Delete();
  this->TicsActor->Delete();

  this->BoundingBox->Delete();
  this->BoundingBoxMapper->Delete();
  this->BoundingBoxActor->Delete();

  this->SetLookupTable(NULL);
}

void vtkVerticalScalarBarActor::SetVarRange(double *r)
{
  this->varRange[0] = r[0];
  this->varRange[1] = r[1];
}

void vtkVerticalScalarBarActor::SetVarRange(double min, double max)
{
  this->varRange[0] = min;
  this->varRange[1] = max;
}

void vtkVerticalScalarBarActor::SetRange(double *r)
{
  this->range[0] = r[0];
  this->range[1] = r[1];
}

void vtkVerticalScalarBarActor::SetRange(double min, double max)
{
  this->range[0] = min;
  this->range[1] = max;
}

// *********************************************************************
//  Modifications:
//    Kathleen Bonnell, Tue Nov  6 08:37:40 PST 2001
//    Don't render tic marks if labels won't be rendered.
//
//    Brad Whitlock, Wed Mar 21 16:03:32 PST 2007
//    Render bounding box.
//
// *********************************************************************
int vtkVerticalScalarBarActor::RenderOverlay(vtkViewport *viewport)
{
  int renderedSomething = 0;
  int i;
  
  // Everything is built, just have to render
  if (this->BoundingBoxVisibility)
    {
    this->BoundingBoxActor->RenderOverlay(viewport);
    }

  if (this->Title != NULL && this->TitleOkayToDraw && this->TitleVisibility)
    {
    renderedSomething += this->TitleActor->RenderOverlay(viewport);
    }

  if ( this->ColorBarVisibility )
    {
    this->ColorBarActor->RenderOverlay(viewport);
    if (this->LabelOkayToDraw && this->LabelVisibility)
      {
      this->TicsActor->RenderOverlay(viewport);
      }

    if (this->LabelOkayToDraw && this->LabelVisibility)
      {
      if (this->RangeVisibility)
        {
        this->RangeActor->RenderOverlay(viewport);
        }
         
      for (i=0; i<this->NumberOfLabelsBuilt; i++)
        {
        renderedSomething += this->LabelActors[i]->RenderOverlay(viewport);
        }
      }
    }
  renderedSomething = (renderedSomething > 0)?(1):(0);

  return renderedSomething;
}

// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Mon Sep 10 09:12:03 PDT 2001
//    Initialized all three components of the titleOrigin (even though the
//    third is totally unused) to fix a crash on east.
//
//    Eric Brugger, Mon Jul 14 11:59:40 PDT 2003
//    I changed the way the title is built.
//
// ****************************************************************************

// Build the title for this actor 
void vtkVerticalScalarBarActor::BuildTitle(vtkViewport *viewport)
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

void vtkVerticalScalarBarActor::BuildRange(vtkViewport *viewport)
{
  int* viewSize = viewport->GetSize(); 

  // 
  // if user hasn't set the range, use the range from the lut
  // 
  double *lutRange = this->LookupTable->GetRange();

  if (range[0] == FLT_MAX || range[1] == FLT_MAX)
    {
    range[0] = lutRange[0];
    range[1] = lutRange[1];
    }

  if (varRange[0] == FLT_MAX || varRange[1] == FLT_MAX)
    {
    varRange[0] = lutRange[0];
    varRange[1] = lutRange[1];
    }

  AdjustRangeFormat(varRange[0], varRange[1]);

  //
  // create the range label
  //
  char *rangeString = new char[256];
  sprintf(rangeString, this->RangeFormat, this->varRange[1], this->varRange[0]);
  this->RangeMapper->SetInput(rangeString);
  delete [] rangeString;

  int fontSize = (int)(this->FontHeight * viewSize[1]); 

  vtkTextProperty *rprop = this->RangeMapper->GetTextProperty();
  rprop->SetFontSize(fontSize);
  rprop->SetBold(this->Bold);
  rprop->SetItalic(this->Italic);
  rprop->SetShadow(this->Shadow);
  rprop->SetFontFamily(this->FontFamily);
  rprop->SetColor(this->GetProperty()->GetColor());

  this->RangeActor->SetProperty(this->GetProperty());

}

// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Mon Jul 22 16:29:15 PDT 2002
//    Initialized all three components of the labelOrig (even though the
//    third is totally unused) to fix an UMR.
//
//    Kathleen Bonnell, Wed Mar 19 14:44:13 PST 2003 
//    Remove possibility of divide by zero errors when using NumberOfLabels -1. 
//    Ensure font-size is determined (for range text) even if labels are not 
//    built. 
//    
//    Eric Brugger, Fri Apr 11 08:52:04 PDT 2003
//    Added code to create at most VTK_MAX_NUMLABELS labels when user defined
//    labels are specified.
//    
//    Kathleen Bonnell, Mon May 19 13:42:19 PDT 2003   
//    When using defined labels, reverse the order in which the labels 
//    are used, if specified. 
//
//    Eric Brugger, Mon Jul 14 11:59:40 PDT 2003
//    I changed the way the labels are built.
//
//    Eric Brugger, Tue Jul 15 11:28:43 PDT 2003
//    I added nLabels argument.
//
//    Hank Childs, Wed Sep  8 17:51:43 PDT 2004
//    Allocate a big enough buffer for long labels.
//
//    Hank Childs, Sat Mar  3 12:52:28 PST 2007
//    Add support for collapsing discrete tables that have each entry
//    as the same color.
//
// ****************************************************************************

void vtkVerticalScalarBarActor:: 
BuildLabels(vtkViewport * viewport, double bo, double bw, double bh, int nLabels)
{
  int i, idx;
  double val;
  char labelString[1024];
  double labelOrig[3] = { 0., 0., 0. };
  double delta; 
  int* viewSize = viewport->GetSize(); 
  double offset;

  labelOrig[0] = (bw + bw*0.25 ) / viewSize[0]; 

  if (this->UseDefinedLabels && !definedLabels.empty())
    {
    delta = bh/nLabels;
    }
  else
    {
    if (nLabels > 1)
        delta = bh/(nLabels-1);
    else    
        delta = bh*0.5;
    }
  bo /= viewSize[1];
  delta /= viewSize[1];
  if (this->UseDefinedLabels && !definedLabels.empty())
    {
    offset = bo + 0.5 * delta;
    if (nLabels < VTK_MAX_NUMLABELS)
      this->NumberOfLabelsBuilt = nLabels;
    else
      this->NumberOfLabelsBuilt = VTK_MAX_NUMLABELS;
    for (i = 0; i < this->NumberOfLabelsBuilt; ++i)
      {
      if (!this->ReverseOrder)
          idx = i;
      else 
          idx = this->NumberOfLabelsBuilt - 1 - i;
      bool singleColor = ShouldCollapseDiscrete();
      if (singleColor)
        strcpy(labelString, "All");
      else
        sprintf(labelString, definedLabels[idx].c_str());
      this->LabelMappers[i]->SetInput(labelString);
      }
    }
  else
    {
    offset = bo;
    double min, max;
    if (this->UseLogScaling)
    {
        min = log10(range[0]);
        max = log10(range[1]);
    }
    else
    {
        min = range[0];
        max = range[1];
    }
    double rangeDiff = max - min; 
    for (i = 0; i < nLabels; i++)
      {
      if (nLabels > 1)
          val = min + (double)i/(nLabels-1) * rangeDiff;
      else 
          val = min; 
      if (this->UseSkewScaling)
        {
        val = this->SkewTheValue(val, min, max);
        }
      else if (this->UseLogScaling)
        {
        val = (double) pow(10., val); 
        }
      sprintf(labelString, this->LabelFormat, val);
      this->LabelMappers[i]->SetInput(labelString);
      }
    this->NumberOfLabelsBuilt = nLabels;
    }

  int fontSize = (int)(FontHeight * viewSize[1]); 

  for (i = 0; i < this->NumberOfLabelsBuilt; ++i)
    {
    vtkTextProperty *tprop = this->LabelMappers[i]->GetTextProperty();
    tprop->SetFontSize(fontSize);
    tprop->SetBold(this->Bold);
    tprop->SetItalic(this->Italic);
    tprop->SetShadow(this->Shadow);
    tprop->SetFontFamily(this->FontFamily);
    tprop->SetColor(this->GetProperty()->GetColor());

    labelOrig[1] = (offset + (double)i*delta);
    this->LabelActors[i]->GetPositionCoordinate()->
      SetCoordinateSystemToNormalizedViewport();
    this->LabelActors[i]->GetPositionCoordinate()->SetValue(labelOrig);
    this->LabelActors[i]->SetProperty(this->GetProperty());
    }

  this->LabelOkayToDraw = 1;
}

// **********************************************************************
//  Modifications:
//    Kathleen Bonnell, Thu Nov  8 10:06:32 PST 2001
//    Made ticPtId to be of type vtkIdType to match VTK 4.0 API.
//
//    Eric Brugger, Mon Jul 14 11:59:40 PDT 2003
//    I changed the way the tics are built.
//
//    Eric Brugger, Tue Jul 15 11:28:43 PDT 2003
//    I added numLabels argument.
//
// **********************************************************************

void vtkVerticalScalarBarActor:: BuildTics(double origin, double width,
                                           double height, int numLabels)
{
  int i;

  vtkPoints *ticPts = vtkPoints::New();
  ticPts->SetNumberOfPoints(2*numLabels);
  vtkCellArray *lines = vtkCellArray::New();
  lines->Allocate(lines->EstimateSize(numLabels, 2));
  this->TicsActor->SetProperty(this->GetProperty());
  this->Tics->Initialize();
  this->Tics->SetPoints(ticPts);
  this->Tics->SetLines(lines);
  ticPts->Delete(); lines->Delete();

  //
  // generate lines for color bar tics
  //
  double x[3]; x[2] = 0.0;
  double delta, offset;
  if (this->UseDefinedLabels && !this->definedLabels.empty()  )
    {
    delta = height/(double)( numLabels);
    offset = origin + 0.5* delta;
    }
  else
    {
    delta = height/(double)( numLabels-1);
    offset = origin;
    }
  double quarterWidth = width*0.25;
  // first tic
  for (i = 0; i < numLabels; i++)
    {
    x[0] = width;
    x[1] = offset + i*delta;
    ticPts->SetPoint(2*i, x); 
    x[0] = width + quarterWidth;
    ticPts->SetPoint(2*i+1, x); 
    }
  vtkIdType ticPtId[2];
  for (i = 0; i < numLabels; i++)
    {
    ticPtId[0] = 2*i;
    ticPtId[1] = ticPtId[0] + 1; 
    lines->InsertNextCell(2, ticPtId);
    }
}

// **********************************************************************
//  Modifications:
//    Kathleen Bonnell, Thu Nov  8 10:06:32 PST 2001
//    Made ptIds to be of type vtkIdType to match VTK 4.0 API.
//
//    Jeremy Meredith, Fri Feb  1 15:04:39 PST 2002
//    Copy the values from the LUT range, not the pointers.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkUnsignedCharArray
//    directly to access colors.
//
//    Kathleen Bonnell, Wed Sep 11 09:21:08 PDT 2002  
//    Use LookupTable directly to determine range.  Use labelColorMap
//    to retrieve index into lookup table when appropriate.
//    
//    Kathleen Bonnell, Wed Mar 19 14:44:13 PST 2003 
//    Added call to AdjustRangeFormat.  Ensure that color bar is always
//    built with full spectrum of color table, regardless of set range.
//    
//    Kathleen Bonnell, Mon May 19 13:42:19 PDT 2003   
//    When using defined labels, reverse the order in which the colors
//    are used, if specified. 
//
//    Hank Childs, Thu Jul 10 11:10:51 PDT 2003
//    Don't modify the lookup table, since that causes display lists to
//    be regenerated.
//
//    Eric Brugger, Mon Jul 14 11:59:40 PDT 2003
//    I changed the way the color bar is built.
//
//    Eric Brugger, Tue Jul 15 11:28:43 PDT 2003
//    Add logic to reduce the number of colors and labels shown when user
//    defined labels are specified and there are  are too many to fit in
//    the available space.
//
//    Hank Childs, Thu Jul 17 17:43:18 PDT 2003
//    Be a little more loose on using an auxiliary lookup table, since
//    near-constant plots are getting drawn incorrectly.
//
//    Hank Childs, Sat Mar  3 12:52:28 PST 2007
//    Add support for collapsing discrete tables that have each entry
//    as the same color.
//
//    Eric Brugger, Thu Oct  2 16:13:10 PDT 2008
//    Corrected an error in the logic that reversed the order of the
//    colors in the color bar so that it did so in all cases that it
//    should.
//
// **********************************************************************

void vtkVerticalScalarBarActor::BuildColorBar(vtkViewport *viewport)
{
  int *viewSize = viewport->GetSize();

  bool singleColor = ShouldCollapseDiscrete();

  //
  // Determine the size and position of the color bar
  //
  int halfFontSize = (int)((this->FontHeight * viewSize[1]) / 2.);

  double barOrigin;
  int rsizePixels[2];
  this->RangeMapper->GetSize(viewport, rsizePixels); 
  if (this->RangeVisibility)
    barOrigin = (double)(rsizePixels[1] + halfFontSize);
  else
    barOrigin = 0.;

  int *titleOrigin;
  double barHeight, barWidth;

  titleOrigin = this->TitleActor->GetPositionCoordinate()->
                                  GetComputedViewportValue(viewport);

  if (this->TitleOkayToDraw && this->TitleVisibility) 
    {
    barHeight = titleOrigin[1] - LastOrigin[1] - halfFontSize - barOrigin; 
    }
  else
    {
    barHeight = LastSize[1] - barOrigin;
    }

  barWidth = (int) (this->BarWidth * viewSize[0]);
  if (singleColor)
    {
    barOrigin += barHeight-barWidth;
    barHeight = barWidth;
    }

  //
  // Determine the number of colors in the color bar.
  //
  int numColors, numLabels; 
  if ( this->UseDefinedLabels && !this->definedLabels.empty() )
    {
    if (singleColor)
      numColors = numLabels = 1;
    else
      {
      numColors = this->definedLabels.size();
      if ((this->FontHeight * 1.1 * numColors * viewSize[1]) > barHeight)
          numColors = (int) (barHeight / (this->FontHeight * 1.1 * viewSize[1]));
      numLabels = numColors;
      }
    }
  else
    {
    numColors = this->MaximumNumberOfColors;
    numLabels = this->NumberOfLabels;
    }

  //
  // Build color bar object
  //
  int numPts = 2*(numColors + 1);
  vtkPoints *pts = vtkPoints::New();
  pts->SetNumberOfPoints(numPts);
  vtkCellArray *polys = vtkCellArray::New();
  polys->Allocate(polys->EstimateSize(numColors,4)); 
  vtkUnsignedCharArray *colors = vtkUnsignedCharArray::New();
  colors->SetNumberOfComponents(3);
  colors->SetNumberOfTuples(numColors);

  this->ColorBarActor->SetProperty(this->GetProperty());
  this->ColorBar->Initialize();
  this->ColorBar->SetPoints(pts);
  this->ColorBar->SetPolys(polys);
  this->ColorBar->GetCellData()->SetScalars(colors);
  pts->Delete(); polys->Delete(); colors->Delete(); 

  //
  // generate points for color bar
  //
  int i, idx;

  double x[3]; x[2] = 0;
  double delta = (double)barHeight/(double)numColors;
  for (i = 0; i < numPts/2; i++)
    {
    x[0] = 0.0; 
    x[1] = i*delta+barOrigin; 
    pts->SetPoint(2*i,x);
    x[0] = barWidth; 
    pts->SetPoint(2*i+1,x);
    }

  //
  //polygons & cell colors for color bar
  //
  unsigned char *rgba, *rgb;
  vtkIdType ptIds[4];
  double *lutRange = this->LookupTable->GetRange();
  double tMin = lutRange[0];
  double tMax = lutRange[1];

  //
  // If we have a constant lookup table, then we still want the scalar bar
  // to appear with a rainbow color scheme.  We have to be careful not to
  // modify the existing LUT, or else display lists will be regenerated.
  // So build a new one in that case.  Make sure the colors are copied
  // so that there is no difference in colors lf the bar between constant
  // vars and non-constant vars.
  //
  vtkLookupTable *useMe = LookupTable;
  vtkLookupTable *tmp = vtkLookupTable::New();
  if ((tMax <= tMin) || ( ((tMax-tMin) < 1e-7) && (tMax > 1e-4)))
    {
    tMax = tMin+1.;
    tmp->DeepCopy(useMe);
    tmp->SetRange(tMin, tMax);
    useMe = tmp;
    }
  for (i=0; i<numColors; i++)
    {
    ptIds[0] = 2*i;
    ptIds[1] = ptIds[0] + 1;
    ptIds[2] = ptIds[1] + 2;
    ptIds[3] = ptIds[0] + 2;
    polys->InsertNextCell(4,ptIds);

    if (!this->ReverseOrder)
        idx = i;
    else 
        idx = numColors-1-i; 
    if (this->UseDefinedLabels && !this->definedLabels.empty() )
      {
      LevelColorMap::iterator it;
      if ((it = labelColorMap.find(definedLabels[idx])) != labelColorMap.end())
        {
        vtkIdType colorIndex = it->second;
        rgba = useMe->GetPointer(colorIndex);
        }
      else
        {
        rgba = useMe->MapValue((double)idx);
        }
      }
    else
      {
      double val = (((double)idx)/(numColors-1.0)) * (tMax - tMin) + tMin;
      rgba = useMe->MapValue(val);
      }
 
    rgb = colors->GetPointer(3*i); //write into array directly
    rgb[0] = rgba[0];
    rgb[1] = rgba[1];
    rgb[2] = rgba[2];
    } // loop on numColors
  tmp->Delete();

  this->BuildTics(barOrigin, barWidth, barHeight, numLabels);
  if (this->LabelVisibility)
    {
    this->BuildLabels(viewport, barOrigin, barWidth, barHeight, numLabels);
    }

} // BuildColorBar


bool
vtkVerticalScalarBarActor::ShouldCollapseDiscrete(void)
{
  if (!this->UseDefinedLabels || this->definedLabels.empty() )
      return false;
  if (definedLabels.size() <= 1)
      return false;


  double *lutRange = this->LookupTable->GetRange();
  unsigned char *rgba;
  unsigned char  rgba_base[4];
  LevelColorMap::iterator it;
  if ((it = labelColorMap.find(definedLabels[0])) != labelColorMap.end())
    {
    vtkIdType colorIndex = it->second;
    rgba = LookupTable->GetPointer(colorIndex);
    }
  else
    {
    rgba = LookupTable->MapValue((double)0);
    }
  rgba_base[0] = rgba[0];
  rgba_base[1] = rgba[1];
  rgba_base[2] = rgba[2];
  rgba_base[3] = rgba[3];
  
  for (size_t i = 1 ; i < definedLabels.size() ; i++)
    {
    if ((it = labelColorMap.find(definedLabels[i])) != labelColorMap.end())
      {
      vtkIdType colorIndex = it->second;
      rgba = LookupTable->GetPointer(colorIndex);
      }
    else
      {
      rgba = LookupTable->MapValue((double)i);
      }
    if (rgba_base[0] != rgba[0])
      return false;
    if (rgba_base[1] != rgba[1])
      return false;
    if (rgba_base[2] != rgba[2])
      return false;
    if (rgba_base[3] != rgba[3])
      return false;
    }

  // All the colors are equal
  return true;
}

// ****************************************************************************
// Method: vtkVerticalScalarBarActor::BuildBoundingBox
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

void vtkVerticalScalarBarActor::BuildBoundingBox(vtkViewport *viewport)
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

  if(this->RangeVisibility)
    {
        int *rangeLL = this->RangeActor->GetPositionCoordinate()->
            GetComputedViewportValue(viewport);
        int rightX = rangeLL[0] + 
            this->RangeMapper->GetWidth(viewport);
        if(rightX > maxX)
            maxX = rightX;
    }

  if (this->LabelOkayToDraw && this->LabelVisibility)
    {
    for (int i=0; i<this->NumberOfLabelsBuilt; i++)
      {
        int *labelLL = this->LabelActors[i]->GetPositionCoordinate()->
            GetComputedViewportValue(viewport);
        int rightX = labelLL[0] + 
            this->LabelMappers[i]->GetWidth(viewport);
        if(rightX > maxX)
            maxX = rightX;
      }
    }
  width = maxX - LL[0];

  const double border = 4;
  double pt[3];
  pt[0] = 0. - border;
  pt[1] = 0. - border;
  pt[2] = 0.;
  pts->SetPoint(0, pt);

  pt[0] = width;
  pt[1] = 0. - border;
  pts->SetPoint(1, pt);

  pt[0] = width;
  pt[1] = height;
  pts->SetPoint(2, pt);

  pt[0] = 0. - border;
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

// *********************************************************************
//  Modifications:
//    Kathleen Bonnell, Tue Nov  6 08:37:40 PST 2001
//    Don't render tic marks if labels won't be rendered.
//
//    Eric Brugger, Mon Jul 14 11:59:40 PDT 2003
//    I changed the way the color bar is built.
//
//    Brad Whitlock, Wed Mar 21 16:05:31 PST 2007
//    Added bounding box.
//
// *********************************************************************

int vtkVerticalScalarBarActor::RenderOpaqueGeometry(vtkViewport *viewport)
{
  int renderedSomething = 0;
  int i;
 
  if ( ! this->LookupTable )
    {
    vtkWarningMacro(<<"Need a LUT to render a scalar bar");
    return 0;
    }

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
  if ( this->GetMTime() > this->BuildTime || 
       this->LookupTable->GetMTime() > this->BuildTime )
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

    if ( this->RangeVisibility )
      {
      this->BuildRange(viewport);
      }

    if ( this->ColorBarVisibility )
      {
      this->BuildColorBar(viewport);
      }

    if( this->BoundingBoxVisibility )
      {
        this->BuildBoundingBox(viewport);
      }

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

  if ( this->ColorBarVisibility )
    {
    this->ColorBarActor->RenderOpaqueGeometry(viewport);
    if (this->LabelOkayToDraw && this->LabelVisibility)
      {
      this->TicsActor->RenderOpaqueGeometry(viewport);
      }

    if (this->LabelOkayToDraw && this->RangeVisibility )
      {
      this->RangeActor->RenderOpaqueGeometry(viewport);
      }

    if (this->LabelOkayToDraw && this->LabelVisibility)
      {
      for (i=0; i<this->NumberOfLabelsBuilt; i++)
        {
        renderedSomething += this->LabelActors[i]->RenderOpaqueGeometry(viewport);
        }
      }
    }

  renderedSomething = (renderedSomething > 0)?(1):(0);
  return renderedSomething;
}

void vtkVerticalScalarBarActor::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  if ( this->LookupTable )
    {
    os << indent << "Lookup Table:\n";
    this->LookupTable->PrintSelf(os,indent.GetNextIndent());
    }
  else
    {
    os << indent << "Lookup Table: (none)\n";
    }

  os << indent << "Title: " << (this->Title ? this->Title : "(none)") << "\n";
  os << indent << "Maximum Number Of Colors: " 
     << this->MaximumNumberOfColors << "\n";
  os << indent << "Number Of Labels: " << this->NumberOfLabels << "\n";
  os << indent << "Number Of Labels Built: " << this->NumberOfLabelsBuilt << "\n";

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
  os << indent << "Label Format: " << this->LabelFormat << "\n";
  os << indent << "Range Format: " << this->RangeFormat << "\n";
  os << indent << "Bar Width: " << this->BarWidth << "\n";
  os << indent << "Title Visibility: " 
     << (this->TitleVisibility ? "On\n" : "Off\n");
  os << indent << "ColorBar Visibility: " 
     << (this->ColorBarVisibility ? "On\n" : "Off\n");
  os << indent << "Label Visibility: " 
     << (this->LabelVisibility ? "On\n" : "Off\n");
  os << indent << "Range Visibility: " 
     << (this->RangeVisibility ? "On\n" : "Off\n");
  os << indent << "BoundingBox Visibility: " 
     << (this->BoundingBoxVisibility ? "On\n" : "Off\n");
  os << indent << "Use Defined Labels: " 
     << (this->UseDefinedLabels ? "On\n" : "Off\n");
  if ( !definedLabels.empty() )
    {
    vtkIndent indent2 = indent.GetNextIndent();
    os << indent << "Defined Labels: \n";
    for (size_t i = 0; i < definedLabels.size(); i++)
       os << indent2 << this->definedLabels[i].c_str() << "\n";
    }
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

void vtkVerticalScalarBarActor::SetDefinedLabels(const stringVector &labels)
{
  this->definedLabels = labels;
}


// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Wed Sep  8 17:51:43 PDT 2004
//    Allocate a big enough buffer for long labels.
//
// ****************************************************************************

void vtkVerticalScalarBarActor::SetDefinedLabels(const doubleVector &values)
{
  this->definedLabels.clear();
  char labelString[1024];
  for (size_t i = 0; i < values.size(); ++i)
    {
    sprintf(labelString, this->LabelFormat, values[i]);
    this->definedLabels.push_back(labelString);
    }
}


void vtkVerticalScalarBarActor::SetPosition(double x[2]) 
{
  this->SetPosition(x[0],x[1]);
} 

void vtkVerticalScalarBarActor::SetPosition(double x, double y) 
{ 
  this->PositionCoordinate->SetCoordinateSystemToNormalizedViewport(); 
  this->PositionCoordinate->SetValue(x,y); 
} 

vtkCoordinate *vtkVerticalScalarBarActor::GetPosition2Coordinate() 
{ 
  return this->Position2Coordinate; 
} 

void vtkVerticalScalarBarActor::SetPosition2(double x[2]) 
{
  this->SetPosition2(x[0],x[1]);
} 

void vtkVerticalScalarBarActor::SetPosition2(double x, double y) 
{ 
  this->Position2Coordinate->SetCoordinateSystemToNormalizedViewport(); 
  this->Position2Coordinate->SetValue(x,y); 
} 

double *vtkVerticalScalarBarActor::GetPosition2() 
{ 
  return this->Position2Coordinate->GetValue(); 
}

void vtkVerticalScalarBarActor::ShallowCopy(vtkProp *prop)
{
  vtkVerticalScalarBarActor *a = vtkVerticalScalarBarActor::SafeDownCast(prop);
  if ( a != NULL )
    {
    this->SetLookupTable(a->GetLookupTable());
    this->SetMaximumNumberOfColors(a->GetMaximumNumberOfColors());
    this->SetNumberOfLabels(a->GetNumberOfLabels());
    this->SetBold(a->GetBold());
    this->SetItalic(a->GetItalic());
    this->SetShadow(a->GetShadow());
    this->SetFontFamily(a->GetFontFamily());
    this->SetFontHeight(a->GetFontHeight());
    this->SetLabelFormat(a->GetLabelFormat());
    this->SetRangeFormat(a->GetRangeFormat());
    this->SetTitle(a->GetTitle());

    this->SetBarWidth(a->GetBarWidth());

    this->SetTitleVisibility(a->GetTitleVisibility());
    this->SetRangeVisibility(a->GetRangeVisibility());
    this->SetLabelVisibility(a->GetLabelVisibility());
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

// ****************************************************************************
//  Modifications:
//
//    Kathleen Bonnell, Thu Feb 19 14:10:21 PST 2004 
//    Removed the scaling portion. 
//
// ****************************************************************************
 
double
vtkVerticalScalarBarActor::SkewTheValue(double val, double min, double max)
{
  if (this->SkewFactor < 0.) this->SkewFactor = 1.;
  if (this->SkewFactor == 1.) return val;

  double rangeDif = max - min;
  double log_skew_inv = 1./(log(this->SkewFactor));
  double k = (this->SkewFactor -1.) / rangeDif;

  double v2 = log((val - min) * k + 1) * log_skew_inv;
  double temp  = (rangeDif * v2 + min) ;
  return temp;
}

// ****************************************************************************
//  Modifications:
//
//    Kathleen Bonnell, Tue Aug 28 08:35:17 PDT 2001
//    Set the modified flag.
//
// ****************************************************************************

void vtkVerticalScalarBarActor::SkewScalingOn()
{
    this->UseSkewScaling = 1;
    this->UseLogScaling = 0;
    this->Modified();
}


// ****************************************************************************
//  Modifications:
//
//    Kathleen Bonnell, Tue Aug 28 08:35:17 PDT 2001
//    Set the modified flag.
//
// ****************************************************************************

void vtkVerticalScalarBarActor::SkewScalingOff()
{
    this->UseSkewScaling = 0;
    this->Modified();
}


// ****************************************************************************
//  Modifications:
//
//    Kathleen Bonnell, Tue Aug 28 08:35:17 PDT 2001
//    Set the modified flag.
//
// ****************************************************************************

void vtkVerticalScalarBarActor::LogScalingOn()
{
    this->UseLogScaling = 1;
    this->UseSkewScaling = 0;
    this->Modified();
}


// ****************************************************************************
//  Modifications:
//
//    Kathleen Bonnell, Tue Aug 28 08:35:17 PDT 2001
//    Set the modified flag.
//
// ****************************************************************************

void vtkVerticalScalarBarActor::LogScalingOff()
{
    this->UseLogScaling = 0;
    this->Modified();
}


// ****************************************************************************
//  Method:      vtkVerticalScalarBarActor::SetNumberOfLabelsToDefault
//
//  Purpose:
//    Resets the number of labels to be the default number (currently 5).
//
//  Programmer:  Kathleen Bonnell
//  Creation:    December 17, 2001 
//
// ****************************************************************************

void vtkVerticalScalarBarActor::SetNumberOfLabelsToDefault()
{
    this->SetNumberOfLabels(DefaultNumLabels);
}


// ****************************************************************************
//  Method:      vtkVerticalScalarBarActor::SetLabelColorMap
//
//  Purpose:
//    Sets up a map that translates a label to its corresponding color index. 
//
//  Programmer:  Kathleen Bonnell
//  Creation:    September 11, 2002 
//
//  Modifications:
//    Brad Whitlock, Fri Nov 15 10:03:40 PDT 2002
//    I changed the argument type.
//
// ****************************************************************************

void vtkVerticalScalarBarActor::SetLabelColorMap(const LevelColorMap &cmap)
{
    this->labelColorMap = cmap;
    this->Modified();
}


// ****************************************************************************
//  Method:      vtkVerticalScalarBarActor::AdjustRangeFormat
//
//  Purpose:
//    Determines the format for Range text. 
//
//  Notes:
//    Modifed from VisWinAxes 'Digits' and 'LabelExponent' methods.
//
//  Programmer:  Kathleen Bonnell
//  Creation:    March 19, 2003
//
//  Modifications:
//    Kathleen Bonnell, Thu Jun 12 11:05:18 PDT 2003
//    Removed 'f' from pow(10.f, ...).
//
// ****************************************************************************

void
vtkVerticalScalarBarActor::AdjustRangeFormat(double min, double max)
{
    double amax = max;
    double amin = min;
    //
    // Determine power of 10 to scale range text.
    //
    double range = (fabs(amin) > fabs(amax) ? fabs(amin) : fabs(amax));
 
    double pow10 = log10(range);
    //
    // Cutoffs for using scientific notation.
    //
    static double  eformat_cut_min = -1.5;
    static double  eformat_cut_max =  3.0;
    static double  cut_min = pow(10., eformat_cut_min);
    static double  cut_max = pow(10., eformat_cut_max);
    double ipow10;
    if (range < cut_min || range > cut_max)
    {
        ipow10 = floor(pow10);
    }
    else
    {
        ipow10 = 0.;
    }
    int exponent = (int) ipow10;
    if (exponent != 0)
    {
        amax /= pow(10., exponent);
        amin /= pow(10., exponent);
    }
 
    double arange = amax - amin;
    double apow10 = log10(arange);
    int   aIpow10 = (int) floor(apow10);
    int digitsPastDecimal = -aIpow10;

    //
    // Use a minimum of 4 decimal places.
    //
    if (digitsPastDecimal < 4)
    {
        digitsPastDecimal = 4;
    }
 
    sprintf(this->RangeFormat, "Max: %%# -9.%dg\nMin: %%# -9.%dg", 
            digitsPastDecimal, digitsPastDecimal);
}
