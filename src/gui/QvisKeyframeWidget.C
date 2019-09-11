// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QvisKeyframeWidget.h>
#include <QvisKeyframeDrawer.h>
#include <QPainter>
#include <QMouseEvent>

// ****************************************************************************
// Method: QvisKeyframeWidget::QvisKeyframeWidget
//
// Purpose: 
//   Constructor.
//
// Arguments:
//   parent : The widget's parent.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:05:47 PST 2008
//
// Modifications:
//   
// ****************************************************************************

QvisKeyframeWidget::QvisKeyframeWidget(QWidget *parent) : QWidget(parent), idToIndex()
{
    pointStyle = Point_Bar;
    movingPoint = -1;
    d = 0;

    setFocusPolicy(Qt::StrongFocus);
}

// ****************************************************************************
// Method: QvisKeyframeWidget::~QvisKeyframeWidget
//
// Purpose: 
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:06:04 PST 2008
//
// Modifications:
//   
// ****************************************************************************

QvisKeyframeWidget::~QvisKeyframeWidget()
{
}

// ****************************************************************************
// Method: QvisKeyframeWidget::setDrawer
//
// Purpose: 
//   Sets the drawer that we'll use to draw the widget.
//
// Arguments:
//   obj : The new drawer.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:06:19 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisKeyframeWidget::setDrawer(QvisKeyframeDrawer *obj)
{
    d = obj;
}

// ****************************************************************************
// Method: QvisKeyframeWidget::drawer
//
// Purpose: 
//   Returns the widget's drawer.
//
// Returns:    The drawer.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:06:43 PST 2008
//
// Modifications:
//   
// ****************************************************************************

QvisKeyframeDrawer *
QvisKeyframeWidget::drawer()
{
    if(d == 0)
        d = new QvisKeyframeDrawer(this);
    return d;
}

// ****************************************************************************
// Method: QvisKeyframeWidget::setPointStyle
//
// Purpose: 
//   The style used to draw the control points.
//
// Arguments:
//   s : The new style.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:07:09 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisKeyframeWidget::setPointStyle(int s)
{
    pointStyle = s;
}

// ****************************************************************************
// Method: QvisKeyframeWidget::setData
//
// Purpose: 
//   Sets the control point to location data into the widget.
//
// Arguments:
//   val : The new data
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:07:38 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisKeyframeWidget::setData(const QMap<int,int> &val)
{
    idToIndex = val;
}

// ****************************************************************************
// Method: QvisKeyframeWidget::getData
//
// Purpose: 
//   Gets the control point data.
//
// Returns:    The control point data.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:08:13 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisKeyframeWidget::getData(QMap<int,int> &val) const
{
    val = idToIndex;
}

// ****************************************************************************
// Method: QvisKeyframeWidget::paintEvent
//
// Purpose: 
//   Paints the widget
//
// Arguments:
//   event : The paint event.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:08:32 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisKeyframeWidget::paintEvent(QPaintEvent *event)
{
    QPainter p(this);

    // Make a list of the valid points.
    QList<int> pts;
    for(QMap<int,int>::const_iterator it = idToIndex.begin();
        it != idToIndex.end(); ++it)
        if(it.value() != -1)
            pts.append(it.value());
    int activePoint = -1;
    if(idToIndex.find(movingPoint) != idToIndex.end())
        activePoint = idToIndex[movingPoint];

    if(pointStyle == Point_Time)
        drawer()->drawTimeHandle(&p, rect(), pts[0], activePoint!=-1);
    else
        drawer()->drawPoints(&p, rect(), pts, pointStyle == Point_Diamond, activePoint);
}

// ****************************************************************************
// Method: QvisKeyframeWidget::mousePressEvent
//
// Purpose: 
//   This method gets called on mouse press and we use it to determine whether
//   we have pressed a control point.
//
// Arguments:
//   e : The mouse event.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:09:20 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisKeyframeWidget::mousePressEvent(QMouseEvent *e)
{
    movingPoint = -1;
    for(QMap<int,int>::iterator it = idToIndex.begin();
        it != idToIndex.end(); ++it)
    {
        if(drawer()->handleRect(it.value(),2).contains(e->pos()))
        {
            if(idToIndex.size() > 1 &&
               e->button() == Qt::RightButton &&
               pointStyle != Point_Time)
            {
                it.value() = -1; // Mark the point as -1 so we'll delete it.
                update();
                return;
            }
            else
            {
                movingPoint = it.key();
                update();
                break;
            }
        }
    }

    setMouseTracking(movingPoint>=0);
}

// ****************************************************************************
// Method: QvisKeyframeWidget::mouseMoveEvent
//
// Purpose: 
//   This method gets called as we move the mouse and we use it to update
//   the widget as the location of the moving point changes.
//
// Arguments:
//   e : The mouse event.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:10:09 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisKeyframeWidget::mouseMoveEvent(QMouseEvent *e)
{
    if(movingPoint != -1)
    {
        int id = drawer()->xToFrame(e->pos().x());
        if(id != -1 && idToIndex[movingPoint] != id)
        {
            idToIndex[movingPoint] = id;
            update();
        }
    }
}

// ****************************************************************************
// Method: QvisKeyframeWidget::mouseReleaseEvent
//
// Purpose: 
//   This method is called when we release the mouse. We use it to emit a 
//   commit signal so we have a trigger for actions that we need to deal with
//   when the widget changes its data.
//
// Arguments:
//   e : The mouse event.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:10:54 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisKeyframeWidget::mouseReleaseEvent(QMouseEvent *e)
{
    setMouseTracking(false);
    if(movingPoint != -1)
        emit commit(this);
}
