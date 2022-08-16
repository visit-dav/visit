// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <stdlib.h>
#include <math.h>
#include <vtkDashedXorGridMapper2D.h>

#include <visit-config.h> // For LIB_VERSION_GE

#include <vtkActor2D.h>
#include <vtkCellArray.h>
#if LIB_VERSION_GE(VTK,9,1,0)
#include <vtkCellArrayIterator.h> // used by vtkDashedXorGridMapper2D_body.C, which is #included below
#endif
#include <vtkCellLinks.h>
#include <vtkDataSet.h>
#include <vtkFieldData.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProperty2D.h>
#include <vtkScalarsToColors.h>
#include <vtkUnsignedCharArray.h>
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

struct vtkDashedXorGridMapper2DPrivate
{
    QWidget *widget;
    int      bestRenderer;
    QLabel  *overlay;

    vtkDashedXorGridMapper2DPrivate()
    {
        widget = 0;
        bestRenderer = -1;
        overlay = 0;
    }

    vtkDashedXorGridMapper2DPrivate(const vtkDashedXorGridMapper2DPrivate &obj)
    {
        widget = obj.widget;
        bestRenderer = obj.bestRenderer;
        overlay = 0;
    }

    ~vtkDashedXorGridMapper2DPrivate()
    { 
        ReleaseGraphicsResources();
    }

    void operator = (const vtkDashedXorGridMapper2DPrivate &obj)
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

#if defined(__APPLE__) || defined (_WIN32)
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
    privateInstance = new vtkDashedXorGridMapper2DPrivate;
    SetDots(2, 3);
    SetHorizontalBias(true);
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
    if(privateInstance != 0)
    {
        delete privateInstance;
        privateInstance = 0;
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
    privateInstance->ReleaseGraphicsResources();

    // Call the superclass's ReleaseGraphicsResources method.
    vtkPolyDataMapper2D::ReleaseGraphicsResources(win);
}

// ****************************************************************************
// Method: vtkDashedXorGridMapper2D::SetWidget
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
vtkDashedXorGridMapper2D::SetWidget(QWidget *w)
{
    privateInstance->widget = w;
}

void
vtkDashedXorGridMapper2D::SetDots(int drawn, int spaced)
{
    pixelDrawn = drawn;
    pixelSpaced = spaced;
}

// ****************************************************************************
// Method: vtkDashedXorGridMapper2D::RenderOverlay
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
vtkDashedXorGridMapper2D::RenderOverlay(vtkViewport* viewport, vtkActor2D* actor)
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


// ****************************************************************************
//
// X11 coding and macros
//
// ****************************************************************************

void
vtkDashedXorGridMapper2D::RenderOverlay_X11(vtkViewport* viewport, vtkActor2D* actor)
{
#if defined(HAVE_XLIB)
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

    Display* displayId = (Display*) QX11Info::display();
    Window windowId = (Window) privateInstance->widget->winId();

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

#include <vtkDashedXorGridMapper2D_body.C>
#endif
}

// ****************************************************************************
//
// Qt coding and macros
//
//  Modifications:
//    Kathleen Biagas, Mon Jun 11 15:44:12 MST 2012
//    Remove the ifdef preventing use of this method on _WIN32.
//
// ****************************************************************************

void
vtkDashedXorGridMapper2D::RenderOverlay_Qt(vtkViewport* viewport, vtkActor2D* actor)
{
#define SET_FOREGROUND_D(rgba) \
    painter.setPen(QColor(int(255.*rgba[0]), int(255.*rgba[1]), int(255.*rgba[2])));

#define SET_FOREGROUND(rgba) \
    painter.setPen(QColor(int(255.*rgba[0]), int(255.*rgba[1]), int(255.*rgba[2])));

#define DRAW_XOR_LINE(x1, y1, x2, y2) \
    painter.drawLine(QLine(x1, y1, x2, y2));

#define FLUSH_AND_SYNC() \
    privateInstance->overlay->setPixmap(pixmap);

#define CLEAN_UP()

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
        // overlay->setAttribute(Qt::WA_TransparentForMouseEvents);
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

#include <vtkDashedXorGridMapper2D_body.C>
}


