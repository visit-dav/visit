/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkKatCubeAxesActor.h,v $
  Language:  C++
  Date:      $Date: 2001/08/03 20:08:22 $
  Version:   $Revision: 1.22 $
  Thanks:    Kathleen Bonnell, B Division, Lawrence Livermore Nat'l Laboratory 

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
// .NAME vtkKatCubeAxesActor - create a  plot of a bounding box edges - 
// used for navigation
// .SECTION Description
// vtkKatCubeAxesActor is a composite actor that draws axes of the 
// bounding box of an input dataset. The axes include labels and titles
// for the x-y-z axes. The algorithm selects which axes to draw based
// on the user-defined 'fly' mode.  (STATIC is default).
// 'STATIC' constructs axes from all edges of the bounding box.
// 'CLOSEST_TRIAD' consists of the three axes x-y-z forming a triad that 
// lies closest to the specified camera. 
// 'FURTHEST_TRIAD' consists of the three axes x-y-z forming a triad that 
// lies furthest from the specified camera. 
// 'OUTER_EDGES' is constructed from edges that are on the "exterior" of the 
// bounding box, exterior as determined from examining outer edges of the 
// bounding box in projection (display) space. 
// 
// To use this object you must define a bounding box and the camera used
// to render the vtkKatCubeAxesActor. You can optionally turn on/off labels,
// ticks, gridlines, and set tick location, number of labels, and text to
// use for axis-titles.  A 'corner offset' can also be set.  This allows 
// the axes to be set partially away from the actual bounding box to perhaps
// prevent overlap of labels between the various axes.
//
// The bounding box to use is defined in one of three ways. First, if the Input
// ivar is defined, then the input dataset's bounds is used. If the Input is 
// not defined, and the Prop (superclass of all actors) is defined, then the
// Prop's bounds is used. If neither the Input or Prop is defined, then the
// Bounds instance variable (an array of six floats) is used.
// 
// .SECTION See Also
// vtkActor vtkAxisActor vtkCubeAxesActor2D

#ifndef __vtkKatCubeAxesActor_h
#define __vtkKatCubeAxesActor_h
#include <visit_vtk_exports.h>

#include "vtkKatAxisActor.h"
#include "vtkCamera.h"

#define VTK_FLY_OUTER_EDGES     0
#define VTK_FLY_CLOSEST_TRIAD   1
#define VTK_FLY_FURTHEST_TRIAD  2
#define VTK_FLY_STATIC_TRIAD    3
#define VTK_FLY_STATIC_EDGES    4

#define VTK_TICKS_INSIDE        0
#define VTK_TICKS_OUTSIDE       1
#define VTK_TICKS_BOTH          2

class VISIT_VTK_API vtkKatCubeAxesActor : public vtkActor
{
public:
  vtkTypeMacro(vtkKatCubeAxesActor,vtkActor);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Instantiate object with label format "6.3g" and the number of labels 
  // per axis set to 3.
  static vtkKatCubeAxesActor *New();
  
  // Description:
  // Draw the axes as per the vtkProp superclass' API.
  int RenderOverlay(vtkViewport*);
  int RenderOpaqueGeometry(vtkViewport*);
  int RenderTranslucentGeometry(vtkViewport *) {return 0;}

  // Description:
  // Use the bounding box of this input dataset to draw the cube axes. If this
  // is not specified, then the class will attempt to determine the bounds from
  // the defined Prop or Bounds.
  virtual void SetInput(vtkDataSet*);
  vtkGetObjectMacro(Input, vtkDataSet);

  // Description:
  // Use the bounding box of this prop to draw the cube axes. The Prop is used 
  // to determine the bounds only if the Input is not defined.
  virtual void SetProp(vtkProp*);
  vtkGetObjectMacro(Prop, vtkProp);
  
  // Description:
  // Explicitly specify the region in space around which to draw the bounds.
  // The bounds is used only when no Input or Prop is specified. The bounds
  // are specified according to (xmin,xmax, ymin,ymax, zmin,zmax), making
  // sure that the min's are less than the max's.
  vtkSetVector6Macro(Bounds,float);
  float *GetBounds();
  void GetBounds(float& xmin, float& xmax, float& ymin, float& ymax, 
                 float& zmin, float& zmax);
  void GetBounds(float bounds[6]);

  // Description:
  // Set/Get the camera to perform scaling and translation of the 
  // vtkKatCubeAxesActor.
  virtual void SetCamera(vtkCamera*);
  vtkGetObjectMacro(Camera,vtkCamera);

  // Description:
  // Specify a mode to control how the axes are drawn: either static, 
  // closest triad, furthest triad or outer edges in relation to the 
  // camera position.
  vtkSetClampMacro(FlyMode, int, VTK_FLY_OUTER_EDGES, VTK_FLY_STATIC_EDGES);
  vtkGetMacro(FlyMode, int);
  void SetFlyModeToOuterEdges()
    {this->SetFlyMode(VTK_FLY_OUTER_EDGES);};
  void SetFlyModeToClosestTriad()
    {this->SetFlyMode(VTK_FLY_CLOSEST_TRIAD);};
  void SetFlyModeToFurthestTriad()
    {this->SetFlyMode(VTK_FLY_FURTHEST_TRIAD);};
  void SetFlyModeToStaticTriad()
    {this->SetFlyMode(VTK_FLY_STATIC_TRIAD);};
  void SetFlyModeToStaticEdges()
    {this->SetFlyMode(VTK_FLY_STATIC_EDGES);};

  // Description:
  // Set/Get the labels for the x, y, and z axes. By default, 
  // use "X-Axis", "Y-Axis" and "Z-Axis".
  vtkSetStringMacro(XTitle);
  vtkGetStringMacro(XTitle);
  vtkSetStringMacro(XUnits);
  vtkGetStringMacro(XUnits);
  vtkSetStringMacro(YTitle);
  vtkGetStringMacro(YTitle);
  vtkSetStringMacro(YUnits);
  vtkGetStringMacro(YUnits);
  vtkSetStringMacro(ZTitle);
  vtkGetStringMacro(ZTitle);
  vtkSetStringMacro(ZUnits);
  vtkGetStringMacro(ZUnits);

  // Description:
  // Set/Get the format with which to print the labels on each of the
  // x-y-z axes.
  vtkSetStringMacro(XLabelFormat);
  vtkGetStringMacro(XLabelFormat);
  vtkSetStringMacro(YLabelFormat);
  vtkGetStringMacro(YLabelFormat);
  vtkSetStringMacro(ZLabelFormat);
  vtkGetStringMacro(ZLabelFormat);
  
  // Description:
  // Set/Get the inertial factor that controls how often (i.e, how
  // many renders) the axes can switch position (jump from one axes 
  // to another).
  vtkSetClampMacro(Inertia, int, 1, VTK_LARGE_INTEGER);
  vtkGetMacro(Inertia, int);

  // Description:
  // Specify an offset value to "pull back" the axes from the corner at
  // which they are joined to avoid overlap of axes labels. The 
  // "CornerOffset" is the fraction of the axis length to pull back.
  vtkSetMacro(CornerOffset, float);
  vtkGetMacro(CornerOffset, float);

  // Description:
  // Release any graphics resources that are being consumed by this actor.
  // The parameter window could be used to determine which graphic
  // resources to release.
  void ReleaseGraphicsResources(vtkWindow *);

  // Description:
  // Turn on and off the visibility of each axis.
  vtkSetMacro(XAxisVisibility,int);
  vtkGetMacro(XAxisVisibility,int);
  vtkBooleanMacro(XAxisVisibility,int);
  vtkSetMacro(YAxisVisibility,int);
  vtkGetMacro(YAxisVisibility,int);
  vtkBooleanMacro(YAxisVisibility,int);
  vtkSetMacro(ZAxisVisibility,int);
  vtkGetMacro(ZAxisVisibility,int);
  vtkBooleanMacro(ZAxisVisibility,int);

  // Description:
  // Turn on and off the visibility of labels for each axis.
  vtkSetMacro(XAxisLabelVisibility,int);
  vtkGetMacro(XAxisLabelVisibility,int);
  vtkBooleanMacro(XAxisLabelVisibility,int);

  vtkSetMacro(YAxisLabelVisibility,int);
  vtkGetMacro(YAxisLabelVisibility,int);
  vtkBooleanMacro(YAxisLabelVisibility,int);

  vtkSetMacro(ZAxisLabelVisibility,int);
  vtkGetMacro(ZAxisLabelVisibility,int);
  vtkBooleanMacro(ZAxisLabelVisibility,int);

  // Description:
  // Turn on and off the visibility of ticks for each axis.
  vtkSetMacro(XAxisTickVisibility,int);
  vtkGetMacro(XAxisTickVisibility,int);
  vtkBooleanMacro(XAxisTickVisibility,int);

  vtkSetMacro(YAxisTickVisibility,int);
  vtkGetMacro(YAxisTickVisibility,int);
  vtkBooleanMacro(YAxisTickVisibility,int);

  vtkSetMacro(ZAxisTickVisibility,int);
  vtkGetMacro(ZAxisTickVisibility,int);
  vtkBooleanMacro(ZAxisTickVisibility,int);

  // Description:
  // Turn on and off the visibility of minor ticks for each axis.
  vtkSetMacro(XAxisMinorTickVisibility,int);
  vtkGetMacro(XAxisMinorTickVisibility,int);
  vtkBooleanMacro(XAxisMinorTickVisibility,int);

  vtkSetMacro(YAxisMinorTickVisibility,int);
  vtkGetMacro(YAxisMinorTickVisibility,int);
  vtkBooleanMacro(YAxisMinorTickVisibility,int);

  vtkSetMacro(ZAxisMinorTickVisibility,int);
  vtkGetMacro(ZAxisMinorTickVisibility,int);
  vtkBooleanMacro(ZAxisMinorTickVisibility,int);

  vtkSetMacro(DrawXGridlines,int);
  vtkGetMacro(DrawXGridlines,int);
  vtkBooleanMacro(DrawXGridlines,int);

  vtkSetMacro(DrawYGridlines,int);
  vtkGetMacro(DrawYGridlines,int);
  vtkBooleanMacro(DrawYGridlines,int);

  vtkSetMacro(DrawZGridlines,int);
  vtkGetMacro(DrawZGridlines,int);
  vtkBooleanMacro(DrawZGridlines,int);

  // Description:
  // Set/Get the location of ticks marks. 
  vtkSetClampMacro(TickLocation, int, VTK_TICKS_INSIDE, VTK_TICKS_BOTH);
  vtkGetMacro(TickLocation, int);

  void SetTickLocationToInside(void) 
    { this->SetTickLocation(VTK_TICKS_INSIDE); };
  void SetTickLocationToOutside(void) 
    { this->SetTickLocation(VTK_TICKS_OUTSIDE); };
  void SetTickLocationToBoth(void) 
    { this->SetTickLocation(VTK_TICKS_BOTH); };
  
  // Description:
  // Shallow copy of a KatCubeAxesActor.
  void ShallowCopy(vtkKatCubeAxesActor *actor);

protected:
  vtkKatCubeAxesActor();
  ~vtkKatCubeAxesActor();

  vtkDataSet *Input;     //Define bounds from input data, or
  vtkProp    *Prop;      //Define bounds from actor/assembly, or
  float       Bounds[6]; //Define bounds explicitly

  vtkCamera *Camera;
  int FlyMode;
 
  // to control all axes  
  // [0] always for 'Major' axis during non-static fly modes.
  vtkKatAxisActor *XAxes[4];
  vtkKatAxisActor *YAxes[4];
  vtkKatAxisActor *ZAxes[4];

  char *XTitle;
  char *XUnits;
  char *YTitle;
  char *YUnits;
  char *ZTitle;
  char *ZUnits;

  int TickLocation;

  int XAxisVisibility;
  int YAxisVisibility;
  int ZAxisVisibility;

  int XAxisTickVisibility;
  int YAxisTickVisibility;
  int ZAxisTickVisibility;

  int XAxisMinorTickVisibility;
  int YAxisMinorTickVisibility;
  int ZAxisMinorTickVisibility;

  int XAxisLabelVisibility;
  int YAxisLabelVisibility;
  int ZAxisLabelVisibility;

  int DrawXGridlines;
  int DrawYGridlines;
  int DrawZGridlines;

  char  *XLabelFormat;
  char  *YLabelFormat;
  char  *ZLabelFormat;
  float CornerOffset;
  int   Inertia;
  int   RenderCount;
  int   InertiaLocs[3];
  
  int RenderSomething;
  
private:
  vtkKatCubeAxesActor(const vtkKatCubeAxesActor&);
  void operator=(const vtkKatCubeAxesActor&);

  vtkTimeStamp BuildTime;
  int lastPow;
  int lastXAxisDigits;
  int lastYAxisDigits;
  int lastZAxisDigits;
  float LastXExtent;
  float LastYExtent;
  float LastZExtent;
  int   LastFlyMode;

  int   renderAxesX[4];
  int   renderAxesY[4];
  int   renderAxesZ[4];
  int   numAxesX;
  int   numAxesY;
  int   numAxesZ;

  // various helper methods
  void  TransformBounds(vtkViewport *viewport, const float bounds[6], 
                        float pts[8][3]);
  void  AdjustAxes(float bounds[6], float xCoords[4][6], float yCoords[4][6], 
                   float zCoords[4][6], float xRange[2], float yRange[2], 
                   float zRange[2]);

  void  ComputeTickSize(float bounds[6]);
  void  AdjustValues(const float bounds[6]);
  void  AdjustRange(const float bounds[6]);
  void  SetValueScaleFactor(float scale);
  void  UnSetValueScaleFactor();
  void  BuildAxes(vtkViewport *);
  void  DetermineRenderAxes(vtkViewport *);
  void  SetNonDependentAttributes(void);

  

  // hide the superclass' ShallowCopy() from the user and the compiler.
  void ShallowCopy(vtkProp *prop) { this->vtkProp::ShallowCopy( prop ); };
};


#endif
