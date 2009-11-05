/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Instantiate object. 
  static vtkLineLegend *New();
  
  // Description:
  // Access the Position instance variable. Reimplemented from base
  // class to ensure normalized viewport coordinates
  // This variable controls the lower left corner of the legend. 
  void SetPosition(double,double);
  void SetPosition(double x[2]);

  // Description:
  // Access the Position2 instance variable. This variable controls
  // the upper right corner of the legend. It is by default
  // relative to Position1 and in Normalized Viewport coordinates.
  void SetPosition2(double,double);
  void SetPosition2(double x[2]);
  vtkCoordinate *GetPosition2Coordinate();
  double *GetPosition2();
 
  // Description:
  // Draw the legend and annotation text to the screen.
  int RenderOpaqueGeometry(vtkViewport* viewport);
  int RenderTranslucentGeometry(vtkViewport*) { return 0; };
  virtual int RenderOverlay(vtkViewport* viewport);

  // Description:
  // Release any graphics resources that are being consumed by this actor.
  // The parameter window could be used to determine which graphic
  // resources to release.
  virtual void ReleaseGraphicsResources(vtkWindow *);

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
  // Set/Get the title of the scalar bar actor,
  vtkSetStringMacro(Title);
  vtkGetStringMacro(Title);

  // Description:
  // Set/Get the visibility of the title annotation text. 
  vtkSetMacro(TitleVisibility, int);
  vtkGetMacro(TitleVisibility, int);
  vtkBooleanMacro(TitleVisibility, int);

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
  vtkSetMacro(BoundingBoxVisibility, int);
  vtkGetMacro(BoundingBoxVisibility, int);
  vtkBooleanMacro(BoundingBoxVisibility, int);

  // Description:
  // Set the bounding box color
  vtkSetVector4Macro(BoundingBoxColor, double);

  // Shallow copy of a scalar bar actor. Overloads the virtual vtkProp method.
  void ShallowCopy(vtkProp *prop);

protected:
  vtkLineLegend();
  virtual ~vtkLineLegend();

  void BuildTitle(vtkViewport *);
  void BuildLine(vtkViewport *);
  void BuildBoundingBox(vtkViewport *);

  int   Bold;
  int   Italic;
  int   Shadow;
  int   FontFamily;
  double FontHeight;

  vtkCoordinate *Position2Coordinate;

  double BarWidth;
  
  char          *Title;
  vtkTextMapper *TitleMapper;
  vtkActor2D    *TitleActor;
  int            TitleVisibility;
  int            TitleOkayToDraw;
  int            BoundingBoxVisibility;

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

