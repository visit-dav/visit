/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <QvisScreenPositioner.h>
#include <qpainter.h>
#include <qpen.h>

const int QvisScreenPositioner::minXScreenSize = 100;
const int QvisScreenPositioner::minYScreenSize = 100;

// ****************************************************************************
// Method: QvisScreenPositioner::QvisScreenPositioner
//
// Purpose: 
//   Constructor for the QvisScreenPositioner class.
//
// Arguments:
//   parent : The widget's parent.
//   name   : The widget's name.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 1 14:11:15 PST 2003
//
// Modifications:
//   
// ****************************************************************************

QvisScreenPositioner::QvisScreenPositioner(QWidget *parent, const char *name,
    WFlags flags) : QFrame(parent, name, flags)
{
    xPosition = xTempPosition = minXScreenSize / 2;
    yPosition = yTempPosition = minYScreenSize / 2;
    xScreenSize = minXScreenSize;
    yScreenSize = minYScreenSize;
    setPageIncrement(10);
    dragging = false;
    paging = false;

    setMinimumSize(minimumSize());

    setFocusPolicy(StrongFocus);

    // Set the default size policy.
    setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,
        QSizePolicy::MinimumExpanding));

    // By default we don't want a frame.
    setFrameStyle(NoFrame);
}

// ****************************************************************************
// Method: QvisScreenPositioner::~QvisScreenPositioner
//
// Purpose: 
//   Destructor for the QvisScreenPositioner class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 1 14:15:16 PST 2003
//
// Modifications:
//   
// ****************************************************************************

QvisScreenPositioner::~QvisScreenPositioner()
{
}

// ****************************************************************************
// Method: QvisScreenPositioner::sizeHint
//
// Purpose: 
//   Returns the widget's preferred size.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 1 14:15:36 PST 2003
//
// Modifications:
//   
// ****************************************************************************

QSize
QvisScreenPositioner::sizeHint() const
{
    return QSize(minXScreenSize, minYScreenSize);
}

// ****************************************************************************
// Method: QvisScreenPositioner::minimumSize
//
// Purpose: 
//   Returns the widget's min size.
//
// Returns:    The min size.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 1 14:15:57 PST 2003
//
// Modifications:
//   
// ****************************************************************************

QSize
QvisScreenPositioner::minimumSize() const
{
    return QSize(minXScreenSize, minYScreenSize);
}

// ****************************************************************************
// Method: QvisScreenPositioner::setScreenSize
//
// Purpose: 
//   Sets the "screen" size.
//
// Arguments:
//   xs, ys : The screen size.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 1 14:16:21 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisScreenPositioner::setScreenSize(int xs, int ys)
{
    if(xs >= minXScreenSize)
        xScreenSize = xs;
    if(ys >= minYScreenSize)
        yScreenSize = ys;

    if(xPosition > xScreenSize)
        xPosition = xScreenSize - 1;
    if(yPosition > yScreenSize)
        yPosition = yScreenSize - 1;

    update();
}

// ****************************************************************************
// Method: QvisScreenPositioner::setScreenPosition
//
// Purpose: 
//   Sets the position of the screen crosshairs.
//
// Arguments:
//   xp, yp : The screen position.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 1 14:17:27 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisScreenPositioner::setScreenPosition(int xp, int yp)
{
    xPosition = xp;
    yPosition = yp;

    if(xPosition < 0)
        xPosition = 0;
    else if(xPosition >= xScreenSize)
        xPosition = xScreenSize - 1;

    if(yPosition < 0)
        yPosition = 0;
    else if(yPosition >= yScreenSize)
        yPosition = yScreenSize - 1;

    update();
}

void
QvisScreenPositioner::setScreenPosition(double xp, double yp)
{
    int ixp = int(xp * double(xScreenSize));
    int iyp = int(yp * double(yScreenSize));
    setScreenPosition(ixp, iyp);
}

// ****************************************************************************
// Method: QvisScreenPositioner::sendNewScreenPosition
//
// Purpose: 
//   Emits signals that tell the clients the new screen position.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 1 14:19:28 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisScreenPositioner::sendNewScreenPosition()
{
    if((xPosition != xTempPosition) || (yPosition != yTempPosition))
    {
        xPosition = xTempPosition;
        yPosition = yTempPosition;

        update();

        emit screenPositionChanged(xPosition, yPosition);

        double xp = double(xPosition) / double(xScreenSize);
        double yp = double(yPosition) / double(yScreenSize);
        emit screenPositionChanged(xp, yp);
    }
}

// ****************************************************************************
// Method: QvisScreenPositioner::setTempPositionFromWidgetCoords
//
// Purpose: 
//   Sets the temp position using widget coordinates.
//
// Arguments:
//   wx : The x coordinate.
//   wy : The y coordinate.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 1 14:20:01 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisScreenPositioner::setTempPositionFromWidgetCoords(int wx, int wy)
{
    bool needsUpdate = false;
    int W = contentsRect().width();
    int H = contentsRect().height();
    int clipped_wx;
    if(wx >= 0 && wx < W)
        clipped_wx = wx;
    else if(wx < 0)
        clipped_wx = 0;
    else
        clipped_wx = W - 1;
    int xp = int(double(clipped_wx) / double(W-1) * double(xScreenSize));
    if(xTempPosition != xp)
    {
        needsUpdate = true;
        xTempPosition = xp;
    }

    int clipped_wy;
    if(wy >= 0 && wy < H)
        clipped_wy = wy;
    else if(wy < 0)
        clipped_wy = 0;
    else
        clipped_wy = H - 1;
    int yp = int((1. - double(clipped_wy) / double(H-1)) * double(yScreenSize));
    if(yTempPosition != yp)
    {
        needsUpdate = true;
        yTempPosition = yp;
    }

    if(needsUpdate)
        update();
}

// ****************************************************************************
// Method: QvisScreenPositioner::popupShow
//
// Purpose: 
//   Shows the widget in a popup mode, which means that it is ready for
//   dragging.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 1 17:15:01 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisScreenPositioner::popupShow()
{
    // Make the widget have a visible frame
    setFrameStyle(PopupPanel);
    setFrameShadow(Raised);
    setLineWidth(2);

    xTempPosition = xPosition;
    yTempPosition = yPosition;
    dragging = true;
    show();
}

// ****************************************************************************
// Method: QvisScreenPositioner::keyPressEvent
//
// Purpose: 
//   Processes key events for the widget.
//
// Arguments:
//   e : The key event.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 1 14:21:13 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisScreenPositioner::keyPressEvent(QKeyEvent *e)
{
    // Handle the key strokes.
    switch(e->key())
    {
    case Key_Return:
    case Key_Enter:
        sendNewScreenPosition();
        break;
    case Key_Left:
        xTempPosition -= ((paging) ? (pageIncrement()) : 1);
        if(xTempPosition < 0)
            xTempPosition = 0;
        update();
        break;
    case Key_Right:
        xTempPosition += ((paging) ? (pageIncrement()) : 1);
        if(xTempPosition >= xScreenSize)
            xTempPosition = xScreenSize - 1;
        update();
        break;
    case Key_Down:
        yTempPosition -= ((paging) ? (pageIncrement()) : 1);
        if(yTempPosition < 0)
            yTempPosition = 0;
        update();
        break;
    case Key_Up:
        yTempPosition += ((paging) ? (pageIncrement()) : 1);
        if(yTempPosition >= yScreenSize)
            yTempPosition = yScreenSize - 1;
        update();
        break;
    case Key_Space:
        xTempPosition = xScreenSize / 2;
        yTempPosition = yScreenSize / 2;
        update();
        sendNewScreenPosition();
        break;
    case Key_Shift:
        paging = true;
        break;
    }
}

// ****************************************************************************
// Method: QvisScreenPositioner::keyReleaseEvent
//
// Purpose: 
//   Processes key release events.
//
// Arguments:
//   e : The key event.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 1 14:21:40 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisScreenPositioner::keyReleaseEvent(QKeyEvent *e)
{
    // Handle the key strokes.
    if(e->key() == Key_Shift)
        paging = false;
}

// ****************************************************************************
// Method: QvisScreenPositioner::mousePressEvent
//
// Purpose: 
//   This method is called when the mouse is pressed.
//
// Arguments: 
//   e : The mouse event.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 1 14:25:54 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisScreenPositioner::mousePressEvent(QMouseEvent *e)
{
    dragging = true;
    xTempPosition = xPosition;
    yTempPosition = yPosition;
    setTempPositionFromWidgetCoords(e->x(), e->y());
}

// ****************************************************************************
// Method: QvisScreenPositioner::mouseMoveEvent
//
// Purpose: 
//   This method is called when the mouse is moved.
//
// Arguments:
//   e : The mouse event.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 1 14:26:20 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisScreenPositioner::mouseMoveEvent(QMouseEvent *e)
{
    if(dragging)
    {
        setTempPositionFromWidgetCoords(e->x(), e->y());

        emit intermediateScreenPositionChanged(xTempPosition, yTempPosition);

        double xp = double(xTempPosition) / double(xScreenSize);
        double yp = double(yTempPosition) / double(yScreenSize);
        emit intermediateScreenPositionChanged(xp, yp);
    }
}

// ****************************************************************************
// Method: QvisScreenPositioner::mouseReleaseEvent
//
// Purpose: 
//   This method is called when the mouse is released.
//
// Arguments:
//   e : The mouse event.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 1 14:26:45 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisScreenPositioner::mouseReleaseEvent(QMouseEvent *e)
{
    dragging = false;
    setTempPositionFromWidgetCoords(e->x(), e->y());
    sendNewScreenPosition();
}

// ****************************************************************************
// Method: QvisScreenPositioner::paintEvent
//
// Purpose: 
//   This method is called when the widget must be painted.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 1 14:27:12 PST 2003
//
// Modifications:
//   
// ****************************************************************************

#define CLAMP_VALUE(VAL, MINVAL, MAXVAL) \
    if(VAL < (MINVAL)) VAL = (MINVAL); \
    if(VAL > (MAXVAL)) VAL = (MAXVAL);

void
QvisScreenPositioner::drawContents(QPainter *paint)
{
    QPen pen(colorGroup().foreground());
    int cx = contentsRect().x();
    int cy = contentsRect().y();
    int w = contentsRect().width();
    int h = contentsRect().height();

    // Draw the old position
    double t = double(xPosition) / double(xScreenSize);
    int oldwx = int(t * double(w - 1));
    CLAMP_VALUE(oldwx, 0, w - 1);
    t = 1. - double(yPosition) / double(yScreenSize);
    int oldwy = int(t * double(h - 1));
    CLAMP_VALUE(oldwy, 0, h - 1);
    paint->setPen(pen);
    paint->drawLine(oldwx + cx, cy, oldwx + cx, cy + h);
    paint->drawLine(cx, cy + oldwy, cx + w, cy + oldwy);

    // Draw the temporary position
    t = double(xTempPosition) / double(xScreenSize);
    int twx = int(t * double(w - 1));
    CLAMP_VALUE(twx, 0, w - 1);
    t = 1. - double(yTempPosition) / double(yScreenSize);
    int twy = int(t * double(h - 1));
    CLAMP_VALUE(twy, 0, h - 1);
    pen.setStyle(Qt::DotLine);
    paint->setPen(pen);
    paint->drawLine(twx + cx, cy, twx + cx, cy + h);
    paint->drawLine(cx, cy + twy, cx + w, cy + twy);
}
