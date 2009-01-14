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

#include "QvisScribbleOpacityBar.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPolygon>
#include <QPixmap>
#include <QImage>

#include <visitstream.h>
#include <math.h>
#include <stdlib.h>

#include <ColorControlPointList.h>

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
//    Brad Whitlock, Wed Jun  4 09:20:11 PDT 2008
//    Qt 4.
//
// ****************************************************************************

QvisScribbleOpacityBar::QvisScribbleOpacityBar(QWidget *parent)
    : QvisAbstractOpacityBar(parent)
{
    setFrameStyle( QFrame::Panel | QFrame::Sunken );
    setLineWidth( 2 );
    setMinimumHeight(50);
    setMinimumWidth(128);

    nvalues = 256;
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
//  Modifications:
//    Gunther H. Weber, April 6, 2007
//    Added possibility of having a "color table" background instead of solid
//    black background.
//
//    Brad Whitlock, Wed Jun  4 09:55:53 PDT 2008
//    Qt 4.
//
//    Brad Whitlock, Thu Dec 18 11:45:20 PST 2008
//    I rewrote the method so it uses drawFilledCurve.
//
// ****************************************************************************

void
QvisScribbleOpacityBar::drawOpacities()
{
    QColor white(255, 255, 255 );
    if(histTexture != 0)
        drawFilledCurve(values, nvalues, white, 0.7f);
    else
        drawFilledCurve(values, nvalues, white, 1.f);
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
//  Modifications:
//    Brad Whitlock, Wed Jun  4 09:58:43 PDT 2008
//    Use update().
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

    imageDirty();
    update();
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
//  Modifications:
//    Brad Whitlock, Wed Jun  4 09:58:43 PDT 2008
//    Use update().
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

    imageDirty();
    update();
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
//  Modifications:
//    Brad Whitlock, Wed Jun  4 09:58:43 PDT 2008
//    Use update().
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

    imageDirty();
    update();

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
//  Modifications:
//    Gunther H. Weber, April 6, 2007
//    Added possibility of having a "color table" background instead of solid
//    black background.
//
//    Brad Whitlock, Wed Jun  4 10:24:39 PDT 2008
//    poke directly into the QImage.
//
//    Brad Whitlock, Thu Dec 18 11:46:45 PST 2008
//    I removed the code to poke into the image.
//
// ****************************************************************************

void
QvisScribbleOpacityBar::setValue(float xval, float yval)
{
    int x = int(xval * float(nvalues-1));
    values[x] = yval;
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
//    Brad Whitlock, Thu Dec 18 14:09:40 PST 2008
//    I changed how the image gets invalidated.
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

    imageDirty();
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
//    Brad Whitlock, Thu Dec 18 14:09:40 PST 2008
//    I changed how the image gets invalidated.
//   
// ****************************************************************************

void
QvisScribbleOpacityBar::makeTotallyZero()
{
    // Set all the alphas to zero.
    for(int i = 0; i < nvalues; ++i)
        values[i] = 0.;

    imageDirty();
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
//    Brad Whitlock, Thu Dec 18 14:09:40 PST 2008
//    I changed how the image gets invalidated.
//   
// ****************************************************************************

void
QvisScribbleOpacityBar::makeLinearRamp()
{
    // Make a ramp.
    for(int i = 0; i < nvalues; ++i)
        values[i] = float(i) * float(1. / nvalues);

    imageDirty();
    update();

    // Emit a signal indicating that the values changed.
    emit opacitiesChanged();
}

// ****************************************************************************
// Method: QvisScribbleOpacityBar::makeInverseLinearRamp
//
// Purpose: 
//   This is a Qt slot function that sets the alpha values to be an inverse
//   linear ramp.
//
// Programmer: Gunther H. Weber (based on makeLinearRamp() by Brad Whitlock)
// Creation:   Thu Apr  5 15:59:05 PDT 2007
//
// Modifications:
//   Brad Whitlock, Thu Dec 18 14:09:40 PST 2008
//   I changed how the image gets invalidated.
//   
// ****************************************************************************

void
QvisScribbleOpacityBar::makeInverseLinearRamp()
{
    // Make a ramp.
    for(int i = 0; i < nvalues; ++i)
        values[i] = float(nvalues - i - 1) * float(1. / nvalues);

    imageDirty();
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
//   Brad Whitlock, Thu Dec 18 14:09:40 PST 2008
//   I changed how the image gets invalidated.
//   
// ****************************************************************************

void
QvisScribbleOpacityBar::makeTotallyOne()
{
    // Set all the alphas to 255.
    for(int i = 0; i < nvalues; ++i)
        values[i] = 1.;

    imageDirty();
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
//   Brad Whitlock, Thu Dec 18 14:09:40 PST 2008
//   I changed how the image gets invalidated.
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

    imageDirty();
    update();

    // Emit a signal indicating that the values changed.
    emit opacitiesChanged();
}
