/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkKatAxisActor.cxx,v $
  Language:  C++
  Date:      $Date: 2001/09/14 09:11:06 $
  Version:   $Revision: 1.1 $
  Thanks:    Kathleen Bonnell, B Division, Lawrence Livermore Nat'l Laboratory

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
#include <float.h>
#include "vtkKatAxisActor.h"
#include <vtkObjectFactory.h>
#include <vtkProperty.h>
#include <vtkViewport.h>

// ****************************************************************
// Modifications:
//   Kathleen Bonnell, Wed Mar  6 13:48:48 PST 2002 
//   Replace 'New' method with macro to match VTK 4.0 API.
// ****************************************************************

vtkStandardNewMacro(vtkKatAxisActor);
vtkCxxSetObjectMacro(vtkKatAxisActor, Camera, vtkCamera); 

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
// ****************************************************************

vtkKatAxisActor::vtkKatAxisActor()
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
  sprintf(this->LabelFormat,"%s","%-#6.3g");

  this->TitleVector = vtkVectorText::New();
  this->TitleMapper = vtkPolyDataMapper::New();
  this->TitleMapper->SetInput(this->TitleVector->GetOutput());
  this->TitleActor = vtkFollower::New();
  this->TitleActor->SetMapper(this->TitleMapper);
  
  // to avoid deleting/rebuilding create once up front
  this->NumberOfLabelsBuilt = 0;
  this->LabelVectors = NULL; 
  this->LabelMappers = NULL; 
  this->LabelActors = NULL; 

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

  this->valueScaleFactor = 1.;
  this->mustAdjustValue = false;
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
  this->ForceLabelReset = false;
}


// ****************************************************************
// Modifications:
//   Kathleen Bonnell, Wed Mar  6 13:48:48 PST 2002 
//   Added call to set camera to null.
// ****************************************************************

vtkKatAxisActor::~vtkKatAxisActor()
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
void vtkKatAxisActor::ReleaseGraphicsResources(vtkWindow *win)
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
// ****************************************************************

void vtkKatAxisActor::ShallowCopy(vtkProp *prop)
{
  vtkKatAxisActor *a = vtkKatAxisActor::SafeDownCast(prop);
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
    this->mustAdjustValue = a->mustAdjustValue;
    this->valueScaleFactor = a->valueScaleFactor;
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
// ****************************************************************

int vtkKatAxisActor::RenderOpaqueGeometry(vtkViewport *viewport)
{
  int i, renderedSomething=0;

  this->BuildAxis(viewport);

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


// ****************************************************************
// Render the axis, ticks, title, and labels.
//
// Modifications:
//   Kathleen Bonnell, Thu May 16 10:13:56 PDT 2002 
//   Don't render a zero-length axis. 
// ****************************************************************

int vtkKatAxisActor::RenderOverlay(vtkViewport *viewport)
{
  int i, renderedSomething=0;

  // Everything is built, just have to render

  if (!this->AxisHasZeroLength)
    {
    if (this->Title != NULL && this->Title[0] != 0 && this->TitleVisibility)
      {
      renderedSomething += this->TitleActor->RenderOverlay(viewport);
      }

    if (this->AxisVisibility || this->TickVisibility)
      {
      renderedSomething += this->AxisActor->RenderOverlay(viewport);
      }
    
    if (this->LabelVisibility)
      {
      for (i=0; i<this->NumberOfLabelsBuilt; i++)
        {
        renderedSomething += this->LabelActors[i]->RenderOverlay(viewport);
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
// **************************************************************************

void vtkKatAxisActor::BuildAxis(vtkViewport *viewport)
{
  // We'll do our computation in world coordinates. First determine the
  // location of the endpoints.
  float *x, p1[3], p2[3];
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

  if (this->GetMTime()     < this->BuildTime.GetMTime() &&
      viewport->GetMTime() < this->BuildTime.GetMTime())
    {
    return; //already built
    }

  vtkDebugMacro(<<"Rebuilding axis");

  if (this->GetProperty()->GetMTime() > this->BuildTime.GetMTime())
    {
    this->AxisActor->SetProperty(this->GetProperty());
    this->TitleActor->SetProperty(this->GetProperty());
    }

  // Compute the location of tick marks and labels (if range has changed)
  if (this->Range[0] != this->LastRange[0] ||
      this->Range[1] != this->LastRange[1] )
    {
    this->AdjustTicksComputeRange(this->Range); 
    }

  //
  // Generate the axis and tick marks.
  //
  if (this->AxisType == VTK_AXIS_TYPE_X)
      this->BuildXTypeAxis(viewport, p1, p2, this->Range[1] - this->Range[0]);
  else if (this->AxisType == VTK_AXIS_TYPE_Y)
      this->BuildYTypeAxis(viewport, p1, p2, this->Range[1] - this->Range[0]);
  else
      this->BuildZTypeAxis(viewport, p1, p2, this->Range[1] - this->Range[0]);

  this->LastAxisPosition = this->AxisPosition;
  this->LastTickLocation = this->TickLocation;

  this->LastRange[0] = this->Range[0];
  this->LastRange[1] = this->Range[1];
  this->BuildTime.Modified();
}

// **********************************************************************
// Build an X-axis.  Determine end-points for ticks, create
// the necessary lines.  Call methods to build labels and title.
//
// Modifications:
//   Kathleen Bonnell, Wed Nov  7 17:45:20 PST 2001 
//   Moved bulk of code to SetAxisPointsAndLines.
//   Added logic so that sub-parts are not rebuilt every time the
//   viewport changes. 
//
//   Kathleen Bonnell, Fri Nov 30 17:02:41 PST 2001
//   Added test for modified Range before call to ScaleAndSetLabels. 
//
//   Kathleen Bonnell, Mon Dec  3 16:49:01 PST 2001
//   Added test for modified Bounds before call to ScaleAndSetLabels. 
// **********************************************************************
void 
vtkKatAxisActor::BuildXTypeAxis(vtkViewport *viewport, float p1[3], 
                                float p2[3], float ext) 
{
  bool ticksRebuilt = BuildTickPointsForXType(p1, p2);
  bool tickVisChanged = this->TickVisibilityChanged();
  if (ticksRebuilt || tickVisChanged)
   {
   SetAxisPointsAndLines(p1, p2);
   }

  if (this->LabelVisibility)
    {
    if (this->BuildLabels(p2[0], ext) ||
        this->BuildTime.GetMTime() <  this->BoundsTime.GetMTime() || 
        this->AxisPosition != this->LastAxisPosition ||
        this->LastRange[0] != this->Range[0] ||
        this->LastRange[1] != this->Range[1])
      {
      this->ScaleAndSetLabels(viewport);
      }
    }

  if (this->Title != NULL && this->Title[0] != 0 && this->TitleVisibility)
    {
    this->BuildTitle(p1, p2);
    }
}


// **********************************************************************
// Build a Y-axis.  Determine end-points for ticks, create
// the necessary lines.  Call methods to build labels and title.
//
// Modifications:
//   Kathleen Bonnell, Wed Nov  7 17:45:20 PST 2001 
//   Moved bulk of code to SetAxisPointsAndLines.
//   Added logic so that sub-parts are not rebuilt every time the
//   viewport changes. 
//
//   Kathleen Bonnell, Fri Nov 30 17:02:41 PST 2001
//   Added test for modified Range before call to ScaleAndSetLabels. 
//
//   Kathleen Bonnell, Mon Dec  3 16:49:01 PST 2001
//   Added test for modified Bounds before call to ScaleAndSetLabels. 
// **********************************************************************

void 
vtkKatAxisActor::BuildYTypeAxis(vtkViewport *viewport, float p1[3], 
                                float p2[3], float ext) 
{
  bool ticksRebuilt = BuildTickPointsForYType(p1, p2);
  bool tickVisChanged = this->TickVisibilityChanged();

  if (ticksRebuilt || tickVisChanged)
   {
   SetAxisPointsAndLines(p1, p2);
   }

  if (this->LabelVisibility)
    {
    if (this->BuildLabels(p2[1], ext) ||
        this->BuildTime.GetMTime() <  this->BoundsTime.GetMTime() || 
        this->AxisPosition != this->LastAxisPosition ||
        this->LastRange[0] != this->Range[0] ||
        this->LastRange[1] != this->Range[1])
      {
      this->ScaleAndSetLabels(viewport);
      }
    }

  // Now build the title
  if (this->Title != NULL && this->Title[0] != 0 && this->TitleVisibility)
    {
    this->BuildTitle(p1, p2);
    }
}

// **********************************************************************
// Build a Z-axis.  Determine end-points for ticks, create
// the necessary lines.  Call methods to build labels and title.
//
// Modifications:
//   Kathleen Bonnell, Wed Nov  7 17:45:20 PST 2001 
//   Moved bulk of code to SetAxisPointsAndLines.
//   Added logic so that sub-parts are not rebuilt every time the
//   viewport changes. 
//
//   Kathleen Bonnell, Fri Nov 30 17:02:41 PST 2001
//   Added test for modified Range before call to ScaleAndSetLabels. 
//
//   Kathleen Bonnell, Mon Dec  3 16:49:01 PST 2001
//   Added test for modified Bounds before call to ScaleAndSetLabels. 
// **********************************************************************

void 
vtkKatAxisActor::BuildZTypeAxis(vtkViewport * viewport, float p1[3], 
                                float p2[3], float ext) 
{
  bool ticksRebuilt = BuildTickPointsForZType(p1, p2);
  bool tickVisChanged = this->TickVisibilityChanged();

  if (ticksRebuilt || tickVisChanged)
   {
   SetAxisPointsAndLines(p1, p2);
   }

  if (this->LabelVisibility)
    {
    if (this->BuildLabels(p2[2], ext) || 
        this->BuildTime.GetMTime() <  this->BoundsTime.GetMTime() || 
        this->AxisPosition != this->LastAxisPosition ||
        this->LastRange[0] != this->Range[0] ||
        this->LastRange[1] != this->Range[1])
      {
      this->ScaleAndSetLabels(viewport); 
      }
    }

  // Now build the title
  if (this->Title != NULL && this->Title[0] != 0 && this->TitleVisibility)
    {
    this->BuildTitle(p1, p2);
    } 
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
// ****************************************************************

bool vtkKatAxisActor::BuildLabels(float lastVal, float extents)
{
  char string[512];
  int labelCount = 0;
  float val = this->MajorStart;

  // figure out how many labels we need:
  while (val <= lastVal && labelCount < VTK_MAX_LABELS)
    {
    labelCount++;
    val += this->DeltaMajor;
    }

  // do we really need to do more?
  if (!this->ForceLabelReset && 
      labelCount == this->NumberOfLabelsBuilt  &&
      this->MajorStart == this->LastLabelStart &&
      this->Range[0] == this->LastRange[0]     &&
      this->Range[1] == this->LastRange[1] )
    {
    for (int i = 0; i < labelCount; i++)
      {
      this->LabelActors[i]->SetCamera(this->Camera);
      this->LabelActors[i]->SetProperty(this->GetProperty());
      }
      return false;
    }

  // allocate storage for LabelVectors
  this->SetNumberOfLabels(labelCount);
  
  val = this->MajorStart;
  for (int i = 0; i < labelCount; i++)
    {
    if (fabs(val) < 0.01 && extents > 1)
      {
      // We just happened to fall at something near zero and the range is
      // large, so set it to zero to avoid ugliness.
      val = 0.;  
      }
    if (this->mustAdjustValue)
      {
      sprintf(string, this->LabelFormat, val * valueScaleFactor);
      }
    else
      {
      sprintf(string, this->LabelFormat, val);
      }
    this->LabelVectors[i]->SetText(string);
    this->LabelActors[i]->SetCamera(this->Camera);
    this->LabelActors[i]->SetProperty(this->GetProperty());
    val += this->DeltaMajor;
    }
  this->LastLabelStart = this->MajorStart;
  this->LabelBuildTime.Modified();
  return true;
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
// *******************************************************************
void 
vtkKatAxisActor::ScaleAndSetLabels(vtkViewport *viewport) 
{
  float target, scale = 1., length, maxLength = 0.;
  float bounds[6], center[3], tick[3], pos[3];
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
  for (i = 0; i < this->NumberOfLabelsBuilt; i++)
    {
    ptIdx = 4*i + 1;
    majorTickPts->GetPoint(ptIdx, tick);
    //
    // As the label actors are re-used, 'prime' the actor by setting its
    // position to that of the corresponding tick, otherwise it 
    // will use previously set position in determining length
    //
    this->LabelActors[i]->SetPosition(tick[0], tick[1], tick[2]);
    this->LabelActors[i]->SetScale(1.);
    length = this->LabelActors[i]->GetLength();
    maxLength = (length > maxLength ? length : maxLength);
    }

  //
  // xadjust & yadjust are used for positioning the label correctly
  // depending upon the 'orientation' of the axis as determined
  // by its position in view space (via transformed bounds). 
  //
  float displayBounds[6] = { 0., 0., 0., 0., 0., 0.};
  this->TransformBounds(viewport, displayBounds);
  float xadjust = (displayBounds[0] > displayBounds[1] ? -1 : 1);
  float yadjust = (displayBounds[2] > displayBounds[3] ? -1 : 1);

  //
  // Gather information about the bounding box, used in setting
  // a target value for scaling.
  //
  float bWidth  = this->Bounds[1] - this->Bounds[0];
  float bHeight = this->Bounds[3] - this->Bounds[2];

  float bLength = sqrt (bWidth * bWidth + bHeight * bHeight);


  //
  // An arbitrary target that appears to work for our purposes.
  //
  target = bLength * 0.04;
  if (maxLength != 0.) 
    {
    scale = target / maxLength;
    }

  for (i=0; i < this->NumberOfLabelsBuilt; i++)
    {
    ptIdx = 4*i + 1;
    majorTickPts->GetPoint(ptIdx, tick);

    this->LabelActors[i]->SetScale(scale);
    this->LabelActors[i]->GetBounds(bounds);

    float halfWidth  = (bounds[1] - bounds[0]) * 0.5;
    float halfHeight = (bounds[3] - bounds[2]) * 0.5;

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
// **********************************************************************
void
vtkKatAxisActor::BuildTitle(float p1[3], float p2[3])
{
  float labBounds[6], titleBounds[6], center[3], pos[3];
  float labHeight, maxHeight = 0, labWidth, maxWidth = 0;
  float scale, target, bWidth, bHeight, bLength;
  float halfTitleWidth, halfTitleHeight, titleLength;

  int xmult, ymult;

  if (this->LabelBuildTime.GetMTime() < this->BuildTime.GetMTime() &&
      this->BoundsTime.GetMTime() < this->BuildTime.GetMTime() &&
      this->AxisPosition == this->LastAxisPosition)
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
  this->TitleActor->SetCamera(this->Camera);
  this->TitleActor->SetScale(1.);
  this->TitleActor->SetPosition(p2[0], p2[1], p2[2]);
  titleLength = this->TitleActor->GetLength();

  //
  // Target/scale use information about the bounding box,
  // gather that here.
  //
  bWidth = this->Bounds[1] - this->Bounds[0];
  bHeight = this->Bounds[3] - this->Bounds[2];
  bLength = sqrt (bWidth * bWidth + bHeight * bHeight);

  scale = 1.;

  //
  // An arbitrary target that appears to work for our purposes.
  //
  target = bLength * 0.10;
  if (titleLength != 0.)
    {
    scale = target / titleLength;
    }

  this->TitleActor->SetScale(scale);
  this->TitleActor->GetBounds(titleBounds);
  halfTitleWidth  = (titleBounds[1] - titleBounds[0]) * 0.5; 
  halfTitleHeight = (titleBounds[3] - titleBounds[2]) * 0.5; 
  
  center[0] = p1[0] + (p2[0] - p1[0]) / 2.0;
  center[1] = p1[1] + (p2[1] - p1[1]) / 2.0;
  center[2] = p1[2] + (p2[2] - p1[2]) / 2.0;

  center[0] += xmult * (halfTitleWidth + maxWidth); 
  center[1] += ymult * (halfTitleHeight + 2*maxHeight);

  pos[0] = center[0] - halfTitleWidth;
  pos[1] = center[1] - halfTitleHeight;
  pos[2] = center[2]; 
  this->TitleActor->SetPosition(pos[0], pos[1], pos[2]);
}

//
//  Transform the bounding box to display coordinates.  Used
//  in determining orientation of the axis.
//
void vtkKatAxisActor::TransformBounds(vtkViewport *viewport, float bnds[6])
{
    float minPt[3], maxPt[3], transMinPt[3], transMaxPt[3];
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



inline float ffix(float value)
{
  int ivalue = (int)value;
  return (float) ivalue;
}

inline float fsign(float value, float sign)
{
  value = fabs(value);
  if (sign < 0.)
    {
    value *= -1.;
    }
  return value;
}

// *******************************************************************
// Method: vtkKatAxisActor::AdjustTicksComputeRange
//
// Purpose: Sets private members controlling the number and position
//          of ticks.
//   
// Arguments:
//   inRange   The range for this axis.
//
// Note:    The bulk of this method was taken from vtkHankAxisActor.C
//          The original method was reduced to serve the purposes
//          of this class.   
//
// Programmer: Kathleen Bonnell 
// Creation:   29 August, 2001 
//
// *******************************************************************

void 
vtkKatAxisActor::AdjustTicksComputeRange(float inRange[2]) 
{
  double sortedRange[2], range;
  double fxt, fnt, frac;
  double div, major, minor;
  double majorStart, minorStart; 
  int numTicks;

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

  fxt = pow(10.f, ffix(pow10));
    
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

  // Figure out the first major and minor tick locations, relative to the
  // start of the axis.
  if (sortedRange[0] <= 0.)
    {
    majorStart = major*(ffix(sortedRange[0]*(1./major)) + 0.);
    minorStart = minor*(ffix(sortedRange[0]*(1./minor)) + 0.);
    }
  else
    {
    majorStart = major*(ffix(sortedRange[0]*(1./major)) + 1.);
    minorStart = minor*(ffix(sortedRange[0]*(1./minor)) + 1.);
    }

  this->MinorStart = minorStart; 
  this->MajorStart = majorStart; 

  this->DeltaMinor = minor; 
  this->DeltaMajor = major; 
}

// ****************************************************************
// Modifications:
//   Kathleen Bonnell, Wed Mar  6 13:48:48 PST 2002
//   Call superclass's method in new VTK 4.0 way.
// ****************************************************************

void vtkKatAxisActor::PrintSelf(ostream& os, vtkIndent indent)
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

// ****************************************************************
// Modifications:
//   Kathleen Bonnell, Thu Aug  1 13:44:02 PDT 2002 
//   Set a flag that forces label values to be reset. 
// ****************************************************************
void
vtkKatAxisActor::SetValueScaleFactor(const float scale)
{
    if (!this->mustAdjustValue || this->valueScaleFactor != scale)
    {
       this->ForceLabelReset = true;
    }
    else
    {
       this->ForceLabelReset = false;
    }
    this->mustAdjustValue = true;
    this->valueScaleFactor = scale;
}

// ****************************************************************
// Modifications:
//   Kathleen Bonnell, Thu Aug  1 13:44:02 PDT 2002 
//   Set a flag that forces label values to be reset. 
// ****************************************************************
void
vtkKatAxisActor::UnSetValueScaleFactor()
{
    if (this->mustAdjustValue)
    {
       this->Modified();
       this->ForceLabelReset = true;
    }
    else
    {
       this->ForceLabelReset = false;
    }
    this->mustAdjustValue = false;
    this->valueScaleFactor = 1.0;
}

// **************************************************************************
// Allocates memory (if necessary) for requested number of labels. 
//
// Programmer:  Kathleen Bonnell
// Creation:    November 7, 2001
// **************************************************************************
void
vtkKatAxisActor::SetNumberOfLabels(const int numLabels)
{
  if (this->NumberOfLabelsBuilt == numLabels)
    {
        return;
    }
 

  if (this->LabelMappers != NULL)
    {
    for (int i=0; i < this->NumberOfLabelsBuilt; i++)
      {
      this->LabelVectors[i]->Delete();
      this->LabelMappers[i]->Delete();
      this->LabelActors[i]->Delete();
      }
    delete [] this->LabelVectors;
    delete [] this->LabelMappers;
    delete [] this->LabelActors;
    }

  this->LabelVectors = new vtkVectorText * [numLabels];
  this->LabelMappers = new vtkPolyDataMapper * [numLabels];
  this->LabelActors = new vtkFollower * [numLabels];
  for ( int i=0; i < numLabels; i++)
    {
    this->LabelVectors[i] = vtkVectorText::New();
    this->LabelMappers[i] = vtkPolyDataMapper::New();
    this->LabelMappers[i]->SetInput(this->LabelVectors[i]->GetOutput());
    this->LabelActors[i] = vtkFollower::New();
    this->LabelActors[i]->SetMapper(this->LabelMappers[i]);
    }
  this->NumberOfLabelsBuilt = numLabels;
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
// **************************************************************************
bool vtkKatAxisActor::BuildTickPointsForXType(float p1[3], float p2[3])
{

  if ((this->AxisPosition == this->LastAxisPosition) &&
      (this->TickLocation == this->LastTickLocation ) &&
      (this->BoundsTime.GetMTime() < this->BuildTime.GetMTime()))
    {
    return false;
    }


  float xPoint1[3], xPoint2[3], yPoint[3], zPoint[3], x;
  int numTicks;

  this->minorTickPts->Reset();
  this->majorTickPts->Reset();
  this->gridlinePts->Reset();

  //
  // Ymult & Zmult control adjustments to tick position based
  // upon "where" this axis is located in relation to the underlying
  // assumed bounding box.
  //
  int Ymult = multiplierTable1[this->AxisPosition];
  int Zmult = multiplierTable2[this->AxisPosition];

  //
  // Build Minor Ticks
  //
  if (this->TickLocation == VTK_TICKS_OUTSIDE) 
    {
    xPoint1[1] = xPoint2[1] = zPoint[1] = p1[1]; 
    xPoint1[2] = xPoint2[2] = yPoint[2] = p1[2]; 
    yPoint[1] = p1[1] + Ymult * this->MinorTickSize; 
    zPoint[2] = p1[2] + Zmult * this->MinorTickSize; 
    }
  else if (this->TickLocation == VTK_TICKS_INSIDE) 
    {
    yPoint[1] = xPoint2[1] = zPoint[1] = p1[1]; 
    xPoint1[2] = yPoint[2] = zPoint[2] = p1[2]; 
    xPoint1[1] = p1[1] - Ymult * this->MinorTickSize; 
    xPoint2[2] = p1[2] - Zmult * this->MinorTickSize; 
    }
  else // both sides
    {
    xPoint2[1] = zPoint[1] = p1[1]; 
    xPoint1[2] = yPoint[2] = p1[2]; 
    yPoint[1] = p1[1] + Ymult * this->MinorTickSize; 
    zPoint[2] = p1[2] + Zmult * this->MinorTickSize; 
    xPoint1[1] = p1[1] - Ymult * this->MinorTickSize; 
    xPoint2[2] = p1[2] - Zmult * this->MinorTickSize; 
    }
  x = this->MinorStart;
  numTicks = 0;
  while (x <= p2[0] && numTicks < VTK_MAX_TICKS)
    {
    xPoint1[0] = xPoint2[0] = yPoint[0] = zPoint[0] = x;
    // xy-portion
    this->minorTickPts->InsertNextPoint(xPoint1);
    this->minorTickPts->InsertNextPoint(yPoint);
    // xz-portion
    this->minorTickPts->InsertNextPoint(xPoint2);
    this->minorTickPts->InsertNextPoint(zPoint);
    x+= this->DeltaMinor;
    numTicks++;
    }

  //
  // Gridline points 
  //
  yPoint[1] = xPoint2[1] = zPoint[1] = p1[1];
  xPoint1[1] = p1[1] - Ymult * this->GridlineYLength; 
  xPoint1[2] = yPoint[2] = zPoint[2] = p1[2]; 
  xPoint2[2] = p1[2] - Zmult * this->GridlineZLength; 

  x = this->MajorStart;
  numTicks = 0;
  while (x <= p2[0] && numTicks < VTK_MAX_TICKS)
    {
    xPoint1[0] = xPoint2[0] = yPoint[0] = zPoint[0] = x;
    // xy-portion
    this->gridlinePts->InsertNextPoint(xPoint1);
    this->gridlinePts->InsertNextPoint(yPoint);
    // xz-portion
    this->gridlinePts->InsertNextPoint(xPoint2);
    this->gridlinePts->InsertNextPoint(zPoint);
    x += this->DeltaMajor;
    numTicks++;
    }

  //
  // Major ticks
  //
  if (this->TickLocation == VTK_TICKS_OUTSIDE) 
    {
    xPoint1[1] = xPoint2[1] = zPoint[1] = p1[1]; 
    xPoint1[2] = xPoint2[2] = yPoint[2] = p1[2]; 
    yPoint[1] = p1[1] + Ymult * this->MajorTickSize; 
    zPoint[2] = p1[2] + Zmult * this->MajorTickSize; 
    }
  else if (this->TickLocation == VTK_TICKS_INSIDE) 
    {
    yPoint[1] = xPoint2[1] = zPoint[1] = p1[1]; 
    xPoint1[2] = yPoint[2] = zPoint[2] = p1[2]; 
    xPoint1[1] = p1[1] - Ymult * this->MajorTickSize; 
    xPoint2[2] = p1[2] - Zmult * this->MajorTickSize; 
    }
  else // both sides
    {
    xPoint2[1] = zPoint[1] = p1[1]; 
    xPoint1[2] = yPoint[2] = p1[2]; 
    yPoint[1] = p1[1] + Ymult * this->MajorTickSize; 
    zPoint[2] = p1[2] + Zmult * this->MajorTickSize; 
    xPoint1[1] = p1[1] - Ymult * this->MajorTickSize; 
    xPoint2[2] = p1[2] - Zmult * this->MajorTickSize; 
    }
  x = this->MajorStart;
  numTicks = 0;
  while (x <= p2[0] && numTicks < VTK_MAX_TICKS)
    {
    xPoint1[0] = xPoint2[0] = yPoint[0] = zPoint[0] = x;
    // xy-portion
    this->majorTickPts->InsertNextPoint(xPoint1);
    this->majorTickPts->InsertNextPoint(yPoint);
    // xz-portion
    this->majorTickPts->InsertNextPoint(xPoint2);
    this->majorTickPts->InsertNextPoint(zPoint);
    x += this->DeltaMajor;
    numTicks++;
    }

  return true;
}

// **************************************************************************
// Creates points for ticks (minor, major, gridlines) in correct position 
// for Y-type axis.
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
// **************************************************************************
bool vtkKatAxisActor::BuildTickPointsForYType(float p1[3], float p2[3])
{
  if ((this->AxisPosition  == this->LastAxisPosition) &&
      (this->TickLocation == this->LastTickLocation) &&
      (this->BoundsTime.GetMTime() < this->BuildTime.GetMTime()))
    {
    return false;
    }

  float yPoint1[3], yPoint2[3], xPoint[3], zPoint[3], y;
  int numTicks;

  this->minorTickPts->Reset();
  this->majorTickPts->Reset();
  this->gridlinePts->Reset();
  //
  // Xmult & Zmult control adjustments to tick position based
  // upon "where" this axis is located in relation to the underlying
  // assumed bounding box.
  //

  int Xmult = multiplierTable1[this->AxisPosition];
  int Zmult = multiplierTable2[this->AxisPosition];

  
  //
  // The ordering of the tick endpoints is important because
  // label position is defined by them.
  //

  //
  // minor ticks
  //
  if (this->TickLocation == VTK_TICKS_INSIDE)      
    {
    yPoint1[2] = xPoint[2] = zPoint[2] = p1[2]; 
    yPoint2[0] = xPoint[0] = zPoint[0] = p1[0];
    yPoint1[0] = p1[0] - Xmult * this->MinorTickSize;
    yPoint2[2] = p1[2] - Zmult * this->MinorTickSize; 
    }
  else if (this->TickLocation == VTK_TICKS_OUTSIDE) 
    {
    yPoint1[0] = yPoint2[0] = zPoint[0] = p1[0];
    yPoint1[2] = yPoint2[2] = xPoint[2] = p1[2];
    xPoint[0] = p1[0] + Xmult * this->MinorTickSize;
    zPoint[2] = p1[2] + Zmult * this->MinorTickSize;
    }
  else                              // both sides
    {
    yPoint1[2] = xPoint[2] = p1[2]; 
    yPoint2[0] = zPoint[0] = p1[0];
    yPoint1[0] = p1[0] - Xmult * this->MinorTickSize;
    yPoint2[2] = p1[2] + Zmult * this->MinorTickSize;
    xPoint[0]  = p1[0] + Xmult * this->MinorTickSize;
    zPoint[2]  = p1[2] - Zmult * this->MinorTickSize;
    }
  y = this->MinorStart;
  numTicks = 0;
  while (y < p2[1] && numTicks < VTK_MAX_TICKS)
    {
    yPoint1[1] = xPoint[1] = yPoint2[1] = zPoint[1] = y;
    // yx portion
    this->minorTickPts->InsertNextPoint(yPoint1);
    this->minorTickPts->InsertNextPoint(xPoint);
    // yz portion
    this->minorTickPts->InsertNextPoint(yPoint2);
    this->minorTickPts->InsertNextPoint(zPoint);
    y += this->DeltaMinor;
    numTicks++;
    }

  //
  // gridlines
  // 
  yPoint1[0] = p1[0] - Xmult * this->GridlineXLength;
  yPoint2[2] = p1[2] - Zmult * this->GridlineZLength;
  yPoint2[0] = xPoint[0] = zPoint[0]  = p1[0];
  yPoint1[2] = xPoint[2] = zPoint[2]  = p1[2];
    
  y = this->MajorStart;
  numTicks = 0;
  while (y < p2[1] && numTicks < VTK_MAX_TICKS)
    {
    yPoint1[1] = xPoint[1] = yPoint2[1] = zPoint[1] = y;
    // yx portion
    this->gridlinePts->InsertNextPoint(yPoint1);
    this->gridlinePts->InsertNextPoint(xPoint);
    // yz portion
    this->gridlinePts->InsertNextPoint(yPoint2);
    this->gridlinePts->InsertNextPoint(zPoint);
    y += this->DeltaMajor;
    numTicks++;
    }

  //
  // major ticks
  //
  if (this->TickLocation == VTK_TICKS_INSIDE)
    {
    yPoint1[2] = xPoint[2] = zPoint[2] = p1[2]; 
    yPoint2[0] = xPoint[0] = zPoint[0] = p1[0];
    yPoint1[0] = p1[0] - Xmult * this->MajorTickSize;
    yPoint2[2] = p1[2] - Zmult * this->MajorTickSize; 
    }
  else if (this->TickLocation == VTK_TICKS_OUTSIDE) 
    {
    yPoint1[0] = yPoint2[0] = zPoint[0] = p1[0];
    yPoint1[2] = yPoint2[2] = xPoint[2] = p1[2];
    xPoint[0] = p1[0] + Xmult * this->MajorTickSize;
    zPoint[2] = p1[2] + Zmult * this->MajorTickSize;
    }
  else                              // both sides
    {
    yPoint1[2] = xPoint[2] = p1[2]; 
    yPoint2[0] = zPoint[0] = p1[0];
    yPoint1[0] = p1[0] - Xmult * this->MajorTickSize;
    yPoint2[2] = p1[2] + Zmult * this->MajorTickSize;
    xPoint[0]  = p1[0] + Xmult * this->MajorTickSize;
    zPoint[2]  = p1[2] - Zmult * this->MajorTickSize;
    }
  y = this->MajorStart;
  numTicks = 0;
  while (y < p2[1] && numTicks < VTK_MAX_TICKS)
    {
    yPoint1[1] = xPoint[1] = yPoint2[1] = zPoint[1] = y;
    // yx portion
    this->majorTickPts->InsertNextPoint(yPoint1);
    this->majorTickPts->InsertNextPoint(xPoint);
    // yz portion
    this->majorTickPts->InsertNextPoint(yPoint2);
    this->majorTickPts->InsertNextPoint(zPoint);
    y += this->DeltaMajor;
    numTicks++;
    }
  return true;
}

// **************************************************************************
// Creates points for ticks (minor, major, gridlines) in correct position 
// for Z-type axis.
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
// **************************************************************************

bool vtkKatAxisActor::BuildTickPointsForZType(float p1[3], float p2[3])
{
  if ((this->AxisPosition  == this->LastAxisPosition) &&
      (this->TickLocation == this->LastTickLocation) &&
      (this->BoundsTime.GetMTime() < this->BuildTime.GetMTime()))
    {
    return false;
    }

  this->minorTickPts->Reset();
  this->majorTickPts->Reset();
  this->gridlinePts->Reset();

  //
  // Xmult & Ymult control adjustments to tick position based
  // upon "where" this axis is located in relation to the underlying
  // assumed bounding box.
  //
  int Xmult = multiplierTable1[this->AxisPosition];
  int Ymult = multiplierTable2[this->AxisPosition];

  float zPoint1[3], zPoint2[3], xPoint[3], yPoint[3], z;
  int numTicks;

  //
  // The ordering of the tick endpoints is important because
  // label position is defined by them.
  //

  //
  // minor ticks
  //
  if (this->TickLocation == VTK_TICKS_INSIDE)      
    {
    zPoint1[0] = p1[0] - Xmult * this->MinorTickSize;
    zPoint2[1] = p1[1] - Ymult * this->MinorTickSize;
    zPoint2[0] = xPoint[0] = yPoint[0]  = p1[0];
    zPoint1[1] = xPoint[1] = yPoint[1]  = p1[1];
    }
  else if (this->TickLocation == VTK_TICKS_OUTSIDE) 
    {
    xPoint[0]  = p1[0] + Xmult * this->MinorTickSize;
    yPoint[1]  = p1[1] + Ymult * this->MinorTickSize;
    zPoint1[0] = zPoint2[0] = yPoint[0] = p1[0];
    zPoint1[1] = zPoint2[1] = xPoint[1] = p1[1];
    }
  else                              // both sides
    {
    zPoint1[0] = p1[0] - Xmult * this->MinorTickSize;
    xPoint[0]  = p1[0] + Xmult * this->MinorTickSize;
    zPoint2[1] = p1[1] - Ymult * this->MinorTickSize;
    yPoint[1]  = p1[1] + Ymult * this->MinorTickSize;
    zPoint1[1] = xPoint[1] = p1[1];
    zPoint2[0] = yPoint[0] = p1[0];
    }
  z = this->MinorStart;
  numTicks = 0;
  while (z < p2[2] && numTicks < VTK_MAX_TICKS)
    {
    zPoint1[2] = zPoint2[2] = xPoint[2] = yPoint[2] = z;
    // zx-portion
    this->minorTickPts->InsertNextPoint(zPoint1);
    this->minorTickPts->InsertNextPoint(xPoint);
    // zy-portion
    this->minorTickPts->InsertNextPoint(zPoint2);
    this->minorTickPts->InsertNextPoint(yPoint);
    z += this->DeltaMinor;
    numTicks++;
    }

  //
  // gridlines
  // 
  zPoint1[0] = p1[0] - Xmult * this->GridlineXLength;
  zPoint2[1] = p1[1] - Ymult * this->GridlineYLength;
  zPoint1[1] = xPoint[1] = yPoint[1] = p1[1];
  zPoint2[0] = xPoint[0] = yPoint[0] = p1[0];

  z = this->MajorStart;
  numTicks = 0;
  while (z < p2[2] && numTicks < VTK_MAX_TICKS)
    {
    zPoint1[2] = zPoint2[2] = xPoint[2] = yPoint[2] = z;
    // zx-portion
    this->gridlinePts->InsertNextPoint(zPoint1);
    this->gridlinePts->InsertNextPoint(xPoint);
    // zy-portion
    this->gridlinePts->InsertNextPoint(zPoint2);
    this->gridlinePts->InsertNextPoint(yPoint);
    z += this->DeltaMajor;
    numTicks++;
    }

  //
  // major ticks
  //

  if (this->TickLocation == VTK_TICKS_INSIDE)   
    {
    zPoint1[0] = p1[0] - Xmult * this->MajorTickSize;
    zPoint2[1] = p1[1] - Ymult * this->MajorTickSize;
    zPoint2[0] = xPoint[0] = yPoint[0]  = p1[0];
    zPoint1[1] = xPoint[1] = yPoint[1]  = p1[1];
    }
  else if (this->TickLocation == VTK_TICKS_OUTSIDE) 
    {
    xPoint[0]  = p1[0] + Xmult * this->MajorTickSize;
    yPoint[2]  = p1[1] + Ymult * this->MajorTickSize;
    zPoint1[0] = zPoint2[0] = yPoint[0] = p1[0];
    zPoint1[1] = zPoint2[1] = xPoint[1] = p1[1];
    }
  else                              // both sides
    {
    zPoint1[0] = p1[0] - Xmult * this->MajorTickSize;
    xPoint[0]  = p1[0] + Xmult * this->MajorTickSize;
    zPoint2[1] = p1[1] - Ymult * this->MajorTickSize;
    yPoint[1]  = p1[1] + Ymult * this->MajorTickSize;
    zPoint1[1] = xPoint[1] = p1[1];
    zPoint2[0] = yPoint[0] = p1[0];
    }
  z = this->MajorStart;
  numTicks = 0;
  while (z < p2[2] && numTicks < VTK_MAX_TICKS)
    {
    zPoint1[2] = zPoint2[2] = xPoint[2] = yPoint[2] = z;
    // zx-portion
    this->majorTickPts->InsertNextPoint(zPoint1);
    this->majorTickPts->InsertNextPoint(xPoint);
    // zy-portion
    this->majorTickPts->InsertNextPoint(zPoint2);
    this->majorTickPts->InsertNextPoint(yPoint);
    z += this->DeltaMajor;
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
// **************************************************************************
void vtkKatAxisActor::SetAxisPointsAndLines(float p1[3], float p2[3])
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
    ptIds[0] = pts->InsertNextPoint(p1); //first axis point
    ptIds[1] = pts->InsertNextPoint(p2); //last axis point
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
vtkKatAxisActor::TickVisibilityChanged()
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
vtkKatAxisActor::SetBounds(float b[6])
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
float * vtkKatAxisActor::GetBounds()
{
    return this->Bounds;
}

// *********************************************************************
// Retrieves the bounds of this actor. 
//
// Programmer:  Kathleen Bonnell
// Creation:    November 7, 2001
// *********************************************************************

void vtkKatAxisActor::GetBounds(float b[6])
{
    for (int i = 0; i < 6; i++)
      b[i] = this->Bounds[i];
}
