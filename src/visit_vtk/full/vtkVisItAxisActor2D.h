/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItAxisActor2D.h,v $
  Language:  C++
  Date:      $Date: 2000/06/08 09:11:06 $
  Version:   $Revision: 1.14 $

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
// .NAME vtkVisItAxisActor2D - Create an axis with tick marks and labels
// .SECTION Description
// vtkVisItAxisActor2D creates an axis with tick marks, labels, and/or a title,
// depending on the particular instance variable settings. To use this class,
// you typically specify two points defining the start and end points of the
// line (x-y definition using vtkCoordinate class), the number of labels, and
// the data range (min,max). You can also control what parts of the axis are
// visible including the line, the tick marks, the labels, and the title. It
// is also possible to control the font family, the font style (bold and/or
// italic), and whether font shadows are drawn. You can also specify the
// label format (a printf style format).
//
// This class decides what font size to use and how to locate the labels. It
// also decides how to create reasonable tick marks and labels. The number
// of labels and the range of values may not match the number specified, but
// should be close.
//
// Labels are drawn on the "right" side of the axis. The "right" side is
// the side of the axis on the right as you move from Point1 to Point2. The
// way the labels and title line up with the axis and tick marks depends on
// whether the line is considered horizontal or vertical.
//
// The instance variables Point1 and Point2 are instances of vtkCoordinate.
// What this means is that you can specify the axis in a variety of coordinate
// systems. Also, the axis does not have to be either horizontal or vertical.
// The tick marks are created so that they are perpendicular to the axis.

// .SECTION See Also
// vtkActor2D vtkTextMapper vtkPolyDataMapper2D vtkScalarBarActor
// vtkCoordinate

#ifndef __vtkVisItAxisActor2D_h
#define __vtkVisItAxisActor2D_h
#include <visit_vtk_exports.h>

#include "vtkActor2D.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkTextMapper.h"

#define VTK_MAX_LABELS 1000

class VISIT_VTK_API vtkVisItAxisActor2D : public vtkActor2D
{
public:
  vtkTypeMacro(vtkVisItAxisActor2D,vtkActor2D);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Instantiate object.
  static vtkVisItAxisActor2D *New();
  
  // Description:
  // Specify the position of the first point defining the axis.
  vtkViewportCoordinateMacro(Point1);
  
  // Description:
  // Specify the position of the second point defining the axis. Note that
  // the order from Point1 to Point2 controls which side the tick marks
  // are drawn on (ticks are drawn on the right, if visible).
  vtkViewportCoordinateMacro(Point2);
  
  // Description:
  // Set/Get the use axis orientation flag.
  vtkSetClampMacro(UseOrientationAngle, int, 0, 1);
  vtkGetMacro(UseOrientationAngle, int);

  // Description:
  // Set/Get the axis orientation angle.
  vtkSetMacro(OrientationAngle, double);
  vtkGetMacro(OrientationAngle, double);

  // Description:
  // Specify the (min,max) axis range. This will be used in the generation
  // of labels, if labels are visible.
  vtkSetVector2Macro(Range,double);
  vtkGetVectorMacro(Range,double,2);

  // Description:
  // Set/Get the number of annotation labels to show.
  vtkSetClampMacro(NumberOfLabels, int, 2, VTK_MAX_LABELS);
  vtkGetMacro(NumberOfLabels, int);
  
  // Description:
  // Set/Get the format with which to print the labels on the scalar
  // bar.
  vtkSetStringMacro(LabelFormat);
  vtkGetStringMacro(LabelFormat);

  vtkSetStringMacro(LogLabelFormat);
  vtkGetStringMacro(LogLabelFormat);

  // Description:
  // Set/Get the flag that controls whether the labels are
  // adjusted for "nice" numerical values to make it easier to read 
  // the labels. The adjustment is based in the Range instance variable.
  vtkSetMacro(AdjustLabels, int);
  vtkGetMacro(AdjustLabels, int);
  vtkBooleanMacro(AdjustLabels, int);

  // Description:
  // Set/Get the scale factor for the major tick mark labels.
  vtkSetMacro(MajorTickLabelScale, double);
  vtkGetMacro(MajorTickLabelScale, double);

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
  
  // Description:
  // Set/Get the flag that controls whether the minor ticks are visible. 
  vtkSetMacro(MinorTicksVisible, int);
  vtkGetMacro(MinorTicksVisible, int);
  vtkBooleanMacro(MinorTicksVisible, int);

  // Description:
  // Set/Get the flag that controls whether the title is placed at the end
  // of the aixs.
  vtkSetMacro(TitleAtEnd, int);
  vtkGetMacro(TitleAtEnd, int);
  vtkBooleanMacro(TitleAtEnd, int);

  // Description:
  // Set/Get the title of the scalar bar actor,
  vtkSetStringMacro(Title);
  vtkGetStringMacro(Title);

  // Description:
  // Enable/Disable bolding annotation text.
  vtkSetMacro(Bold, int);
  vtkGetMacro(Bold, int);
  vtkBooleanMacro(Bold, int);

  // Description:
  // Enable/Disable italicizing annotation text.
  vtkSetMacro(Italic, int);
  vtkGetMacro(Italic, int);
  vtkBooleanMacro(Italic, int);

  // Description:
  // Enable/Disable creating shadows on the annotation text. Shadows make 
  // the text easier to read.
  vtkSetMacro(Shadow, int);
  vtkGetMacro(Shadow, int);
  vtkBooleanMacro(Shadow, int);

  // Description:
  // Set/Get the font family for the annotation text. Three font types 
  // are available: Arial (VTK_ARIAL), Courier (VTK_COURIER), and 
  // Times (VTK_TIMES).
  vtkSetMacro(FontFamily, int);
  vtkGetMacro(FontFamily, int);
  void SetFontFamilyToArial() {this->SetFontFamily(VTK_ARIAL);};
  void SetFontFamilyToCourier() {this->SetFontFamily(VTK_COURIER);};
  void SetFontFamilyToTimes() {this->SetFontFamily(VTK_TIMES);};

  // Description:
  // Set/Get the length of the tick marks (expressed in pixels or display
  // coordinates). 
  vtkSetClampMacro(TickLength, int, 0, 100);
  vtkGetMacro(TickLength, int);
  
  // Description:
  // Set/Get the offset of the labels (expressed in pixels or display
  // coordinates). The offset is the distance of labels from tick marks
  // or other objects.
  vtkSetClampMacro(TickOffset, int, 0, 100);
  vtkGetMacro(TickOffset, int);

  // Description:
  // Set/Get the location of the ticks.
  // (inside==0, outside==1 bothsides==2). 
  vtkSetClampMacro(TickLocation, int, 0, 2);
  vtkGetMacro(TickLocation, int);
  
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

  // Description:
  // Set/Get the axes tick label font height.
  vtkSetClampMacro(LabelFontHeight, double, 0.002, 0.2);
  vtkGetMacro(LabelFontHeight, double);

  // Description:
  // Set/Get the axes title font height.
  vtkSetClampMacro(TitleFontHeight, double, 0.002, 0.2);
  vtkGetMacro(TitleFontHeight, double);

  // Description:
  // Set/Get scaling of the axis.
  vtkSetMacro(LogScale, int);
  vtkGetMacro(LogScale, int);
  vtkBooleanMacro(LogScale, int);

  // Description:
  // Set/Get the string width multiplier e.g. for horizontal centering
  vtkSetMacro(EndStringHOffsetFactor, double);
  vtkGetMacro(EndStringHOffsetFactor, double);

  // Set/Get the string height multiplier e.g. for vertical centering
  vtkSetMacro(EndStringVOffsetFactor, double);
  vtkGetMacro(EndStringVOffsetFactor, double);


  // Description:
  // Draw the axis. 
  int RenderOverlay(vtkViewport* viewport);
  int RenderOpaqueGeometry(vtkViewport* viewport);
  int RenderTranslucentGeometry(vtkViewport *) {return 0;}

  // Description:
  // Release any graphics resources that are being consumed by this actor.
  // The parameter window could be used to determine which graphic
  // resources to release.
  void ReleaseGraphicsResources(vtkWindow *);

  // Description:
  // Computes the range of the axis given an input range.  It does not 
  // necessarily place tick marks at the endpoints so that it can place
  // them accurately where "nice" numbers lie.
  static void AdjustLabelsComputeRange(double inRange[2], double outRange[2],
                                       int inNumTicks, int &outNumTicks,
                                       double *proportion, double *ticksize, 
                                       int minorVisible, int drawGrids, 
                                       int logScale);

  // Description:
  // Computes the range based on the major tick mark location, the major tick
  // mark spacing, and the minor tick mark spacing.
  static void SpecifiedComputeRange(double inRange[2], double outRange[2],
                                    double majorTickMinimum,
                                    double majorTickMaximum,
                                    double majorTickSpacing,
                                    double minorTickSpacing, int &numTicks,
                                    double *proportion, double *ticksize,
                                    int minorVisible, int drawGrids,
                                    int logScale);

  static void ComputeLogTicks(double inRange[2], double sortedRange[2],
                             int &outNumTicks, double *proportion, 
                             double *ticksize, int minorVisible, int drawGrids);

  // Description:
  // Shallow copy of an axis actor. Overloads the virtual vtkProp method.
  void ShallowCopy(vtkProp *prop);

  // Description:
  // Return this object's MTime.
  unsigned long GetMTime(); 
   
protected:
  vtkVisItAxisActor2D();
  ~vtkVisItAxisActor2D();

  vtkCoordinate *Point1Coordinate;
  vtkCoordinate *Point2Coordinate;
  int    UseOrientationAngle;
  double OrientationAngle; 
  char   *Title;
  double  Range[2];
  int    NumberOfLabels;
  char   *LabelFormat;
  char   *LogLabelFormat;
  int    NumberOfLabelsBuilt;
  int    AdjustLabels;
  double MajorTickLabelScale;
  double MajorTickMinimum;
  double MajorTickMaximum;
  double MajorTickSpacing;
  double MinorTickSpacing;
  int    MinorTicksVisible;
  int    TitleAtEnd;
  double  LabelFontHeight;
  double  TitleFontHeight;
  int    TickLength;
  int    TickOffset;
  int    TickLocation;

  int    Bold;
  int    Italic;
  int    Shadow;
  int    FontFamily;

  int    DrawGridlines;
  double  GridlineXLength;
  double  GridlineYLength;
  
  int    AxisVisibility;
  int    TickVisibility;
  int    LabelVisibility;
  int    TitleVisibility;
  
  int    LastPoint1[2];
  int    LastPoint2[2];

  int    LogScale;

  double EndStringHOffsetFactor;
  double EndStringVOffsetFactor;
  
private:
  vtkVisItAxisActor2D(const vtkVisItAxisActor2D&);
  void operator=(const vtkVisItAxisActor2D&);

  void BuildAxis(vtkViewport *viewport);
  static double ComputeStringOffset(double width, double height, double theta);
  static void SetOffsetPosition(double xTick[3], double theta, int stringHeight, 
                                int stringWidth, int offset, vtkActor2D *actor,
                                int titleAtEnd, double endStringHOffsetFactor,
                                double endStringVOffsetFactor);

  void           SetNumberOfLabelsBuilt(const int);
  vtkTextMapper *TitleMapper;
  vtkActor2D    *TitleActor;

  vtkTextMapper **LabelMappers;
  vtkActor2D    **LabelActors;

  vtkPolyData         *Axis;
  vtkPolyDataMapper2D *AxisMapper;
  vtkActor2D          *AxisActor;

  vtkTimeStamp  BuildTime;
};


#endif
