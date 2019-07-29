// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
#include <vtkTextActor.h>

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
//
//    Brad Whitlock, Mon Feb 27 16:11:35 PST 2012
//    Switch actor over to vtkTextActor instead of mapper/actor pair since the
//    new way is so much faster.
//
// ****************************************************************************

class PLOTTER_API vtkVisItScalarBarActor : public vtkActor2D
{
public:
  vtkTypeMacro(vtkVisItScalarBarActor,vtkActor2D);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Instantiate object.
  static vtkVisItScalarBarActor *New();

  // Description:
  // Access the Position instance variable. Reimplemented from base
  // class to ensure normalized viewport coordinates
  // This variable controls the lower left corner of the scalarbar.
  void SetPosition(double,double) override;
  void SetPosition(double x[2]) override;

  // Description:
  // Access the Position2 instance variable. This variable controls
  // the upper right corner of the scalarbar. It is by default
  // relative to Position1 and in Normalized Viewport coordinates.
  void SetPosition2(double,double) override;
  void SetPosition2(double x[2]) override;
  vtkCoordinate *GetPosition2Coordinate() override;
  double *GetPosition2() override;

  // Description:
  // Draw the scalar bar and annotation text to the screen.
  int RenderOpaqueGeometry(vtkViewport* viewport) override;
  int RenderTranslucentPolygonalGeometry(vtkViewport*) override { return 0; }
  int HasTranslucentPolygonalGeometry() override { return 0; }
  int RenderOverlay(vtkViewport* viewport) override;


  // Description:
  // Release any graphics resources that are being consumed by this actor.
  // The parameter window could be used to determine which graphic
  // resources to release.
  void ReleaseGraphicsResources(vtkWindow *) override;

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
  vtkSetClampMacro(MaximumNumberOfColors, int, 2, VTK_INT_MAX);
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
  vtkSetMacro(Bold, bool);
  vtkGetMacro(Bold, bool);
  vtkBooleanMacro(Bold, bool);

  // Description:
  // Enable/Disable italicizing annotation text.
  vtkSetMacro(Italic, bool);
  vtkGetMacro(Italic, bool);
  vtkBooleanMacro(Italic, bool);

  // Description:
  // Enable/Disable creating shadows on the annotation text. Shadows make
  // the text easier to read.
  vtkSetMacro(Shadow, bool);
  vtkGetMacro(Shadow, bool);
  vtkBooleanMacro(Shadow, bool);

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
  vtkSetMacro(RangeVisibility, bool);
  vtkGetMacro(RangeVisibility, bool);
  vtkBooleanMacro(RangeVisibility, bool);

  // Description:
  // Set/Get the visibility of the color bar.
  vtkSetMacro(ColorBarVisibility, bool);
  vtkGetMacro(ColorBarVisibility, bool);
  vtkBooleanMacro(ColorBarVisibility, bool);

  // Description:
  // Set/Get the visibility of the title annotation text.
  vtkSetMacro(TitleVisibility, bool);
  vtkGetMacro(TitleVisibility, bool);
  vtkBooleanMacro(TitleVisibility, bool);

  // Description:
  // Set/Get the visibility of the bounding box.
  vtkSetMacro(BoundingBoxVisibility, bool);
  vtkGetMacro(BoundingBoxVisibility, bool);
  vtkBooleanMacro(BoundingBoxVisibility, bool);

  // Description:
  // Set the bounding box color
  vtkSetVector4Macro(BoundingBoxColor, double);

  // Description:
  // Shallow copy of a scalar bar actor. Overloads the virtual vtkProp method.
  void ShallowCopy(vtkProp *prop) override;

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
  vtkSetMacro(ReverseOrder, bool);
  vtkGetMacro(ReverseOrder, bool);
  vtkBooleanMacro(ReverseOrder, bool);

  typedef enum { VERTICAL_TEXT_ON_RIGHT,
                 VERTICAL_TEXT_ON_LEFT,
                 HORIZONTAL_TEXT_ON_TOP,
                 HORIZONTAL_TEXT_ON_BOTTOM
               } BarOrientation;

  vtkSetMacro(Orientation, BarOrientation);
  vtkGetMacro(Orientation, BarOrientation);

  // Description:
  // Enable/Disable inclusion of min and max in tick values.
  vtkSetMacro(MinMaxInclusive, bool);
  vtkGetMacro(MinMaxInclusive, bool);
  vtkBooleanMacro(MinMaxInclusive, bool);

  // Description:
  // Enable/Disable use of user-supplied labels.
  vtkSetMacro(UseSuppliedLabels, bool);
  vtkGetMacro(UseSuppliedLabels, bool);
  vtkBooleanMacro(UseSuppliedLabels, bool);

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
  void GetTextActorSize(vtkViewport *viewport, vtkTextActor *text, int size[2]);

  bool   ShouldCollapseDiscrete(void);

  vtkLookupTable *LookupTable;
  int   MaximumNumberOfColors;
  int   NumberOfLabels;
  int   NumberOfLabelsBuilt;
  char  *Title;

  bool   Bold;
  bool   Italic;
  bool   Shadow;
  int   FontFamily;
  double FontHeight;
  char  *LabelFormat;
  char  *RangeFormat;
  vtkCoordinate *Position2Coordinate;

  bool TitleVisibility;
  bool RangeVisibility;
  bool ColorBarVisibility;
  bool ReverseOrder;
  bool BoundingBoxVisibility;
  BarOrientation Orientation;

  double BarWidth;

  vtkPolyData         *ColorBar;
  vtkPolyDataMapper2D *ColorBarMapper;
  vtkActor2D          *ColorBarActor;

  vtkTextActor  *TitleActor;

  vtkTextActor  *RangeActor;

  vtkTextActor  **LabelActors;

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
  bool TitleOkayToDraw;
  bool LabelOkayToDraw;

  bool UseSkewScaling;
  bool UseLogScaling;
  double SkewFactor;
  stringVector definedLabels;
  doubleVector definedDoubleLabels;
  double *range;
  double *varRange;

  LevelColorMap labelColorMap;

  bool MinMaxInclusive;
  bool UseSuppliedLabels;
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

