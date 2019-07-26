// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef __vtkAxesActor2D_h
#define __vtkAxesActor2D_h

#include <plotter_exports.h>

#include <vtkActor2D.h>
#include "vtkVisItAxisActor2D.h"


// ****************************************************************************
// Class: vtkAxesActor2D
//
// Purpose:
//   Class to manage x and y axis actors that annotate a 2d plot.
//
// Notes:
//   Access to individual axis is granted for ease of direct modification. 
//
// Programmer: Kathleen Biagas
// Creation:   January 8, 2014
//
// Modifications:
//   
// ****************************************************************************



class PLOTTER_API vtkAxesActor2D : public vtkActor2D
{
public:
  vtkTypeMacro(vtkAxesActor2D,vtkActor2D);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Instantiate object.
  static vtkAxesActor2D *New();
  
  // Description:
  // Draw the axes. 
  int RenderOverlay(vtkViewport* viewport) override;
  int RenderOpaqueGeometry(vtkViewport* viewport) override;
  int RenderTranslucentPolygonalGeometry(vtkViewport *) override {return 0;}
  int HasTranslucentPolygonalGeometry() override {return 0;}

  // Description:
  // Release any graphics resources that are being consumed by this actor.
  // The parameter window could be used to determine which graphic
  // resources to release.
  void ReleaseGraphicsResources(vtkWindow *) override;

  // GENERAL
  //

  // Description:
  // Sets axis coordinates values from passed viewport values.
  void SetCoordinateValuesFromViewport(double vl, double vb, double vr, double vt);

  // Description:
  // Set the line width
  void SetLineWidth(int width);

  // Description:
  // Set the color for both axes
  void SetColor(double r, double g, double b);

  // Description:
  // Sets the log scaling for the axes.
  void SetXLogScaling(bool logScale);
  void SetYLogScaling(bool logScale);


  // LABELS
  //  

  // Description:
  // Sets the label ranges for the axes.
  void SetXRange(double minX, double maxX);
  void SetYRange(double minY, double maxY);

  // Description:
  // Sets the label visibility
  void SetXLabelVisibility(bool vis);
  void SetYLabelVisibility(bool vis);

  // Description:
  // Sets the label scaling for the axes.
  void SetXLabelScale(double labelScale);
  void SetYLabelScale(double labelScale);

  // Description:
  // Sets the label format string
  void SetXLabelFormatString(char *format);
  void SetYLabelFormatString(char *format);

  // Description:
  // Sets the label format string for log scaling
  void SetXLogLabelFormatString(char *format);
  void SetYLogLabelFormatString(char *format);

  // Description:
  // Sets the label font height
  void SetXLabelFontHeight(double height);
  void SetYLabelFontHeight(double height);  
  
  // Description:
  // Sets the flag specifying if labels are auto adjusted.
  void SetAutoAdjustLabels(bool autoAdjust);

  // Description:
  // Retrieve the text property for labels
  vtkTextProperty * GetXLabelTextProperty(void);
  vtkTextProperty * GetYLabelTextProperty(void);


  // TITLES
  //  

  // Description:
  // Sets the Title
  void SetXTitle(char *title);  
  void SetYTitle(char *title);

  // Description:
  // Sets the title font height
  void SetXTitleFontHeight(double height);
  void SetYTitleFontHeight(double height);
  
  // Description:
  // Sets the title visibility
  void SetXTitleVisibility(bool vis);
  void SetYTitleVisibility(bool vis);

  // Description:
  // Retrieve the text property for titles
  vtkTextProperty * GetXTitleTextProperty(void);
  vtkTextProperty * GetYTitleTextProperty(void);

  // TICKS
  //  

  // Description:
  // Set the location of tick marks.
  void SetTickLocation(int loc);

  // Description:
  // Sets the visibility of ticks.
  void SetXMinorTickVisibility(bool vis);
  void SetXMajorTickVisibility(bool vis);
  void SetYMinorTickVisibility(bool vis);
  void SetYMajorTickVisibility(bool vis);

  // Description:
  // Sets the minimum value Major Ticks
  void SetXMajorTickMinimum(double minV);
  void SetYMajorTickMinimum(double minV);

  // Description:
  // Sets the maximum value Major Ticks
  void SetXMajorTickMaximum(double maxV);
  void SetYMajorTickMaximum(double maxV);

  // Description:
  // Sets the spacing for major ticks
  void SetXMajorTickSpacing(double spacing);
  void SetYMajorTickSpacing(double spacing);

  // Description:
  // Sets the spacing for minor ticks
  void SetXMinorTickSpacing(double spacing);
  void SetYMinorTickSpacing(double spacing);

  // GRID
  //  

  // Description:
  // Sets the title visibility
  void SetXGridVisibility(bool vis);
  void SetYGridVisibility(bool vis);


  // Description:
  // Return this object's MTime.
  vtkMTimeType GetMTime() override;
   
protected:
  vtkAxesActor2D();
  ~vtkAxesActor2D();

  vtkVisItAxisActor2D *XAxis;
  vtkVisItAxisActor2D *YAxis;
  
private:
  vtkAxesActor2D(const vtkAxesActor2D&);
  void operator=(const vtkAxesActor2D&);
};


#endif
