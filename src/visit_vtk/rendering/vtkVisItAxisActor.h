/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItAxisActor.h,v $
  Language:  C++
  Date:      $Date: 2001/09/14 09:11:06 $
  Version:   $Revision: 1.1 $
  Thanks:    Kathleen Bonnell, B Division, Lawrence Livermore Nat'l Laboratory

Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen 
All rights reserved.
     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/
// .NAME vtkVisItAxisActor - Create an axis with tick marks and labels
// .SECTION Description
// vtkVisItAxisActor creates an axis with tick marks, labels, and/or a title,
// depending on the particular instance variable settings. It is assumed that
// the axes is part of a bounding box and is orthoganal to one of the
// coordinate axes.  To use this class, you typically specify two points 
// defining the start and end points of the line (xyz definition using 
// vtkCoordinate class), the axis type (X, Y or Z), the axis location in 
// relation to the bounding box, the bounding box, the number of labels, and
// the data range (min,max). You can also control what parts of the axis are
// visible including the line, the tick marks, the labels, and the title. It
// is also possible to control gridlines, and specifiy on which 'side' the
// tickmarks are drawn (again with respect to the underlying assumed 
// bounding box). You can also specify the label format (a printf style format).
//
// This class decides how to locate the labels, and how to create reasonable 
// tick marks and labels. 
//
// Labels follow the camera so as to be legible from any viewpoint.
//
// The instance variables Point1 and Point2 are instances of vtkCoordinate.
// All calculations and references are in World Coordinates.
//
// .SECTION Notes
// This class was adapted from a 2D version created by Hank Childs called
// vtkHankAxisActor2D.
//
// .SECTION See Also
// vtkActor vtkVectorText vtkPolyDataMapper vtkAxisActor2D vtkCoordinate

#ifndef __vtkVisItAxisActor_h
#define __vtkVisItAxisActor_h
#include <rendering_visit_vtk_exports.h>

#include <vector>
#include <string>

#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkCoordinate.h>
#include <vtkFollower.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkMultiFontVectorText.h>
#include <vtkTextProperty.h>

#define VTK_MAX_LABELS    200
#define VTK_MAX_TICKS     1000

#define VTK_AXIS_TYPE_X   0
#define VTK_AXIS_TYPE_Y   1
#define VTK_AXIS_TYPE_Z   2

#define VTK_TICKS_INSIDE  0
#define VTK_TICKS_OUTSIDE 1
#define VTK_TICKS_BOTH    2

#define VTK_AXIS_POS_MINMIN 0
#define VTK_AXIS_POS_MINMAX 1
#define VTK_AXIS_POS_MAXMAX 2
#define VTK_AXIS_POS_MAXMIN 3

// ****************************************************************************
//  Modifications:
//    Kathleen Bonnell, Tue Aug 31 16:17:43 PDT 2004
//    Added TitleTextTime timestamp, so that title can be updated appropriately
//    when its text changes.  Changed Titles Set macro for a user-defined
//    set so TitleTextTime can be updated.
//
//    Brad Whitlock, Thu Mar 27 10:55:43 PDT 2008
//    Added support for independent title and label text properties.
//
//    Eric Brugger, Tue Oct 21 12:00:23 PDT 2008
//    Added support for specifying tick mark locations.
//
// ****************************************************************************


class RENDERING_VISIT_VTK_API vtkVisItAxisActor : public vtkActor
{
public:
  vtkTypeMacro(vtkVisItAxisActor,vtkActor);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Instantiate object.
  static vtkVisItAxisActor *New();
  
  // Description:
  // Specify the position of the first point defining the axis.
  vtkWorldCoordinateMacro(Point1); 

  // Description:
  // Specify the position of the second point defining the axis.
  vtkWorldCoordinateMacro(Point2); 

  // Description:
  // Specify the (min,max) axis range. This will be used in the generation
  // of labels, if labels are visible.
  vtkSetVector2Macro(Range,double);
  vtkGetVectorMacro(Range,double,2);

  // Description:
  void   SetBounds(double[6]);
  double *GetBounds(void);
  void   GetBounds(double[6]);

  // Description:
  // Set/Get the format with which to print the labels on the axis.
  vtkSetStringMacro(LabelFormat);
  vtkGetStringMacro(LabelFormat);

  // Description:
  // Set/Get the flag that controls whether the minor ticks are visible. 
  vtkSetMacro(MinorTicksVisible, int);
  vtkGetMacro(MinorTicksVisible, int);
  vtkBooleanMacro(MinorTicksVisible, int);

  // Description:
  // Set/Get the title of the axis actor,
  void SetTitle(const char *t);
  vtkGetStringMacro(Title);

  // Description:
  // Set/Get the size of the major tick marks 
  vtkSetMacro(MajorTickSize, double);
  vtkGetMacro(MajorTickSize, double);
  
  // Description:
  // Set/Get the size of the major tick marks 
  vtkSetMacro(MinorTickSize, double);
  vtkGetMacro(MinorTickSize, double);

  // Description:
  // Set/Get the location of the ticks.
  vtkSetClampMacro(TickLocation, int, VTK_TICKS_INSIDE, VTK_TICKS_BOTH);
  vtkGetMacro(TickLocation, int);

  void SetTickLocationToInside(void)
    { this->SetTickLocation(VTK_TICKS_INSIDE); };
  void SetTickLocationToOutside(void)
    { this->SetTickLocation(VTK_TICKS_OUTSIDE); };
  void SetTickLocationToBoth(void)
    { this->SetTickLocation(VTK_TICKS_BOTH); };
  
  // Description:
  // Set/Get visibility of the axis line.
  vtkSetMacro(AxisVisibility, int);
  vtkGetMacro(AxisVisibility, int);
  vtkBooleanMacro(AxisVisibility, int);

  // Description:
  // Set/Get visibility of the axis tick marks.
  vtkSetMacro(TickVisibility, int);
  vtkGetMacro(TickVisibility, int);
  vtkBooleanMacro(TickVisibility, int);

  // Description:
  // Set/Get visibility of the axis labels.
  vtkSetMacro(LabelVisibility, int);
  vtkGetMacro(LabelVisibility, int);
  vtkBooleanMacro(LabelVisibility, int);

  // Description:
  // Set/Get visibility of the axis title.
  vtkSetMacro(TitleVisibility, int);
  vtkGetMacro(TitleVisibility, int);
  vtkBooleanMacro(TitleVisibility, int);

  // Description:
  // Get axis title text property.
  void SetTitleTextProperty(vtkTextProperty *);
  vtkGetMacro(TitleTextProperty, vtkTextProperty *);

  // Description:
  // Get axis labels text property.
  void SetLabelTextProperty(vtkTextProperty *);
  vtkGetMacro(LabelTextProperty, vtkTextProperty *);

  // Description:
  // Set/Get whether gridlines should be drawn.
  vtkSetMacro(DrawGridlines, int);
  vtkGetMacro(DrawGridlines, int);
  vtkBooleanMacro(DrawGridlines, int);

  // Description:
  // Set/Get the length to use when drawing gridlines. 
  vtkSetMacro(GridlineXLength, double);
  vtkGetMacro(GridlineXLength, double);
  vtkSetMacro(GridlineYLength, double);
  vtkGetMacro(GridlineYLength, double);
  vtkSetMacro(GridlineZLength, double);
  vtkGetMacro(GridlineZLength, double);

  // Description:
  // Set/Get the type of this axis.
  vtkSetClampMacro(AxisType, int, VTK_AXIS_TYPE_X, VTK_AXIS_TYPE_Z);
  vtkGetMacro(AxisType, int);
  void SetAxisTypeToX(void) { this->SetAxisType(VTK_AXIS_TYPE_X); };
  void SetAxisTypeToY(void) { this->SetAxisType(VTK_AXIS_TYPE_Y); };
  void SetAxisTypeToZ(void) { this->SetAxisType(VTK_AXIS_TYPE_Z); };

  // Description:
  // Set/Get the position of this axis (in relation to an an
  // assumed bounding box).  For an x-type axis, MINMIN corresponds
  // to the x-edge in the bounding box where Y values are minimum and 
  // Z values are minimum. For a y-type axis, MAXMIN corresponds to the 
  // y-edge where X values are maximum and Z values are minimum.
  // 
  vtkSetClampMacro(AxisPosition, int, VTK_AXIS_POS_MINMIN, VTK_AXIS_POS_MAXMIN);
  vtkGetMacro(AxisPosition, int);

  void SetAxisPositionToMinMin(void)
      { this->SetAxisPosition(VTK_AXIS_POS_MINMIN); };
  void SetAxisPositionToMinMax(void)
      { this->SetAxisPosition(VTK_AXIS_POS_MINMAX); };
  void SetAxisPositionToMaxMax(void)
      { this->SetAxisPosition(VTK_AXIS_POS_MAXMAX); };
  void SetAxisPositionToMaxMin(void)
      { this->SetAxisPosition(VTK_AXIS_POS_MAXMIN); };

  // Description:
  // Set/Get the camera for this axis.  The camera is used by the
  // labels to 'follow' the camera and be legible from any viewpoint.
  virtual void SetCamera(vtkCamera*);
  vtkGetObjectMacro(Camera, vtkCamera);

  // Description:
  // Draw the axis. 
  virtual int RenderOpaqueGeometry(vtkViewport* viewport);
  virtual int RenderTranslucentGeometry(vtkViewport *) {return 0;}

  // Description:
  // Release any graphics resources that are being consumed by this actor.
  // The parameter window could be used to determine which graphic
  // resources to release.
  void ReleaseGraphicsResources(vtkWindow *);

  // Description:
  // Shallow copy of an axis actor. Overloads the virtual vtkProp method.
  void ShallowCopy(vtkProp *prop);

  double ComputeMaxLabelLength(const double [3]);
  double ComputeTitleLength(const double [3]);
  void SetLabelScale(const double);
  void SetTitleScale(const double);

  // Description:
  // Set/Get the starting position for minor and major tick points,
  // and the delta values that determine their spacing. 
  vtkSetMacro(MinorStart, double);
  vtkGetMacro(MinorStart, double);
  vtkSetMacro(MajorStart, double);
  vtkGetMacro(MajorStart, double);
  vtkSetMacro(DeltaMinor, double);
  vtkGetMacro(DeltaMinor, double);
  vtkSetMacro(DeltaMajor, double);
  vtkGetMacro(DeltaMajor, double);

  void SetLabels(const std::vector<std::string> &labels);

  void BuildAxis(vtkViewport *viewport, bool);

  // Description:
  // Set/Get the flag that controls whether the labels are
  // adjusted for "nice" numerical values to make it easier to read
  // the labels. The adjustment is based in the Range instance variable.
  vtkSetMacro(AdjustLabels, int);
  vtkGetMacro(AdjustLabels, int);
  vtkBooleanMacro(AdjustLabels, int);

  // Description:
  // Set/Get for the major tick mark minimum.
  vtkSetMacro(MajorTickMinimum, double);
  vtkGetMacro(MajorTickMinimum, double);

  // Description:
  // Set/Get for the major tick mark maximum.
  vtkSetMacro(MajorTickMaximum, double);
  vtkGetMacro(MajorTickMaximum, double);

  // Description:
  // Set/Get for the major tick mark spacing.
  vtkSetMacro(MajorTickSpacing, double);
  vtkGetMacro(MajorTickSpacing, double);

  // Description:
  // Set/Get for the minor tick mark spacing.
  vtkSetMacro(MinorTickSpacing, double);
  vtkGetMacro(MinorTickSpacing, double);

protected:
  vtkVisItAxisActor();
  ~vtkVisItAxisActor();

  char  *Title;
  double  Range[2];
  double  LastRange[2];
  char  *LabelFormat;
  int    NumberOfLabelsBuilt;
  int    MinorTicksVisible;
  int    LastMinorTicksVisible;
  int    TickLocation;
 
  int    DrawGridlines;
  int    LastDrawGridlines;
  double  GridlineXLength;
  double  GridlineYLength;
  double  GridlineZLength;
  
  int    AxisVisibility;
  int    TickVisibility;
  int    LastTickVisibility;
  int    LabelVisibility;
  int    TitleVisibility;
  
  int    AxisType;
  int    AxisPosition;
  double  Bounds[6];
  
  int    AdjustLabels;
  double  MajorTickMinimum;
  double  MajorTickMaximum;
  double  MajorTickSpacing;
  double  MinorTickSpacing;
  int    LastAdjustLabels;
  double  LastMajorTickMinimum;
  double  LastMajorTickMaximum;
  double  LastMajorTickSpacing;
  double  LastMinorTickSpacing;

private:
  vtkVisItAxisActor(const vtkVisItAxisActor&);
  void operator=(const vtkVisItAxisActor&);


  void TransformBounds(vtkViewport *, double bnds[6]);

  void BuildLabels(vtkViewport *, bool);
  void SetLabelPositions(vtkViewport *, bool);

  void BuildTitle(bool);

  void SetAxisPointsAndLines(void);
  bool BuildTickPointsForXType(double p1[3], double p2[3], bool);
  bool BuildTickPointsForYType(double p1[3], double p2[3], bool);
  bool BuildTickPointsForZType(double p1[3], double p2[3], bool);

  bool TickVisibilityChanged(void);
  vtkProperty *NewTitleProperty();
  vtkProperty *NewLabelProperty();

  vtkCoordinate *Point1Coordinate;
  vtkCoordinate *Point2Coordinate;

  double  MajorTickSize;
  double  MinorTickSize;

  double  MajorStart;
  double  MinorStart;

  double  DeltaMinor;
  double  DeltaMajor;

  int    LastAxisPosition;
  int    LastAxisType;
  int    LastTickLocation;
  double  LastLabelStart;

  vtkPoints         *minorTickPts;
  vtkPoints         *majorTickPts;
  vtkPoints         *gridlinePts;

  vtkMultiFontVectorText *TitleVector;
  vtkPolyDataMapper      *TitleMapper;
  vtkFollower            *TitleActor;
  vtkTextProperty        *TitleTextProperty;

  vtkMultiFontVectorText **LabelVectors;
  vtkPolyDataMapper      **LabelMappers;
  vtkFollower            **LabelActors;
  vtkTextProperty         *LabelTextProperty;

  vtkPolyData        *Axis;
  vtkPolyDataMapper  *AxisMapper;
  vtkActor           *AxisActor;

  vtkCamera          *Camera;
  vtkTimeStamp        BuildTime;
  vtkTimeStamp        BoundsTime;
  vtkTimeStamp        LabelBuildTime;
  vtkTimeStamp        TitleTextTime;

  int                 AxisHasZeroLength;
};


#endif
