/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkDashedXorGridMapper2D.cxx,v $
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
#include "vtkDashedXorGridMapper2D.h"
#include "vtkObjectFactory.h"
#include <vtkPolyData.h>
#include <vtkProperty2D.h>
#include <vtkScalarsToColors.h>
#include <vtkViewport.h>

#if defined(_WIN32)
#include <windows.h>
#else
#include <X11/Intrinsic.h>
#endif

vtkStandardNewMacro(vtkDashedXorGridMapper2D);

void
vtkDashedXorGridMapper2D::SetDots(int drawn, int spaced)
{
    pixelDrawn = drawn;
    pixelSpaced = spaced;
}

// ****************************************************************************
// Modifications:
//    Brad Whitlock, Tue Jul 15 10:55:17 PDT 2003
//    Fixed offset applied to Windows lines so they are not drawn in the
//    wrong location anymore. The offset used to be correct, but at some point,
//    it broke so now I'm making it so no offset is used.
//   
// ****************************************************************************

void vtkDashedXorGridMapper2D::RenderOverlay(vtkViewport* viewport, vtkActor2D* actor)
{
    vtkWindow*  window = viewport->GetVTKWindow();

#if defined(_WIN32)
//
// Win32 coding and macros
//
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
#else
//
// X11 coding and macros
//

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
 
    vtkDebugMacro (<< "vtkDashedXorGridMapper2D::RenderOverlay");

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
 
    if (pixelDrawn <= 0 || pixelSpaced <= 0)
    {
        vtkDebugMacro(<< "Bad settings. Try SetDots first.");
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

            int delta = pixelDrawn + pixelSpaced;
            // Divide the two cases.
            
            bool horizontal;
            
            // If we're asked for a point
            if (X == lastX && Y == lastY)
                horizontal = horizontalBias;
            else
                horizontal = (Y == lastY);
            
            // Horizontal line
            if (horizontal)
            {
                // Ensure we're drawing left to right
                if (X > lastX)
                {
                    int tmp = X;
                    X = lastX;
                    lastX = tmp;
                }
                
                int nextX;
                
                // Three cases for first dashed line
                // If we're on a white space, we draw nothing, and advance
                // to the next state.
                if (!IsDash(X))
                {
                    X = NextDash(X);
                }
                // If we're in the middle of a dash
                else if (!IsBeginningDash(X))
                {
                    nextX = NextDash(X);
                    nextX -= (pixelSpaced);
                    // Special case: if there's only part of a dash.
                    if (nextX > lastX)
                    {
                        // Draw the line segment to the end of line.
                        DRAW_XOR_LINE( X, Y, lastX, Y);
                    }
                    else
                    {
                        // Draw the segment
                        DRAW_XOR_LINE( X, Y, nextX, Y);
                    }
                    // Advance X to the next dash
                    X = nextX + pixelSpaced;
                }
                // If we're at the beginning of a dash, we're fine.

                nextX = X + pixelDrawn;

                for (;;)
                {
                    // If X and nextX strattle lastX, draw a final segment.
                    // End loop.
                    if ( X <= lastX && nextX >= lastX)
                    {
                        DRAW_XOR_LINE(X, Y, lastX, Y);
                        break;
                    }

                    // If X is past lastX, End Loop.
                    if ( X > lastX)
                        break;

                    // Draw the next dash
                    DRAW_XOR_LINE(X, Y, nextX, Y);
                    X += delta;
                    nextX += delta;
                }
            }

            // Vertical line
            else
            {
                // Ensure we're drawing down to up
                if (Y > lastY)
                {
                    int tmp = Y;
                    Y = lastY;
                    lastY = tmp;
                }
                
                int nextY;
                
                // Three cases for first dashed line
                // If we're on a white space, we draw nothing, and advance
                // to the next state.
                if (!IsDash(Y))
                {
                    Y = NextDash(Y);
                }
                // If we're in the middle of a dash
                else if (!IsBeginningDash(Y))
                {
                    nextY = NextDash(Y);
                    nextY -= (pixelSpaced);
                    // Special case: if there's only part of a dash.
                    if (nextY > lastY)
                    {
                        // Draw the line segment to the end of line.
                        DRAW_XOR_LINE( X, Y, X, lastY);
                    }
                    else
                    {
                        // Draw the segment
                        DRAW_XOR_LINE( X, Y, X, nextY);
                    }
                    // Advance Y to the next dash
                    Y = nextY + pixelSpaced;
                }
                // If we're at the beginning of a dash, we're fine.

                nextY = Y + pixelDrawn;
                
                for (;;)
                {
                    // If Y and nextY strattle lastY, draw a final segment.
                    // End loop.
                    if ( Y <= lastY && nextY >= lastY)
                    {
                        DRAW_XOR_LINE(X, Y, X, lastY);
                        break;
                    }

                    // If Y is past lastY, End Loop.
                    if ( Y > lastY)
                        break;

                    // Draw the next dash
                    DRAW_XOR_LINE(X, Y, X, nextY);
                    Y += delta;
                    nextY += delta;
                }
            }
        }
    }

    // Finish drawing.
    FLUSH_AND_SYNC();

    // Clean up.
    delete [] points;
    if ( this->TransformCoordinate )
        p->Delete();
}
 
