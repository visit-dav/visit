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

#include <vtkActor2D.h>
#include <vtkCellArray.h>
#include <vtkCoordinate.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkProperty2D.h>
#include <vtkScalarsToColors.h>
#include <vtkViewport.h>
#include <vtkWindow.h>

#if defined(_WIN32)
#  include <windows.h>
#else
#  include <QLabel>
#  include <QPixmap>
#  include <QPainter>
#  include <QLine>
#  if defined(HAVE_XLIB)
#    include <QX11Info>
#    include <X11/Intrinsic.h>
#  endif
#endif

struct vtkRubberBandMapper2DPrivate
{
    QWidget *widget;
    int      bestRenderer;
    QLabel  *overlay;

    vtkRubberBandMapper2DPrivate()
    {
        widget = 0;
        bestRenderer = -1;
        overlay = 0;
    }

    ~vtkRubberBandMapper2DPrivate()
    { 
        ReleaseGraphicsResources();
    }

    void ReleaseGraphicsResources()
    {
        if(overlay != 0)
            delete overlay;
        overlay = 0;
    }

    int SelectBestRenderer()
    {
        if(bestRenderer != -1)
            return bestRenderer;

#if defined(_WIN32)
        bestRenderer = 0;
#elif defined(__APPLE__)
        bestRenderer = 2;
#elif defined(HAVE_XLIB)
        bestRenderer = 1;
#if 0
// X is not creating the Qt overlay as transparent. Disable for now.
        // See if we're displaying to Apple X11. If so we want Qt renderer.
        int nExt = 0, appleDisplay = 0;
        char **ext = XListExtensions(QX11Info::display(), &nExt);
        for(int e = 0; e < nExt; ++e)
        {
            if(strcmp(ext[e], "Apple-DRI") == 0 ||
               strcmp(ext[e], "Apple-WM") == 0)
            {
                appleDisplay++;
            }
        }
        XFreeExtensionList(ext);
        if(appleDisplay == 2)
            bestRenderer = 2;
#endif
#else
        bestRenderer = 2;
#endif

        return bestRenderer;
    }
};

// ***************************************************************************
//  Modifications:
//    Kathleen Bonnell, Wed Mar  6 15:14:29 PST 2002 
//    Replace 'New' method with Macro to match VTK 4.0 API.
// ***************************************************************************

vtkStandardNewMacro(vtkRubberBandMapper2D);

// ****************************************************************************
// Method: vtkRubberBandMapper2D::vtkRubberBandMapper2D
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

vtkRubberBandMapper2D::vtkRubberBandMapper2D() : vtkPolyDataMapper2D()
{
    d = new vtkRubberBandMapper2DPrivate;
}

// ****************************************************************************
// Method: vtkRubberBandMapper2D::~vtkRubberBandMapper2D
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

vtkRubberBandMapper2D::~vtkRubberBandMapper2D()
{
    if(d != 0)
    {
        delete d;
        d = 0;
    }
}

// ****************************************************************************
// Method: vtkRubberBandMapper2D::ReleaseGraphicsResources
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
vtkRubberBandMapper2D::ReleaseGraphicsResources(vtkWindow *win)
{
    d->ReleaseGraphicsResources();

    // Call the superclass's ReleaseGraphicsResources method.
    vtkPolyDataMapper2D::ReleaseGraphicsResources(win);
}

// ****************************************************************************
// Method: vtkRubberBandMapper2D::SetWidget
//
// Purpose: 
//   Set the widget that we want to draw over.
//
// Arguments:
//   w : The widget that we want to draw over.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 14 14:27:04 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

void
vtkRubberBandMapper2D::SetWidget(QWidget *w)
{
    d->widget = w;
}

// ****************************************************************************
// Method: vtkRubberBandMapper2D::RenderOverlay
//
// Purpose: 
//   Render the polydata as an "overlay".
//
// Arguments:
//   viewport : The viewport into which we're rendering.
//   actor    : The actor.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 14 14:19:53 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

void
vtkRubberBandMapper2D::RenderOverlay(vtkViewport* viewport, vtkActor2D* actor)
{
    if(d->widget != 0)
    {
        switch(d->SelectBestRenderer())
        {
        case 0:
            RenderOverlay_Win32(viewport, actor);
            break;
        case 1:
            RenderOverlay_X11(viewport, actor);
            break;
        case 2:
            RenderOverlay_Qt(viewport, actor);
            break;
        }
    }
}

// ****************************************************************************
//
// Win32 coding and macros
//
// ****************************************************************************

void
vtkRubberBandMapper2D::RenderOverlay_Win32(vtkViewport* viewport, vtkActor2D* actor)
{
#if defined(_WIN32)
#define STORE_POINT(P, X, Y) P.x = long((X) + borderL);  P.y = long((Y)+borderT);

#define SET_FOREGROUND_D(rgba) if(validPen) DeleteObject(pen); \
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

#define BEGIN_POLYLINE(X,Y)

#define END_POLYLINE()

#define CLEAN_UP() delete [] points;

    HPEN pen = 0;
    bool validPen = false;
    POINT *points = new POINT[1024];
    POINT oldPoint;
    HDC hdc = GetWindowDC((HWND)d->widget->winId());
    int borderT = 0; //GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CYSIZE);
    int borderL = 0; //GetSystemMetrics(SM_CXFRAME);

    // Make the drawing mode be XOR.
    SetROP2(hdc, R2_XORPEN);

    // Set the line color
    double whitergb[] = {1.,1.,1.};
    SET_FOREGROUND_D(whitergb);

#include <vtkRubberBandMapper2D_body.C>
#endif
}

// ***************************************************************************
//
// X11 coding and macros
//
// ***************************************************************************

void
vtkRubberBandMapper2D::RenderOverlay_X11(vtkViewport* viewport, vtkActor2D* actor)
{
#if defined(HAVE_XLIB)
#define STORE_POINT(P, X, Y) P.x = short(X); P.y = short(Y);

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

#define BEGIN_POLYLINE(X,Y)

#define END_POLYLINE()

#define CLEAN_UP() delete [] points;

    XColor aColor;
    XPoint *points = new XPoint [1024];

    Display* displayId = (Display*) QX11Info::display();
    Window windowId = (Window) d->widget->winId();

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
    Drawable drawable = (Drawable) windowId;
    if (!drawable) vtkErrorMacro(<<"Window returned NULL drawable!");
  
    // Set up the forground color
    XWindowAttributes attr;
    XGetWindowAttributes(displayId,windowId,&attr);

    // Set the line color
    double* actorColor = actor->GetProperty()->GetColor();
    SET_FOREGROUND_D(actorColor);

#include <vtkRubberBandMapper2D_body.C>
#endif
}

// ***************************************************************************
//
// Qt coding and macros
//
// ***************************************************************************

void
vtkRubberBandMapper2D::RenderOverlay_Qt(vtkViewport* viewport, vtkActor2D* actor)
{
#if !defined(_WIN32)
#define STORE_POINT(P, X, Y) cerr << "STORE_POINT macro for Qt." << endl;

#define SET_FOREGROUND_D(rgba) \
    painter.setPen(QColor(int(255.*rgba[0]), int(255.*rgba[1]), int(255.*rgba[2])));

#define SET_FOREGROUND(rgba) \
    painter.setPen(QColor(int(255.*rgba[0]), int(255.*rgba[1]), int(255.*rgba[2])));

#define DRAW_POLYGON(points, npts) \
    cerr << "DRAW_POLYGON macro for Qt." << endl;

#define RESIZE_POINT_ARRAY(points, npts, currSize) \
    cerr << "RESIZE_POINT_ARRAY macro for Qt." << endl;

#define DRAW_XOR_LINE(x1, y1, x2, y2) \
    painter.drawLine(QLine(x1, y1, x2, y2));

#define FLUSH_AND_SYNC() \
    d->overlay->setPixmap(pixmap);

#define CLEAN_UP()

#define BEGIN_POLYLINE(X, Y)

#define END_POLYLINE()

    int x,y,w,h;
    QPoint tl(d->widget->mapToGlobal(QPoint(0,0)));
    x = tl.x();
    y = tl.y();
    w = d->widget->width();
    h = d->widget->height();

    QPixmap pixmap(w, h);
    pixmap.fill(Qt::transparent);

    //
    // Try and create the window if we've not yet created it.
    //
    if(d->overlay == 0)
    {
        d->overlay = new QLabel(0, Qt::FramelessWindowHint);
        d->overlay->setAttribute(Qt::WA_TranslucentBackground);
        // FIXME? We should need the following, but it triggers a Qt
        // bug and  strangely everything seems to work without it.
        // overlay->setAttribute(Qt::WA_TransparentForMouseEvents);
        d->overlay->setAutoFillBackground(false);
        d->overlay->setPixmap(pixmap);
    }

    d->overlay->setGeometry(x, y, w, h);
    d->overlay->show();

    // Clear the window so it's ready for us to draw.
    QPainter painter(&pixmap);

    // Set the line color
    double* actorColor = actor->GetProperty()->GetColor();
    SET_FOREGROUND_D(actorColor);

#include <vtkRubberBandMapper2D_body.C>
#endif
}
