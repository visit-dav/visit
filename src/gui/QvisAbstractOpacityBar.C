#include "QvisAbstractOpacityBar.h"

#include <qpainter.h>
#include <qpointarray.h>
#include <qpixmap.h>
#include <qimage.h>

#include <iostream.h>
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
// ****************************************************************************

QvisAbstractOpacityBar::QvisAbstractOpacityBar(QWidget *parent, const char *name)
    : QFrame(parent, name)
{
    setFrameStyle( QFrame::Panel | QFrame::Sunken );
    setLineWidth( 2 );
    setMinimumHeight(50);
    setMinimumWidth(128);
    pix = new QPixmap;
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
