// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QvisKeyframePlotRangeWidget.h>
#include <QvisKeyframeDrawer.h>
#include <QPainter>
#include <QMouseEvent>

// ****************************************************************************
// Method: QvisKeyframePlotRangeWidget::QvisKeyframePlotRangeWidget
//
// Purpose: 
//   Constructor.
//
// Arguments:
//   parent : The widget's parent.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:47:26 PST 2008
//
// Modifications:
//   
// ****************************************************************************

QvisKeyframePlotRangeWidget::QvisKeyframePlotRangeWidget(QWidget *parent) : QWidget(parent)
{
    endpoints[0] = endpoints[1] = 0;
    movingPoint = -1;
    d = 0;

    setFocusPolicy(Qt::StrongFocus);
}

// ****************************************************************************
// Method: QvisKeyframePlotRangeWidget::~QvisKeyframePlotRangeWidget
//
// Purpose: 
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:47:52 PST 2008
//
// Modifications:
//   
// ****************************************************************************

QvisKeyframePlotRangeWidget::~QvisKeyframePlotRangeWidget()
{
}

// ****************************************************************************
// Method: QvisKeyframePlotRangeWidget::setDrawer
//
// Purpose: 
//   Set the drawer that we'll use for this widget.
//
// Arguments:
//   obj : The new drawer.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:48:08 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisKeyframePlotRangeWidget::setDrawer(QvisKeyframeDrawer *obj)
{
    d = obj;
}

// ****************************************************************************
// Method: QvisKeyframePlotRangeWidget::drawer
//
// Purpose: 
//   Get the widget's drawer.
//
// Returns:    The widget's drawer.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:48:36 PST 2008
//
// Modifications:
//   
// ****************************************************************************

QvisKeyframeDrawer *
QvisKeyframePlotRangeWidget::drawer()
{
    if(d == 0)
        d = new QvisKeyframeDrawer(this);
    return d;
}

// ****************************************************************************
// Method: QvisKeyframePlotRangeWidget::setData
//
// Purpose: 
//   Set the widget's start/end points.
//
// Arguments:
//   start : The plot start.
//   end   : The plot end.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:49:01 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisKeyframePlotRangeWidget::setData(int start, int end)
{
    endpoints[0] = start;
    endpoints[1] = end;
}

// ****************************************************************************
// Method: QvisKeyframePlotRangeWidget::getData
//
// Purpose: 
//   Get the widget's plot start/end data.
//
// Arguments:
//   start : The plot start.
//   end   : The plot end.
//   
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:49:30 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisKeyframePlotRangeWidget::getData(int &start, int &end) const
{
    start = endpoints[0];
    end = endpoints[1];
}

// ****************************************************************************
// Method: QvisKeyframePlotRangeWidget::paintEvent
//
// Purpose: 
//   This method draws the widget. We call our drawer to do the work.
//
// Arguments:
//   event : The paint event.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:49:54 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisKeyframePlotRangeWidget::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    drawer()->drawPlotRange(&p, rect(), endpoints[0], endpoints[1], true, movingPoint);
}

// ****************************************************************************
// Method: QvisKeyframePlotRangeWidget::mousePressEvent
//
// Purpose: 
//   Called when the mouse is pressed.
//
// Arguments:
//   e : The mouse event.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:50:39 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisKeyframePlotRangeWidget::mousePressEvent(QMouseEvent *e)
{
    if(drawer()->handleRect(endpoints[0],2).contains(e->pos()))
    {
        movingPoint = 0;
        update();
    }
    else if(drawer()->handleRect(endpoints[1],2).contains(e->pos()))
    {
        movingPoint = 1;
        update();
    }
    else
        movingPoint = -1;

    setMouseTracking(movingPoint>=0);
}

// ****************************************************************************
// Method: QvisKeyframePlotRangeWidget::mouseMoveEvent
//
// Purpose: 
//   This method lets us move one of the plot end control points.
//
// Arguments:
//   e : The mouse event.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:51:08 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisKeyframePlotRangeWidget::mouseMoveEvent(QMouseEvent *e)
{
    if(movingPoint != -1)
    {
        int id = drawer()->xToFrame(e->pos().x());
        if(id != -1 && endpoints[movingPoint] != id)
        {
            endpoints[movingPoint] = id;
            update();
        }
    }
}

// ****************************************************************************
// Method: QvisKeyframePlotRangeWidget::mouseReleaseEvent
//
// Purpose: 
//   This method lets us release one of the plot end control points.
//
// Arguments:
//   e : The mouse event.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:51:08 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisKeyframePlotRangeWidget::mouseReleaseEvent(QMouseEvent *e)
{
    setMouseTracking(false);
    if(movingPoint != -1)
        emit commit(this);
}
