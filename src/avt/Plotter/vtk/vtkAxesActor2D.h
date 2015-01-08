/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Instantiate object.
  static vtkAxesActor2D *New();
  
  // Description:
  // Draw the axes. 
  int RenderOverlay(vtkViewport* viewport);
  int RenderOpaqueGeometry(vtkViewport* viewport);
  int RenderTranslucentPolygonalGeometry(vtkViewport *) {return 0;}
  int HasTranslucentPolygonalGeometry() {return 0;}

  // Description:
  // Release any graphics resources that are being consumed by this actor.
  // The parameter window could be used to determine which graphic
  // resources to release.
  void ReleaseGraphicsResources(vtkWindow *);

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
  void SetXLogScaling(int logScale);
  void SetYLogScaling(int logScale);


  // LABELS
  //  

  // Description:
  // Sets the label ranges for the axes.
  void SetXRange(double minX, double maxX);
  void SetYRange(double minY, double maxY);

  // Description:
  // Sets the label visibility
  void SetXLabelVisibility(int vis);
  void SetYLabelVisibility(int vis);

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
  void SetAutoAdjustLabels(int autoAdjust);

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
  void SetXTitleVisibility(int vis);
  void SetYTitleVisibility(int vis);

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
  void SetXMinorTickVisibility(int vis);
  void SetXMajorTickVisibility(int vis);
  void SetYMinorTickVisibility(int vis);
  void SetYMajorTickVisibility(int vis);

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
  void SetXGridVisibility(int vis);
  void SetYGridVisibility(int vis);


  // Description:
  // Return this object's MTime.
  unsigned long GetMTime(); 
   
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
