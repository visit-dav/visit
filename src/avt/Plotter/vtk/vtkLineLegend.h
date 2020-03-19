// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef __vtkLineLegend_h
#define __vtkLineLegend_h
#include <plotter_exports.h>

#include <vtkActor2D.h>
#include <vtkTextMapper.h>

class vtkPolyData;
class vtkPolyDataMapper2D;
class vtkTransform;
class vtkTransformPolyDataFilter;


class PLOTTER_API vtkLineLegend : public vtkActor2D
{
public:
  vtkTypeMacro(vtkLineLegend,vtkActor2D);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Instantiate object.
  static vtkLineLegend *New();

  // Description:
  // Access the Position instance variable. Reimplemented from base
  // class to ensure normalized viewport coordinates
  // This variable controls the lower left corner of the legend.
  void SetPosition(double,double) override;
  void SetPosition(double x[2]) override;

  // Description:
  // Access the Position2 instance variable. This variable controls
  // the upper right corner of the legend. It is by default
  // relative to Position1 and in Normalized Viewport coordinates.
  void SetPosition2(double,double) override;
  void SetPosition2(double x[2]) override;
  vtkCoordinate *GetPosition2Coordinate() override;
  double *GetPosition2() override;

  // Description:
  // Draw the legend and annotation text to the screen.
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
  // Set/Get the title of the scalar bar actor,
  vtkSetStringMacro(Title);
  vtkGetStringMacro(Title);

  // Description:
  // Set/Get the visibility of the title annotation text.
  vtkSetMacro(TitleVisibility, bool);
  vtkGetMacro(TitleVisibility, bool);
  vtkBooleanMacro(TitleVisibility, bool);

  vtkTextProperty * GetTitleProperty()
     { return this->TitleMapper->GetTextProperty();};

  vtkProperty2D * GetLineProperty()
     { return this->LineActor->GetProperty();};

  // Description:
  // Set/Get the scalar bar width.
  vtkSetClampMacro(BarWidth,double, 0.0, 0.5);
  vtkGetMacro(BarWidth,double);

  // Description:
  // Set/Get the visibility of the bounding box.
  vtkSetMacro(BoundingBoxVisibility, bool);
  vtkGetMacro(BoundingBoxVisibility, bool);
  vtkBooleanMacro(BoundingBoxVisibility, bool);

  // Description:
  // Set the bounding box color
  vtkSetVector4Macro(BoundingBoxColor, double);

  // Shallow copy of a scalar bar actor. Overloads the virtual vtkProp method.
  void ShallowCopy(vtkProp *prop) override;

protected:
  vtkLineLegend();
  virtual ~vtkLineLegend();

  void BuildTitle(vtkViewport *);
  void BuildLine(vtkViewport *);
  void BuildBoundingBox(vtkViewport *);

  bool   Bold;
  bool   Italic;
  bool   Shadow;
  int   FontFamily;
  double FontHeight;

  vtkCoordinate *Position2Coordinate;

  double BarWidth;

  char          *Title;
  vtkTextMapper *TitleMapper;
  vtkActor2D    *TitleActor;
  bool            TitleVisibility;
  bool            TitleOkayToDraw;
  bool            BoundingBoxVisibility;

  vtkPolyData                *Line;
  vtkPolyDataMapper2D        *LineMapper;
  vtkActor2D                 *LineActor;

  vtkPolyData                *BoundingBox;
  vtkPolyDataMapper2D        *BoundingBoxMapper;
  vtkActor2D                 *BoundingBoxActor;
  double                      BoundingBoxColor[4];

  vtkTimeStamp  BuildTime;
  int LastSize[2];
  int LastOrigin[2];

private:
  vtkLineLegend(const vtkLineLegend&);
  void operator=(const vtkLineLegend&);
};


#endif

