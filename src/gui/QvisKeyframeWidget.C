/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
