/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItCubeAxesActor.h,v $
  Language:  C++
  Date:      $Date: 2001/08/03 20:08:22 $
  Version:   $Revision: 1.22 $
  Thanks:    Kathleen Bonnell, B Division, Lawrence Livermore Nat'l Laboratory

Copyright (c) 1993-2001 Ken Martin, Will Schroeder, Bill Lorensen
All rights reserve
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/
// .NAME vtkVisItCubeAxesActor - create a  plot of a bounding box edges -
// used for navigation
// .SECTION Description
// vtkVisItCubeAxesActor is a composite actor that draws axes of the
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
// to render the vtkVisItCubeAxesActor. You can optionally turn on/off labels,
// ticks, gridlines, and set tick location, number of labels, and text to
// use for axis-titles.  A 'corner offset' can also be set.  This allows
// the axes to be set partially away from the actual bounding box to perhaps
// prevent overlap of labels between the various axes.
//
// The Bounds instance variable (an array of six doubles) is used to determine
// the bounding box.
//
// .SECTION See Also
// vtkActor vtkAxisActor vtkCubeAxesActor2D
//
// Modifications:
//   Jeremy Meredith, Wed May  5 14:31:37 EDT 2010
//   Added support for title visibility separate from label visibility.
//
//   Jeremy Meredith, Tue May 18 12:49:48 EDT 2010
//   Renamed some instances of Range to Bounds to reflect their true
//   usage (since in theory, the range of an axis need not be tied to
//   its location in physical space).
//
//   Jeremy Meredith, Tue May 18 13:14:37 EDT 2010
//   Removed unused CornerOffset.
//
//   Jeremy Meredith, Tue May 18 13:23:21 EDT 2010
//   Added a concept of Range which is independent of Bounds.
//   Removed trivial and now mislabeled AdjustAxes
//   Renamed AdjustBounds and AdjustValues to ComputeLabelExponent and
//   ComputeLabelFormat, respectively.
//
//   Alister Maguire, Tue Apr 28 14:54:38 PDT 2020
//   Added an override of SetVisibility to handle setting the visibility
//   of individual characteristics. NOTE: this is currently needed for
//   our OSPRay integration.
//

#ifndef __vtkVisItCubeAxesActor_h
#define __vtkVisItCubeAxesActor_h
#include <plotter_exports.h>

#include "vtkVisItAxisActor.h"
#include "vtkCamera.h"
#include <vtkTextProperty.h>

#define VTK_FLY_OUTER_EDGES     0
#define VTK_FLY_CLOSEST_TRIAD   1
#define VTK_FLY_FURTHEST_TRIAD  2
#define VTK_FLY_STATIC_TRIAD    3
#define VTK_FLY_STATIC_EDGES    4

#define VTK_TICKS_INSIDE        0
#define VTK_TICKS_OUTSIDE       1
#define VTK_TICKS_BOTH          2

class vtkCollection;

class PLOTTER_API vtkVisItCubeAxesActor : public vtkActor
{
public:
  vtkTypeMacro(vtkVisItCubeAxesActor,vtkActor);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Instantiate object with label format "6.3g" and the number of labels
  // per axis set to 3.
  static vtkVisItCubeAxesActor *New();

  // Description:
  // Draw the axes as per the vtkProp superclass' API.
  int RenderOpaqueGeometry(vtkViewport*) override;
  int RenderTranslucentPolygonalGeometry(vtkViewport *) override {return 0;}
  int HasTranslucentPolygonalGeometry() override {return 0;}

  // Descrition
  // Build the axes that can be used with VTK's scenegraph API
  void BuildGeometry(vtkViewport*, vtkCollection* collection);

  // Description:
  // Explicitly specify the region in space around which to draw the bounds.
  // The bounds is used only when no Input or Prop is specified. The bounds
  // are specified according to (xmin,xmax, ymin,ymax, zmin,zmax), making
  // sure that the min's are less than the max's.
  vtkSetVector6Macro(Bounds,double);
  double *GetBounds() override;
  void GetBounds(double& xmin, double& xmax, double& ymin, double& ymax,
                 double& zmin, double& zmax);
  void GetBounds(double bounds[6]);

  // Description:
  // Explicitly specify the actual min/max values assigned to each
  // of the three axes.  This is what controls the actual numbers
  // displayed on the axes, and for the typical auto-set axes with
  // no scaling, will be exactly the same as the bounds.
  vtkSetVector6Macro(Ranges,double);
  double *GetRanges();

  // Description:
  // Set/Get the camera to perform scaling and translation of the
  // vtkVisItCubeAxesActor.
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
  vtkSetClampMacro(Inertia, int, 1, VTK_INT_MAX);
  vtkGetMacro(Inertia, int);

  // Description:
  // Release any graphics resources that are being consumed by this actor.
  // The parameter window could be used to determine which graphic
  // resources to release.
  void ReleaseGraphicsResources(vtkWindow *) override;

  // Description:
  // Set visibility for ALL available attributes.
  virtual void SetVisibility(bool);

  // Description:
  // Turn on and off the visibility of each axis.
  vtkSetMacro(XAxisVisibility,bool);
  vtkGetMacro(XAxisVisibility,bool);
  vtkBooleanMacro(XAxisVisibility,bool);

  vtkSetMacro(YAxisVisibility,bool);
  vtkGetMacro(YAxisVisibility,bool);
  vtkBooleanMacro(YAxisVisibility,bool);

  vtkSetMacro(ZAxisVisibility,bool);
  vtkGetMacro(ZAxisVisibility,bool);
  vtkBooleanMacro(ZAxisVisibility,bool);

  // Description:
  // Turn on and off the visibility of labels for each axis.
  vtkSetMacro(XAxisLabelVisibility,bool);
  vtkGetMacro(XAxisLabelVisibility,bool);
  vtkBooleanMacro(XAxisLabelVisibility,bool);

  vtkSetMacro(YAxisLabelVisibility,bool);
  vtkGetMacro(YAxisLabelVisibility,bool);
  vtkBooleanMacro(YAxisLabelVisibility,bool);

  vtkSetMacro(ZAxisLabelVisibility,bool);
  vtkGetMacro(ZAxisLabelVisibility,bool);
  vtkBooleanMacro(ZAxisLabelVisibility,bool);

  // Description:
  // Turn on and off the visibility of titles for each axis.
  vtkSetMacro(XAxisTitleVisibility,bool);
  vtkGetMacro(XAxisTitleVisibility,bool);
  vtkBooleanMacro(XAxisTitleVisibility,bool);

  vtkSetMacro(YAxisTitleVisibility,bool);
  vtkGetMacro(YAxisTitleVisibility,bool);
  vtkBooleanMacro(YAxisTitleVisibility,bool);

  vtkSetMacro(ZAxisTitleVisibility,bool);
  vtkGetMacro(ZAxisTitleVisibility,bool);
  vtkBooleanMacro(ZAxisTitleVisibility,bool);

  // Description:
  // Turn on and off the visibility of ticks for each axis.
  vtkSetMacro(XAxisTickVisibility,bool);
  vtkGetMacro(XAxisTickVisibility,bool);
  vtkBooleanMacro(XAxisTickVisibility,bool);

  vtkSetMacro(YAxisTickVisibility,bool);
  vtkGetMacro(YAxisTickVisibility,bool);
  vtkBooleanMacro(YAxisTickVisibility,bool);

  vtkSetMacro(ZAxisTickVisibility,bool);
  vtkGetMacro(ZAxisTickVisibility,bool);
  vtkBooleanMacro(ZAxisTickVisibility,bool);

  // Description:
  // Turn on and off the visibility of minor ticks for each axis.
  vtkSetMacro(XAxisMinorTickVisibility,bool);
  vtkGetMacro(XAxisMinorTickVisibility,bool);
  vtkBooleanMacro(XAxisMinorTickVisibility,bool);

  vtkSetMacro(YAxisMinorTickVisibility,bool);
  vtkGetMacro(YAxisMinorTickVisibility,bool);
  vtkBooleanMacro(YAxisMinorTickVisibility,bool);

  vtkSetMacro(ZAxisMinorTickVisibility,bool);
  vtkGetMacro(ZAxisMinorTickVisibility,bool);
  vtkBooleanMacro(ZAxisMinorTickVisibility,bool);

  vtkSetMacro(DrawXGridlines,bool);
  vtkGetMacro(DrawXGridlines,bool);
  vtkBooleanMacro(DrawXGridlines,bool);

  vtkSetMacro(DrawYGridlines,bool);
  vtkGetMacro(DrawYGridlines,bool);
  vtkBooleanMacro(DrawYGridlines,bool);

  vtkSetMacro(DrawZGridlines,bool);
  vtkGetMacro(DrawZGridlines,bool);
  vtkBooleanMacro(DrawZGridlines,bool);

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

  void SetLabelScaling(bool, int, int, int);

  // Description:
  // Returns the text property for the title on an axis.
  vtkTextProperty *GetTitleTextProperty(int);

  // Description:
  // Returns the text property for the labels on an axis.
  vtkTextProperty *GetLabelTextProperty(int);

  void SetTitleScale(double,double,double);
  vtkGetVector3Macro(TitleScale, double);

  void SetLabelScale(double,double,double);
  vtkGetVector3Macro(LabelScale, double);

  // Description:
  // Set/Get the tick mark location properties for each of the x, y and
  // z axes.
  vtkSetMacro(AdjustLabels, bool);
  vtkGetMacro(AdjustLabels, bool);
  vtkBooleanMacro(AdjustLabels, bool);

  vtkSetMacro(XMajorTickMinimum, double);
  vtkGetMacro(XMajorTickMinimum, double);
  vtkSetMacro(XMajorTickMaximum, double);
  vtkGetMacro(XMajorTickMaximum, double);
  vtkSetMacro(XMajorTickSpacing, double);
  vtkGetMacro(XMajorTickSpacing, double);
  vtkSetMacro(XMinorTickSpacing, double);
  vtkGetMacro(XMinorTickSpacing, double);
  vtkSetMacro(YMajorTickMinimum, double);
  vtkGetMacro(YMajorTickMinimum, double);
  vtkSetMacro(YMajorTickMaximum, double);
  vtkGetMacro(YMajorTickMaximum, double);
  vtkSetMacro(YMajorTickSpacing, double);
  vtkGetMacro(YMajorTickSpacing, double);
  vtkSetMacro(YMinorTickSpacing, double);
  vtkGetMacro(YMinorTickSpacing, double);
  vtkSetMacro(ZMajorTickMinimum, double);
  vtkGetMacro(ZMajorTickMinimum, double);
  vtkSetMacro(ZMajorTickMaximum, double);
  vtkGetMacro(ZMajorTickMaximum, double);
  vtkSetMacro(ZMajorTickSpacing, double);
  vtkGetMacro(ZMajorTickSpacing, double);
  vtkSetMacro(ZMinorTickSpacing, double);
  vtkGetMacro(ZMinorTickSpacing, double);

  // Description:
  // Shallow copy of a VisItCubeAxesActor.
  void ShallowCopy(vtkVisItCubeAxesActor *actor);

protected:
  vtkVisItCubeAxesActor();
  ~vtkVisItCubeAxesActor();

  double       Bounds[6]; //Define bounds explicitly
  double       Ranges[6]; //Define ranges explicitly

  vtkCamera *Camera;
  int FlyMode;

  // to control all axes
  // [0] always for 'Major' axis during non-static fly modes.
  vtkVisItAxisActor *XAxes[4];
  vtkVisItAxisActor *YAxes[4];
  vtkVisItAxisActor *ZAxes[4];

  char *XTitle;
  char *XUnits;
  char *YTitle;
  char *YUnits;
  char *ZTitle;
  char *ZUnits;

  char *ActualXLabel;
  char *ActualYLabel;
  char *ActualZLabel;

  int TickLocation;

  bool XAxisVisibility;
  bool YAxisVisibility;
  bool ZAxisVisibility;

  bool XAxisTickVisibility;
  bool YAxisTickVisibility;
  bool ZAxisTickVisibility;

  bool XAxisMinorTickVisibility;
  bool YAxisMinorTickVisibility;
  bool ZAxisMinorTickVisibility;

  bool XAxisLabelVisibility;
  bool YAxisLabelVisibility;
  bool ZAxisLabelVisibility;

  bool XAxisTitleVisibility;
  bool YAxisTitleVisibility;
  bool ZAxisTitleVisibility;

  bool DrawXGridlines;
  bool DrawYGridlines;
  bool DrawZGridlines;

  char  *XLabelFormat;
  char  *YLabelFormat;
  char  *ZLabelFormat;
  double CornerOffset;
  int   Inertia;
  int   RenderCount;
  int   InertiaLocs[3];

  bool  RenderSomething;

  vtkTextProperty *TitleTextProperty[3];
  vtkTextProperty *LabelTextProperty[3];
  double           TitleScale[3];
  double           LabelScale[3];

  bool   AdjustLabels;
  double XMajorTickMinimum;
  double XMajorTickMaximum;
  double XMajorTickSpacing;
  double XMinorTickSpacing;
  double YMajorTickMinimum;
  double YMajorTickMaximum;
  double YMajorTickSpacing;
  double YMinorTickSpacing;
  double ZMajorTickMinimum;
  double ZMajorTickMaximum;
  double ZMajorTickSpacing;
  double ZMinorTickSpacing;
  bool   LastAdjustLabels;
  double LastXMajorTickMinimum;
  double LastXMajorTickMaximum;
  double LastXMajorTickSpacing;
  double LastXMinorTickSpacing;
  double LastYMajorTickMinimum;
  double LastYMajorTickMaximum;
  double LastYMajorTickSpacing;
  double LastYMinorTickSpacing;
  double LastZMajorTickMinimum;
  double LastZMajorTickMaximum;
  double LastZMajorTickSpacing;
  double LastZMinorTickSpacing;

private:
  vtkVisItCubeAxesActor(const vtkVisItCubeAxesActor&);
  void operator=(const vtkVisItCubeAxesActor&);

  vtkSetStringMacro(ActualXLabel);
  vtkSetStringMacro(ActualYLabel);
  vtkSetStringMacro(ActualZLabel);

  vtkTimeStamp BuildTime;
  int lastXPow;
  int lastYPow;
  int lastZPow;
  int userXPow;
  int userYPow;
  int userZPow;
  bool autoLabelScaling;
  int lastXAxisDigits;
  int lastYAxisDigits;
  int lastZAxisDigits;
  double LastXBounds[2];
  double LastYBounds[2];
  double LastZBounds[2];
  int   LastFlyMode;

  int   renderAxesX[4];
  int   renderAxesY[4];
  int   renderAxesZ[4];
  int   numAxesX;
  int   numAxesY;
  int   numAxesZ;

  bool mustAdjustXValue;
  bool mustAdjustYValue;
  bool mustAdjustZValue;
  bool ForceXLabelReset;
  bool ForceYLabelReset;
  bool ForceZLabelReset;

  bool scalingChanged;

  // various helper methods
  void  TransformBounds(vtkViewport *viewport, const double bounds[6],
                        double pts[8][3]);
  bool  ComputeTickSize(double bounds[6]);
  void  ComputeLabelExponent(const double bounds[6]);
  void  ComputeLabelFormat(const double bounds[6]);
  void  BuildAxes(vtkViewport *);
  void  DetermineRenderAxes(vtkViewport *);
  void  SetNonDependentAttributes(void);
  void  BuildLabels(vtkVisItAxisActor *axes[4]);
  void  AdjustTicksComputeRange(vtkVisItAxisActor *axes[4]);



  // hide the superclass' ShallowCopy() from the user and the compiler.
  void ShallowCopy(vtkProp *prop) override { this->vtkProp::ShallowCopy( prop ); }
};


#endif
