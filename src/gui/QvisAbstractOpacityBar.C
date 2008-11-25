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

#include "QvisAbstractOpacityBar.h"

#include <QPainter>
#include <QPolygon>
#include <QPixmap>
#include <QImage>

#include <visitstream.h>
#include <math.h>
#include <stdlib.h>


// ****************************************************************************
//  Method:  QvisAbstractOpacityBar::QvisAbstractOpacityBar
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 31, 2001
//
//  Modifications:
//    Gunther Weber, Fri Apr  6 16:04:52 PDT 2007
//    Initialize backgroundColorControlPoints.
//
//    Brad Whitlock, Fri May 30 09:32:57 PDT 2008
//    Qt 4.
//
// ****************************************************************************

QvisAbstractOpacityBar::QvisAbstractOpacityBar(QWidget *parent)
    : QFrame(parent)
{
    setFrameStyle( QFrame::Panel | QFrame::Sunken );
    setLineWidth( 2 );
    setMinimumHeight(50);
    setMinimumWidth(128);
    setContentsMargins(0,0,0,0);
    QSizePolicy sp(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
    setSizePolicy(sp);

    image = 0;
    backgroundColorControlPoints = 0;
}

// ****************************************************************************
//  Method:  QvisAbstractOpacityBar::~QvisAbstractOpacityBar
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 31, 2001
//
//  Modifications:
//    Brad Whitlock, Thu Feb 14 13:19:19 PST 2002
//    Deleted pix.
//
// ****************************************************************************

QvisAbstractOpacityBar::~QvisAbstractOpacityBar()
{
    delete image;
    image = 0;
}

// ****************************************************************************
//  Method:  QvisAbstractOpacityBar::SetBackgroundColorControlPoints
//
//  Purpose: Set color control points for color transfer function backdrop
//    
//
//  Programmer:  Gunther H. Weber
//  Creation:    April 5, 2007
//
//  Modifications:
//
// ****************************************************************************

void QvisAbstractOpacityBar::SetBackgroundColorControlPoints(const ColorControlPointList *ccp)
{
    backgroundColorControlPoints = ccp;
    drawOpacities(contentsRect().width(), contentsRect().height());
    update();
}

// ****************************************************************************
//  Method:  QvisAbstractOpacityBar::val2x
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 31, 2001
//
// ****************************************************************************
int
QvisAbstractOpacityBar::val2x(float val)
{
    QRect c = contentsRect();
    int w = c.width();
    int l = c.left();
    int x = int(val*float(w) + l);
    x = qMax(l, qMin(l+w, x));
    return x;
}

// ****************************************************************************
//  Method:  QvisAbstractOpacityBar::x2val
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 31, 2001
//
// ****************************************************************************
float
QvisAbstractOpacityBar::x2val(int x)
{
    QRect c = contentsRect();
    int w = c.width();
    int l = c.left();
    float val = float(x-l)/float(w);
    val = qMax(0.f, qMin(1.f, val));
    return val;
}

// ****************************************************************************
//  Method:  QvisAbstractOpacityBar::val2y
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 31, 2001
//
// ****************************************************************************
int
QvisAbstractOpacityBar::val2y(float val)
{
    QRect c = contentsRect();
    int h = c.height();
    int t = c.top();
    int y = int((1-val)*float(h) + t);
    y = qMax(t, qMin(t+h, y));
    return y;
}

// ****************************************************************************
//  Method:  QvisAbstractOpacityBar::y2val
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 31, 2001
//
// ****************************************************************************
float
QvisAbstractOpacityBar::y2val(int y)
{
    QRect c = contentsRect();
    int h = c.height();
    int t = c.top();
    float val = float(y-t)/float(h);
    val = qMax(0.f, qMin(1.f, (1.f-val)));
    return val;
}

// ****************************************************************************
//  Method:  QvisAbstractOpacityBar::paintEvent
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 31, 2001
//
//  Modifications:
//    Brad Whitlock, Wed Jun  4 09:53:23 PDT 2008
//    Qt 4.
//
// ****************************************************************************
void
QvisAbstractOpacityBar::paintEvent(QPaintEvent *e)
{
    QFrame::paintEvent(e);

    int w = contentsRect().width();
    int h = contentsRect().height();
    if(ensureImageExists(w, h))
        drawOpacities(w,h);

    QPainter p(this);
    QPoint pos(contentsRect().left(),contentsRect().top());
    p.drawImage(pos, *image);
}

// ****************************************************************************
//  Method:  QvisAbstractOpacityBar::resizeEvent
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 31, 2001
//
//  Modifications:
//    Brad Whitlock, Fri Jul 18 15:34:34 PDT 2008
//    Qt 4.
//
// ****************************************************************************
void
QvisAbstractOpacityBar::resizeEvent(QResizeEvent*)
{
    QRect framerect(rect());
    framerect.setLeft(framerect.left()     +5);
    framerect.setRight(framerect.right()   -10);
    setFrameRect(framerect);

    int w=contentsRect().width();
    int h=contentsRect().height();

    delete image;
    image = new QImage(w, h, QImage::Format_RGB32);
    drawOpacities(w,h);
    update();
}

// ****************************************************************************
// Method: QvisAbstractOpacityBar::ensureImageExists
//
// Purpose: 
//   Ensure that the image to which we're drawing exists.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 18 14:38:14 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

bool
QvisAbstractOpacityBar::ensureImageExists(int w, int h)
{
    bool retval = image == 0;
    if(retval)
        image = new QImage(w, h, QImage::Format_RGB32);
    else if(w != image->width() || h != image->height())
    {
        delete image;
        retval = true;
        image = new QImage(w, h, QImage::Format_RGB32);
    }
    return retval;
}
