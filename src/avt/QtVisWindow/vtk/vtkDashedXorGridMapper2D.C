// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <stdlib.h>
#include <math.h>
#include <vtkDashedXorGridMapper2D.h>

#include <vtkActor2D.h>
#include <vtkCellArray.h>
#include <vtkCellArrayIterator.h> // used by vtkDashedXorGridMapper2D_body.C, which is #included below
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

struct vtkDashedXorGridMapper2DPrivate
{
    QWidget *widget;
    QLabel  *overlay;

    vtkDashedXorGridMapper2DPrivate()
    {
        widget = 0;
        overlay = 0;
    }

    vtkDashedXorGridMapper2DPrivate(const vtkDashedXorGridMapper2DPrivate &obj)
    {
        widget = obj.widget;
        overlay = 0;
    }

    ~vtkDashedXorGridMapper2DPrivate()
    { 
        ReleaseGraphicsResources();
    }

    void operator = (const vtkDashedXorGridMapper2DPrivate &obj)
    {
        widget = obj.widget;
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
        RenderOverlay_Qt(viewport, actor);
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


