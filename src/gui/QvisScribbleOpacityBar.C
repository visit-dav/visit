#include "QvisScribbleOpacityBar.h"

#include <qpainter.h>
#include <qpointarray.h>
#include <qpixmap.h>
#include <qimage.h>

#include <iostream.h>
#include <math.h>
#include <stdlib.h>

// ****************************************************************************
//  Method:  QvisScribbleOpacityBar::QvisScribbleOpacityBar
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 31, 2001
//
//  Modifications:
//    Brad Whitlock, Thu Feb 14 12:57:49 PDT 2002
//    Added deletion of values array and the internal pixmap.
//
// ****************************************************************************

QvisScribbleOpacityBar::QvisScribbleOpacityBar(QWidget *parent, const char *name)
    : QvisAbstractOpacityBar(parent, name)
{
    setFrameStyle( QFrame::Panel | QFrame::Sunken );
    setLineWidth( 2 );
    setMinimumHeight(50);
    setMinimumWidth(128);

    nvalues = 256; //contentsRect().width();
    values = new float[nvalues];
    for (int i=0; i<nvalues; ++i)
    {
        values[i] = float(i)/float(nvalues-1);
    }
    mousedown = false;
}

// ****************************************************************************
//  Method:  QvisScribbleOpacityBar::~QvisScribbleOpacityBar
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 31, 2001
//
//  Modifications:
//    Brad Whitlock, Thu Feb 14 12:57:49 PDT 2002
//    Added deletion of values array.
//
// ****************************************************************************

QvisScribbleOpacityBar::~QvisScribbleOpacityBar()
{
    delete [] values;
}


// ****************************************************************************
//  Method:  QvisScribbleOpacityBar::paintToPixmap
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 31, 2001
//
// ****************************************************************************
void
QvisScribbleOpacityBar::paintToPixmap(int w, int h)
{
    QImage img(w,h, 32);

    if (w != nvalues)
    {
        int nvalues2 = w;
        float *values2 = new float[nvalues2];
        if (nvalues2 > nvalues)
        {
            for (int i=0; i<nvalues2; i++)
                values2[i] = values[(i * nvalues) / nvalues2];
        }
        else
        {
            for (int i=0; i<nvalues; i++)
                values2[(i * nvalues2) / nvalues] = values[i];
        }
        delete[] values;
        values = values2;
        nvalues = nvalues2;
    }

    QColor white(255, 255, 255 );
    QColor black(0,   0,   0 );
    QRgb cw = white.rgb();
    QRgb cb = black.rgb();
    for (int x = 0; x < w; x++)
    {
        float yval = values[x];
        for (int y = 0; y < h; y++)
        {
            float yval2 = 1 - float(y)/float(h-1);
            if (yval2 < yval)
                img.setPixel(x,y, cw); 
            else
                img.setPixel(x,y, cb);
       }
    }

    if (!pix || 
        pix->height() != w ||
        pix->width()  != h)
    {
        delete pix;
        pix = new QPixmap;
        pix->convertFromImage(img);
    }
    
    QPainter p(this);
    p.drawPixmap(contentsRect().left(),contentsRect().top(),*pix);
}


// ****************************************************************************
//  Method:  QvisScribbleOpacityBar::mousePressEvent
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 31, 2001
//
// ****************************************************************************
void
QvisScribbleOpacityBar::mousePressEvent(QMouseEvent *e)
{
    int x = e->x();
    int y = e->y();
    setValue(x2val(x), y2val(y));
    lastx = x;
    lasty = y;
    mousedown = true;

    QPainter p(this);
    p.drawPixmap(contentsRect().left(),contentsRect().top(),*pix);
}


// ****************************************************************************
//  Method:  QvisScribbleOpacityBar::mouseMoveEvent
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 31, 2001
//
// ****************************************************************************
void
QvisScribbleOpacityBar::mouseMoveEvent(QMouseEvent *e)
{
    if (!mousedown)
        return;

    int x = e->x();
    int y = e->y();
    setValues(lastx, lasty,
              x,     y);
    lastx = x;
    lasty = y;

    QPainter p(this);
    p.drawPixmap(contentsRect().left(),contentsRect().top(),*pix);
}


// ****************************************************************************
//  Method:  QvisScribbleOpacityBar::mouseReleaseEvent
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 31, 2001
//
// ****************************************************************************
void
QvisScribbleOpacityBar::mouseReleaseEvent(QMouseEvent *e)
{
    int x = e->x();
    int y = e->y();
    setValues(lastx, lasty,
              x,     y);
    mousedown = false;

    QPainter p(this);
    p.drawPixmap(contentsRect().left(),contentsRect().top(),*pix);

    emit mouseReleased();
}


// ****************************************************************************
//  Method:  QvisScribbleOpacityBar::setValues
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 31, 2001
//
// ****************************************************************************
void
QvisScribbleOpacityBar::setValues(int x1, int y1, int x2, int y2)
{
    if (x1==x2)
    {
        setValue(x2val(x2), y2val(y2));
        return;
    }

    int   xdiff = abs(x2 - x1) + 1;
    int   step  = (x1 < x2) ? 1 : -1;
    float slope = float(y2 - y1) / float (x2 - x1);
    for (int i=0; i<xdiff; i++)
        setValue(x2val(x1 + i*step),
                 y2val(y1 + int(float(i)*slope*step)));

}


// ****************************************************************************
//  Method:  QvisScribbleOpacityBar::setValue
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 31, 2001
//
// ****************************************************************************
void
QvisScribbleOpacityBar::setValue(float xval, float yval)
{
    QPainter p(pix);
    QColor white(255, 255, 255 );
    QColor black(0,   0,   0 );
    int h = contentsRect().height();
    int x = int(xval * float(nvalues-1));
    values[x] = yval;
    for (int i = 0; i < h; i++)
    {
        float yval2 = 1 - float(i)/float(h);
        if (yval2 < yval)
            p.setPen(white);
        else
            p.setPen(black);
        p.drawPoint(x,i);
    }
}


// ****************************************************************************
//  Method:  QvisScribbleOpacityBar::getRawOpacities
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 31, 2001
//
// ****************************************************************************
float *
QvisScribbleOpacityBar::getRawOpacities(int n)
{
    int nvalues2 = n;
    float *values2 = new float[nvalues2];
    if (nvalues2 > nvalues)
    {
        for (int i=0; i<nvalues2; i++)
            values2[i] = values[(i * nvalues) / nvalues2];
    }
    else
    {
        for (int i=0; i<nvalues; i++)
            values2[(i * nvalues2) / nvalues] = values[i];
    }

    return values2;    
}


// ****************************************************************************
//  Method:  QvisScribbleOpacityBar::setRawOpacities
//
//  Purpose:
//    Sets all of the opacities in the widget.
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 31, 2001
//
//  Modifications:
//    Brad Whitlock, Fri Apr 6 12:27:22 PDT 2001
//    I added code to emit a valueChanged signal.
//
// ****************************************************************************
void
QvisScribbleOpacityBar::setRawOpacities(int n, float *v)
{
    if (n < nvalues)
    {
        for (int i=0; i<nvalues; i++)
            values[i] = v[(i * n) / nvalues];
    }
    else
    {
        for (int i=0; i<n; i++)
            values[(i * nvalues) / n] = v[i];
    }

    paintToPixmap(contentsRect().width(), contentsRect().height());
    update();

    // Emit a signal indicating that the values changed.
    emit opacitiesChanged();
}

// ****************************************************************************
// Method: QvisScribbleOpacityBar::makeTotallyZero
//
// Purpose: 
//   This is a Qt slot function that sets all of the alpha values to zero
//   and updates the widget.
//
// Programmer: Brad Whitlock
// Creation:   Mon Feb 5 13:41:25 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisScribbleOpacityBar::makeTotallyZero()
{
    // Set all the alphas to zero.
    for(int i = 0; i < nvalues; ++i)
        values[i] = 0.;

    paintToPixmap(contentsRect().width(), contentsRect().height());
    update();

    // Emit a signal indicating that the values changed.
    emit opacitiesChanged();
}

// ****************************************************************************
// Method: QvisScribbleOpacityBar::makeLinearRamp
//
// Purpose: 
//   This is a Qt slot function that sets the alpha values to be a linear ramp.
//
// Programmer: Brad Whitlock
// Creation:   Mon Feb 5 13:42:01 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisScribbleOpacityBar::makeLinearRamp()
{
    // Make a ramp.
    for(int i = 0; i < nvalues; ++i)
        values[i] = float(i) * float(1. / nvalues);

    paintToPixmap(contentsRect().width(), contentsRect().height());
    update();

    // Emit a signal indicating that the values changed.
    emit opacitiesChanged();
}

// ****************************************************************************
// Method: QvisScribbleOpacityBar::makeTotallyOne
//
// Purpose: 
//   This is a Qt slot function that sets all of the alpha values to
//   maximum strength.
//
// Programmer: Brad Whitlock
// Creation:   Mon Feb 5 13:42:35 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisScribbleOpacityBar::makeTotallyOne()
{
    // Set all the alphas to 255.
    for(int i = 0; i < nvalues; ++i)
        values[i] = 1.;

    paintToPixmap(contentsRect().width(), contentsRect().height());
    update();

    // Emit a signal indicating that the values changed.
    emit opacitiesChanged();
}

// ****************************************************************************
// Method: QvisScribbleOpacityBar::smoothCurve
//
// Purpose: 
//   This is a Qt slot function that applies a simple filter to the alphas
//   in order to smooth them out.
//
// Programmer: Brad Whitlock
// Creation:   Mon Feb 5 13:43:15 PST 2001
//
// Modifications:
//   Brad Whitlock, Thu Feb 14 13:07:26 PST 2002
//   Fixed an ABR.
//
// ****************************************************************************

void
QvisScribbleOpacityBar::smoothCurve()
{
    // Smooth the curve
    for(int i = 1; i < nvalues - 1; ++i)
    {
        // 1 3 1 filter.
        float smooth = (0.2 * values[i - 1]) +
                       (0.6 * values[i]) + 
                       (0.2 * values[i + 1]);
        values[i] = (smooth > 1.) ? 1. : smooth;
    }

    paintToPixmap(contentsRect().width(), contentsRect().height());
    update();

    // Emit a signal indicating that the values changed.
    emit opacitiesChanged();
}
