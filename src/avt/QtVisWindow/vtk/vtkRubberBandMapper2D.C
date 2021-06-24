// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
#endif
#include <QLabel>
#include <QPixmap>
#include <QPainter>
#include <QLine>
#if defined(HAVE_XLIB)
#  include <QtX11Extras/QX11Info>
#  include <X11/Intrinsic.h>
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

    vtkRubberBandMapper2DPrivate(const vtkRubberBandMapper2DPrivate &obj)
    {
        widget = obj.widget;
        bestRenderer = obj.bestRenderer;
        overlay = 0;
    }

    ~vtkRubberBandMapper2DPrivate()
    {
        ReleaseGraphicsResources();
    }

    void operator = (const vtkRubberBandMapper2DPrivate &obj)
    {
        widget = obj.widget;
        bestRenderer = obj.bestRenderer;
        overlay = 0;
    }

    // Delete the overlay with deleteLater so we don't clobber a widget
    // that Qt assumes is still valid during a mouse event.
    void ReleaseGraphicsResources()
    {
        if(overlay != 0)
            overlay->deleteLater();
        overlay = 0;
    }

    int SelectBestRenderer()
    {
        if(bestRenderer != -1)
            return bestRenderer;

#if defined(__APPLE__) || defined(_WIN32)
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
    privateInstance = new vtkRubberBandMapper2DPrivate;
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
    if(privateInstance != 0)
    {
        delete privateInstance;
        privateInstance = 0;
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
    privateInstance->ReleaseGraphicsResources();

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
    privateInstance->widget = w;
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
    if(privateInstance->widget != 0)
    {
        switch(privateInstance->SelectBestRenderer())
        {
        case 1:
            RenderOverlay_X11(viewport, actor);
            break;
        case 2:
            RenderOverlay_Qt(viewport, actor);
            break;
        }
    }
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
//  Modifications:
//    Kathleen Biagas, Mon Jun 11 15:48:10 MST 2012
//    Remove ifdef preventing this method from bein used on _WIN32
//
//    Alister Maguire, Thu Jun 24 10:57:30 PDT 2021
//    Scale by the devicePixelRatio so that we avoid issues with retina
//    displays. I also moved code from vtkRubberBandMapper2D_body.C into
//    this file for the sake of readability.
//
// ***************************************************************************

void
vtkRubberBandMapper2D::RenderOverlay_Qt(vtkViewport* viewport, vtkActor2D* actor)
{
#define STORE_POINT(P, X, Y) cerr << "STORE_POINT macro for Qt." << endl;

#define SET_FOREGROUND_D(rgba) \
    painter.setPen(QColor(int(255.*rgba[0]), int(255.*rgba[1]), int(255.*rgba[2])));

#define SET_FOREGROUND(rgba) \
    painter.setPen(QColor(int(255.*rgba[0]), int(255.*rgba[1]), int(255.*rgba[2])));

#define DRAW_POLYGON(points, npts) \
    cerr << "DRAW_POLYGON macro for Qt." << endl;

#define RESIZE_POINT_ARRAY(points, npts, currSize) \
    currSize = currSize; \
    cerr << "RESIZE_POINT_ARRAY macro for Qt." << endl;

#define DRAW_XOR_LINE(x1, y1, x2, y2) \
    painter.drawLine(QLine(x1, y1, x2, y2));

#define FLUSH_AND_SYNC() \
    privateInstance->overlay->setPixmap(pixmap);

#define CLEAN_UP()

#define BEGIN_POLYLINE(X, Y)

#define END_POLYLINE()

    int x,y,w,h;
    QPoint tl(privateInstance->widget->mapToGlobal(QPoint(0,0)));

    x = tl.x();
    y = tl.y();
    w = privateInstance->widget->width();
    h = privateInstance->widget->height();

    QPixmap pixmap(w, h);
    pixmap.fill(Qt::transparent);

    //
    // Try and create the window if we've not yet created it.
    //
    if(privateInstance->overlay == 0)
    {
        privateInstance->overlay = new QLabel(0, Qt::FramelessWindowHint);
        privateInstance->overlay->setAttribute(Qt::WA_TranslucentBackground);
        // FIXME? We should need the following, but it triggers a Qt
        // bug and  strangely everything seems to work without it.
        //privateInstance->overlay->setAttribute(Qt::WA_TransparentForMouseEvents);
        privateInstance->overlay->setAutoFillBackground(false);
        privateInstance->overlay->setPixmap(pixmap);
    }

    privateInstance->overlay->setGeometry(x, y, w, h);
    privateInstance->overlay->show();

    // Clear the window so it's ready for us to draw.
    QPainter painter(&pixmap);

    // Set the line color
    double* actorColor = actor->GetProperty()->GetColor();
    SET_FOREGROUND_D(actorColor);

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
    int currSize = 1024;
    vtkDebugMacro (<< "vtkRubberBandMapper2D::RenderOverlay");

    if ( input == NULL )
    {
        vtkErrorMacro(<< "No input!");
        CLEAN_UP();
        return;
    }
    else
    {
        numPts = input->GetNumberOfPoints();
    }

    if (numPts == 0)
    {
        vtkDebugMacro(<< "No points!");
        CLEAN_UP();
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
                rgba = c->GetPointer(4*pts[0]);

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

    //
    // Draw the lines.
    // We need to scale our coordinates by the devicePixelRatio, which takes
    // the OSX retina display into account. From the docs:
    //
    //     "Common values are 1 for normal-dpi displays and 2 for high-dpi
    //     'retina' displays."
    //
    int devicePixelRatio = privateInstance->widget->devicePixelRatio();
    aPrim = input->GetLines();
    for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); cellNum++)
    {
        if (c && cellScalars)
        {
            rgba = c->GetPointer(4*cellNum);
            SET_FOREGROUND(rgba);
        }
        ftmp = p->GetPoint(pts[0]);

        lastX = (int)(actorPos[0] + ftmp[0]) / devicePixelRatio;
        lastY = (int)(actorPos[1] - ftmp[1]) / devicePixelRatio;
        BEGIN_POLYLINE(lastX, lastY);

        for (j = 1; j < npts; j++)
        {
            ftmp = p->GetPoint(pts[j]);
            if (c && !cellScalars)
            {
                rgba = c->GetPointer(4*pts[j]);
                SET_FOREGROUND(rgba)
            }
            X = (int)(actorPos[0] + ftmp[0]) / devicePixelRatio;
            Y = (int)(actorPos[1] - ftmp[1]) / devicePixelRatio;

            DRAW_XOR_LINE(lastX, lastY, X, Y);

            lastX = X;
            lastY = Y;
        }

        END_POLYLINE();
    }

    // Finish drawing.
    FLUSH_AND_SYNC();

    // Clean up.
    CLEAN_UP();
    if ( this->TransformCoordinate )
        p->Delete();

#undef STORE_POINT
#undef DRAW_POLYGON
#undef RESIZE_POINT_ARRAY
#undef SET_FOREGROUND_D
#undef SET_FOREGROUND
#undef DRAW_XOR_LINE
#undef FLUSH_AND_SYNC
#undef CLEAN_UP
}
