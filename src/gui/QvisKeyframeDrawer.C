/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
#include <QvisKeyframeDrawer.h>
#include <QPainter>

// ****************************************************************************
// Method: QvisKeyframeDrawer::QvisKeyframeDrawer
//
// Purpose: 
//   Constructor
//
// Arguments:
//   parent : The object's parent.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 14:55:34 PST 2008
//
// Modifications:
//   
// ****************************************************************************

QvisKeyframeDrawer::QvisKeyframeDrawer(QObject *parent) : QObject(parent),
    kfHandle(0,0,0),
    kfHandleH(255,255,255),
    kfBG(192,192,192),
    kfVLine(224,224,224),
    kfHLine(150,150,150),
    kfTimeLine(0,0,255),
    kfPlotRange(50,180,80),
    kfPlotRangeH(100,230,130),
    kfPlotRangeLines(100,200,100),
    rect(),
    numFrames(0),
    currentIndex(-1)
{
}

// ****************************************************************************
// Method: QvisKeyframeDrawer::~QvisKeyframeDrawer
//
// Purpose: 
//   Destructor
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 14:56:03 PST 2008
//
// Modifications:
//   
// ****************************************************************************

QvisKeyframeDrawer::~QvisKeyframeDrawer()
{
}

// ****************************************************************************
// Method: QvisKeyframeDrawer::setNumFrames
//
// Purpose: 
//   Set/Get the number of frames used to draw the items.
//
// Arguments:
//   n : The number of frames.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 14:56:16 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisKeyframeDrawer::setNumFrames(int n)
{
    numFrames = n;
}

int
QvisKeyframeDrawer::getNumFrames() const
{
    return numFrames;
}

// ****************************************************************************
// Method: QvisKeyframeDrawer::setCurrentIndex
//
// Purpose: 
//   Set the current index, which is the current animation frame.
//
// Arguments:
//   ci : The new current index.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 14:56:52 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisKeyframeDrawer::setCurrentIndex(int ci)
{
    currentIndex = ci;
}

// ****************************************************************************
// Method: QvisKeyframeDrawer::getX
//
// Purpose: 
//   Given the current rect and the index, return the x location of the specified
//   index.
//
// Arguments:
//   i : The index whose x value we want.
//
// Returns:    The x value for index i.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 14:57:24 PST 2008
//
// Modifications:
//   
// ****************************************************************************

int
QvisKeyframeDrawer::getX(int i) const
{
    int x = int((float(i+0.5)*float(rect.width()))/float(numFrames));
    x += rect.x();
    return x;
}

// ****************************************************************************
// Method: QvisKeyframeDrawer::drawVerticalLines
//
// Purpose: 
//   Draws vertical lines in a rectangle.
//
// Arguments:
//   p  : The painter to use.
//   r  : The rect in which to draw the lines.
//   x0 : The start index
//   xn : The end index.
//   c1 : The line color.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 14:58:09 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisKeyframeDrawer::drawVerticalLines(QPainter *p, const QRect &r, int x0, int xn, const QColor &c1) const
{
    p->setPen(c1);
    for(int i = x0; i < xn; ++i)
    {
        int x = getX(i);
        p->drawLine(x, r.top(), x, r.bottom());
    }
}

// ****************************************************************************
// Method: QvisKeyframeDrawer::handleRect
//
// Purpose: 
//   Returns a rect containing the id'th index.
//
// Arguments:
//   id      : The id of the handle that we want.
//   padding : The padding in pixels around the point that count towards
//             the id'th index.
//
// Returns:    The rectangle containing the index.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 14:59:12 PST 2008
//
// Modifications:
//   
// ****************************************************************************

QRect
QvisKeyframeDrawer::handleRect(int id, int padding) const
{
    int x = getX(id);
    int h=rect.height();
    int y=rect.y() + rect.height()/2;
    return QRect(x-2-padding, y-8-padding, 5+2*padding, 17+2*padding);
}

// ****************************************************************************
// Method: QvisKeyframeDrawer::drawBackground
//
// Purpose: 
//   Draws the background into a rectangle.
//
// Arguments:
//   p : The painter to use.
//   r : The rectangle in which to draw the background.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:00:34 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisKeyframeDrawer::drawBackground(QPainter *p, const QRect &r)
{
    rect = r;

    // Draw the background and the vertical lines.
    p->fillRect(rect, kfBG);
    drawVerticalLines(p, rect, 0, numFrames, kfVLine);

    // Draw the horizontal line.
    p->setPen(kfHLine);
    int y = (rect.top() + rect.bottom()) / 2;
    int firstx = getX(0);
    int lastx  = getX(numFrames-1);
    p->drawLine(firstx, y, lastx, y);

    // Draw the current time
    if (currentIndex >= 0)
    {
        QPen pen(kfTimeLine);
        pen.setWidth(3);
        p->setPen(pen);
        int x = getX(currentIndex);
        p->drawLine(x, rect.top(), x, rect.bottom());
    }
}

// ****************************************************************************
// Method: QvisKeyframeDrawer::drawPlotRange
//
// Purpose: 
//   Draw the plot range.
//
// Arguments:
//   p           : The painter to use.
//   r           : The rect in which to draw the plot range.
//   start       : The starting plot index.
//   end         : The ending plot index.
//   highlight   : Whether the plot range should be highlighted.
//   activePoint : The index of the active point or -1 if none are active.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:01:15 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisKeyframeDrawer::drawPlotRange(QPainter *p, const QRect &r, int start, int end, 
    bool highlight, int activePoint)
{
    // Draw the background.
    drawBackground(p, r);

    // draw the big bar
    int x0 = getX(start);
    int xn = getX(end);
    int y=rect.y() + rect.height()/2;
    p->fillRect(x0, y-6, xn-x0, 13, highlight ? kfPlotRangeH : kfPlotRange);
    QRect r2(rect.x(), y-6, rect.width(), 13);
    if(start != end)
        drawVerticalLines(p, r2, start, end, kfPlotRangeLines);

    // Draw the end points.
    p->fillRect(handleRect(start), (activePoint==0) ? kfHandleH : kfHandle);
    p->fillRect(handleRect(end), (activePoint==1) ? kfHandleH : kfHandle);
}

// ****************************************************************************
// Method: QvisKeyframeDrawer::drawPoints
//
// Purpose: 
//   Draws keyframing control points.
//
// Arguments:
//   p : The painter to use.
//   r : The rect inwhich to draw.
//   indices : The list of points to draw.
//   diamond : Whether the points should be drawn as diamonds.
//   activePoint : The index of the active point or -1 if none are active.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:02:42 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisKeyframeDrawer::drawPoints(QPainter *p, const QRect &r, const QList<int> &indices,
    bool diamond, int activePoint)
{
    // Draw the background.
    drawBackground(p, r);

    p->setBrush(QBrush(kfHandle));
    p->setPen(kfHandle);
    int y = rect.y() + rect.height()/2;
    for(int i = 0; i < indices.size(); ++i)
    {
        if(indices[i] == activePoint)
            p->setBrush(QBrush(kfHandleH));

        int x = getX(indices[i]);
        QPolygon pa(4);
        if (diamond)
        {
            pa.setPoint(0, x,y-5);
            pa.setPoint(1, x-5,y);
            pa.setPoint(2, x,y+5);
            pa.setPoint(3, x+5,y);
        }
        else
        {
            pa.setPoint(0, x-2,y-5);
            pa.setPoint(1, x-2,y+5);
            pa.setPoint(2, x+2,y+5);
            pa.setPoint(3, x+2,y-5);
        }
        p->drawPolygon(pa);

        if(indices[i] == activePoint)
            p->setBrush(QBrush(kfHandle));
    } 
}

// ****************************************************************************
// Method: QvisKeyframeDrawer::drawTimeHandle
//
// Purpose: 
//   Draws the time handle.
//
// Arguments:
//   p : The painter to use.
//   r : The rect in which to draw.
//   ci : The current index.
//   active : Whether the handle should be drawn as active.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:04:04 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisKeyframeDrawer::drawTimeHandle(QPainter *p, const QRect &r, int ci, bool active)
{
    // Draw the background.
    drawBackground(p, r);

    if(currentIndex >= 0)
    {
        p->setBrush(QBrush(active ? kfHandleH : kfHandle));
        p->setPen(kfHandle);
        int x = getX(ci);
        int y = rect.top();
        QPolygon a;
        a.setPoints(3, x, y+8, x-8, y, x+8, y);
        p->drawPolygon(a);
    }
}

// ****************************************************************************
// Method: QvisKeyframeDrawer::xToFrame
//
// Purpose: 
//   Converts an x value into an id.
//
// Arguments:
//   x : The value to convert.
//
// Returns:    The id of the point at x.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:04:52 PST 2008
//
// Modifications:
//   
// ****************************************************************************

int
QvisKeyframeDrawer::xToFrame(int x) const
{
    int retval = -1;
    if(numFrames < 2)
        return 0;
    int *xc = new int[numFrames];
    for(int i = 0; i < numFrames; ++i)
        xc[i] = getX(i);
    int epsilon = (xc[1] - xc[0]) / 2;
    for(int i = 0; i < numFrames; ++i)
    {
        if(x > xc[i]-epsilon &&
           x < xc[i]+epsilon)
        {
            retval = i;
            break;
        }
    }
    delete [] xc;
 
    return retval;
}
