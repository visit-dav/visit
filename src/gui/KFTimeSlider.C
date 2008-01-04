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

#include "KFBase.h"
#include "KFTimeSlider.h"
#include "KFListView.h"
#include <qpixmap.h>
#include <qimage.h>
#include <qpainter.h>
#include <qpointarray.h>


// ****************************************************************************
//  Constructor:  KFTimeSlider::KFTimeSlider
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 10, 2001
//
// ****************************************************************************

KFTimeSlider::KFTimeSlider(QWidget *parent, const char *name,
                       KFListView *lv_)
    : QFrame(parent, name)
{
    setFrameStyle( QFrame::Panel | QFrame::Sunken );
    setLineWidth( 2 );
    setMinimumHeight(12);
    setMaximumHeight(30);
    setMinimumWidth(50);
    lv = lv_;
    value=-1;
}

// ****************************************************************************
//  Destructor:  KFTimeSlider::~KFTimeSlider
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 10, 2001
//
// ****************************************************************************

KFTimeSlider::~KFTimeSlider()
{
}


// ****************************************************************************
//  Method:  KFTimeSlider::setNSteps
//
//  Purpose:
//    Set the number of time steps in the slider.
//
//  Arguments:
//    n          the number of steps
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  2, 2001
//
// ****************************************************************************
void
KFTimeSlider::setNSteps(int n)
{
    nsteps = n;
}


// ****************************************************************************
//  Method:  KFTimeSlider::val2x
//
//  Purpose:
//    Converts a time step to a coordinate.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 10, 2001
//
// ****************************************************************************
int
KFTimeSlider::val2x(int val)
{
    QRect c = contentsRect();
    int w = lv->columnWidth(1);//c.width();
    int l = c.left();
    int x = val*w/(nsteps) + l + int(lv->kfstart());
    x = QMAX(l, QMIN(l+w, x));
    return x;
}


// ****************************************************************************
//  Method:  KFTimeSlider::x2val
//
//  Purpose:
//    Converts a coordinate to a time step.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 10, 2001
//
// ****************************************************************************
int
KFTimeSlider::x2val(int x)
{
    QRect c = contentsRect();
    int w = lv->columnWidth(1);//c.width();
    int l = c.left();
    int val = int(double(x-l-0.5)*(nsteps)/double(w));
    val = QMAX(0, QMIN(nsteps-1, val));
    return val;
}


// ****************************************************************************
//  Method:  KFTimeSlider::paintEvent
//
//  Purpose:
//    Handles drawing the window.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 10, 2001
//
// ****************************************************************************
void
KFTimeSlider::paintEvent(QPaintEvent *e)
{
    QFrame::paintEvent(e);
    QPainter p(this);

    p.fillRect(contentsRect(),gray2);

    p.setPen(gray1);
    for (int i=0; i<nsteps; i++)
    {
        int x=int((float(i+0.5)*float(lv->columnWidth(1)))/float(nsteps));
        x += contentsRect().left()+1;
        p.drawLine(x,contentsRect().top(),
                   x,contentsRect().bottom());
    }

    QColorGroup g = colorGroup();
    p.setPen( g.foreground() );
    p.setBrush( g.foreground() );
    QPointArray a;

    int x = val2x(value)+1;
    int y = contentsRect().top();// + contentsRect().height()+1;
    a.setPoints( 3, x, y, x-8, y+8, x+8, y+8 );
    //erase( 0, y, width(), 6 );
    p.drawPolygon( a );
}


// ****************************************************************************
//  Method:  KFTimeSlider::resizeEvent
//
//  Purpose:
//    Handles a resize event.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 10, 2001
//
// ****************************************************************************
void
KFTimeSlider::resizeEvent(QResizeEvent*)
{
    updateSize();
}


// ****************************************************************************
//  Method:  KFTimeSlider::updateSize
//
//  Purpose:
//    Updates it size after a resize event.
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2003
//
//  Modifications:
//    Jeremy Meredith, Fri Jan 31 09:44:26 PST 2003
//    The frame was a pixel off....
//
// ****************************************************************************
void
KFTimeSlider::updateSize()
{
    QRect framerect(rect());
    framerect.setTop(framerect.top());
    framerect.setBottom(framerect.bottom());
    framerect.setLeft(framerect.left() + lv->columnWidth(0) - 1);
    framerect.setRight(framerect.right());// + lv->columnWidth(1));
    setFrameRect(framerect);
    repaint();
}

// ****************************************************************************
//  Method:  KFTimeSlider::mouseMoveEvent
//
//  Purpose:
//    Handles events received when the mouse is moved.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 10, 2001
//
// ****************************************************************************
void
KFTimeSlider::mouseMoveEvent(QMouseEvent *e)
{
    setValue( x2val(e->x()) );
}


// ****************************************************************************
//  Method:  KFTimeSlider::mousePressEvent
//
//  Purpose:
//    Handles events received when the mouse button is pressed.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 10, 2001
//
// ****************************************************************************
void
KFTimeSlider::mousePressEvent(QMouseEvent *e)
{
    setValue( x2val(e->x()) );
}


// ****************************************************************************
//  Method:  KFTimeSlider::mouseReleaseEvent
//
//  Purpose:
//    Handles events received when the mouse button in released.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 10, 2001
//
// ****************************************************************************
void
KFTimeSlider::mouseReleaseEvent(QMouseEvent *e)
{
    setValue( x2val(e->x()) );
    emit mouseReleased();
    emit valueChanged(value);
}


// ****************************************************************************
//  Method:  KFTimeSlider::setValue
//
//  Purpose:
//    Sets the position of the slider.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 10, 2001
//
// ****************************************************************************
void
KFTimeSlider::setValue(int val)
{
    if (value == val)
        return;

    value = QMAX(0, QMIN(nsteps-1, val));
    repaint( FALSE );
}
