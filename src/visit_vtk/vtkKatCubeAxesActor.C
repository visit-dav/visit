/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkKatCubeAxesActor.cxx,v $
  Language:  C++
  Date:      $Date: 2001/08/03 21:55:01 $
  Version:   $Revision: 1.22 $
  Thanks:    Kathleen Bonnell, B Division, Lawrence Livermore National Lab

Copyright (c) 1993-2001 Ken Martin, Will Schroeder, Bill Lorensen 
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
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include "vtkKatCubeAxesActor.h"
#include "vtkKatAxisActor.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include <vtkProperty.h>
#include <vtkViewport.h>
#include <DebugStream.h>
#include <float.h>

int LabelExponent(float min, float max);
int Digits(float min, float max, int pow);

// *************************************************************************
// Modifications:
//   Kathleen Bonnell, Wed Mar  6 13:48:48 PST 2002
//   Replace 'New' method with Macro to match VTK 4.0 API. 
//
// *************************************************************************

vtkStandardNewMacro(vtkKatCubeAxesActor);
vtkCxxSetObjectMacro(vtkKatCubeAxesActor, Input, vtkDataSet);
vtkCxxSetObjectMacro(vtkKatCubeAxesActor, Prop, vtkProp);
vtkCxxSetObjectMacro(vtkKatCubeAxesActor, Camera,vtkCamera);


// *************************************************************************
// Instantiate this object.
//
// Modifications:
//   Kathleen Bonnell, Wed Oct 31 07:57:49 PST 2001
//   Intialize new members lastPow, last*AxisDigits.
//
//   Kathleen Bonnell, Wed Nov  7 16:19:16 PST 2001
//   Intialize new members:  Last*Extent, LastFlyMode, 
//   renderAxes*, numAxes*.
//
//   Hank Childs, Fri Sep 27 17:15:07 PDT 2002
//   Initialize new members for units.
//
// *************************************************************************

vtkKatCubeAxesActor::vtkKatCubeAxesActor()
{
  this->Input = NULL;
  this->Prop = NULL;
  this->Bounds[0] = -1.0; this->Bounds[1] = 1.0;
  this->Bounds[2] = -1.0; this->Bounds[3] = 1.0;
  this->Bounds[4] = -1.0; this->Bounds[5] = 1.0;

  this->TickLocation = VTK_TICKS_INSIDE;
  this->Camera = NULL;
  this->FlyMode = VTK_FLY_CLOSEST_TRIAD;
  int i;
  for (i = 0; i < 4; i++)
    {
    this->XAxes[i] = vtkKatAxisActor::New();
    this->XAxes[i]->SetTickVisibility(1);
    this->XAxes[i]->SetMinorTicksVisible(1);
    this->XAxes[i]->SetLabelVisibility(1);
    this->XAxes[i]->SetTitleVisibility(1);
    this->XAxes[i]->SetAxisTypeToX();
    this->XAxes[i]->SetAxisPosition(i);

    this->YAxes[i] = vtkKatAxisActor::New();
    this->YAxes[i]->SetTickVisibility(1);
    this->YAxes[i]->SetMinorTicksVisible(1);
    this->YAxes[i]->SetLabelVisibility(1);
    this->YAxes[i]->SetTitleVisibility(1);
    this->YAxes[i]->SetAxisTypeToY();
    this->YAxes[i]->SetAxisPosition(i);

    this->ZAxes[i] = vtkKatAxisActor::New();
    this->ZAxes[i]->SetTickVisibility(1);
    this->ZAxes[i]->SetMinorTicksVisible(1);
    this->ZAxes[i]->SetLabelVisibility(1);
    this->ZAxes[i]->SetTitleVisibility(1);
    this->ZAxes[i]->SetAxisTypeToZ();
    this->ZAxes[i]->SetAxisPosition(i);
    }

  this->XLabelFormat = new char[8]; 
  sprintf(this->XLabelFormat,"%s","%-#6.3g");
  this->YLabelFormat = new char[8]; 
  sprintf(this->YLabelFormat,"%s","%-#6.3g");
  this->ZLabelFormat = new char[8]; 
  sprintf(this->ZLabelFormat,"%s","%-#6.3g");
  this->CornerOffset = 0.05;
  this->Inertia = 1;
  this->RenderCount = 0;

  this->XAxisVisibility = 1;
  this->YAxisVisibility = 1;
  this->ZAxisVisibility = 1;

  this->XAxisTickVisibility = 1;
  this->YAxisTickVisibility = 1;
  this->ZAxisTickVisibility = 1;

  this->XAxisMinorTickVisibility = 1;
  this->YAxisMinorTickVisibility = 1;
  this->ZAxisMinorTickVisibility = 1;

  this->XAxisLabelVisibility = 1;
  this->YAxisLabelVisibility = 1;
  this->ZAxisLabelVisibility = 1;

  this->DrawXGridlines = 0;
  this->DrawYGridlines = 0;
  this->DrawZGridlines = 0;

  this->XTitle = new char[7];
  sprintf(this->XTitle,"%s","X-Axis");
  this->XUnits = NULL;
  this->YTitle = new char[7];
  sprintf(this->YTitle,"%s","Y-Axis");
  this->YUnits = NULL;
  this->ZTitle = new char[7];
  sprintf(this->ZTitle,"%s","Z-Axis");
  this->ZUnits = NULL;

  this->lastPow = 0;
  this->lastXAxisDigits = 3;
  this->lastYAxisDigits = 3;
  this->lastZAxisDigits = 3;

  this->LastXExtent = FLT_MAX;
  this->LastYExtent = FLT_MAX;
  this->LastZExtent = FLT_MAX;

  this->LastFlyMode = -1;
  for (i = 0; i < 4; i++)
  {
      this->renderAxesX[i] = i;
      this->renderAxesY[i] = i;
      this->renderAxesZ[i] = i;
  }
  this->numAxesX = this->numAxesY = this->numAxesZ = 1;
}

// Shallow copy of an actor.
//
// Modifications:
//   Kathleen Bonnell, Wed Mar  6 13:48:48 PST 2002
//   Call superclass method the new VTK 4.0 way.
//
void vtkKatCubeAxesActor::ShallowCopy(vtkKatCubeAxesActor *actor)
{
  this->Superclass::ShallowCopy(actor);
  this->SetXLabelFormat(actor->GetXLabelFormat());
  this->SetYLabelFormat(actor->GetYLabelFormat());
  this->SetZLabelFormat(actor->GetZLabelFormat());
  this->SetCornerOffset(actor->GetCornerOffset());
  this->SetInertia(actor->GetInertia());
  this->SetXTitle(actor->GetXTitle());
  this->SetYTitle(actor->GetYTitle());
  this->SetZTitle(actor->GetZTitle());
  this->SetFlyMode(actor->GetFlyMode());
  this->SetInput(actor->GetInput());
  this->SetProp(actor->GetProp());
  this->SetCamera(actor->GetCamera());
  this->SetBounds(actor->GetBounds());
}

// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Fri Sep 27 17:15:07 PDT 2002
//    Destruct new data members for units.
//
// ****************************************************************************

vtkKatCubeAxesActor::~vtkKatCubeAxesActor()
{
  this->SetInput(NULL);
  this->SetProp(NULL);
  this->SetCamera(NULL);

  for (int i = 0; i < 4; i++)
    {
    if (this->XAxes[i]) 
      {
      this->XAxes[i]->Delete();
      this->XAxes[i] = NULL;
      }
    if (this->YAxes[i]) 
      {
      this->YAxes[i]->Delete();
      this->YAxes[i] = NULL;
      }
    if (this->ZAxes[i]) 
      {
      this->ZAxes[i]->Delete();
      this->ZAxes[i] = NULL;
      }
    }
  
  if (this->XLabelFormat) 
    {
    delete [] this->XLabelFormat;
    this->XLabelFormat = NULL;
    }
  
  if (this->YLabelFormat) 
    {
    delete [] this->YLabelFormat;
    this->YLabelFormat = NULL;
    }
  
  if (this->ZLabelFormat) 
    {
    delete [] this->ZLabelFormat;
    this->ZLabelFormat = NULL;
    }
  
  if (this->XTitle)
    {
    delete [] this->XTitle;
    this->XTitle = NULL;
    }
  if (this->XUnits)
    {
    delete [] this->XUnits;
    this->XUnits = NULL;
    }
  if (this->YUnits)
    {
    delete [] this->YUnits;
    this->YUnits = NULL;
    }
  if (this->ZUnits)
    {
    delete [] this->ZUnits;
    this->ZUnits = NULL;
    }
}


// *************************************************************************
//
// Modifications:
//   Kathleen Bonnell, Wed Nov  7 16:19:16 PST 2001
//   Only render those axes needed for current FlyMode.
//
// *************************************************************************
int vtkKatCubeAxesActor::RenderOverlay(vtkViewport *viewport)
{
  int i, renderedSomething=0;

  // Initialization
  if (! this->RenderSomething)
    {
    return 0;
    }
 

  //Render the axes

  if (this->XAxisVisibility)
    {
    for (i = 0; i < this->numAxesX; i++)
      {
      renderedSomething += 
          this->XAxes[this->renderAxesX[i]]->RenderOverlay(viewport);
      }
    }
  if (this->YAxisVisibility)
    {
    for (i = 0; i < numAxesY; i++)
      {
      renderedSomething += 
          this->YAxes[this->renderAxesY[i]]->RenderOverlay(viewport);
      }
    }

  if (this->ZAxisVisibility)
    {
    for (i = 0; i < numAxesZ; i++)
      {
      renderedSomething += 
          this->ZAxes[this->renderAxesZ[i]]->RenderOverlay(viewport);
      }
    }

  return renderedSomething;
}

// *************************************************************************
// Project the bounding box and compute edges on the border of the bounding
// cube. Determine which parts of the edges are visible via intersection 
// with the boundary of the viewport (minus borders).
//
//  Modifications:
//    Kathleen Bonnell, Wed Oct 31 07:57:49 PST 2001
//    Added calls to AdjustValues, AdjustRange. 
//
//   Kathleen Bonnell, Wed Nov  7 16:19:16 PST 2001
//   Only render those axes needed for current FlyMode.  
//   Moved bulk of 'build' code to BuildAxes method, added calls to
//   BuildAxes and DetermineRenderAxes methods.
//
// *************************************************************************

int vtkKatCubeAxesActor::RenderOpaqueGeometry(vtkViewport *viewport)
{
  int i, renderedSomething=0;
  
  // Initialization
  if (!this->Camera)
    {
    vtkErrorMacro(<<"No camera!");
    this->RenderSomething = 0;
    return 0;
    }
 
  this->BuildAxes(viewport); 
  this->DetermineRenderAxes(viewport); 

  //Render the axes
  if (this->XAxisVisibility)
    {
    for (i = 0; i < this->numAxesX; i++)
      { 
      renderedSomething += 
          this->XAxes[this->renderAxesX[i]]->RenderOpaqueGeometry(viewport);
      } 
    }

  if (this->YAxisVisibility)
    {
    for (i = 0; i < this->numAxesY; i++)
      {
      renderedSomething += 
          this->YAxes[this->renderAxesY[i]]->RenderOpaqueGeometry(viewport);
      }
    }

  if (this->ZAxisVisibility)
    {
    for (i = 0; i < this->numAxesZ; i++)
      {
      renderedSomething += 
          this->ZAxes[this->renderAxesZ[i]]->RenderOpaqueGeometry(viewport);
      }
    }
  return renderedSomething;
}

// Do final adjustment of axes to control offset, etc.
void 
vtkKatCubeAxesActor::AdjustAxes(float bounds[6], float xCoords[4][6], 
                                float yCoords[4][6], float zCoords[4][6],
                                float xRange[2], float yRange[2], 
                                float zRange[2])
{
  xRange[0] = bounds[0];
  xRange[1] = bounds[1];
  
  yRange[0] = bounds[2];
  yRange[1] = bounds[3];
  
  zRange[0] = bounds[4];
  zRange[1] = bounds[5];
  
  // Pull back the corners if specified
  if (this->CornerOffset > 0.0)
   {
   for (int i = 0; i < 4; i++)
     {
     float ave;

     // x-axis
     ave = (xCoords[i][0] + xCoords[i][2]) / 2.0;
     xCoords[i][0] = xCoords[i][0] - this->CornerOffset * (xCoords[i][0] - ave);
     xCoords[i][2] = xCoords[i][2] - this->CornerOffset * (xCoords[i][2] - ave);
    
     ave = (xCoords[i][1] + xCoords[i][3]) / 2.0;
     xCoords[i][1] = xCoords[i][1] - this->CornerOffset * (xCoords[i][1] - ave);
     xCoords[i][3] = xCoords[i][3] - this->CornerOffset * (xCoords[i][3] - ave);

     ave = (xRange[1] + xRange[0]) / 2.0;
     xRange[0] = xRange[0] - this->CornerOffset * (xRange[0] - ave);
     xRange[1] = xRange[1] - this->CornerOffset * (xRange[1] - ave);
   
     // y-axis
     ave = (yCoords[i][0] + yCoords[i][2]) / 2.0;
     yCoords[i][0] = yCoords[i][0] - this->CornerOffset * (yCoords[i][0] - ave);
     yCoords[i][2] = yCoords[i][2] - this->CornerOffset * (yCoords[i][2] - ave);
    
     ave = (yCoords[i][1] + yCoords[i][3]) / 2.0;
     yCoords[i][1] = yCoords[i][1] - this->CornerOffset * (yCoords[i][1] - ave);
     yCoords[i][3] = yCoords[i][3] - this->CornerOffset * (yCoords[i][3] - ave);

     ave = (yRange[1] + yRange[0]) / 2.0;
     yRange[0] = yRange[0] - this->CornerOffset * (yRange[0] - ave);
     yRange[1] = yRange[1] - this->CornerOffset * (yRange[1] - ave);
    
     // z-axis
     ave = (zCoords[i][0] + zCoords[i][2]) / 2.0;
     zCoords[i][0] = zCoords[i][0] - this->CornerOffset * (zCoords[i][0] - ave);
     zCoords[i][2] = zCoords[i][2] - this->CornerOffset * (zCoords[i][2] - ave);
    
     ave = (zCoords[i][1] + zCoords[i][3]) / 2.0;
     zCoords[i][1] = zCoords[i][1] - this->CornerOffset * (zCoords[i][1] - ave);
     zCoords[i][3] = zCoords[i][3] - this->CornerOffset * (zCoords[i][3] - ave);

     ave = (zRange[1] + zRange[0]) / 2.0;
     zRange[0] = zRange[0] - this->CornerOffset * (zRange[0] - ave);
     zRange[1] = zRange[1] - this->CornerOffset * (zRange[1] - ave);
     }
   }
}

// Release any graphics resources that are being consumed by this actor.
// The parameter window could be used to determine which graphic
// resources to release.
void vtkKatCubeAxesActor::ReleaseGraphicsResources(vtkWindow *win)
{
  for (int i = 0; i < 4; i++)
    {
    this->XAxes[i]->ReleaseGraphicsResources(win);
    this->YAxes[i]->ReleaseGraphicsResources(win);
    this->ZAxes[i]->ReleaseGraphicsResources(win);
    }
}

// Compute the bounds
void vtkKatCubeAxesActor::GetBounds(float bounds[6])
{
  float *propBounds;
  int i;

  if (this->Input)
    {
    this->Input->Update();
    this->Input->GetBounds(bounds);
    for (i=0; i< 6; i++)
      {
      this->Bounds[i] = bounds[i];
      }
    }

  else if (this->Prop && 
  ((propBounds = this->Prop->GetBounds()) && propBounds != NULL) )
    {
    for (i=0; i< 6; i++)
      {
      bounds[i] = this->Bounds[i] = propBounds[i];
      }
    }
  else
    {
    for (i=0; i< 6; i++)
      {
      bounds[i] = this->Bounds[i];
      }
    }
}

// Compute the bounds
void vtkKatCubeAxesActor::GetBounds(float& xmin, float& xmax, 
                                   float& ymin, float& ymax,
                                   float& zmin, float& zmax)
{
  float bounds[6];
  this->GetBounds(bounds);
  xmin = bounds[0];
  xmax = bounds[1];
  ymin = bounds[2];
  ymax = bounds[3];
  zmin = bounds[4];
  zmax = bounds[5];
}

// Compute the bounds
float *vtkKatCubeAxesActor::GetBounds()
{
  float bounds[6];
  this->GetBounds(bounds);
  return this->Bounds;
}

// ******************************************************************
// Modifications:
//   Kathleen Bonnell, Wed Mar  6 13:48:48 PST 2002
//   Call superclass method the new VTK 4.0 way.
//
// ******************************************************************

void vtkKatCubeAxesActor::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  if (this->Input)
    {
    os << indent << "Input: (" << (void *)this->Input << ")\n";
    }
  else
    {
    os << indent << "Input: (none)\n";
    }

  if (this->Prop)
    {
    os << indent << "Prop: (" << (void *)this->Prop << ")\n";
    }
  else
    {
    os << indent << "Prop: (none)\n";
    }

  os << indent << "Bounds: \n";
  os << indent << "  Xmin,Xmax: (" << this->Bounds[0] << ", " 
     << this->Bounds[1] << ")\n";
  os << indent << "  Ymin,Ymax: (" << this->Bounds[2] << ", " 
     << this->Bounds[3] << ")\n";
  os << indent << "  Zmin,Zmax: (" << this->Bounds[4] << ", " 
     << this->Bounds[5] << ")\n";
  
  if (this->Camera)
    {
    os << indent << "Camera:\n";
    this->Camera->PrintSelf(os,indent.GetNextIndent());
    }
  else
    {
    os << indent << "Camera: (none)\n";
    }

  if (this->FlyMode == VTK_FLY_CLOSEST_TRIAD)
    {
    os << indent << "Fly Mode: CLOSEST_TRIAD\n";
    }
  else if (this->FlyMode == VTK_FLY_FURTHEST_TRIAD)
    {
    os << indent << "Fly Mode: FURTHEST_TRIAD\n";
    }
  else if (this->FlyMode == VTK_FLY_STATIC_TRIAD)
    {
    os << indent << "Fly Mode: STATIC_TRIAD\n";
    }
  else if (this->FlyMode == VTK_FLY_STATIC_EDGES)
    {
    os << indent << "Fly Mode: STATIC_EDGES\n";
    }
  else 
    {
    os << indent << "Fly Mode: OUTER_EDGES\n";
    }

  os << indent << "X Axis Title: " << this->XTitle << "\n";
  os << indent << "Y Axis Title: " << this->YTitle << "\n";
  os << indent << "Z Axis Title: " << this->ZTitle << "\n";
  
  os << indent << "X Axis Visibility: " 
     << (this->XAxisVisibility ? "On\n" : "Off\n");
  os << indent << "Y Axis Visibility: " 
     << (this->YAxisVisibility ? "On\n" : "Off\n");
  os << indent << "Z Axis Visibility: " 
     << (this->ZAxisVisibility ? "On\n" : "Off\n");

  os << indent << "X Axis Label Format: " << this->XLabelFormat << "\n";
  os << indent << "Y Axis Label Format: " << this->YLabelFormat << "\n";
  os << indent << "Z Axis Label Format: " << this->ZLabelFormat << "\n";
  os << indent << "Inertia: " << this->Inertia << "\n";
  os << indent << "Corner Offset: " << this->CornerOffset << "\n";
}


void vtkKatCubeAxesActor::TransformBounds(vtkViewport *viewport, 
                                          const float bounds[6], 
                                          float pts[8][3])
{
  int i, j, k, idx;
  float x[3];

  //loop over verts of bounding box
  for (k=0; k<2; k++)
    {
    x[2] = bounds[4+k];
    for (j=0; j<2; j++)
      {
      x[1] = bounds[2+j];
      for (i=0; i<2; i++)
        {
        idx = i + 2*j + 4*k;
        x[0] = bounds[i];
        viewport->SetWorldPoint(x[0],x[1],x[2],1.0);
        viewport->WorldToDisplay();
        viewport->GetDisplayPoint(pts[idx]);
        }
      }
    }
}

// ***********************************************************************
//
//  Calculate the size (length) of major and minor ticks,
//  based on an average of the coordinate direction ranges.
//  Set the necessary Axes methods with the calculated information.
//
//  Modifications:
//    Kathleen Bonnell, Wed Nov  7 16:19:16 PST 2001
//    Added logic for early-termination.
//
// ***********************************************************************
void vtkKatCubeAxesActor::ComputeTickSize(float bounds[6])
{
    int i;
    float xExt = bounds[1] - bounds[0];
    float yExt = bounds[3] - bounds[2];
    float zExt = bounds[5] - bounds[4];

    if (xExt == this->LastXExtent &&
        yExt == this->LastYExtent &&
        zExt == this->LastZExtent)
      {
          // no need to re-compute ticksize.
          return;
      }

    this->LastXExtent = xExt;
    this->LastYExtent = yExt;
    this->LastZExtent = zExt;

    float major = 0.02 * (xExt + yExt + zExt) / 3.;
    float minor = 0.5 * major;
    for (i = 0; i < 4; i++)
    {
        this->XAxes[i]->SetMajorTickSize(major);
        this->XAxes[i]->SetMinorTickSize(minor);

        this->YAxes[i]->SetMajorTickSize(major);
        this->YAxes[i]->SetMinorTickSize(minor);

        this->ZAxes[i]->SetMajorTickSize(major);
        this->ZAxes[i]->SetMinorTickSize(minor);

        this->XAxes[i]->SetGridlineXLength(xExt);
        this->XAxes[i]->SetGridlineYLength(yExt);
        this->XAxes[i]->SetGridlineZLength(zExt);

        this->YAxes[i]->SetGridlineXLength(xExt);
        this->YAxes[i]->SetGridlineYLength(yExt);
        this->YAxes[i]->SetGridlineZLength(zExt);

        this->ZAxes[i]->SetGridlineXLength(xExt);
        this->ZAxes[i]->SetGridlineYLength(yExt);
        this->ZAxes[i]->SetGridlineZLength(zExt);
    }
}

// ****************************************************************************
//  Method: vtkKatCubeAxesActor::AdjustValues
//
//  Purpose:
//      If the range of values is too big or too small, put them in scientific
//      notation and changes the labels.
//
//  Arguments:
//      bnds     The min/max values in each coordinate direction:
//                 (min_x, max_x, min_y, max_y, min_z, max_x).
//
//  Note:       This code is partially stolen from old MeshTV code,
//              /meshtvx/toolkit/plotgrid.c, axlab[x|y].
//
//  Programmer: Hank Childs
//  Creation:   July 11, 2000
//
//  Modifications:
//    Kathleen Bonnell, Wed Oct 31 07:57:49 PST 2001 
//    Regardless of individual ranges, if any coord direction has too 
//    small/large a range, all will have a scale factor set for scaling their 
//    label values, and their titles adjusted accordingly. 
//
//    Kathleen Bonnell, Thu Sep  5 17:32:16 PDT 2002 
//    Only use dimensions with range > 0 for determining scale factor. 
//    
//    Hank Childs, Fri Sep 27 17:15:07 PDT 2002
//    Account for units.
//
// ****************************************************************************

void
vtkKatCubeAxesActor::AdjustValues(const float bnds[6])
{
    int largestPow = 0;

    if (bnds[0] != bnds[1])
    {
        int xPow = LabelExponent(bnds[0], bnds[1]);
        largestPow = (fabs(float(xPow)) > fabs(float(largestPow)) ? xPow : largestPow);
    }

    if (bnds[2] != bnds[3])
    {
        int yPow = LabelExponent(bnds[2], bnds[3]);
        largestPow = (fabs(float(yPow)) > fabs(float(largestPow)) ? yPow : largestPow);
    }

    if (bnds[4] != bnds[5])
    {
        int zPow = LabelExponent(bnds[4], bnds[5]);
        largestPow = (fabs(float(zPow)) > fabs(float(largestPow)) ? zPow : largestPow);
    }

    if (largestPow != 0)
    { 
       this->SetValueScaleFactor(1.0 / pow(10.f, largestPow));
    }
    else 
    { 
       this->UnSetValueScaleFactor();
    }
  
    char xTitle[256];
    char yTitle[256];
    char zTitle[256];
    if (largestPow == 0)
    {
        if (XUnits == NULL || XUnits[0] == '\0')
            sprintf(xTitle, "X-Axis");
        else
            sprintf(xTitle, "X-Axis (%s)", XUnits);
        if (YUnits == NULL || YUnits[0] == '\0')
            sprintf(yTitle, "Y-Axis");
        else
            sprintf(yTitle, "Y-Axis (%s)", YUnits);
        if (ZUnits == NULL || ZUnits[0] == '\0')
            sprintf(zTitle, "Z-Axis");
        else
            sprintf(zTitle, "Z-Axis (%s)", ZUnits);
    }
    else
    {
        if (XUnits == NULL || XUnits[0] == '\0')
            sprintf(xTitle, "X-Axis (e%d)", largestPow);
        else
            sprintf(xTitle, "X-Axis (e%d %s)", largestPow, XUnits);
        if (YUnits == NULL || YUnits[0] == '\0')
            sprintf(yTitle, "Y-Axis (e%d)", largestPow);
        else
            sprintf(yTitle, "Y-Axis (e%d %s)", largestPow, YUnits);
        if (ZUnits == NULL || ZUnits[0] == '\0')
            sprintf(zTitle, "Z-Axis (e%d)", largestPow);
        else
            sprintf(zTitle, "Z-Axis (e%d %s)", largestPow, ZUnits);
    }
    this->lastPow = largestPow;
    SetXTitle(xTitle);
    SetYTitle(yTitle);
    SetZTitle(zTitle);
}


// ****************************************************************************
//  Method: vtkKatCubeAxesActor::AdjustRange
//
//  Purpose:
//    If the range is small, adjust the precision of the values displayed.
//
//  Arguments:
//    bnds    The minimum and maximum values in each coordinate direction
//            (min_x, max_x, min_y, max_y, min_z, max_z).
//
//  Programmer: Hank Childs
//  Creation:   July 11, 2000
//
//  Modifications:
//    Kathleen Bonnell, Wed Oct 31 07:57:49 PST 2001 
//    Moved from VisWinAxes3D.
//
//    Kathleen Bonnell, Thu Aug  1 14:05:05 PDT 2002 
//    Send lastPos as argument to Digits. 
//
// ****************************************************************************

void
vtkKatCubeAxesActor::AdjustRange(const float bnds[6])
{
    int xAxisDigits = Digits(bnds[0], bnds[1], this->lastPow);
    if (xAxisDigits != this->lastXAxisDigits)
    {
        char  format[16];
        sprintf(format, "%%.%df", xAxisDigits);
        this->SetXLabelFormat(format);
        this->lastXAxisDigits = xAxisDigits;
    }

    int yAxisDigits = Digits(bnds[2], bnds[3], this->lastPow);
    if (yAxisDigits != this->lastYAxisDigits)
    {
        char  format[16];
        sprintf(format, "%%.%df", yAxisDigits);
        this->SetYLabelFormat(format);
        this->lastYAxisDigits = yAxisDigits;
    }

    int zAxisDigits = Digits(bnds[4], bnds[5], this->lastPow);
    if (zAxisDigits != this->lastZAxisDigits)
    {
        char  format[16];
        sprintf(format, "%%.%df", zAxisDigits);
        this->SetZLabelFormat(format);
        this->lastZAxisDigits = zAxisDigits;
    }
}


// ****************************************************************************
//  Function: Digits
//
//  Purpose:
//      Determines the appropriate number of digits for a given range.
//
//  Arguments:
//      min    The minimum value in the range.
//      max    The maximum value in the range.
//
//  Returns:   The appropriate number of digits.
//
//  Programmer: Hank Childs
//  Creation:   July 11, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Sep 18 11:58:33 PDT 2001
//    Cast ipow10 to get rid of compiler warning.
//
//    Kathleen Bonnell, Wed Oct 31 07:57:49 PST 2001 
//    Moved from VisWinAxes3D.
//
//    Kathleen Bonnell, Thu Aug  1 13:44:02 PDT 2002 
//    Added lastPow argument, it specifies whether or not scientific notation
//    is being used on the labels.
// ****************************************************************************

int
Digits(float min, float max, int lastPow)
{
    float  range = max - min;
    float  pow10   = log10(range);
    int    ipow10  = (int)floor(pow10);

    int    digitsPastDecimal = -ipow10;

    if (digitsPastDecimal < 0 && lastPow == 0)
    {
        //
        // The range is more than 10, but not so big we need scientific
        // notation, we don't need to worry about decimals.
        //
        digitsPastDecimal = 0;
    }
    else
    {
        //
        // We want one more than the range since there is more than one
        // tick per decade.  If in scientific notation, may be negative.
        //
        digitsPastDecimal = abs(digitsPastDecimal) + 1;

        //
        // Anything more than 5 is just noise.  (and probably 5 is noise with
        // floating point is the part before the decimal is big).
        //
        if (digitsPastDecimal > 5)
        {
            digitsPastDecimal = 5;
        }
    }
 
    return digitsPastDecimal;
}


// ****************************************************************************
//  Function: LabelExponent
//
//  Purpose:
//      Determines the proper exponent for the min and max values.
//
//  Arguments:
//      min     The minimum value along a certain axis.
//      max     The maximum value along a certain axis.
//
//  Note:       This code is mostly stolen from old MeshTV code,
//              /meshtvx/toolkit/plotgrid.c, axlab_format.
//
//  Programmer: Hank Childs
//  Creation:   July 11, 2000
//
//  Modifications:
//    Eric Brugger, Tue Sep 18 09:18:17 PDT 2001
//    Change a few static local variables to be non-static to get around a
//    compiler bug with the MIPSpro 7.2.1.3 compiler.
//
//    Hank Childs, Tue Sep 18 11:58:33 PDT 2001
//    Cast return value to get rid of compiler warning.
//
//    Kathleen Bonnell, Wed Oct 31 07:57:49 PST 2001 
//    Moved from VisWinAxes3D.
//
// ****************************************************************************

int
LabelExponent(float min, float max)
{
    //
    // Determine power of 10 to scale axis labels to.
    //
    float range = (fabs(min) > fabs(max) ? fabs(min) : fabs(max));
    float pow10 = log10(range);

    //
    // Cutoffs for using scientific notation.  The following 4 variables
    // should all be static for maximum performance but were made non-static
    // to get around a compiler bug with the MIPSpro 7.2.1.3 compiler.
    //
    float eformat_cut_min = -1.5;
    float eformat_cut_max =  3.0;
    float cut_min = pow(10.f, eformat_cut_min);
    float cut_max = pow(10.f, eformat_cut_max);
    float ipow10;
    if (range < cut_min || range > cut_max)
    {
        //
        // We are going to use scientific notation and round the exponents to
        // the nearest multiple of three.
        //
        ipow10 = (floor(floor(pow10)/3.))*3;
    }
    else
    {
        ipow10 = 0;
    }

    return (int)ipow10;
}    



// ****************************************************************************
//  Method: VisWinAxes3D::SetValueScaleFactor
//
//  Purpose:
//    For each axis, sets a scale factor by which label values are scaled.
//
//  Arguments:
//    scale     The scale factor.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 30, 2001 
//
// ****************************************************************************

void
vtkKatCubeAxesActor::SetValueScaleFactor(float scale)
{
    for (int i = 0; i < 4; i++)
    {
        this->XAxes[i]->SetValueScaleFactor(scale);
        this->YAxes[i]->SetValueScaleFactor(scale);
        this->ZAxes[i]->SetValueScaleFactor(scale);
    }
}


// ****************************************************************************
//  Method: VisWinAxes3D::UnSetValueScaleFactor
//
//  Purpose:
//    For each axis, unsets the scale factor by which label values are scaled.
//
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 9, 2002 
//
// ****************************************************************************

void
vtkKatCubeAxesActor::UnSetValueScaleFactor()
{
    for (int i = 0; i < 4; i++)
    {
        this->XAxes[i]->UnSetValueScaleFactor();
        this->YAxes[i]->UnSetValueScaleFactor();
        this->ZAxes[i]->UnSetValueScaleFactor();
    }
}

// *************************************************************************
//  Build the axes. Determine coordinates, position, etc. 
//
//  Note:  Bulk of code moved here from RenderOpaqueGeomtry.  
//         Early-termination test added.
//
//  Programmer:  Kathleen Bonnell
//  Creation:    November 7, 2001
//
//  Modifications:
//    Kathleen Bonnell, Mon Dec  3 16:49:01 PST 2001
//    Compare vtkTimeStamps correctly.
// *************************************************************************

void vtkKatCubeAxesActor::BuildAxes(vtkViewport *viewport)
{
  float bounds[6]; 
  float pts[8][3];
  int i; 

  if ((this->GetMTime() < this->BuildTime.GetMTime())) 
    {
    return;
    }

  this->SetNonDependentAttributes();
  // determine the bounds to use (input, prop, or user-defined)
  this->GetBounds(bounds);


  // Build the axes (almost always needed so we don't check mtime)
  // Transform all points into display coordinates (to determine which closest
  // to camera).

  this->TransformBounds(viewport, bounds, pts);
  this->ComputeTickSize(bounds);

  // Setup the axes for plotting
  float xCoords[4][6], yCoords[4][6], zCoords[4][6];

  // these arrays are accessed by 'location':  mm, mX, XX, or Xm.
  int mm1[4] = { 0, 0, 1, 1 };
  int mm2[4] = { 0, 1, 1, 0 };

  for (i = 0; i < 4; i++)
    {
    this->XAxes[i]->SetAxisPosition(i);
    xCoords[i][0] = bounds[0];
    xCoords[i][3] = bounds[1];
    xCoords[i][1] = xCoords[i][4] = bounds[2+mm1[i]];
    xCoords[i][2] = xCoords[i][5] = bounds[4+mm2[i]];

    this->YAxes[i]->SetAxisPosition(i);
    yCoords[i][0] = yCoords[i][3] = bounds[0+mm1[i]];
    yCoords[i][1] = bounds[2];
    yCoords[i][4] = bounds[3];
    yCoords[i][2] = yCoords[i][5] = bounds[4+mm2[i]];

    this->ZAxes[i]->SetAxisPosition(i);
    zCoords[i][0] = zCoords[i][3] = bounds[0+mm1[i]];
    zCoords[i][1] = zCoords[i][4] = bounds[2+mm2[i]];
    zCoords[i][2] = bounds[4];
    zCoords[i][5] = bounds[5];
    }

  float xRange[2], yRange[2], zRange[2];

  // this method sets the Coords, and offsets if necessary.
  this->AdjustAxes(bounds, xCoords, yCoords, zCoords, xRange, yRange, zRange);

  // adjust for sci. notation if necessary 
  // May set a flag for each axis specifying that label values should
  // be scaled, may change title of each axis, may change label format.
  this->AdjustValues(this->Bounds);
  this->AdjustRange(this->Bounds);

  // Prepare axes for rendering with user-definable options 
  for (i = 0; i < 4; i++)
    {
    this->XAxes[i]->GetPoint1Coordinate()->SetValue(xCoords[i][0], 
                                                    xCoords[i][1],
                                                    xCoords[i][2]);
    this->XAxes[i]->GetPoint2Coordinate()->SetValue(xCoords[i][3], 
                                                    xCoords[i][4],
                                                    xCoords[i][5]);
    this->YAxes[i]->GetPoint1Coordinate()->SetValue(yCoords[i][0], 
                                                    yCoords[i][1],
                                                    yCoords[i][2]);
    this->YAxes[i]->GetPoint2Coordinate()->SetValue(yCoords[i][3], 
                                                    yCoords[i][4],
                                                    yCoords[i][5]);
    this->ZAxes[i]->GetPoint1Coordinate()->SetValue(zCoords[i][0], 
                                                    zCoords[i][1],
                                                    zCoords[i][2]);
    this->ZAxes[i]->GetPoint2Coordinate()->SetValue(zCoords[i][3], 
                                                    zCoords[i][4],
                                                    zCoords[i][5]);


    this->XAxes[i]->SetRange(xRange[0], xRange[1]);
    this->YAxes[i]->SetRange(yRange[0], yRange[1]);
    this->ZAxes[i]->SetRange(zRange[0], zRange[1]);

    this->XAxes[i]->SetTitle(this->XTitle);
    this->YAxes[i]->SetTitle(this->YTitle);
    this->ZAxes[i]->SetTitle(this->ZTitle);

    this->XAxes[i]->SetLabelFormat(this->XLabelFormat);
    this->YAxes[i]->SetLabelFormat(this->YLabelFormat);
    this->ZAxes[i]->SetLabelFormat(this->ZLabelFormat);
    }

  //
  // Ticks on non-major axes are not drawn unless fly-mode is STATIC,
  // however, user may want to see gridlines in another fly-mode.  If
  // that is the case, turn on tick visibility for non-major axes.
  //
  for (i = 0; i < 4; i++)
    {
    this->XAxes[i]->BuildAxis(viewport);
    this->YAxes[i]->BuildAxis(viewport);
    this->ZAxes[i]->BuildAxis(viewport);
    }
  this->RenderSomething = 1;
  this->BuildTime.Modified();
  this->LastFlyMode = this->FlyMode;
}


// *************************************************************************
//  Sends attributes to each vtkKatAxisActor.  Only sets those that are 
//  not dependent upon viewport changes, and thus do not need to be set 
//  very often.
//
//  Programmer:  Kathleen Bonnell
//  Creation:    November 7, 2001
//
//  Modifications:
//    Kathleen Bonnell, Thu Oct  3 14:33:15 PDT 2002
//    Disable lighting for the axes by setting the ambient coefficient to 1
//    and the diffuse coeeficient to 0.
// *************************************************************************

void vtkKatCubeAxesActor::SetNonDependentAttributes()
{
  vtkProperty *prop = this->GetProperty();
  prop->SetAmbient(1.0);
  prop->SetDiffuse(0.0);
  for (int i = 0; i < 4; i++)
    {
    this->XAxes[i]->SetCamera(this->Camera);
    this->XAxes[i]->SetProperty(prop);
    this->XAxes[i]->SetTickLocation(this->TickLocation);
    this->XAxes[i]->SetDrawGridlines(this->DrawXGridlines);
    this->XAxes[i]->SetBounds(this->Bounds);
    this->XAxes[i]->AxisVisibilityOn();
    this->XAxes[i]->SetLabelVisibility(this->XAxisLabelVisibility);
    this->XAxes[i]->SetTitleVisibility(this->XAxisLabelVisibility);
    this->XAxes[i]->SetTickVisibility(this->XAxisTickVisibility);
    this->XAxes[i]->SetMinorTicksVisible(this->XAxisMinorTickVisibility);

    this->YAxes[i]->SetCamera(this->Camera);
    this->YAxes[i]->SetProperty(prop);
    this->YAxes[i]->SetTickLocation(this->TickLocation);
    this->YAxes[i]->SetDrawGridlines(this->DrawYGridlines);
    this->YAxes[i]->SetBounds(this->Bounds);
    this->YAxes[i]->AxisVisibilityOn();
    this->YAxes[i]->SetLabelVisibility(this->YAxisLabelVisibility);
    this->YAxes[i]->SetTitleVisibility(this->YAxisLabelVisibility);
    this->YAxes[i]->SetTickVisibility(this->YAxisTickVisibility);
    this->YAxes[i]->SetMinorTicksVisible(this->YAxisMinorTickVisibility);

    this->ZAxes[i]->SetCamera(this->Camera);
    this->ZAxes[i]->SetProperty(prop);
    this->ZAxes[i]->SetTickLocation(this->TickLocation);
    this->ZAxes[i]->SetDrawGridlines(this->DrawZGridlines);
    this->ZAxes[i]->SetBounds(this->Bounds);
    this->ZAxes[i]->AxisVisibilityOn();
    this->ZAxes[i]->SetLabelVisibility(this->ZAxisLabelVisibility);
    this->ZAxes[i]->SetTitleVisibility(this->ZAxisLabelVisibility);
    this->ZAxes[i]->SetTickVisibility(this->ZAxisTickVisibility);
    this->ZAxes[i]->SetMinorTicksVisible(this->ZAxisMinorTickVisibility);
    }
}

// Static variable describes locations in cube, relative to the type
// of axis:  mm for an X-axis means the x-edge at min-y and min-z.
// mX for a Y-axis means the y-edge at min-x and max-z, and so on.

enum {mm = 0, mX, XX, Xm };
//
// For CLOSEST_TRIAD, and FURTHEST_TRIAD, this variable determines 
// which locations in the cube each 'Major' axis should take.
//
static int Triads[8][3] = {{mm,mm,mm}, {mm,Xm,Xm}, {Xm,mm,mX}, {Xm,Xm,XX},
                           {mX,mX,mm}, {mX,XX,Xm}, {XX,mX,mX}, {XX,XX,XX}};
static int Conn[8][3] = {{1,2,4}, {0,3,5}, {3,0,6}, {2,1,7},
                         {5,6,0}, {4,7,1}, {7,4,2}, {6,5,3}};

// *************************************************************************
// Determine which of the axes in each coordinate direction actually should
// be rendered.  For STATIC FlyMode, all axes are rendered.  For other
// FlyModes, either 1 or 2 per coordinate direction are rendered.
//
// Programmer:  Kathleen Bonnell
// Creation:    November 7, 2001
// 
// Modifications:
//   Kathleen Bonnell, Thu Jul 18 10:33:07 PDT 2002  
//   Ensure that primary axes visibility flags are set properly, and
//   that secondary axes visibility flags are turned off.
// *************************************************************************
void vtkKatCubeAxesActor::DetermineRenderAxes(vtkViewport *viewport)
{
  float bounds[6], slope = 0.0, minSlope, num, den;
  float pts[8][3], d2, d2Min, min, max;
  int i, idx = 0;
  int xIdx, yIdx = 0, zIdx = 0, zIdx2;
  int xAxes = 0, yAxes, zAxes, xloc, yloc, zloc;
 
  if (this->FlyMode == VTK_FLY_STATIC_EDGES) 
    {
    for (i = 0; i < 4; i++)
      {
      this->renderAxesX[i] = i; 
      this->renderAxesY[i] = i; 
      this->renderAxesZ[i] = i; 
      }
    this->numAxesX = this->numAxesY = this->numAxesZ = 4;
    return;
    }
  if (this->FlyMode == VTK_FLY_STATIC_TRIAD) 
    {
    this->renderAxesX[0] = 0; 
    this->renderAxesY[0] = 0; 
    this->renderAxesZ[0] = 0; 
    if (this->DrawXGridlines)
      {
      this->renderAxesX[1] = 2; 
      this->numAxesX = 2; 
      this->XAxes[renderAxesX[1]]->SetTickVisibility(0);
      this->XAxes[renderAxesX[1]]->SetLabelVisibility(0);
      this->XAxes[renderAxesX[1]]->SetTitleVisibility(0);
      this->XAxes[renderAxesX[1]]->SetMinorTicksVisible(0);
      }
    else 
      {
      this->numAxesX = 1; 
      }
    if (this->DrawYGridlines)
      {
      this->renderAxesY[1] = 2; 
      this->numAxesY = 2; 
      this->YAxes[renderAxesY[1]]->SetTickVisibility(0);
      this->YAxes[renderAxesY[1]]->SetLabelVisibility(0);
      this->YAxes[renderAxesY[1]]->SetTitleVisibility(0);
      this->YAxes[renderAxesY[1]]->SetMinorTicksVisible(0);
      }
    else 
      {
      this->numAxesY = 1; 
      }
    if (this->DrawZGridlines)
      {
      this->renderAxesZ[1] = 2; 
      this->numAxesZ = 2; 
      this->ZAxes[renderAxesZ[1]]->SetTickVisibility(0);
      this->ZAxes[renderAxesZ[1]]->SetLabelVisibility(0);
      this->ZAxes[renderAxesZ[1]]->SetTitleVisibility(0);
      this->ZAxes[renderAxesZ[1]]->SetMinorTicksVisible(0);
      }
    else 
      {
      this->numAxesZ = 1; 
      }
    return;
    }

  // determine the bounds to use (input, prop, or user-defined)
  this->GetBounds(bounds);

  this->TransformBounds(viewport, bounds, pts);

  // Take into account the inertia. Process only so often.
  if (this->RenderCount++ == 0 || !(this->RenderCount % this->Inertia))
    {
    if (this->FlyMode == VTK_FLY_CLOSEST_TRIAD)
      {
      // Loop over points and find the closest point to the camera
      min = VTK_LARGE_FLOAT;
      for (i=0; i < 8; i++)
        {
        if (pts[i][2] < min)
          {
          idx = i;
          min = pts[i][2];
          }
        }
      xloc = Triads[idx][0];
      yloc = Triads[idx][1];
      zloc = Triads[idx][2];

      } // closest-triad
    else if (this->FlyMode == VTK_FLY_FURTHEST_TRIAD)
      {
      // Loop over points and find the furthest point from the camera
      max = -VTK_LARGE_FLOAT;
      for (i=0; i < 8; i++)
        {
        if (pts[i][2] > max)
          {
          idx = i;
          max = pts[i][2];
          }
        }
      xloc = Triads[idx][0];
      yloc = Triads[idx][1];
      zloc = Triads[idx][2];

      } // furthest-triad
    else
      {
      float e1[2], e2[2], e3[2];

      // Find distance to origin
      d2Min = VTK_LARGE_FLOAT;
      for (i=0; i < 8; i++)
        {
        d2 = pts[i][0]*pts[i][0] + pts[i][1]*pts[i][1];
        if (d2 < d2Min)
          {
          d2Min = d2;
          idx = i;
          }
        }

      // find minimum slope point connected to closest point and on 
      // right side (in projected coordinates). This is the first edge.
      minSlope = VTK_LARGE_FLOAT;
      for (xIdx=0, i=0; i<3; i++)
        {
        num = (pts[Conn[idx][i]][1] - pts[idx][1]);
        den = (pts[Conn[idx][i]][0] - pts[idx][0]);
        if (den != 0.0)
          {
          slope = num / den;
          }
        if (slope < minSlope && den > 0)
          {
          xIdx = Conn[idx][i];
          yIdx = Conn[idx][(i+1)%3];
          zIdx = Conn[idx][(i+2)%3];
          xAxes = i;
          minSlope = slope;
          }
        }

      // find edge (connected to closest point) on opposite side
      for ( i=0; i<2; i++)
        {
        e1[i] = (pts[xIdx][i] - pts[idx][i]);
        e2[i] = (pts[yIdx][i] - pts[idx][i]);
        e3[i] = (pts[zIdx][i] - pts[idx][i]);
        }
      vtkMath::Normalize(e1);
      vtkMath::Normalize(e2);
      vtkMath::Normalize(e3);

      if (vtkMath::Dot(e1,e2) < vtkMath::Dot(e1,e3))
        {
        yAxes = (xAxes + 1) % 3;
        }
      else
        {
        yIdx = zIdx;
        yAxes = (xAxes + 2) % 3;
        }

      // Find the final point by determining which global x-y-z axes have not 
      // been represented, and then determine the point closest to the viewer.
      zAxes = (xAxes != 0 && yAxes != 0 ? 0 :
              (xAxes != 1 && yAxes != 1 ? 1 : 2));
      if (pts[Conn[xIdx][zAxes]][2] < pts[Conn[yIdx][zAxes]][2])
        {
        zIdx = xIdx;
        zIdx2 = Conn[xIdx][zAxes];
        }
      else
        {
        zIdx = yIdx;
        zIdx2 = Conn[yIdx][zAxes];
        }

      int mini = (idx < xIdx ? idx : xIdx);
      switch (xAxes)
        {
        case 0 : xloc = Triads[mini][0]; break;
        case 1 : yloc = Triads[mini][1]; break;
        case 2 : zloc = Triads[mini][2]; break;
        }
      mini = (idx < yIdx ? idx : yIdx);
      switch (yAxes)
        {
        case 0 : xloc = Triads[mini][0]; break;
        case 1 : yloc = Triads[mini][1]; break;
        case 2 : zloc = Triads[mini][2]; break;
        }
      mini = (zIdx < zIdx2 ? zIdx : zIdx2);
      switch (zAxes)
        {
        case 0 : xloc = Triads[mini][0]; break;
        case 1 : yloc = Triads[mini][1]; break;
        case 2 : zloc = Triads[mini][2]; break;
        }

      }//else boundary edges fly mode

    this->InertiaLocs[0] = xloc;
    this->InertiaLocs[1] = yloc;
    this->InertiaLocs[2] = zloc;
    } //inertia
  else
    {
    // don't change anything, use locations from last render
    xloc = this->InertiaLocs[0];
    yloc = this->InertiaLocs[1];
    zloc = this->InertiaLocs[2];
    }
  
 
  this->renderAxesX[0] = xloc % 4;
  if (this->DrawXGridlines)
    { 
    this->renderAxesX[1] = (xloc + 2) % 4;
    this->numAxesX = 2;
    this->XAxes[renderAxesX[1]]->SetTickVisibility(0);
    this->XAxes[renderAxesX[1]]->SetLabelVisibility(0);
    this->XAxes[renderAxesX[1]]->SetTitleVisibility(0);
    this->XAxes[renderAxesX[1]]->SetMinorTicksVisible(0);
    } 
  else
    { 
    this->numAxesX = 1;
    } 

  this->renderAxesY[0] = yloc % 4;
  if (this->DrawYGridlines)
    { 
    this->renderAxesY[1] = (yloc + 2) % 4;
    this->numAxesY = 2;
    this->YAxes[renderAxesY[1]]->SetTickVisibility(0);
    this->YAxes[renderAxesY[1]]->SetLabelVisibility(0);
    this->YAxes[renderAxesY[1]]->SetTitleVisibility(0);
    this->YAxes[renderAxesY[1]]->SetMinorTicksVisible(0);
    } 
  else
    { 
    this->numAxesY = 1;
    } 

  this->renderAxesZ[0] = zloc % 4;
  if (this->DrawZGridlines)
    { 
    this->renderAxesZ[1] = (zloc + 2) % 4;
    this->numAxesZ = 2;
    this->ZAxes[renderAxesZ[1]]->SetTickVisibility(0);
    this->ZAxes[renderAxesZ[1]]->SetLabelVisibility(0);
    this->ZAxes[renderAxesZ[1]]->SetTitleVisibility(0);
    this->ZAxes[renderAxesZ[1]]->SetMinorTicksVisible(0);
    } 
  else
    { 
    this->numAxesZ = 1;
    } 
  //
  //  Make sure that the primary axis visibility flags are set correctly.
  //
  this->XAxes[renderAxesX[0]]->SetLabelVisibility(this->XAxisLabelVisibility);
  this->XAxes[renderAxesX[0]]->SetTitleVisibility(this->XAxisLabelVisibility);
  this->XAxes[renderAxesX[0]]->SetTickVisibility(this->XAxisTickVisibility);
  this->XAxes[renderAxesX[0]]->SetMinorTicksVisible(this->XAxisMinorTickVisibility);

  this->YAxes[renderAxesY[0]]->SetLabelVisibility(this->YAxisLabelVisibility);
  this->YAxes[renderAxesY[0]]->SetTitleVisibility(this->YAxisLabelVisibility);
  this->YAxes[renderAxesY[0]]->SetTickVisibility(this->YAxisTickVisibility);
  this->YAxes[renderAxesY[0]]->SetMinorTicksVisible(this->YAxisMinorTickVisibility);

  this->ZAxes[renderAxesZ[0]]->SetLabelVisibility(this->ZAxisLabelVisibility);
  this->ZAxes[renderAxesZ[0]]->SetTitleVisibility(this->ZAxisLabelVisibility);
  this->ZAxes[renderAxesZ[0]]->SetTickVisibility(this->ZAxisTickVisibility);
  this->ZAxes[renderAxesZ[0]]->SetMinorTicksVisible(this->ZAxisMinorTickVisibility);

}


