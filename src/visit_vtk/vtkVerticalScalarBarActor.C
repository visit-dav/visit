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

#include <vtkObjectFactory.h>
#include <vtkTextProperty.h>
#include <vtkViewport.h>
#include <vtkPolyData.h>
#include <limits.h>
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
  this->TitleFraction = 0.04;
  this->LabelFraction = 0.3;

  this->Bold = 0;
  this->Italic = 0;
  this->Shadow = 0;
  this->FontFamily = VTK_ARIAL;
  this->FontSize = 12;
  this->LabelFormat = new char[10]; 
  sprintf(this->LabelFormat,"%s","%# -9.4g");
  this->RangeFormat = new char[30]; 
  sprintf(this->RangeFormat, "%s", "\nMax: %# -9.4g\nMin: %# -9.4g");

  this->TitleMapper = vtkTextMapper::New();
  this->TitleMapper->GetTextProperty()->SetJustificationToLeft();
  this->TitleActor = vtkActor2D::New();
  this->TitleActor->SetMapper(this->TitleMapper);
  this->TitleActor->GetPositionCoordinate()->
    SetReferenceCoordinate(this->PositionCoordinate);

  this->varRange = new float [2];
  this->range = new float [2];
  varRange[0] = varRange[1] = range[0] = range[1] = FLT_MAX;  
  this->RangeMapper = vtkTextMapper::New();
  this->RangeMapper->GetTextProperty()->SetJustificationToLeft();
  this->RangeMapper->GetTextProperty()->SetFontSize(0);
  this->RangeMapper->GetTextProperty()->SetVerticalJustificationToTop();
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

  this->LastOrigin[0] = 0;
  this->LastOrigin[1] = 0;
  this->LastSize[0] = 0;
  this->LastSize[1] = 0;
  this->TitleVisibility = 1;
  this->LabelVisibility = 1;
  this->RangeVisibility = 1;
  this->ColorBarVisibility = 1;
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
}


// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Fri Jan 25 10:50:27 PST 2002
//    Fixed memory leak.
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

  this->SetLookupTable(NULL);
}

void vtkVerticalScalarBarActor::SetVarRange(float *r)
{
  this->varRange[0] = r[0];
  this->varRange[1] = r[1];
}

void vtkVerticalScalarBarActor::SetVarRange(float min, float max)
{
  this->varRange[0] = min;
  this->varRange[1] = max;
}


void vtkVerticalScalarBarActor::SetRange(float *r)
{
  this->range[0] = r[0];
  this->range[1] = r[1];
}

void vtkVerticalScalarBarActor::SetRange(float min, float max)
{
  this->range[0] = min;
  this->range[1] = max;
}

void vtkVerticalScalarBarActor::SetWidth(float w)
{
  float *pos;

  pos = this->Position2Coordinate->GetValue();
  this->Position2Coordinate->SetCoordinateSystemToNormalizedViewport();
  this->Position2Coordinate->SetValue(w,pos[1]);
}

void vtkVerticalScalarBarActor::SetHeight(float h)
{
  float *pos;

  pos = this->Position2Coordinate->GetValue();
  this->Position2Coordinate->SetCoordinateSystemToNormalizedViewport();
  this->Position2Coordinate->SetValue(pos[0],h);
}
    
// *********************************************************************
//  Modifications:
//    Kathleen Bonnell, Tue Nov  6 08:37:40 PST 2001
//    Don't render tic marks if labels won't be rendered.
// *********************************************************************
int vtkVerticalScalarBarActor::RenderOverlay(vtkViewport *viewport)
{
  int renderedSomething = 0;
  int i;
  
#ifndef NO_ANNOTATIONS
  // Everything is built, just have to render
  if (this->Title != NULL && this->TitleOkayToDraw && this->TitleVisibility)
    {
    renderedSomething += this->TitleActor->RenderOverlay(viewport);
    }
#endif

  if ( this->ColorBarVisibility )
    {
    this->ColorBarActor->RenderOverlay(viewport);
    if (this->LabelOkayToDraw && this->LabelVisibility)
      {
      this->TicsActor->RenderOverlay(viewport);
      }

#ifndef NO_ANNOTATIONS
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
#endif
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
// ****************************************************************************

// Build the title for this actor 
void vtkVerticalScalarBarActor::BuildTitle(vtkViewport *viewport)
{
  float titleOrigin[3] = { 0., 0., 0. };

  this->TitleMapper->SetInput(this->Title);
  int *viewSize = viewport->GetSize();

  if (0 == viewSize[0]  && 0 == viewSize[1] )
    {
    this->TitleOkayToDraw = 0;
    return;
    }
 
  int titleWidth = viewSize[0];
  int titleHeight = (int) (this->TitleFraction * viewSize[1]);

  int fontSize = this->TitleMapper->SetConstrainedFontSize(viewport,
                                                           titleWidth,
                                                           titleHeight);

  //
  //  HACK!!  KSB:  Constrain the title font size to look good, but this 
  //  may not be correct for all displays!!!
  //
  if (fontSize > 16 ) fontSize = 16;

  int tsizePixels[2];
  //
  // Set the final font size determined for the title
  // and get the size in pixels
  //
  vtkTextProperty *tprop = this->TitleMapper->GetTextProperty();
  tprop->SetFontSize(fontSize);
  this->TitleMapper->GetSize(viewport, tsizePixels); 

  // title Position must be expressed in reference to
  // the legend origin ... 
  // expressed as a percentage of the viewport 

  titleOrigin[0] = 0;
  int legURy = LastOrigin[1] + LastSize[1];
  int distFromOrigin = (legURy-tsizePixels[1]-LastOrigin[1]);

  titleOrigin[1] = (float)(distFromOrigin) /(float)viewSize[1] ; 

  tprop->SetBold(this->Bold);
  tprop->SetItalic(this->Italic);
  tprop->SetShadow(this->Shadow);
  tprop->SetFontFamily(this->FontFamily);

  this->TitleActor->SetProperty(this->GetProperty());
  this->TitleActor->GetPositionCoordinate()->
                    SetCoordinateSystemToNormalizedViewport();
  this->TitleActor->GetPositionCoordinate()->SetValue(titleOrigin);

  this->TitleOkayToDraw = 1;
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
// ****************************************************************************

void vtkVerticalScalarBarActor:: 
BuildLabels(vtkViewport * viewport, int bw, int bh)
{
  int i, idx;
  float val;
  char labelString[30];
  float labelOrig[3] = { 0., 0., 0. };
  float delta; 
  int* viewSize = viewport->GetSize(); 
  int targetWidth, targetHeight ;
  int labelSize[2] = { 0, 0 };
  float offset = 0.;

  labelOrig[0] = ((float)bw + bw*0.25 ) / viewSize[0]; 

  if (this->UseDefinedLabels && !definedLabels.empty())
    {
    delta = (float)bh/(this->definedLabels.size());
    }
  else
    {
    if (this->NumberOfLabels > 1)
        delta = (float)bh/(this->NumberOfLabels-1);
    else    
        delta = (float)bh*0.5;
    }
  targetHeight = (int) delta;
  delta /= viewSize[1];
  targetWidth = (int) 1.5*bw; 
  if (this->UseDefinedLabels && !definedLabels.empty())
    {
    offset = 0.5 * delta;
    if (this->definedLabels.size() < VTK_MAX_NUMLABELS)
      this->NumberOfLabelsBuilt = this->definedLabels.size();
    else
      this->NumberOfLabelsBuilt = VTK_MAX_NUMLABELS;
    for (i = 0; i < this->NumberOfLabelsBuilt; ++i)
      {
      if (!this->ReverseOrder)
          idx = i;
      else 
          idx = this->NumberOfLabelsBuilt - 1 - i;
      sprintf(labelString, definedLabels[idx].c_str());
      this->LabelMappers[i]->SetInput(labelString);
      }
    }
  else
    {
    float min, max;
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
    float rangeDiff = max - min; 
    for (i = 0; i < this->NumberOfLabels; i++)
      {
      if (this->NumberOfLabels > 1)
          val = min + (float)i/(this->NumberOfLabels-1) * rangeDiff;
      else 
          val = min; 
      if (this->UseSkewScaling)
        {
        val = this->SkewTheValue(val, min, max);
        }
      else if (this->UseLogScaling)
        {
        val = (float) pow(10.f, val); 
        }
      sprintf(labelString, this->LabelFormat, val);
      this->LabelMappers[i]->SetInput(labelString);
      }
    this->NumberOfLabelsBuilt = this->NumberOfLabels;
    }

  int fs;
  if (this->NumberOfLabelsBuilt > 0)
    {
    fs = vtkTextMapper::SetMultipleConstrainedFontSize(viewport,
                                               targetWidth, 
                                               targetHeight, 
                                               this->LabelMappers, 
                                               this->NumberOfLabelsBuilt, 
                                               labelSize);
    if (labelSize[1] > targetHeight || labelSize[1] < 6)
      {
      vtkDebugMacro(<<"Available space for labels too small."
                      << "  Labels will not be drawn." );
      this->LabelOkayToDraw = 0;
      this->NumberOfLabelsBuilt = 0;
      return;
      }
    }
  else
    {
    fs = this->RangeMapper->SetConstrainedFontSize(viewport,
                                                   targetWidth,
                                                   targetHeight);
    }
   
  //
  //  HACK!!  KSB:  Constrain the label  font size to look good, but this 
  //  may not be correct for all displays!!!
  //
  if (fs > 12)
  {
      fs = 12;
  }
  for (i = 0; i < this->NumberOfLabelsBuilt; ++i)
    {
    labelOrig[1] = (offset + (float)i*delta);
    vtkTextProperty *tprop = this->LabelMappers[i]->GetTextProperty();
    tprop->SetFontSize(fs);
    tprop->SetBold(this->Bold);
    tprop->SetItalic(this->Italic);
    tprop->SetShadow(this->Shadow);
    tprop->SetFontFamily(this->FontFamily);
    this->LabelActors[i]->GetPositionCoordinate()->
      SetCoordinateSystemToNormalizedViewport();
    this->LabelActors[i]->GetPositionCoordinate()->SetValue(labelOrig);
    this->LabelActors[i]->SetProperty(this->GetProperty());
    }

  this->RangeMapper->GetTextProperty()->SetFontSize(fs);
  this->LabelOkayToDraw = 1;
}

// **********************************************************************
//  Modifications:
//    Kathleen Bonnell, Thu Nov  8 10:06:32 PST 2001
//    Made ticPtId to be of type vtkIdType to match VTK 4.0 API.
// **********************************************************************

void vtkVerticalScalarBarActor:: BuildTics(float width, float height)
{
  int i, numLabels;
  if (this->UseDefinedLabels && !this->definedLabels.empty() )
    {
    numLabels = this->definedLabels.size();
    }
  else
    {
    numLabels = this->NumberOfLabels;
    }
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
  float x[3]; x[2] = 0.0;
  float delta, offset;
  if (this->UseDefinedLabels && !this->definedLabels.empty()  )
    {
    delta = height/(float)( numLabels);
    offset = 0.5* delta;
    }
  else
    {
    delta = height/(float)( numLabels-1);
    offset = 0.0;
    }
  float quarterWidth = width*0.25;
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
// **********************************************************************

void vtkVerticalScalarBarActor::BuildColorBar(vtkViewport *viewport)
{
  // we hard code how many steps to display unless user has defined labels
  int numColors; 
  if ( this->UseDefinedLabels && !this->definedLabels.empty() )
    {
    numColors = this->definedLabels.size();
    }
  else
    {
    numColors = this->MaximumNumberOfColors;
    }

  // 
  // if user hasn't set the range, use the range from the lut
  // 
  float *lutRange = this->LookupTable->GetRange();

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

  //
  // now that we have the range, we need to 
  // create the range mapper input 
  //

  AdjustRangeFormat(varRange[0], varRange[1]);

  char *labelString = new char[256];
  sprintf(labelString, this->RangeFormat, varRange[1], varRange[0]);
  this->RangeMapper->SetInput(labelString);
  delete [] labelString;
  vtkTextProperty *rprop = this->RangeMapper->GetTextProperty();
  rprop->SetBold(this->Bold);
  rprop->SetItalic(this->Italic);
  rprop->SetShadow(this->Shadow);
  rprop->SetFontFamily(this->FontFamily);
  this->RangeActor->SetProperty(this->GetProperty());

  //
  // finish color bar 
  //

  int *titleOrigin;
  int barHeight, barWidth;

  titleOrigin = this->TitleActor->GetPositionCoordinate()->
                                  GetComputedViewportValue(viewport);

  if (this->TitleOkayToDraw && this->TitleVisibility) 
    {
    barHeight = titleOrigin[1] - 5 - LastOrigin[1]; 
    }
  else
    {
    barHeight = LastSize[1];
    }

  barWidth = (int) (LastSize[0] * (1.0 - this->LabelFraction));

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

  float x[3]; x[2] = 0;
  float delta = (float)barHeight/(float)numColors;
  for (i = 0; i < numPts/2; i++)
    {
    x[0] = 0.0; 
    x[1] = i*delta; 
    pts->SetPoint(2*i,x);
    x[0] = barWidth; 
    pts->SetPoint(2*i+1,x);
    }

  //
  //polygons & cell colors for color bar
  //
  unsigned char *rgba, *rgb;
  vtkIdType ptIds[4];
  float tMin = lutRange[0];
  float tMax = lutRange[1];

  //
  // If we have a constant lookup table, then we still want the scalar bar
  // to appear with a rainbow color scheme.  We have to be careful not to
  // modify the existing LUT, or else display lists will be regenerated.
  // So build a new one in that case.
  //
  vtkLookupTable *useMe = LookupTable;
  vtkLookupTable *tmp = vtkLookupTable::New();
  if (tMax <= tMin)
    {
    tMax = tMin+1.;
    tmp->SetRange(tMin, tMax);
    tmp->Build();
    useMe = tmp;
    }
  for (i=0; i<numColors; i++)
    {
    ptIds[0] = 2*i;
    ptIds[1] = ptIds[0] + 1;
    ptIds[2] = ptIds[1] + 2;
    ptIds[3] = ptIds[0] + 2;
    polys->InsertNextCell(4,ptIds);

    if (this->UseDefinedLabels && !this->definedLabels.empty() )
      {
      LevelColorMap::iterator it;
      if (!this->ReverseOrder)
          idx = i;
      else 
          idx = numColors-1-i; 
      if ((it = labelColorMap.find(definedLabels[idx])) != labelColorMap.end())
        {
        vtkIdType colorIndex = it->second;
        rgba = useMe->GetPointer(colorIndex);
        }
      else
        {
        rgba = useMe->MapValue((float)i);
        }
      }
    else
      {
      float val = (((float)i)/(numColors-1.0)) * (tMax - tMin) + tMin;
      rgba = useMe->MapValue(val);
      }
 
    rgb = colors->GetPointer(3*i); //write into array directly
    rgb[0] = rgba[0];
    rgb[1] = rgba[1];
    rgb[2] = rgba[2];
    } // loop on numColors
  tmp->Delete();

  this->BuildTics(barWidth, x[1]);
  if (this->LabelVisibility)
    {
    this->BuildLabels(viewport, barWidth, barHeight);
    }

} // BuildColorBar


// *********************************************************************
//  Modifications:
//    Kathleen Bonnell, Tue Nov  6 08:37:40 PST 2001
//    Don't render tic marks if labels won't be rendered.
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

    if ( this->ColorBarVisibility )
      {
      this->BuildColorBar(viewport);
      }

    this->BuildTime.Modified();
    }
    

  // Everything is built, just have to render

#ifndef NO_ANNOTATIONS
  if (this->Title != NULL && this->TitleOkayToDraw && this->TitleVisibility)
    {
    renderedSomething += this->TitleActor->RenderOpaqueGeometry(viewport);
    }
#endif

  if ( this->ColorBarVisibility )
    {
    this->ColorBarActor->RenderOpaqueGeometry(viewport);
    if (this->LabelOkayToDraw && this->LabelVisibility)
      {
      this->TicsActor->RenderOpaqueGeometry(viewport);
      }

#ifndef NO_ANNOTATIONS
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
#endif
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

  os << indent << "FontSize: " << this->FontSize << "\n";
  os << indent << "Bold: " << (this->Bold ? "On\n" : "Off\n");
  os << indent << "Italic: " << (this->Italic ? "On\n" : "Off\n");
  os << indent << "Shadow: " << (this->Shadow ? "On\n" : "Off\n");
  os << indent << "Label Format: " << this->LabelFormat << "\n";
  os << indent << "Range Format: " << this->RangeFormat << "\n";
  os << indent << "Title Fraction: " << this->TitleFraction << "\n";
  os << indent << "Label Fraction: " << this->LabelFraction << "\n";
  os << indent << "Title Visibility: " 
     << (this->TitleVisibility ? "On\n" : "Off\n");
  os << indent << "ColorBar Visibility: " 
     << (this->ColorBarVisibility ? "On\n" : "Off\n");
  os << indent << "Label Visibility: " 
     << (this->LabelVisibility ? "On\n" : "Off\n");
  os << indent << "Range Visibility: " 
     << (this->RangeVisibility ? "On\n" : "Off\n");
  os << indent << "Use Defined Labels: " 
     << (this->UseDefinedLabels ? "On\n" : "Off\n");
  if ( !definedLabels.empty() )
    {
    vtkIndent indent2 = indent.GetNextIndent();
    os << indent << "Defined Labels: \n";
    for (int i = 0; i < definedLabels.size(); i++)
       os << indent2 << this->definedLabels[i].c_str() << "\n";
    }
  os << indent << "Position: " << this->PositionCoordinate << "\n";
  this->PositionCoordinate->PrintSelf(os, indent.GetNextIndent());
  os << indent << "Width: " << this->GetWidth() << "\n";
  os << indent << "Height: " << this->GetHeight() << "\n";
   

}


void vtkVerticalScalarBarActor::SetDefinedLabels(const stringVector &labels)
{
  this->definedLabels = labels;
}

void vtkVerticalScalarBarActor::SetDefinedLabels(const doubleVector &values)
{
  this->definedLabels.clear();
  char labelString[30];
  for (int i = 0; i < values.size(); ++i)
    {
    sprintf(labelString, this->LabelFormat, values[i]);
    this->definedLabels.push_back(labelString);
    }
}


void vtkVerticalScalarBarActor::SetPosition(float x[2]) 
{
  this->SetPosition(x[0],x[1]);
} 

void vtkVerticalScalarBarActor::SetPosition(float x, float y) 
{ 
  this->PositionCoordinate->SetCoordinateSystemToNormalizedViewport(); 
  this->PositionCoordinate->SetValue(x,y); 
} 

vtkCoordinate *vtkVerticalScalarBarActor::GetPosition2Coordinate() 
{ 
    vtkDebugMacro(<< this->GetClassName() << " (" << this 
                  << "): returning Position2Coordinate address " 
                  << this->Position2Coordinate ); 
    return this->Position2Coordinate; 
} 

void vtkVerticalScalarBarActor::SetPosition2(float x[2]) 
{
  this->SetPosition2(x[0],x[1]);
} 

void vtkVerticalScalarBarActor::SetPosition2(float x, float y) 
{ 
  this->Position2Coordinate->SetCoordinateSystemToNormalizedViewport(); 
  this->Position2Coordinate->SetValue(x,y); 
} 

float *vtkVerticalScalarBarActor::GetPosition2() 
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
    this->SetFontSize(a->GetFontSize());
    this->SetLabelFormat(a->GetLabelFormat());
    this->SetRangeFormat(a->GetRangeFormat());
    this->SetTitle(a->GetTitle());

    this->SetTitleFraction(a->GetTitleFraction());
    this->SetLabelFraction(a->GetLabelFraction());

    this->SetTitleVisibility(a->GetTitleVisibility());
    this->SetRangeVisibility(a->GetRangeVisibility());
    this->SetLabelVisibility(a->GetLabelVisibility());

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

float
vtkVerticalScalarBarActor::SkewTheValue(float val, float min, float max)
{
  if (this->SkewFactor < 0.) this->SkewFactor = 1.;
  if (this->SkewFactor == 1.) return val;

  int expmax = (int) floor(log10(max));
  int dif = abs(expmax) %3;
  int useexp = (expmax > 0) ? (expmax -dif) : (expmax +dif);
  float scale = pow(10.0, (double)useexp);
  float rangeDif = max - min;
  float log_skew_inv = 1./(log(this->SkewFactor));
  float k = (this->SkewFactor -1.) / rangeDif;

  float v2 = log((val - min) * k + 1) * log_skew_inv;
  float temp  = (rangeDif * v2 + min) / scale ;
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
 
    sprintf(this->RangeFormat, "\nMax: %%# -9.%dg\nMin: %%# -9.%dg", 
            digitsPastDecimal, digitsPastDecimal);
}
