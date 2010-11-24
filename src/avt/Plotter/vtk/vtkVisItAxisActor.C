/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItAxisActor.cxx,v $
  Language:  C++
  Date:      $Date: 2001/09/14 09:11:06 $
  Version:   $Revision: 1.1 $
  Thanks:    Kathleen Bonnell, B Division, Lawrence Livermore Nat'l Laboratory

Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen 
All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

#include <math.h>
#include <float.h>

#include "vtkVisItAxisActor.h"
#include <vtkCellArray.h>
#include <vtkObjectFactory.h>
#include <vtkProperty.h>
#include <vtkViewport.h>
#include <snprintf.h>
#include <algorithm>

using std::string;
using std::vector;
using std::min;
using std::max;

// ****************************************************************
// Modifications:
//   Kathleen Bonnell, Wed Mar  6 13:48:48 PST 2002 
//   Replace 'New' method with macro to match VTK 4.0 API.
// ****************************************************************

vtkStandardNewMacro(vtkVisItAxisActor);
vtkCxxSetObjectMacro(vtkVisItAxisActor, Camera, vtkCamera); 

// ****************************************************************
// Instantiate this object.
//
// Modifications:
//   Kathleen Bonnell, Wed Oct 31 07:57:49 PST 2001
//   Initialize new members mustAdjustValue and valueScaleFactor.
//
//   Kathleen Bonnell, Wed Nov  7 16:19:16 PST 2001 
//   No longer allocate large amounts of memory for labels, instead
//   allocate dynamically.  Initialize new members: 
//   LastLabelStart; LastAxisPosition; LastTickLocation; LastTickVisibility; 
//   LastDrawGridlines; LastMinorTicksVisible; LastRange; minorTickPts; 
//   majorTickPts; gridlinePts.
//
//   Kathleen Bonnell, Thu May 16 10:13:56 PDT 2002 
//   Initialize new member AxisHasZeroLength. 
//
//   Kathleen Bonnell, Thu Aug  1 13:44:02 PDT 2002 
//   Initialize new member ForceLabelReset. 
//
//   Kathleen Bonnell, Fri Jul 25 14:37:32 PDT 2003 
//   Removed mustAdjustValue, valueScaleFator, ForceLabelReset.
//
//   Kathleen Bonnell, Thu Apr 29 17:02:10 PDT 2004
//   Initialize MinorStart, MajorStart, DeltaMinor, DeltaMajor.
//
//   Brad Whitlock, Wed Mar 26 11:26:05 PDT 2008
//   Added TitleTextProperty, LabelTextProperty.
//
//   Eric Brugger, Tue Oct 21 12:02:53 PDT 2008
//   Added support for specifying tick mark locations.
//
// ****************************************************************

vtkVisItAxisActor::vtkVisItAxisActor()
{
  this->Point1Coordinate = vtkCoordinate::New();
  this->Point1Coordinate->SetCoordinateSystemToWorld();
  this->Point1Coordinate->SetValue(0.0, 0.0, 0.0);

  this->Point2Coordinate = vtkCoordinate::New();
  this->Point2Coordinate->SetCoordinateSystemToWorld();
  this->Point2Coordinate->SetValue(0.75, 0.0, 0.0);

  this->Camera = NULL;
  this->Title = NULL;
  this->MinorTicksVisible = 1;
  this->MajorTickSize = 1.0;
  this->MinorTickSize = 0.5;
  this->TickLocation = VTK_TICKS_INSIDE; 
  this->Range[0] = 0.0;
  this->Range[1] = 1.0;
  
  this->Bounds[0] = this->Bounds[2] = this->Bounds[4] = -1;
  this->Bounds[1] = this->Bounds[3] = this->Bounds[5] = 1;

  this->LabelFormat = new char[8]; 
  SNPRINTF(this->LabelFormat,8, "%s","%-#6.3g");

  this->TitleVector = vtkMultiFontVectorText::New();
  this->TitleMapper = vtkPolyDataMapper::New();
  this->TitleMapper->SetInput(this->TitleVector->GetOutput());
  this->TitleActor = vtkFollower::New();
  this->TitleActor->SetMapper(this->TitleMapper);

  this->TitleTextProperty = vtkTextProperty::New();
  this->TitleTextProperty->SetColor(0.,0.,0.);
  this->TitleTextProperty->SetFontFamilyToArial();

  // to avoid deleting/rebuilding create once up front
  this->NumberOfLabelsBuilt = 0;
  this->LabelVectors = NULL; 
  this->LabelMappers = NULL; 
  this->LabelActors = NULL; 

  this->LabelTextProperty = vtkTextProperty::New();
  this->LabelTextProperty->SetColor(0.,0.,0.);
  this->LabelTextProperty->SetFontFamilyToArial();

  this->Axis = vtkPolyData::New();
  this->AxisMapper = vtkPolyDataMapper::New();
  this->AxisMapper->SetInput(this->Axis);
  this->AxisActor = vtkActor::New();
  this->AxisActor->SetMapper(this->AxisMapper);
  
  this->AxisVisibility = 1;
  this->TickVisibility = 1;
  this->LabelVisibility = 1;
  this->TitleVisibility = 1;
  
  this->DrawGridlines = 0;
  this->GridlineXLength = 1.;  
  this->GridlineYLength = 1.;  
  this->GridlineZLength = 1.;  

  this->AxisType = VTK_AXIS_TYPE_X; 
  //
  // AxisPosition denotes which of the four possibilities in relation
  // to the bounding box.  An x-Type axis with min min, means the x-axis
  // at minimum y and minimum z values of the bbox.
  //
  this->AxisPosition = VTK_AXIS_POS_MINMIN;

  this->LastLabelStart = 100000;

  this->LastAxisPosition = -1;
  this->LastTickLocation = -1; 
  this->LastTickVisibility = -1; 
  this->LastDrawGridlines = -1; 
  this->LastMinorTicksVisible = -1; 
  this->LastRange[0] = -1.0;
  this->LastRange[1] = -1.0;

  this->minorTickPts = vtkPoints::New();
  this->majorTickPts = vtkPoints::New();
  this->gridlinePts  = vtkPoints::New();

  this->AxisHasZeroLength = false;

  this->MinorStart = 0.;
  this->MajorStart = 0.;
  this->DeltaMinor = 1.;
  this->DeltaMajor = 1.;

  this->AdjustLabels = 1;
  this->MajorTickMinimum = 0.0;
  this->MajorTickMaximum = 1.0;
  this->MajorTickSpacing = 1.0;
  this->MinorTickSpacing = 0.1;
  this->LastAdjustLabels = -1;
  this->LastMajorTickMinimum = -1.0;
  this->LastMajorTickMaximum = -1.0;
  this->LastMajorTickSpacing = -1.0;
  this->LastMinorTickSpacing = -1.0;
}


// ****************************************************************
// Modifications:
//   Kathleen Bonnell, Wed Mar  6 13:48:48 PST 2002 
//   Added call to set camera to null.
//
//   Brad Whitlock, Wed Mar 26 11:28:34 PDT 2008
//   Added deletion of title and label text properties.
//
// ****************************************************************

vtkVisItAxisActor::~vtkVisItAxisActor()
{
  this->SetCamera(NULL);

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

  if (this->TitleVector)
    {
    this->TitleVector->Delete();
    this->TitleVector = NULL;
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

  if (this->TitleTextProperty)
    {
       this->TitleTextProperty->Delete();
       this->TitleTextProperty = NULL;
    }

  if (this->LabelMappers != NULL)
    {
    for (int i=0; i < this->NumberOfLabelsBuilt; i++)
      {
      this->LabelVectors[i]->Delete();
      this->LabelMappers[i]->Delete();
      this->LabelActors[i]->Delete();
      }
    this->NumberOfLabelsBuilt = 0;
    delete [] this->LabelVectors;
    delete [] this->LabelMappers;
    delete [] this->LabelActors;
    this->LabelVectors = NULL;
    this->LabelMappers = NULL;
    this->LabelActors = NULL;
    }
  if (this->LabelTextProperty)
    {
       this->LabelTextProperty->Delete();
       this->LabelTextProperty = NULL;
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

  if (this->minorTickPts)
    {
    this->minorTickPts ->Delete();
    this->minorTickPts = NULL; 
    }
  if (this->majorTickPts)
    {
    this->majorTickPts->Delete();
    this->majorTickPts = NULL; 
    }
  if (this->gridlinePts)
    {
    this->gridlinePts->Delete();
    this->gridlinePts = NULL; 
    }
}

// Release any graphics resources that are being consumed by this actor.
// The parameter window could be used to determine which graphic
// resources to release.
void vtkVisItAxisActor::ReleaseGraphicsResources(vtkWindow *win)
{
  this->TitleActor->ReleaseGraphicsResources(win);
  for (int i=0; i < this->NumberOfLabelsBuilt; i++)
    {
    this->LabelActors[i]->ReleaseGraphicsResources(win);
    }
  this->AxisActor->ReleaseGraphicsResources(win);
}


// ****************************************************************
//
// Modifications:
//   Kathleen Bonnell, Wed Oct 31 07:57:49 PST 2001
//   Copy over mustAdjustValue and valueScaleFactor.
//
//   Kathleen Bonnell, Wed Mar  6 13:48:48 PST 2002
//   Call superclass's method in new VTK 4.0 way.
//
//   Kathleen Bonnell, Fri Jul 25 14:37:32 PDT 2003 
//   Removed mustAdjustValue, valueScaleFator.
//
//   Brad Whitlock, Wed Mar 26 11:34:24 PDT 2008
//   Added TitleTextProperty, LabelTextProperty
//
//   Eric Brugger, Tue Oct 21 12:02:53 PDT 2008
//   Added support for specifying tick mark locations.
//
// ****************************************************************

void vtkVisItAxisActor::ShallowCopy(vtkProp *prop)
{
  vtkVisItAxisActor *a = vtkVisItAxisActor::SafeDownCast(prop);
  if (a != NULL)
    {
    this->SetPoint1(a->GetPoint1());
    this->SetPoint2(a->GetPoint2());
    this->SetCamera(a->GetCamera());
    this->SetRange(a->GetRange());
    this->SetLabelFormat(a->GetLabelFormat());
    this->SetTitle(a->GetTitle());
    this->SetAxisVisibility(a->GetAxisVisibility());
    this->SetTickVisibility(a->GetTickVisibility());
    this->SetLabelVisibility(a->GetLabelVisibility());
    this->SetTitleVisibility(a->GetTitleVisibility());
//    this->SetTitleTextProperty(a->GetTitleTextProperty());
//    this->SetLabelTextProperty(a->GetLabelTextProperty());
    this->SetAdjustLabels(a->GetAdjustLabels());
    this->SetMajorTickMinimum(a->GetMajorTickMinimum());
    this->SetMajorTickMaximum(a->GetMajorTickMaximum());
    this->SetMajorTickSpacing(a->GetMajorTickSpacing());
    this->SetMinorTickSpacing(a->GetMinorTickSpacing());
    }

  // Now do superclass
  this->Superclass::ShallowCopy(prop);
}

// ****************************************************************
// Build the axis, ticks, title, and labels and render.
//
// Modifications:
//   Kathleen Bonnell, Thu May 16 10:13:56 PDT 2002 
//   Don't render a zero-length axis. 
//
//   Kathleen Bonnell, Fri Jul 25 14:37:32 PDT 2003 
//   Added bool argument to BuildAxis. 
//
// ****************************************************************

int vtkVisItAxisActor::RenderOpaqueGeometry(vtkViewport *viewport)
{
  int i, renderedSomething=0;

  this->BuildAxis(viewport, false);

  // Everything is built, just have to render

  if (!this->AxisHasZeroLength)
    {
    if (this->Title != NULL && this->Title[0] != 0 && this->TitleVisibility)
      {
      renderedSomething += this->TitleActor->RenderOpaqueGeometry(viewport);
      }

    if (this->AxisVisibility || this->TickVisibility)
      {
      renderedSomething += this->AxisActor->RenderOpaqueGeometry(viewport);
      }
  
    if (this->LabelVisibility)
      {
      for (i=0; i<this->NumberOfLabelsBuilt; i++)
        {
        renderedSomething += this->LabelActors[i]->RenderOpaqueGeometry(viewport);
        }
      }
    }

  return renderedSomething;
}

// **************************************************************************
// Perform some initialization, determine which Axis type we are
// and call the appropriate build method.
//
// Modifications:
//   Kathleen Bonnell, Wed Nov  7 17:45:20 PST 2001
//   Added logic to only rebuild sub-parts if necessary.
//
//   Kathleen Bonnell, Fri Nov 30 17:02:41 PST 2001 
//   Moved setting values for LastRange to end of method, so they
//   can be used in comparisons elsewhere.
//
//   Kathleen Bonnell, Mon Dec  3 16:49:01 PST 2001
//   Compare vtkTimeStamps correctly.
//
//   Kathleen Bonnell, Thu May 16 10:13:56 PDT 2002 
//   Test for zero length axis. 
//
//   Kathleen Bonnell,  Fri Jul 25 14:37:32 PDT 2003
//   Added bool argument that will allow all axis components to be built
//   if set to true.  Removed call to AdjustTicksComputeRange (handled by
//   vtkVisItCubeAxesActor.  Remvoed call to Build?TypeAxis, added calls
//   to BuildLabels, SetAxisPointsAndLines and BuildTitle, (which used to
//   be handled in Build?TypeAxis).
//
//   Brad Whitlock, Wed Mar 26 11:42:11 PDT 2008
//   Added code to set the title actor property's color from the title
//   text property.
//
//   Eric Brugger, Tue Oct 21 12:02:53 PDT 2008
//   Added support for specifying tick mark locations.
//
//   Kathleen Bonnell, Wed Nov 24 11:08:03 PST 2010
//   Fix minor memory leak with NewTitleProperty.
// 
// **************************************************************************

void vtkVisItAxisActor::BuildAxis(vtkViewport *viewport, bool force)
{
  // We'll do our computation in world coordinates. First determine the
  // location of the endpoints.
  double *x, p1[3], p2[3];
  x = this->Point1Coordinate->GetValue();
  p1[0] = x[0]; p1[1] = x[1]; p1[2] = x[2];
  x = this->Point2Coordinate->GetValue();
  p2[0] = x[0]; p2[1] = x[1]; p2[2] = x[2];

  //
  //  Test for axis of zero length.
  //
  if (p1[0] == p2[0] && p1[1] == p2[1] && p1[2] == p2[2])
  {
      vtkDebugMacro(<<"Axis has zero length, not building.");
      this->AxisHasZeroLength = true;
      return;
  }
  this->AxisHasZeroLength = false;

  if (!force && this->GetMTime() < this->BuildTime.GetMTime() &&
      viewport->GetMTime() < this->BuildTime.GetMTime())
    {
    return; //already built
    }

  vtkDebugMacro(<<"Rebuilding axis");

  if (force || 
      this->GetProperty()->GetMTime() > this->BuildTime.GetMTime() ||
      this->TitleTextProperty->GetMTime() > this->BuildTime.GetMTime()
      )
    {
    vtkProperty *newProp = this->NewTitleProperty();
    this->TitleActor->SetProperty(newProp);
    this->AxisActor->SetProperty(this->GetProperty());
    newProp->Delete();
    }

  //
  // Generate the axis and tick marks.
  //
  bool ticksRebuilt = this->BuildTickPoints(p1, p2, force);

  bool tickVisChanged = this->TickVisibilityChanged();

  if (force || ticksRebuilt || tickVisChanged)
   {
   SetAxisPointsAndLines();
   }

  this->BuildLabels(viewport, force);

  if (this->Title != NULL && this->Title[0] != 0) 
    {
    this->BuildTitle(force);
    }

  this->LastAxisPosition = this->AxisPosition;
  this->LastTickLocation = this->TickLocation;

  this->LastRange[0] = this->Range[0];
  this->LastRange[1] = this->Range[1];

  this->LastAdjustLabels = this->AdjustLabels;
  this->LastMajorTickMinimum = this->MajorTickMinimum;
  this->LastMajorTickMaximum = this->MajorTickMaximum;
  this->LastMajorTickSpacing = this->MajorTickSpacing;
  this->LastMinorTickSpacing = this->MinorTickSpacing;

  this->BuildTime.Modified();
}


// ****************************************************************
//
//  Set label values and properties. 
//
// Modifications:
//   Kathleen Bonnell, Wed Oct 31 07:57:49 PST 2001
//   Use valueScaleFactor to scale value if necessary. 
//
//   Kathleen Bonnell, Wed Nov  7 17:45:20 PST 2001 
//   Added code for early termination.  Added call to SetNumberOfLabels
//   for dynamic memory allocation. Number of labels limited to 200.
//
//   Kathleen Bonnell, Fri Nov 30 17:02:41 PST 2001
//   Added test for modified range to determine if labels really need to 
//   be built.
//
//   Kathleen Bonnell, Thu May 16 10:13:56 PDT 2002 
//   Use defined constant to limit number of labels.   
//
//   Kathleen Bonnell, Fri Jul 25 14:37:32 PDT 2003 
//   Remvoed determination of label text, added call to 
//   SetLabelPositions.
//
//   Brad Whitlock, Wed Mar 26 11:44:03 PDT 2008
//   Set the label property color from the label text property.
//
//   Eric Brugger, Tue Oct 21 12:02:53 PDT 2008
//   Added support for specifying tick mark locations.
//
//   Kathleen Bonnell, Wed Nov 24 11:10:51 PST 2010
//   Fixed minor memory leak due to use of NewLabelProperty.
//
// ****************************************************************

void
vtkVisItAxisActor::BuildLabels(vtkViewport *viewport, bool force)
{
  if (!force && !this->LabelVisibility)
      return;

  vtkProperty *newProp = this->NewLabelProperty();
  for (int i = 0; i < this->NumberOfLabelsBuilt; i++)
    {
    this->LabelActors[i]->SetCamera(this->Camera);
    this->LabelActors[i]->SetProperty(newProp);
    }

  newProp->Delete();

  if (force || this->BuildTime.GetMTime() <  this->BoundsTime.GetMTime() || 
      this->LastAxisPosition != this->AxisPosition ||
      this->LastRange[0] != this->Range[0] ||
      this->LastRange[1] != this->Range[1] ||
      this->LastAdjustLabels != this->AdjustLabels ||
      this->LastMajorTickMinimum != this->MajorTickMinimum ||
      this->LastMajorTickMaximum != this->MajorTickMaximum ||
      this->LastMajorTickSpacing != this->MajorTickSpacing ||
      this->LastMinorTickSpacing != this->MinorTickSpacing)
    {
    this->SetLabelPositions(viewport, force);
    }
}

int multiplierTable1[4] = { -1, -1, 1,  1};
int multiplierTable2[4] = { -1,  1, 1, -1};

// *******************************************************************
// Determine and set scale factor and position for labels.
//
// Modifications:
//   Kathleen Bonnell, Fri Nov 30 17:02:41 PST 2001
//   Reset labels scale to 1. before testing length, in order to
//   ensure proper scaling.  Use Bounds[1] and Bounds[0] for bWidth
//   instead of Bounds[5] and Bounds[4].
//
//   Kathleen Bonnell, Tue Dec  4 09:55:03 PST 2001 
//   Ensure that scale does not go below MinScale. 
//
//   Kathleen Bonnell, Tue Apr  9 14:41:08 PDT 2002  
//   Removed MinScale as it allowed axes with very small ranges
//   to have labels scaled too large for the dataset. 
//
//   Kathleen Bonnell, Fri Jul 18 09:09:31 PDT 2003 
//   Renamed to SetLabelPosition.  Removed calculation of label
//   scale factor, added check for no labels to early return test.
//
//   Eric Brugger, Tue Jul 29 14:42:44 PDT 2003
//   Corrected the test that causes the routine to exit early when
//   no work needs to be done.
//
// *******************************************************************

void 
vtkVisItAxisActor::SetLabelPositions(vtkViewport *viewport, bool force) 
{
  if (!force && (!this->LabelVisibility || this->NumberOfLabelsBuilt == 0)) 
      return;

  double bounds[6], center[3], tick[3], pos[3];
  int i, xmult, ymult;

  switch (this->AxisType)
   {
   case VTK_AXIS_TYPE_X : 
            xmult = 0; 
            ymult = multiplierTable1[this->AxisPosition]; 
            break;
   case VTK_AXIS_TYPE_Y : 
            xmult = multiplierTable1[this->AxisPosition];
            ymult = 0; 
            break;
   case VTK_AXIS_TYPE_Z :
            xmult = multiplierTable1[this->AxisPosition];
            ymult = multiplierTable2[this->AxisPosition]; 
            break;
   }

  int ptIdx;
  //
  // xadjust & yadjust are used for positioning the label correctly
  // depending upon the 'orientation' of the axis as determined
  // by its position in view space (via transformed bounds). 
  //
  double displayBounds[6] = { 0., 0., 0., 0., 0., 0.};
  this->TransformBounds(viewport, displayBounds);
  double xadjust = (displayBounds[0] > displayBounds[1] ? -1 : 1);
  double yadjust = (displayBounds[2] > displayBounds[3] ? -1 : 1);

  for (i=0; i < this->NumberOfLabelsBuilt; i++)
    {
    ptIdx = 4*i + 1;
    majorTickPts->GetPoint(ptIdx, tick);

    this->LabelActors[i]->GetBounds(bounds);

    double halfWidth  = (bounds[1] - bounds[0]) * 0.5;
    double halfHeight = (bounds[3] - bounds[2]) * 0.5;

    center[0] = tick[0] + xmult * (halfWidth  + this->MinorTickSize);
    center[1] = tick[1] + ymult * (halfHeight + this->MinorTickSize); 
    pos[0] = (center[0] - xadjust *halfWidth);
    pos[1] = (center[1] - yadjust *halfHeight);
    pos[2] = tick[2]; 
    this->LabelActors[i]->SetPosition(pos[0], pos[1], pos[2]);
    }
}


// **********************************************************************
//  Determines scale and position for the Title.  Currently,
//  title can only be centered with respect to its axis.
//  
//  Modifications:
//    Kathleen Bonnell, Wed Nov  7 17:45:20 PST 2001
//    Added logic for early-termination.
//
//    Kathleen Bonnell, Mon Dec  3 16:49:01 PST 2001
//    Test for modified bounds before early termination, use
//    MinScale, modified target so title size is a bit more reasonable. 
//
//    Kathleen Bonnell, Tue Apr  9 14:41:08 PDT 2002  
//    Removed MinScale as it allowed axes with very small ranges
//    to have labels scaled too large for the dataset. 
//
//    Kathleen Bonnell, Fri Jul 25 14:37:32 PDT 2003 
//    Added bool argument that allow the build to be forced, even if
//    the title won't be visible. 
//   
//    Kathleen Bonnell, Tue Aug 31 16:17:43 PDT 2004 
//    Added TitleTime test so that the title can be rebuilt when its
//    text has changed.
//
//    Hank Childs, Sun May 13 11:06:12 PDT 2007
//    Fix bug with positioning of titles (the titles were being placed
//    far away from the bounding box in some cases).
//
//    Brad Whitlock, Wed Mar 26 11:54:04 PDT 2008
//    Set the font and bold/italic for the TitleVector, which is now a
//    vtkMultiFontVectorText.
//
// **********************************************************************

void
vtkVisItAxisActor::BuildTitle(bool force)
{
  if (!force && !this->TitleVisibility)
  {
      return;
  }
  double labBounds[6], titleBounds[6], center[3], pos[3];
  double labHeight, maxHeight = 0, labWidth, maxWidth = 0;
  double halfTitleWidth, halfTitleHeight;

  double *p1 = this->Point1Coordinate->GetValue();
  double *p2 = this->Point2Coordinate->GetValue();
  int xmult, ymult;

  if (!force && this->LabelBuildTime.GetMTime() < this->BuildTime.GetMTime() &&
      this->BoundsTime.GetMTime() < this->BuildTime.GetMTime() &&
      this->AxisPosition == this->LastAxisPosition &&
      this->TitleTextTime.GetMTime() < this->BuildTime.GetMTime() &&
      this->TitleTextProperty->GetMTime() < this->BuildTime.GetMTime())
   {
   return;
   }

  switch (this->AxisType)
   {
   case VTK_AXIS_TYPE_X : 
            xmult = 0; 
            ymult = multiplierTable1[this->AxisPosition]; 
            break;
   case VTK_AXIS_TYPE_Y :
            xmult = multiplierTable1[this->AxisPosition];
            ymult = 0; 
            break;
   case VTK_AXIS_TYPE_Z : 
            xmult = multiplierTable1[this->AxisPosition];
            ymult = multiplierTable2[this->AxisPosition]; 
            break;
   }
  //
  //  Title should be in relation to labels (if any)
  //  so find out information about them.
  //
  for (int i = 0; i < this->NumberOfLabelsBuilt; i++)
    {
    this->LabelActors[i]->GetBounds(labBounds);
    labWidth = labBounds[1] - labBounds[0]; 
    maxWidth = (labWidth > maxWidth ? labWidth : maxWidth); 
    labHeight = labBounds[3] - labBounds[2]; 
    maxHeight = (labHeight > maxHeight ? labHeight : maxHeight); 
    }
  this->TitleVector->SetText(this->Title);
  this->TitleVector->SetFontFamily(this->TitleTextProperty->GetFontFamily());
  this->TitleVector->SetBold(this->TitleTextProperty->GetBold());
  this->TitleVector->SetItalic(this->TitleTextProperty->GetItalic());

  this->TitleActor->SetCamera(this->Camera);
  this->TitleActor->SetPosition(p2[0], p2[1], p2[2]);
  this->TitleActor->GetBounds(titleBounds);
  halfTitleWidth  = (titleBounds[1] - titleBounds[0]) * 0.5; 
  halfTitleHeight = (titleBounds[3] - titleBounds[2]) * 0.5; 
  
  center[0] = p1[0] + (p2[0] - p1[0]) / 2.0;
  center[1] = p1[1] + (p2[1] - p1[1]) / 2.0;
  center[2] = p1[2] + (p2[2] - p1[2]) / 2.0;

  center[0] += xmult * (halfTitleWidth + maxWidth); 
  center[1] += ymult * (halfTitleHeight + 2*maxHeight);

  pos[0] = center[0] - xmult*halfTitleWidth;
  pos[1] = center[1] - ymult*halfTitleHeight;
  pos[2] = center[2]; 
  this->TitleActor->SetPosition(pos[0], pos[1], pos[2]);
}

//
//  Transform the bounding box to display coordinates.  Used
//  in determining orientation of the axis.
//
void vtkVisItAxisActor::TransformBounds(vtkViewport *viewport, double bnds[6])
{
    double minPt[3], maxPt[3], transMinPt[3], transMaxPt[3];
    minPt[0] = this->Bounds[0];
    minPt[1] = this->Bounds[2];
    minPt[2] = this->Bounds[4];
    maxPt[0] = this->Bounds[1];
    maxPt[1] = this->Bounds[3];
    maxPt[2] = this->Bounds[5];

    viewport->SetWorldPoint(minPt[0], minPt[1], minPt[2], 1.0);
    viewport->WorldToDisplay();
    viewport->GetDisplayPoint(transMinPt);
    viewport->SetWorldPoint(maxPt[0], maxPt[1], maxPt[2], 1.0);
    viewport->WorldToDisplay();
    viewport->GetDisplayPoint(transMaxPt);

    bnds[0] = transMinPt[0];
    bnds[2] = transMinPt[1];
    bnds[4] = transMinPt[2];
    bnds[1] = transMaxPt[0];
    bnds[3] = transMaxPt[1];
    bnds[5] = transMaxPt[2];
}


// ****************************************************************
// Modifications:
//   Kathleen Bonnell, Wed Mar  6 13:48:48 PST 2002
//   Call superclass's method in new VTK 4.0 way.
// ****************************************************************

void vtkVisItAxisActor::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Title: " << (this->Title ? this->Title : "(none)") << "\n";
  os << indent << "Number Of Labels Built: " 
     << this->NumberOfLabelsBuilt << "\n";
  os << indent << "Range: (" << this->Range[0] 
     << ", " << this->Range[1] << ")\n";

  os << indent << "Label Format: " << this->LabelFormat << "\n";
  
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
}

// **************************************************************************
// Sets text string for label vectors.  Allocates memory if necessary. 
//
// Programmer:  Kathleen Bonnell
// Creation:    July 18, 2003
//
// Modifications:
//   Brad Whitlock, Wed Mar 26 11:56:54 PDT 2008
//   Changed LabelVectors to vtkMultiFontVectorText and set additional
//   font properties on that object.
//
// **************************************************************************
void
vtkVisItAxisActor::SetLabels(const vector<string> &labels)
{
  //
  // If the number of labels has changed, re-allocate the correct
  // amount of memory.
  //
  size_t numLabels = labels.size();
  if (this->NumberOfLabelsBuilt != numLabels)
    {
    if (this->LabelMappers != NULL)
      {
      for (int i = 0; i < this->NumberOfLabelsBuilt; i++)
        {
        this->LabelVectors[i]->Delete();
        this->LabelMappers[i]->Delete();
        this->LabelActors[i]->Delete();
        }
      delete [] this->LabelVectors;
      delete [] this->LabelMappers;
      delete [] this->LabelActors;
      }

    this->LabelVectors = new vtkMultiFontVectorText * [numLabels];
    this->LabelMappers = new vtkPolyDataMapper * [numLabels];
    this->LabelActors = new vtkFollower * [numLabels];

    for (size_t i = 0; i < labels.size(); i++)
      {
      this->LabelVectors[i] = vtkMultiFontVectorText::New();

      this->LabelMappers[i] = vtkPolyDataMapper::New();
      this->LabelMappers[i]->SetInput(this->LabelVectors[i]->GetOutput());
      this->LabelActors[i] = vtkFollower::New();
      this->LabelActors[i]->SetMapper(this->LabelMappers[i]);
      }
    }

  //
  // Set the label vector text. 
  //
  for (size_t i = 0; i < numLabels; i++)
    {
    this->LabelVectors[i]->SetFontFamily(this->LabelTextProperty->GetFontFamily());
    this->LabelVectors[i]->SetBold(this->LabelTextProperty->GetBold());
    this->LabelVectors[i]->SetItalic(this->LabelTextProperty->GetItalic());

    this->LabelVectors[i]->SetText(labels[i].c_str());
    }
  this->NumberOfLabelsBuilt = numLabels;
  this->LabelBuildTime.Modified();
}

// **************************************************************************
// Creates points for ticks (minor, major, gridlines) in correct position 
// for X-type axsis.
//
// Programmer:  Kathleen Bonnell
// Creation:    November 7, 2001
//
// Modifications:
//   Kathleen Bonnell, Mon Dec  3 16:49:01 PST 2001
//   Compare vtkTimeStamps correctly.
//
//   Kathleen Bonnell, Thu May 16 10:13:56 PDT 2002 
//   Use defined constant VTK_MAX_TICKS to prevent infinite loops. 
// 
//   Kathleen Bonnell, Fri Jul 25 14:37:32 PDT 2003 
//   Allow a forced build, despite previous build time.
//
//   Brad Whitlock, Wed Apr  2 14:06:59 PDT 2008
//   Build the labels if the label properties have changed.
//
//   Eric Brugger, Tue Oct 21 12:02:53 PDT 2008
//   Added support for specifying tick mark locations.
//
//   Jeremy Meredith, Tue May 18 11:39:58 EDT 2010
//   Made this apply to all axis types with a little bit of careful indexing.
//
//   Jeremy Meredith, Tue May 18 15:31:04 EDT 2010
//   Use the Range of the axis as the values which we use when labeling
//   and assigning tick marks (e.g. via the user).  Previously, we would
//   assume the range was the same as the physical location of the axis.
//   Also did a tiny bit of cleanup and fixed a bug from my previous change.
//
// **************************************************************************
bool vtkVisItAxisActor::BuildTickPoints(double p1[3], double p2[3], bool force)
{
    if (!force &&
        (this->AxisPosition == this->LastAxisPosition) &&
        (this->TickLocation == this->LastTickLocation) &&
        (this->AdjustLabels == this->LastAdjustLabels) &&
        (this->MajorTickMinimum == this->LastMajorTickMinimum) &&
        (this->MajorTickMaximum == this->LastMajorTickMaximum) &&
        (this->MajorTickSpacing == this->LastMajorTickSpacing) &&
        (this->MinorTickSpacing == this->LastMinorTickSpacing) &&
        (this->BoundsTime.GetMTime() < this->BuildTime.GetMTime()))
    {
        return false;
    }

    double aPoint1[3], aPoint2[3], bPoint[3], cPoint[3];
    double a, aMin, aMax, aDelta;
    int numTicks;

    this->minorTickPts->Reset();
    this->majorTickPts->Reset();
    this->gridlinePts->Reset();

    //
    // Bmult & Cmult control adjustments to tick position based
    // upon "where" this axis is located in relation to the underlying
    // assumed bounding box.
    //
    // x axis: b==y, c==z
    // y axis: b==x, c==z
    // z axis: b==x, c==y
    int aAxis, bAxis, cAxis;
    switch (this->AxisType)
    {
      case VTK_AXIS_TYPE_X: aAxis=0; bAxis=1; cAxis=2; break;
      case VTK_AXIS_TYPE_Y: aAxis=1; bAxis=0; cAxis=2; break;
      case VTK_AXIS_TYPE_Z: aAxis=2; bAxis=0; cAxis=1; break;
    }
    int Bmult = multiplierTable1[this->AxisPosition];
    int Cmult = multiplierTable2[this->AxisPosition];
    double gridBLength, gridCLength;
    switch (this->AxisType)
    {
      case VTK_AXIS_TYPE_X:
        gridBLength=GridlineYLength;
        gridCLength=GridlineZLength;
        break;
      case VTK_AXIS_TYPE_Y:
        gridBLength=GridlineXLength;
        gridCLength=GridlineZLength;
        break;
      case VTK_AXIS_TYPE_Z:
        gridBLength=GridlineXLength;
        gridCLength=GridlineYLength;
        break;
    }

    //
    // The ordering of the tick endpoints is important because
    // label position is defined by them.
    //

    double aSpatialMin = p1[aAxis];
    double aSpatialMax = p2[aAxis];
    double aLower = this->Range[0];
    double aUpper = this->Range[1];

    // Figure out the mapping between logical space (i.e. the
    // axis "Range") and the coordinates in 3D space.
    double aSpatialRange = aSpatialMax-aSpatialMin;
    double aRange = aUpper - aLower;
    double aScale = 1;
    double aOffset = 0;
    if (aRange != 0)
    {
        aScale = aSpatialRange / aRange;
        aOffset = aSpatialMin - aLower*aScale;
    }

    //
    // Minor ticks
    //
    if (this->AdjustLabels)
    {
        aMin = this->MinorStart;
        aMax = aUpper;
        aDelta = this->DeltaMinor;
    }
    else
    {
        aMin = this->MajorTickMinimum +
            ceil((this->MinorStart - this->MajorTickMinimum) /
                 this->MinorTickSpacing) * this->MinorTickSpacing;
        aMin = max(aMin, this->MajorTickMinimum);
        aMax = min(aUpper, this->MajorTickMaximum);
        aMax += this->MinorTickSpacing / 1e6;
        aDelta = this->MinorTickSpacing;
    }

    if (this->TickLocation == VTK_TICKS_OUTSIDE) 
    {
        aPoint1[bAxis] = aPoint2[bAxis] = cPoint[bAxis] = p1[bAxis]; 
        aPoint1[cAxis] = aPoint2[cAxis] = bPoint[cAxis] = p1[cAxis]; 
        bPoint[bAxis] = p1[bAxis] + Bmult * this->MinorTickSize; 
        cPoint[cAxis] = p1[cAxis] + Cmult * this->MinorTickSize; 
    }
    else if (this->TickLocation == VTK_TICKS_INSIDE) 
    {
        bPoint[bAxis] = aPoint2[bAxis] = cPoint[bAxis] = p1[bAxis]; 
        aPoint1[cAxis] = bPoint[cAxis] = cPoint[cAxis] = p1[cAxis]; 
        aPoint1[bAxis] = p1[bAxis] - Bmult * this->MinorTickSize; 
        aPoint2[cAxis] = p1[cAxis] - Cmult * this->MinorTickSize; 
    }
    else // both sides
    {
        aPoint2[bAxis] = cPoint[bAxis] = p1[bAxis]; 
        aPoint1[cAxis] = bPoint[cAxis] = p1[cAxis]; 
        bPoint[bAxis] = p1[bAxis] + Bmult * this->MinorTickSize; 
        cPoint[cAxis] = p1[cAxis] + Cmult * this->MinorTickSize; 
        aPoint1[bAxis] = p1[bAxis] - Bmult * this->MinorTickSize; 
        aPoint2[cAxis] = p1[cAxis] - Cmult * this->MinorTickSize; 
    }

    numTicks = 0;
    for (a = aMin; a <= aMax && numTicks < VTK_MAX_TICKS; a += aDelta)
    {
        double aa = a * aScale + aOffset;
        aPoint1[aAxis] = aPoint2[aAxis] = bPoint[aAxis] = cPoint[aAxis] = aa;
        // ab-portion
        this->minorTickPts->InsertNextPoint(aPoint1);
        this->minorTickPts->InsertNextPoint(bPoint);
        // ac-portion
        this->minorTickPts->InsertNextPoint(aPoint2);
        this->minorTickPts->InsertNextPoint(cPoint);
        numTicks++;
    }

    //
    // Gridline points 
    //
    if (this->AdjustLabels)
    {
        aMin = this->MajorStart;
        aMax = aUpper;
        aDelta = this->DeltaMajor;
    }
    else
    {
        aMin = this->MajorTickMinimum +
            ceil((this->MajorStart - this->MajorTickMinimum) /
                 this->MajorTickSpacing) * this->MajorTickSpacing;
        aMin = max(aMin, this->MajorTickMinimum);
        aMax = min(aUpper, this->MajorTickMaximum);
        aMax = aMax + this->MajorTickSpacing / 1e6;
        aDelta = this->MajorTickSpacing;
    }

    bPoint[bAxis] = aPoint2[bAxis] = cPoint[bAxis] = p1[bAxis];
    aPoint1[bAxis] = p1[bAxis] - Bmult * gridBLength; 
    aPoint1[cAxis] = bPoint[cAxis] = cPoint[cAxis] = p1[cAxis]; 
    aPoint2[cAxis] = p1[cAxis] - Cmult * gridCLength; 

    numTicks = 0;
    for (a = aMin; a <= aMax && numTicks < VTK_MAX_TICKS; a += aDelta)
    {
        double aa = a * aScale + aOffset;
        aPoint1[aAxis] = aPoint2[aAxis] = bPoint[aAxis] = cPoint[aAxis] = aa;
        // ab-portion
        this->gridlinePts->InsertNextPoint(aPoint1);
        this->gridlinePts->InsertNextPoint(bPoint);
        // ac-portion
        this->gridlinePts->InsertNextPoint(aPoint2);
        this->gridlinePts->InsertNextPoint(cPoint);
        numTicks++;
    }

    //
    // Major ticks
    //
    if (this->TickLocation == VTK_TICKS_OUTSIDE) 
    {
        aPoint1[bAxis] = aPoint2[bAxis] = cPoint[bAxis] = p1[bAxis]; 
        aPoint1[cAxis] = aPoint2[cAxis] = bPoint[cAxis] = p1[cAxis]; 
        bPoint[bAxis] = p1[bAxis] + Bmult * this->MajorTickSize; 
        cPoint[cAxis] = p1[cAxis] + Cmult * this->MajorTickSize; 
    }
    else if (this->TickLocation == VTK_TICKS_INSIDE) 
    {
        bPoint[bAxis] = aPoint2[bAxis] = cPoint[bAxis] = p1[bAxis]; 
        aPoint1[cAxis] = bPoint[cAxis] = cPoint[cAxis] = p1[cAxis]; 
        aPoint1[bAxis] = p1[bAxis] - Bmult * this->MajorTickSize; 
        aPoint2[cAxis] = p1[cAxis] - Cmult * this->MajorTickSize; 
    }
    else // both sides
    {
        aPoint2[bAxis] = cPoint[bAxis] = p1[bAxis]; 
        aPoint1[cAxis] = bPoint[cAxis] = p1[cAxis]; 
        bPoint[bAxis] = p1[bAxis] + Bmult * this->MajorTickSize; 
        cPoint[cAxis] = p1[cAxis] + Cmult * this->MajorTickSize; 
        aPoint1[bAxis] = p1[bAxis] - Bmult * this->MajorTickSize; 
        aPoint2[cAxis] = p1[cAxis] - Cmult * this->MajorTickSize; 
    }

    numTicks = 0;
    for (a = aMin; a <= aMax && numTicks < VTK_MAX_TICKS; a += aDelta)
    {
        double aa = a * aScale + aOffset;
        aPoint1[aAxis] = aPoint2[aAxis] = bPoint[aAxis] = cPoint[aAxis] = aa;
        // ab-portion
        this->majorTickPts->InsertNextPoint(aPoint1);
        this->majorTickPts->InsertNextPoint(bPoint);
        // ac-portion
        this->majorTickPts->InsertNextPoint(aPoint2);
        this->majorTickPts->InsertNextPoint(cPoint);
        numTicks++;
    }

    return true;
}
// **************************************************************************
// Creates Poly data (lines) from tickmarks (minor/major), gridlines, and axis. 
//
// Programmer:  Kathleen Bonnell
// Creation:    November 7, 2001
//
// Modifications:
//   Kathleen Bonnell, Thu Nov 15 13:20:44 PST 2001
//   Make ptIds of type vtkIdType to match VTK 4.0 API.
//
//   Kathleen Bonnell, Thu Jul 18 13:24:03 PDT 2002 
//   Allow gridlines to be drawn when enabled, even if ticks are disabled. 
//
//   Kathleen Bonnell, Fri Jul 25 15:10:24 PDT 2003
//   Removed arguments.
//
// **************************************************************************
void 
vtkVisItAxisActor::SetAxisPointsAndLines()
{
  vtkPoints *pts = vtkPoints::New();
  vtkCellArray *lines = vtkCellArray::New();
  this->Axis->SetPoints(pts);
  this->Axis->SetLines(lines);
  pts->Delete();
  lines->Delete();
  int i, numPts, numLines; 
  vtkIdType ptIds[2];

  if (this->TickVisibility) 
    {
    if (this->MinorTicksVisible)
      {
      numPts = this->minorTickPts->GetNumberOfPoints();
      for (i = 0; i < numPts; i++)
        {
        pts->InsertNextPoint(this->minorTickPts->GetPoint(i));
        }
      }

    if (this->DrawGridlines)
      {
      numPts = this->gridlinePts->GetNumberOfPoints();
      for (i = 0; i < numPts; i++)
        {
        pts->InsertNextPoint(this->gridlinePts->GetPoint(i));
        }
      }
    else  // major tick points
      {
      numPts = this->majorTickPts->GetNumberOfPoints();
      for (i = 0; i < numPts; i++)
        {
        pts->InsertNextPoint(this->majorTickPts->GetPoint(i));
        }
      }
    }
  else if (this->DrawGridlines) // grids are enabled but ticks are off
    {
    numPts = this->gridlinePts->GetNumberOfPoints();
    for (i = 0; i < numPts; i++)
      {
      pts->InsertNextPoint(this->gridlinePts->GetPoint(i));
      }
    }

  // create lines
  numLines = pts->GetNumberOfPoints() / 2;
  for (i=0; i < numLines; i++)
    {
    ptIds[0] = 2*i;
    ptIds[1] = 2*i + 1;
    lines->InsertNextCell(2, ptIds);
    }

  if (this->AxisVisibility)
    {
    //first axis point
    ptIds[0] = pts->InsertNextPoint(this->Point1Coordinate->GetValue()); 
    //last axis point
    ptIds[1] = pts->InsertNextPoint(this->Point2Coordinate->GetValue()); 
    lines->InsertNextCell(2, ptIds);
    }
}

// *********************************************************************
// Returns true if any tick vis attribute has changed since last check.
//
// Programmer:  Kathleen Bonnell
// Creation:    November 7, 2001
// *********************************************************************
bool
vtkVisItAxisActor::TickVisibilityChanged()
{
  bool retVal = (this->TickVisibility != this->LastTickVisibility) ||
                (this->DrawGridlines != this->LastDrawGridlines)   || 
                (this->MinorTicksVisible != this->LastMinorTicksVisible);

  this->LastTickVisibility = this->TickVisibility;
  this->LastDrawGridlines = this->DrawGridlines;
  this->LastMinorTicksVisible = this->MinorTicksVisible;

  return retVal; 
}

// *********************************************************************
// Set the bounds for this actor to use.  Sets timestamp BoundsModified.
//
// Programmer:  Kathleen Bonnell
// Creation:    November 7, 2001
// *********************************************************************
void
vtkVisItAxisActor::SetBounds(double b[6])
{
  if ((this->Bounds[0] != b[0]) ||
      (this->Bounds[1] != b[1]) ||
      (this->Bounds[2] != b[2]) ||
      (this->Bounds[3] != b[3]) ||
      (this->Bounds[4] != b[4]) ||
      (this->Bounds[5] != b[5]) )
    {
    for (int i = 0; i < 6; i++)
      this->Bounds[i] = b[i];
    this->BoundsTime.Modified();
    }
}

// *********************************************************************
// Retrieves the bounds of this actor. 
//
// Programmer:  Kathleen Bonnell
// Creation:    November 7, 2001
// *********************************************************************
double * vtkVisItAxisActor::GetBounds()
{
    return this->Bounds;
}

// *********************************************************************
// Retrieves the bounds of this actor. 
//
// Programmer:  Kathleen Bonnell
// Creation:    November 7, 2001
// *********************************************************************

void vtkVisItAxisActor::GetBounds(double b[6])
{
    for (int i = 0; i < 6; i++)
      b[i] = this->Bounds[i];
}

// *********************************************************************
// Method:  vtkVisItAxisActor::ComputeMaxLabelLength
//
// Purpose: Determines the maximum length that a label will occupy
//          if placed at point 'center' and with a scale of 1. 
//
// Arguments:
//   center    The position to use for the label actor 
//
// Returns:
//   the maximum length of all the labels, 0 if there are no labels.
//
// Programmer:  Kathleen Bonnell
// Creation:    July 18, 2003 
//
// Modifications:
//   Kathleen Bonnell, Tue Dec 16 11:06:21 PST 2003
//   Reset the actor's position and scale.
//
//   Brad Whitlock, Wed Mar 26 16:28:24 PDT 2008
//   Changed to a new property created by NewLabelProperty.
//
//   Kathleen Bonnell, Wed Nov 24 11:11:25 PST 2010
//   Fixed minor memory leak due to use of NewLabelProperty.
//
// *********************************************************************

double
vtkVisItAxisActor::ComputeMaxLabelLength(const double center[3])
{
  double length, maxLength = 0.;
  double pos[3];
  double scale;

  vtkProperty *newProp = this->NewLabelProperty();
  for (int i = 0; i < this->NumberOfLabelsBuilt; i++)
    {
    this->LabelActors[i]->GetPosition(pos);
    scale = this->LabelActors[i]->GetScale()[0];

    this->LabelActors[i]->SetCamera(this->Camera);
    this->LabelActors[i]->SetProperty(newProp);
    this->LabelActors[i]->SetPosition(center[0], center[1] , center[2]);
    this->LabelActors[i]->SetScale(1.);
    length = this->LabelActors[i]->GetLength();
    maxLength = (length > maxLength ? length : maxLength); 

    this->LabelActors[i]->SetPosition(pos);
    this->LabelActors[i]->SetScale(scale);
    }
  newProp->Delete();
  return maxLength;
}


// *********************************************************************
// Method:  vtkVisItAxisActor::ComputeTitleLength
//
// Purpose: Determines the length that the title will occupy
//          if placed at point 'center' and with a scale of 1. 
//
// Arguments:
//   center    The position to use for the title actor 
//
// Returns:
//   the length of all the title, 
//
// Programmer:  Kathleen Bonnell
// Creation:    July 25, 2003 
//
// Modifications:
//   Kathleen Bonnell, Tue Dec 16 11:06:21 PST 2003
//   Reset the actor's position and scale.
//
//   Brad Whitlock, Wed Mar 26 16:28:24 PDT 2008
//   Changed to a new property created by NewTitleProperty.
//
//   Kathleen Bonnell, Wed Nov 24 11:09:39 PST 2010
//   Fix minor memory leak due to use of NewTitleProperty.
//
// *********************************************************************

double
vtkVisItAxisActor::ComputeTitleLength(const double center[3])
{
  double pos[3], scale, len;
  this->TitleActor->GetPosition(pos);
  scale = this->TitleActor->GetScale()[0];
  this->TitleVector->SetText(this->Title);
  this->TitleActor->SetCamera(this->Camera);
  vtkProperty *newProp = this->NewTitleProperty();
  this->TitleActor->SetProperty(newProp);
  newProp->Delete();
  this->TitleActor->SetPosition(center[0], center[1] , center[2]);
  this->TitleActor->SetScale(1.);
  len = this->TitleActor->GetLength();

  this->TitleActor->SetPosition(pos);
  this->TitleActor->SetScale(scale);
  return len;
}


// *********************************************************************
// Method:  vtkVisItAxisActor::SetLabelScale
//
// Purpose: Sets the scaling factor for label actors.
//
// Arguments:
//   s      The scale factor to use.
//
// Programmer:  Kathleen Bonnell
// Creation:    July 18, 2003 
//
// *********************************************************************

void
vtkVisItAxisActor::SetLabelScale(const double s)
{
  for (int i=0; i < this->NumberOfLabelsBuilt; i++)
    {
    this->LabelActors[i]->SetScale(s);
    }
}


// *********************************************************************
// Method:  vtkVisItAxisActor::SetTitleScale
//
// Purpose: Sets the scaling factor for the title actor.
//
// Arguments:
//   s      The scale factor to use.
//
// Programmer:  Kathleen Bonnell
// Creation:    July 18, 2003 
//
// *********************************************************************

void
vtkVisItAxisActor::SetTitleScale(const double s)
{
  this->TitleActor->SetScale(s);
}

// *********************************************************************
// Method:  vtkVisItAxisActor::SetTitle
//
// Purpose: Sets the text for the title.  
//
// Notes:   Not using vtkSetStringMacro so that the modification time of 
//          the text can be kept (so the title will be updated as
//          appropriate when the text changes.)
//
// Arguments:
//   t          The text to use. 
//
// Programmer:  Kathleen Bonnell
// Creation:    August 31, 2004 
//
// *********************************************************************

void
vtkVisItAxisActor::SetTitle(const char *t)
{
  if (this->Title == NULL && t == NULL)
    return;
  if (this->Title && (!strcmp(this->Title, t)))
    return;
  if (this->Title)
    delete [] this->Title;
  if (t)
    {
    this->Title = new char[strlen(t)+1];
    strcpy(this->Title, t);
    }
  else 
    {
    this->Title = NULL;
    }
  this->TitleTextTime.Modified();
  this->Modified();
}

// ****************************************************************************
// Method: vtkVisItAxisActor::SetTitleTextProperty
//
// Purpose: 
//   Sets the title text property that we should use.
//
// Arguments:
//   prop : The new text property.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 27 10:53:04 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
vtkVisItAxisActor::SetTitleTextProperty(vtkTextProperty *prop)
{
    if(this->TitleTextProperty != NULL)
        this->TitleTextProperty->Delete();
    if(prop != NULL)
        prop->Register(NULL);
    this->TitleTextProperty = prop;
    this->Modified();
}

// ****************************************************************************
// Method: vtkVisItAxisActor::SetLabelTextProperty
//
// Purpose: 
//   Sets the label text property that we should use.
//
// Arguments:
//   prop : The new text property.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 27 10:53:04 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
vtkVisItAxisActor::SetLabelTextProperty(vtkTextProperty *prop)
{
    if(this->LabelTextProperty != NULL)
        this->LabelTextProperty->Delete();
    if(prop != NULL)
        prop->Register(NULL);
    this->LabelTextProperty = prop;
    this->Modified();
}

// ****************************************************************************
// Method: vtkVisItAxisActor::NewTitleProperty
//
// Purpose: 
//   Creates a new property based on GetProperty but the color and opacity
//   are overridden by values from the title text property.
//
// Arguments:
//
// Returns:    The new property.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 27 10:53:46 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

vtkProperty *
vtkVisItAxisActor::NewTitleProperty()
{
    vtkProperty *newProp = vtkProperty::New();
    newProp->DeepCopy(this->GetProperty());
    newProp->SetColor(this->TitleTextProperty->GetColor());
    // We pass the opacity in the line offset.
    newProp->SetOpacity(this->TitleTextProperty->GetLineOffset());
    return newProp;
}

// ****************************************************************************
// Method: vtkVisItAxisActor::NewLabelProperty
//
// Purpose: 
//   Creates a new property based on GetProperty but the color and opacity
//   are overridden by values from the title label property.
//
// Arguments:
//
// Returns:    The new property.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 27 10:53:46 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

vtkProperty *
vtkVisItAxisActor::NewLabelProperty()
{
    vtkProperty *newProp = vtkProperty::New();
    newProp->DeepCopy(this->GetProperty());
    newProp->SetColor(this->LabelTextProperty->GetColor());
    // We pass the opacity in the line offset.
    newProp->SetOpacity(this->LabelTextProperty->GetLineOffset());
    return newProp;
}
