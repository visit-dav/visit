/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItScalarBarActor.h,v $
  Language:  C++
  Date:      $Date: 2000/11/03 14:10:27 $
  Version:   $Revision: 1.28 $

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
// .NAME vtkVisItScalarBarActor - Create a scalar bar with labels, title and 
// range
// .SECTION Description
// vtkVisItScalarBarActor creates a scalar bar with annotation text. A scalar
// bar is a legend that indicates to the viewer the correspondence between
// color value and data value. The legend consists of a rectangular bar 
// made of rectangular pieces each colored a constant value. Since 
// vtkVisItScalarBarActor is a subclass of vtkActor2D, it is drawn in the 
// image plane (i.e., in the renderer's viewport) on top of the 3D graphics 
// window.
//
// To use vtkVisItScalarBarActor you must associate a vtkLookupTable (or
// subclass) with it. The lookup table defines the colors and the
// range of scalar values used to map scalar data.  Typically, the
// number of colors shown in the scalar bar is not equal to the number
// of colors in the lookup table, in which case sampling of
// the lookup table is performed. 
//
// Other optional capabilities include specifying the fraction of the
// viewport size (both x and y directions) which will control the size
// of the scalar bar, the number of annotation labels, and the font
// attributes of the annotation text. The actual position of the
// scalar bar on the screen is controlled by using the
// vtkActor2D::SetPosition() method (by default the scalar bar is
// position on the right side of the viewport).  Other features include 
// the ability control the format (print style) with which to print the 
// labels on the scalar bar. Also, the vtkVisItScalarBarActor's property 
// is applied to the scalar bar and annotation (including color, layer, and
// compositing operator).  

// .SECTION See Also
// vtkActor2D vtkTextMapper vtkPolyDataMapper2D

#ifndef __vtkVisItScalarBarActor_h
#define __vtkVisItScalarBarActor_h
#include <plotter_exports.h>

#include <vtkActor2D.h>
#include <vtkLookupTable.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkTextMapper.h>

#include <vector>
#include <string>
#include <maptypes.h>

#define VTK_MAX_NUMLABELS     100

typedef std::vector<std::string> stringVector;
typedef std::vector<double> doubleVector;

// ****************************************************************************
//  Modifications:
//    Kathleen Bonnell, Wed Sep 11 09:01:37 PDT 2002
//    Change LookupTable from type vtkScalarsToColors to type vtkLookupTable, 
//    in order to have access to methods defined only in vtkLookupTable.
//    Add labelColorMap, which translates a label to its corresponding color
//    index (into LookupTable). 
//
//    Kathleen Bonnell, Wed Mar 19 14:44:13 PST 2003  
//    Added method AdjustRangeFormat. 
//
//    Kathleen Bonnell, Mon May 19 13:42:19 PDT 2003   
//    Added member ReverseOrder and Set/Get methods. 
//
//    Eric Brugger, Mon Jul 14 11:55:19 PDT 2003
//    I changed the way the scalar bar is built.  I removed TitleFraction,
//    LabelFraction, SetWidth, SetHeight and FontSize.  I added BarWidth
//    and FontHeight.
//
//    Eric Brugger, Wed Jul 16 08:29:27 PDT 2003
//    I added a number of labels argument to BuildTics and BuildLabels.
//
//    Hank Childs, Sat Mar  3 13:25:15 PST 2007
//    Added method "ShouldCollapseDiscrete".
//
//    Dave Bremer, Wed Oct  8 11:36:27 PDT 2008
//    Added Orientation member and accessors.
//
//    Brad Whitlock, Fri Dec 19 15:44:12 PST 2008
//    I removed the skew function.
//
//    Kathleen Bonnell, Thu Oct  1 13:54:33 PDT 2009
//    Renamed class, as it is no longer strictly a vertical scalar bar.
//    Added legend type, and methods/ivars to support user supplied
//    labels and tick values.  Changed label visibility from on/off to modal,
//    with 4 modes.
// ****************************************************************************

class PLOTTER_API vtkVisItScalarBarActor : public vtkActor2D
{
public:
  vtkTypeMacro(vtkVisItScalarBarActor,vtkActor2D);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Instantiate object. 
  static vtkVisItScalarBarActor *New();
  
  // Description:
  // Access the Position instance variable. Reimplemented from base
  // class to ensure normalized viewport coordinates
  // This variable controls the lower left corner of the scalarbar. 
  void SetPosition(double,double);
  void SetPosition(double x[2]);

  // Description:
  // Access the Position2 instance variable. This variable controls
  // the upper right corner of the scalarbar. It is by default
  // relative to Position1 and in Normalized Viewport coordinates.
  void SetPosition2(double,double);
  void SetPosition2(double x[2]);
  vtkCoordinate *GetPosition2Coordinate();
  double *GetPosition2();
  
  // Description:
  // Draw the scalar bar and annotation text to the screen.
  int RenderOpaqueGeometry(vtkViewport* viewport);
  int RenderTranslucentGeometry(vtkViewport*) { return 0; };
  virtual int RenderOverlay(vtkViewport* viewport);


  // Description:
  // Release any graphics resources that are being consumed by this actor.
  // The parameter window could be used to determine which graphic
  // resources to release.
  virtual void ReleaseGraphicsResources(vtkWindow *);

  // Description:
  // Set/Get the scalar bar width.
  vtkSetClampMacro(BarWidth,double, 0.0, 0.5);
  vtkGetMacro(BarWidth,double);

  // Description:
  // Set/Get the vtkLookupTable to use. The lookup table specifies the number
  // of colors to use in the table (if not overridden), as well as the scalar
  // range.
  vtkSetObjectMacro(LookupTable,vtkLookupTable);
  vtkGetObjectMacro(LookupTable,vtkLookupTable);

  // Description:
  // Set/Get the maximum number of color bar segments to show. This may
  // differ from the number of colors in the lookup table, in which case
  // the colors are samples from the lookup table.
  vtkSetClampMacro(MaximumNumberOfColors, int, 2, VTK_LARGE_INTEGER);
  vtkGetMacro(MaximumNumberOfColors, int);
  
  // Description:
  // Set/Get the number of annotation labels to show.
  vtkSetClampMacro(NumberOfLabels, int, 0, VTK_MAX_NUMLABELS);
  vtkGetMacro(NumberOfLabels, int);
  void SetNumberOfLabelsToDefault(void);
  
  // Description:
  // Set/Get user defined labels; 
  void SetDefinedLabels(const stringVector &);
  void SetDefinedLabels(const doubleVector &);
  stringVector &GetDefinedLabels(void) { return definedLabels; } ;

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
  // Set/Get the font height for the annotation text.
  vtkSetClampMacro(FontHeight, double, 0, 0.2);
  vtkGetMacro(FontHeight, double);

  // Description:
  // Set/Get the format with which to print the labels on the scalar
  // bar.
  //vtkSetStringMacro(LabelFormat);
  virtual void SetLabelFormat(const char *fmt);
  vtkGetStringMacro(LabelFormat);

  // Description:
  // Set/Get the format with which to print the range. 
  vtkSetStringMacro(RangeFormat);
  vtkGetStringMacro(RangeFormat);

  // Description:
  // Set/Get the title of the scalar bar actor,
  vtkSetStringMacro(Title);
  vtkGetStringMacro(Title);

  // Description:
  // Set/Get the range for annotation text. 
  void SetRange(double *);
  void SetRange(double, double);
  double *GetRange(void) { return this->range; } ;

  // Description:
  // Set/Get the range for limits text. 
  void SetVarRange(double *);
  void SetVarRange(double, double);
  double *GetVarRange(void) { return this->varRange; } ;

  // Description:
  // Set/Get the visibility of the range annotation text. 
  vtkSetMacro(RangeVisibility, int);
  vtkGetMacro(RangeVisibility, int);
  vtkBooleanMacro(RangeVisibility, int);

  // Description:
  // Set/Get the visibility of the color bar. 
  vtkSetMacro(ColorBarVisibility, int);
  vtkGetMacro(ColorBarVisibility, int);
  vtkBooleanMacro(ColorBarVisibility, int);

  // Description:
  // Set/Get the visibility of the title annotation text. 
  vtkSetMacro(TitleVisibility, int);
  vtkGetMacro(TitleVisibility, int);
  vtkBooleanMacro(TitleVisibility, int);

  // Description:
  // Set/Get the visibility of the bounding box.
  vtkSetMacro(BoundingBoxVisibility, int);
  vtkGetMacro(BoundingBoxVisibility, int);
  vtkBooleanMacro(BoundingBoxVisibility, int);

  // Description:
  // Set the bounding box color
  vtkSetVector4Macro(BoundingBoxColor, double);

  // Description:
  // Shallow copy of a scalar bar actor. Overloads the virtual vtkProp method.
  void ShallowCopy(vtkProp *prop);

  // Description:
  // Set/Get the SkewFactor.
  vtkSetMacro(SkewFactor, double);
  vtkGetMacro(SkewFactor, double);

  // Description:
  // Turn On/Off skew scaling.
  void SkewScalingOn(void);
  void SkewScalingOff(void);

  // Description:
  // Turn On/Off log scaling.
  void LogScalingOn(void);
  void LogScalingOff(void);

  void SetLabelColorMap(const LevelColorMap &);

  // Description:
  // Set/Get the order in which the color bar should be drawn. 
  // Default is bottom-to-top, (min on bottom, max on top).
  // Reverse is top-to-bottom.
  // Has effect only if user-defined labels are set.
  vtkSetMacro(ReverseOrder, int);
  vtkGetMacro(ReverseOrder, int);
  vtkBooleanMacro(ReverseOrder, int);

  typedef enum { VERTICAL_TEXT_ON_RIGHT, 
                 VERTICAL_TEXT_ON_LEFT,
                 HORIZONTAL_TEXT_ON_TOP,
                 HORIZONTAL_TEXT_ON_BOTTOM
               } BarOrientation;

  vtkSetMacro(Orientation, BarOrientation);
  vtkGetMacro(Orientation, BarOrientation);

  // Description:
  // Enable/Disable inclusion of min and max in tick values.
  vtkSetMacro(MinMaxInclusive, int);
  vtkGetMacro(MinMaxInclusive, int);
  vtkBooleanMacro(MinMaxInclusive, int);

  // Description:
  // Enable/Disable use of user-supplied labels.
  vtkSetMacro(UseSuppliedLabels, int);
  vtkGetMacro(UseSuppliedLabels, int);
  vtkBooleanMacro(UseSuppliedLabels, int);

  // Description:
  // Set/Get user defined labels; 
  void SetSuppliedLabels(const stringVector &);
  stringVector &GetSuppliedLabels(void) { return suppliedLabels; }
  void SetSuppliedValues(const doubleVector &);
  doubleVector &GetSuppliedValues(void) { return suppliedValues; }

  void GetCalculatedValues(doubleVector &); 
  void GetCalculatedLabels(stringVector &); 

  typedef enum { DRAW_NO_LABELS,
                 DRAW_VALUES_ONLY,
                 DRAW_LABELS_ONLY,
                 DRAW_VALUES_AND_LABELS
               } TickLabelDrawMode;

  vtkSetMacro(DrawMode, TickLabelDrawMode);
  vtkGetMacro(DrawMode, TickLabelDrawMode);


  typedef enum { VTK_DISCRETE, 
                 VTK_CONTINUOUS
               } LegendType;

  vtkSetMacro(Type, LegendType);
  vtkGetMacro(Type, LegendType);

protected:
  vtkVisItScalarBarActor();
  virtual ~vtkVisItScalarBarActor();

  void BuildTitle(vtkViewport *);
  void BuildRange(vtkViewport *);
  void BuildTics(double, double, double, int);
  void BuildLabels(vtkViewport *, double, double, double, int);
  virtual void BuildColorBar(vtkViewport *);
  void BuildBoundingBox(vtkViewport *viewport);

  bool   ShouldCollapseDiscrete(void);

  vtkLookupTable *LookupTable;
  int   MaximumNumberOfColors;
  int   NumberOfLabels;
  int   NumberOfLabelsBuilt;
  char  *Title;

  int   Bold;
  int   Italic;
  int   Shadow;
  int   FontFamily;
  double FontHeight;
  char  *LabelFormat;
  char  *RangeFormat;
  vtkCoordinate *Position2Coordinate;

  int TitleVisibility;
  int RangeVisibility;
  int ColorBarVisibility;
  int ReverseOrder;
  int BoundingBoxVisibility;
  BarOrientation Orientation;    

  double BarWidth;

  vtkPolyData         *ColorBar;
  vtkPolyDataMapper2D *ColorBarMapper;
  vtkActor2D          *ColorBarActor;
  
  vtkTextMapper *TitleMapper;
  vtkActor2D    *TitleActor;

  vtkTextMapper *RangeMapper;
  vtkActor2D    *RangeActor;

  vtkTextMapper **LabelMappers;
  vtkActor2D    **LabelActors;

  vtkPolyData         *Tics;
  vtkPolyDataMapper2D *TicsMapper;
  vtkActor2D          *TicsActor;

  vtkPolyData         *BoundingBox;
  vtkPolyDataMapper2D *BoundingBoxMapper;
  vtkActor2D          *BoundingBoxActor;
  double               BoundingBoxColor[4];

  vtkTimeStamp  BuildTime;
  int LastSize[2];
  int LastOrigin[2];
  char *AltTitle;
  int TitleOkayToDraw;
  int LabelOkayToDraw;

  int UseSkewScaling;
  int UseLogScaling;
  double SkewFactor;
  stringVector definedLabels;
  doubleVector definedDoubleLabels;
  double *range;
  double *varRange;

  LevelColorMap labelColorMap; 

  int MinMaxInclusive;
  int UseSuppliedLabels;
  stringVector suppliedLabels;
  doubleVector suppliedValues;

  doubleVector calculatedValues;

  TickLabelDrawMode DrawMode;

  LegendType Type;

private:
  vtkVisItScalarBarActor(const vtkVisItScalarBarActor&);
  void operator=(const vtkVisItScalarBarActor&);

  void VerifySuppliedLabels(void);
};


#endif

