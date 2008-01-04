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

#include <qpainter.h>
#include <qpointarray.h>
#include <qpixmap.h>
#include <qimage.h>

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
//
//    Gunther Weber, Fri Apr  6 16:04:52 PDT 2007
//    Initialize backgroundColorControlPoints.
//
// ****************************************************************************

QvisAbstractOpacityBar::QvisAbstractOpacityBar(QWidget *parent, const char *name)
    : QFrame(parent, name)
{
    setFrameStyle( QFrame::Panel | QFrame::Sunken );
    setLineWidth( 2 );
    setMinimumHeight(50);
    setMinimumWidth(128);
    pix = new QPixmap;
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
    delete pix;
    pix = 0;
}

// ****************************************************************************
//  Method:  QvisAbstractOpacityBar::SetBackgroundColorControlPoints(const ColorControlPointList *ccp)
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
    paintToPixmap(contentsRect().width(), contentsRect().height());
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
    x = QMAX(l, QMIN(l+w, x));
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
    val = QMAX(0, QMIN(1, val));
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
    y = QMAX(t, QMIN(t+h, y));
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
    val = QMAX(0, QMIN(1, (1-val)));
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
// ****************************************************************************
void
QvisAbstractOpacityBar::paintEvent(QPaintEvent *e)
{
    QFrame::paintEvent(e);
    if (!pix)
        return;
    
    QPainter p(this);
    p.drawPixmap(contentsRect().left(),contentsRect().top(),*pix);
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
// ****************************************************************************
void
QvisAbstractOpacityBar::resizeEvent(QResizeEvent*)
{
    QRect framerect(rect());
    framerect.setTop(framerect.top()       +5);
    framerect.setBottom(framerect.bottom( )-5);
    framerect.setLeft(framerect.left()     +13);
    framerect.setRight(framerect.right()   -13);
    setFrameRect(framerect);

    int w=contentsRect().width();
    int h=contentsRect().height();

    delete pix;
    pix = new QPixmap;
    paintToPixmap(w,h);
}
