/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkRubberBandMapper.cxx,v $
  Language:  C++
  Date:      $Date: 2000/02/04 17:09:14 $
  Version:   $Revision: 1.12 $


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
#include <stdlib.h>
#include <math.h>
#include "vtkRubberBandMapper2D.h"
#include "vtkObjectFactory.h"
#include <vtkPolyData.h>
#include <vtkProperty2D.h>
#include <vtkScalarsToColors.h>
#include <vtkViewport.h>
#include <vtkActor2D.h>
#include <vtkPointData.h>
#include <vtkCellArray.h>
#include <vtkCoordinate.h>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
int placeholder;
#else
#include <X11/Intrinsic.h>
#endif

// ***************************************************************************
//  Modifications:
//    Kathleen Bonnell, Wed Mar  6 15:14:29 PST 2002 
//    Replace 'New' method with Macro to match VTK 4.0 API.
// ***************************************************************************

vtkStandardNewMacro(vtkRubberBandMapper2D);


// ***************************************************************************
//  Modifications:
//
//    Kathleen Bonnell, Mon Oct 29 13:22:36 PST 2001
//    Make npts, pts of type vtkIdType to match VTK 4.0 API.  
//    Replace vtkScalars with vtkUnsignedCharArray to represent colors, and
//    use correct access methods for this type of array.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    GetColors() no longer a mapper method in VTK 4.0 API.  Use MapScalars
//    instead. 
//
//    Brad Whitlock, Wed Apr 3 10:41:05 PDT 2002
//    Added a Windows implementation.
//
//    Akira Haddox, Tue Mar 18 21:17:05 PST 2003
//    Fixed bug in X11 DRAW_XOR_LINE: Macro now uses arguments instead of
//    ignoring them.
//
//    Brad Whitlock, Tue Jul 15 10:55:17 PDT 2003
//    Fixed offset applied to Windows lines so they are not drawn in the
//    wrong location anymore. The offset used to be correct, but at some point,
//    it broke so now I'm making it so no offset is used.
//
// ****************************************************************************

void vtkRubberBandMapper2D::RenderOverlay(vtkViewport* viewport, vtkActor2D* actor)
{
    vtkWindow*  window = viewport->GetVTKWindow();

#if defined(_WIN32)
// ****************************************************************************
//
// Win32 coding and macros
//
// ****************************************************************************
#define STORE_POINT(P, X, Y) P.x = long((X) + borderL);  P.y = long((Y)+borderT);

#define SET_FOREGROUND_F(rgba) if(validPen) DeleteObject(pen); \
      pen = CreatePen(PS_SOLID, 1, GetNearestColor(hdc, \
          RGB(int(255*rgba[0]),int(255*rgba[0]),int(255*rgba[0])))); \
      SelectObject(hdc, pen); \
      validPen = true;

#define SET_FOREGROUND(rgba) if(validPen) DeleteObject(pen); \
      pen = CreatePen(PS_SOLID, 1, GetNearestColor(hdc, RGB(rgba[0],rgba[0],rgba[0]))); \
      SelectObject(hdc, pen); \
      validPen = true;

#define DRAW_POLYGON(points, npts) Polygon(hdc, points, npts)

#define RESIZE_POINT_ARRAY(points, npts, currSize) \
      if (npts > currSize) \
      { \
      delete [] points; \
      points = new POINT[npts]; \
      currSize = npts; \
      }

#define DRAW_XOR_LINE(x1, y1, x2, y2) \
      MoveToEx(hdc, x1+borderL, y1+borderT, &oldPoint); \
      LineTo(hdc, x2+borderL, y2+borderT); \
      LineTo(hdc, x2+borderL+1, y2+borderT+1);

#define FLUSH_AND_SYNC() if(validPen) DeleteObject(pen);

    HPEN pen = 0;
    bool validPen = false;
    POINT *points = new POINT[1024];
    POINT oldPoint;
    HDC hdc = GetWindowDC((HWND)window->GetGenericWindowId());
    int borderT = 0; //GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CYSIZE);
    int borderL = 0; //GetSystemMetrics(SM_CXFRAME);

    // Make the drawing mode be XOR.
    SetROP2(hdc, R2_XORPEN);

    // Set the line color
    float whitergb[] = {1.,1.,1.};
    SET_FOREGROUND_F(whitergb);
#elif defined(__APPLE__)
// ***************************************************************************
//
// MacOS X Darwin coding and macros
//
// ***************************************************************************

// for now...
int *points = new int[1024];

#define STORE_POINT(P, X, Y) cerr << "STORE_POINT macro for Mac." << endl;
#define SET_FOREGROUND_F(rgba) cerr << "SET_FOREGROUND_F macro for Mac." << endl;
#define SET_FOREGROUND(rgba) cerr << "SET_FOREGROUND macro for Mac." << endl;
#define DRAW_POLYGON(points, npts) cerr << "DRAW_POLYGON macro for Mac." << endl;
#define RESIZE_POINT_ARRAY(points, npts, currSize) cerr << "RESIZE_POINT_ARRAY macro for Mac." << endl;
#define DRAW_XOR_LINE(x1, y1, x2, y2) cerr << "DRAW_XOR_LINE macro for Mac." << endl;
#define FLUSH_AND_SYNC() cerr << "FLUSH_AND_SYNC macro for Mac." << endl;


#else
// ***************************************************************************
//
// X11 coding and macros
//
// ***************************************************************************

#define STORE_POINT(P, X, Y) P.x = short(X); P.y = short(Y);

#define SET_FOREGROUND_F(rgba) \
      aColor.red = (unsigned short) (rgba[0] * 65535.0); \
      aColor.green = (unsigned short) (rgba[1] * 65535.0); \
      aColor.blue = (unsigned short) (rgba[2] * 65535.0); \
      XAllocColor(displayId, attr.colormap, &aColor); \
      XSetForeground(displayId, gc, aColor.pixel); \
      XSetFillStyle(displayId, gc, FillSolid);

#define SET_FOREGROUND(rgba) \
      aColor.red = (unsigned short) (rgba[0] * 256); \
      aColor.green = (unsigned short) (rgba[1] * 256); \
      aColor.blue = (unsigned short) (rgba[2] * 256); \
      XAllocColor(displayId, attr.colormap, &aColor); \
      XSetForeground(displayId, gc, aColor.pixel);

#define DRAW_POLYGON(points, npts) XFillPolygon(displayId, drawable, \
      gc, points, npts, Complex, CoordModeOrigin);

#define RESIZE_POINT_ARRAY(points, npts, currSize) \
      if (npts > currSize) \
      { \
      delete [] points; \
      points = new XPoint [npts]; \
      currSize = npts; \
      }

#define DRAW_XOR_LINE(x1, y1, x2, y2) \
      XDrawLine(displayId, drawable, xorGC, x1, y1, x2, y2);

#define FLUSH_AND_SYNC() XFlush(displayId); XSync(displayId, False); \
      XFreeGC(displayId, gc);

    XColor aColor;
    XPoint *points = new XPoint [1024];

    Display* displayId = (Display*) window->GetGenericDisplayId();
    Window windowId = (Window) window->GetGenericWindowId();

    Screen *screen = XDefaultScreenOfDisplay(displayId);
    int screenN = XScreenNumberOfScreen(screen);
    unsigned long black = BlackPixel(displayId, screenN);
    unsigned long white = WhitePixel(displayId, screenN);
    XGCValues xgcvalues;
    xgcvalues.foreground = black ^ white;
    xgcvalues.background = 0;
    xgcvalues.function = GXxor;
    GC gc = XCreateGC(displayId, windowId, GCForeground | GCBackground | GCFunction,
                      &xgcvalues);
    GC xorGC = XCreateGC(displayId, windowId, GCForeground | GCBackground | GCFunction,
                      &xgcvalues);

    // Get the drawable to draw into
    Drawable drawable = (Drawable) window->GetGenericDrawable();
    if (!drawable) vtkErrorMacro(<<"Window returned NULL drawable!");
  
    // Set up the forground color
    XWindowAttributes attr;
    XGetWindowAttributes(displayId,windowId,&attr);

    // Set the line color
    float* actorColor = actor->GetProperty()->GetColor();
    SET_FOREGROUND_F(actorColor);
#endif

    int numPts;
    vtkPolyData *input= (vtkPolyData *)this->Input;
    vtkIdType npts, *pts;
    int j;
    vtkPoints *p, *displayPts;
    vtkCellArray *aPrim;
    vtkUnsignedCharArray *c=NULL;
    unsigned char *rgba;
    float *ftmp;
    int cellScalars = 0;
    int cellNum = 0;
    int lastX, lastY, X, Y; 
    int currSize = 1024;
 
    vtkDebugMacro (<< "vtkRubberBandMapper2D::RenderOverlay");

    if ( input == NULL ) 
    {
        vtkErrorMacro(<< "No input!");
        return;
    }
    else
    {
        input->Update();
        numPts = input->GetNumberOfPoints();
    } 

    if (numPts == 0)
    {
        vtkDebugMacro(<< "No points!");
        return;
    }
  
    if ( this->LookupTable == NULL )
    {
        this->CreateDefaultLookupTable();
    }

    //
    // if something has changed regenrate colors and display lists
    // if required
    //
    if ( this->GetMTime() > this->BuildTime || 
         input->GetMTime() > this->BuildTime || 
         this->LookupTable->GetMTime() > this->BuildTime ||
         actor->GetProperty()->GetMTime() > this->BuildTime)
    {
         // sets this->Colors as side effect
        this->MapScalars(1.0);
        this->BuildTime.Modified();
    }
  
    // Get the position of the text actor
    int* actorPos = 
        actor->GetPositionCoordinate()->GetComputedLocalDisplayValue(viewport);

    // Transform the points, if necessary
    p = input->GetPoints();
    if ( this->TransformCoordinate )
    {
        int *itmp;
        numPts = p->GetNumberOfPoints();
        displayPts = vtkPoints::New();
        displayPts->SetNumberOfPoints(numPts);
        for ( j=0; j < numPts; j++ )
        {
            this->TransformCoordinate->SetValue(p->GetPoint(j));
            itmp = this->TransformCoordinate->GetComputedDisplayValue(viewport);
            displayPts->SetPoint(j, itmp[0], itmp[1], 0.0);
        }
        p = displayPts;
    }

    // Get colors
    if ( this->Colors )
    {
        c = this->Colors;
        if (!input->GetPointData()->GetScalars())
        {
            cellScalars = 1;
        }
    }

    // Draw the polygons.
    aPrim = input->GetPolys();
    for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); cellNum++)
    { 
        if (c) 
        {
            if (cellScalars) 
                rgba = c->GetPointer(4*cellNum);
            else
                rgba = c->GetPointer(4*pts[j]);

            SET_FOREGROUND(rgba);
        }

        RESIZE_POINT_ARRAY(points, npts, currSize);

        for (j = 0; j < npts; j++) 
        {
            ftmp = p->GetPoint(pts[j]);
            STORE_POINT(points[j],
                        actorPos[0] + ftmp[0],
                        actorPos[1] - ftmp[1]);
        }

        DRAW_POLYGON(points, npts);
    }

    // Draw the lines.
    aPrim = input->GetLines();
    for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); cellNum++)
    { 
        if (c && cellScalars) 
        {
            rgba = c->GetPointer(4*cellNum);
            SET_FOREGROUND(rgba);
        }
        ftmp = p->GetPoint(pts[0]);

        lastX = (int)(actorPos[0] + ftmp[0]);
        lastY = (int)(actorPos[1] - ftmp[1]);

        for (j = 1; j < npts; j++) 
        {
            ftmp = p->GetPoint(pts[j]);
            if (c && !cellScalars)
            {
                rgba = c->GetPointer(4*pts[j]);
                SET_FOREGROUND(rgba)
            }
            X = (int)(actorPos[0] + ftmp[0]);
            Y = (int)(actorPos[1] - ftmp[1]);

            DRAW_XOR_LINE(lastX, lastY, X, Y);

            lastX = X;
            lastY = Y;
        }
    }

    // Finish drawing.
    FLUSH_AND_SYNC();

    // Clean up.
    delete [] points;
    if ( this->TransformCoordinate )
        p->Delete();

}


  
