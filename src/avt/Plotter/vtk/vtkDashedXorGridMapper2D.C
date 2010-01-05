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
#include <vtkDashedXorGridMapper2D.h>

#include <vtkActor2D.h>
#include <vtkCellArray.h>
#include <vtkCellLinks.h>
#include <vtkDataSet.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProperty2D.h>
#include <vtkScalarsToColors.h>
#include <vtkViewport.h>
#include <vtkWindow.h>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#ifdef VTK_USE_COCOA
#include <QLabel>
#include <QPixmap>
#include <QPainter>
#include <QLine>
#else
#include <Carbon/Carbon.h>
#endif
#elif defined(HAVE_XLIB)
#include <X11/Intrinsic.h>
#endif

#if defined(__APPLE__)
#ifndef VTK_USE_COCOA
struct vtkDashedXorGridMapper2DOverlay
{
    WindowRef    window;
    CGContextRef ctx;
};

// Create an overlay window.
static OSStatus CreateOverlayWindow( Rect* inBounds, WindowRef* outOverlayWindow)
{
    UInt32 flags = kWindowHideOnSuspendAttribute | kWindowIgnoreClicksAttribute;
    OSStatus err = CreateNewWindow( kOverlayWindowClass, flags, inBounds, outOverlayWindow);
    require_noerr(err, CreateNewWindowFAILED);
    ShowWindow( *outOverlayWindow );
    
CreateNewWindowFAILED:
    return err;
}
#endif
#else
struct vtkDashedXorGridMapper2DOverlay
{
    int placeholder;
};
#endif

vtkStandardNewMacro(vtkDashedXorGridMapper2D);

// ****************************************************************************
// Method: vtkDashedXorGridMapper2D::vtkDashedXorGridMapper2D
//
// Purpose: 
//   Constructor.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 13 10:04:25 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

vtkDashedXorGridMapper2D::vtkDashedXorGridMapper2D() : vtkPolyDataMapper2D()
{
    overlay = 0;
}

// ****************************************************************************
// Method: vtkDashedXorGridMapper2D::~vtkDashedXorGridMapper2D
//
// Purpose: 
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 13 10:04:03 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

vtkDashedXorGridMapper2D::~vtkDashedXorGridMapper2D()
{
    if(overlay != 0)
    {
#if defined(__APPLE__)
#ifndef VTK_USE_COCOA
        // Release the context that we used to draw on the window.
        QDEndCGContext(GetWindowPort(overlay->window), &overlay->ctx);

        // Destroy the transparent overlay window.
        DisposeWindow(overlay->window);
#endif
#endif
        delete overlay;
        overlay = 0;
    }
}

// ****************************************************************************
// Method: vtkDashedXorGridMapper2D::ReleaseGraphicsResources
//
// Purpose: 
//   Releases the mapper's graphics resources.
//
// Arguments:
//   win : The vtkWindow for which resources are released.
//
// Note:       The APPLE implementation frees a transparent overlay rendering
//             window.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 13 10:01:08 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
vtkDashedXorGridMapper2D::ReleaseGraphicsResources(vtkWindow *win)
{
    if(overlay != 0)
    {
#if defined(__APPLE__)
#ifndef VTK_USE_COCOA
        // Release the context that we used to draw on the window.
        QDEndCGContext(GetWindowPort(overlay->window), &overlay->ctx);

        // Destroy the transparent overlay window.
        DisposeWindow(overlay->window);
#endif
#endif
        delete overlay;
        overlay = 0;
    }

    // Call the superclass's ReleaseGraphicsResources method.
    vtkPolyDataMapper2D::ReleaseGraphicsResources(win);
}


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
//    Brad Whitlock, Mon Mar 13 11:22:50 PDT 2006
//    Added MacOS X implementation.
//
//    Hank Childs, Fri Jun  9 12:54:36 PDT 2006
//    Remove unused variable.
//
//    Gunther H. Weber, Mon Oct 26 10:55:30 PDT 2009
//    Use Qt to create the transparent overlay window on MacOS X if
//    VTK_USE_COACOA is set.
//
// ****************************************************************************

void vtkDashedXorGridMapper2D::RenderOverlay(vtkViewport* viewport, vtkActor2D* actor)
{
    vtkWindow*  window = viewport->GetVTKWindow();

#if defined(_WIN32)
//
// Win32 coding and macros
//
#define SET_FOREGROUND_D(rgba) if(validPen) DeleteObject(pen); \
      pen = CreatePen(PS_SOLID, 1, GetNearestColor(hdc, \
          RGB(int(255*rgba[0]),int(255*rgba[0]),int(255*rgba[0])))); \
      SelectObject(hdc, pen); \
      validPen = true;

#define SET_FOREGROUND(rgba) if(validPen) DeleteObject(pen); \
      pen = CreatePen(PS_SOLID, 1, GetNearestColor(hdc, RGB(rgba[0],rgba[0],rgba[0]))); \
      SelectObject(hdc, pen); \
      validPen = true;

#define DRAW_XOR_LINE(x1, y1, x2, y2) \
      MoveToEx(hdc, x1+borderL, y1+borderT, &oldPoint); \
      LineTo(hdc, x2+borderL, y2+borderT); \
      LineTo(hdc, x2+borderL+1, y2+borderT+1);

#define FLUSH_AND_SYNC() if(validPen) DeleteObject(pen);

#define CLEAN_UP() delete [] points;

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
    double whitergb[] = {1.,1.,1.};
    SET_FOREGROUND_D(whitergb);
#elif defined(__APPLE__)
#ifdef VTK_USE_COCOA
//
// Qt coding and macros
//

#define SET_FOREGROUND_D(rgba) \
    painter.setPen(QColor(255.*rgba[0], 255.*rgba[1], 255.*rgba[2]));

#define SET_FOREGROUND(rgba) \
    painter.setPen(QColor(255.*rgba[0], 255.*rgba[1], 255.*rgba[2]));

#define DRAW_XOR_LINE(x1, y1, x2, y2) \
    painter.drawLine(QLine(x1, y1, x2, y2));

#define FLUSH_AND_SYNC() \
    overlay->setPixmap(pixmap);

#define CLEAN_UP()

    // Get a pointer to the GL widget that the vtkQtRenderWindow owns.
    // Note that this only works because we've made the GenericDisplayId 
    // method return the GL widget pointer. On other platforms where 
    // the display is actually used for something, this does not work.
    typedef struct { int x,y,w,h; } OverlayInfo;
    OverlayInfo *info = (OverlayInfo *)window->GetGenericDisplayId();

    //
    // Try and create the window if we've not yet created it.
    //
    if(overlay == 0)
    {
        
        overlay = new QLabel(0, Qt::FramelessWindowHint);
        overlay->setAttribute(Qt::WA_TranslucentBackground);
        // FIXME? We should need the following, but it triggers a Qt
        // bug and  strangely everything seems to work without it.
        // overlay->setAttribute(Qt::WA_TransparentForMouseEvents);
        overlay->setAutoFillBackground(false);
        overlay->setGeometry(info->x, info->y, info->w, info->h);
        QPixmap pixmap(info->w, info->h);;
        pixmap.fill(Qt::transparent);
        overlay->setPixmap(pixmap);
        overlay->show();
    }

    // Clear the window so it's ready for us to draw.
    QPixmap pixmap(info->w, info->h);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
   
    // Set the line color
    double* actorColor = actor->GetProperty()->GetColor();
    SET_FOREGROUND_D(actorColor);

    // Free the info that we received from the vtkQt window.
    delete info;
#else
//
// MacOS X coding and macros
//

#define SET_FOREGROUND_D(rgba) \
    CGContextSetRGBStrokeColor(overlay->ctx, rgba[0], rgba[1], rgba[2], 1.);

#define SET_FOREGROUND(rgba) \
    CGContextSetRGBStrokeColor(overlay->ctx, rgba[0], rgba[1], rgba[2], 1.);

#define DRAW_XOR_LINE(x1, y1, x2, y2) \
    CGContextBeginPath(overlay->ctx);\
    CGContextMoveToPoint(overlay->ctx, x1,H-(y1));\
    CGContextAddLineToPoint(overlay->ctx, x2, H-(y2)); \
    CGContextStrokePath(overlay->ctx);

#define FLUSH_AND_SYNC() CGContextFlush(overlay->ctx);

#define CLEAN_UP()

    // Get a pointer to the GL widget that the vtkQtRenderWindow owns.
    // Note that this only works because we've made the GenericDisplayId 
    // method return the GL widget pointer. On other platforms where 
    // the display is actually used for something, this does not work.
    typedef struct { int x,y,w,h; } OverlayInfo;
    OverlayInfo *info = (OverlayInfo *)window->GetGenericDisplayId();
    int H = info->h;

    //
    // Try and create the window if we've not yet created it.
    //
    if(overlay == 0)
    {
        // Get the GL widget's global screen coordinates.
        Rect wRect;
        wRect.left = info->x;
        wRect.right = info->x + info->w;
        wRect.top = info->y;
        wRect.bottom = info->y + info->h;
        
        // Try and create the overlay window.        
        overlay = new vtkDashedXorGridMapper2DOverlay;
        OSStatus err = CreateOverlayWindow(&wRect, &overlay->window);
        if (err == noErr)
        {
            // Create the Quartz context that we'll use to draw on the
            // overlay window.
            QDBeginCGContext(GetWindowPort(overlay->window), &overlay->ctx);
        }
        else
        {
            // We could not create the overlay window so delete the
            // overlay object.
            delete overlay;
            overlay = 0;
            return;
        }
    }
    
    // Set the line color
    double* actorColor = actor->GetProperty()->GetColor();
    SET_FOREGROUND_D(actorColor);

    // Clear the window so it's ready for us to draw.
    CGRect r;
    r.origin.x = 0;
    r.origin.y = 0;
    r.size.width = info->w;
    r.size.height = info->h;
    CGContextClearRect(overlay->ctx, r);
#endif
#elif defined(HAVE_XLIB)
//
// X11 coding and macros
//

#define SET_FOREGROUND_D(rgba) \
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

#define DRAW_XOR_LINE(x1, y1, x2, y2) \
      XDrawLine(displayId, drawable, xorGC, x1, y1, x2, y2);

#define FLUSH_AND_SYNC() XFlush(displayId); XSync(displayId, False); \
      XFreeGC(displayId, gc);

#define CLEAN_UP() delete [] points;

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
    double* actorColor = actor->GetProperty()->GetColor();
    SET_FOREGROUND_D(actorColor);
#endif

#if defined(_WIN32) || defined(__APPLE__) || defined(HAVE_XLIB)
    int numPts;
    vtkPolyData *input= vtkPolyData::SafeDownCast(this->GetInput());
    vtkIdType npts, *pts;
    int j;
    vtkPoints *p, *displayPts;
    vtkCellArray *aPrim;
    vtkUnsignedCharArray *c=NULL;
    unsigned char *rgba;
    double *ftmp;
    int cellScalars = 0;
    int cellNum = 0;
    int lastX, lastY, X, Y; 
 
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
    CLEAN_UP();
    if ( this->TransformCoordinate )
        p->Delete();
#endif
}
 
