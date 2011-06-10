/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

#include <ColorControlPointList.h>

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
//    Brad Whitlock, Thu Dec 18 10:56:33 PST 2008
//    I added histogram textures.
//
//    Brad Whitlock, Mon Dec 27 15:37:41 PST 2010
//    I added histogramColor.
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
    histTexture = 0;
    histTextureMask = 0;
    histTextureSize = 0;
    histogramColor = QColor(30,30,30);
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
//    Brad Whitlock, Thu Dec 18 10:56:58 PST 2008
//    Deleted histTexture.
//
// ****************************************************************************

QvisAbstractOpacityBar::~QvisAbstractOpacityBar()
{
    delete image;
    image = 0;
    if(histTexture != 0)
        delete [] histTexture;
    if(histTextureMask != 0)
        delete [] histTextureMask;
}

void
QvisAbstractOpacityBar::imageDirty()
{
    delete image;
    image = 0;
}

// ****************************************************************************
//  Method:  QvisAbstractOpacityBar::setBackgroundColorControlPoints
//
//  Purpose: Set color control points for color transfer function backdrop
//    
//
//  Programmer:  Gunther H. Weber
//  Creation:    April 5, 2007
//
//  Modifications:
//    Brad Whitlock, Thu Dec 18 14:09:40 PST 2008
//    I changed how the image gets invalidated.
//
// ****************************************************************************

void
QvisAbstractOpacityBar::setBackgroundColorControlPoints(const ColorControlPointList *ccp)
{
    backgroundColorControlPoints = ccp;
    imageDirty();
    update();
}

// ****************************************************************************
// Method: QvisAbstractOpacityBar::setHistogramTexture
//
// Purpose: 
//   Sets the histogram texture data that we use to draw the histogram curve.
//
// Arguments:
//   t  : The data.
//   ts : The size of the data.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 18 14:10:00 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisAbstractOpacityBar::setHistogramTexture(const float *t, int ts)
{
    setHistogramTexture(t, 0, ts);
}

void
QvisAbstractOpacityBar::setHistogramTexture(const float *t, const bool *tm, int ts)
{
    if(t == 0)
    {
        if(histTexture != 0)
            delete [] histTexture;
        if(histTextureMask != 0)
            delete [] histTextureMask;
        histTexture = 0;
        histTextureMask = 0;
        histTextureSize = 0;
    }
    else
    {
        // Copy the texture.        
        if(ts != histTextureSize)
        {
            if(histTexture != 0)
                delete [] histTexture;
            histTexture = new float[ts];
        }
        histTextureSize = ts;
        memcpy(histTexture, t, sizeof(float) * histTextureSize);

        // Copy the mask
        if(histTextureMask != 0)
        {
            delete [] histTextureMask;
            histTextureMask = 0;
        }
        if(tm != 0)
        {
            histTextureMask = new bool[ts];
            memcpy(histTextureMask, tm, sizeof(bool) * histTextureSize);
        }
    }

    imageDirty();
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
// Method: QvisAbstractOpacityBar::drawColorBackground
//
// Purpose: 
//   This method draws the background colors into the image.
//
// Note:       Taken from some code the Gunther Weber wrote.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 18 14:07:34 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisAbstractOpacityBar::drawColorBackground()
{
    int w = contentsRect().width();
    int h = contentsRect().height();
    QRgb *bgCols = new QRgb[w];
    if(backgroundColorControlPoints)
    {
        unsigned char *cols = new unsigned char[w*3];
        backgroundColorControlPoints->GetColors(cols, w);
        for (int i=0; i < w; ++i)
            bgCols[i] = QColor(cols[i*3+0], cols[i*3+1], cols[i*3+2]).rgb();
        delete[] cols;
    }
    else 
    {
        QColor black(0,   0,   0 );
        QRgb cb = black.rgb();
        for (int i=0; i < w; ++i) 
            bgCols[i] = cb;
    }
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
            image->setPixel(x,y, bgCols[x]);
    }
    delete [] bgCols;
}

// ****************************************************************************
// Method: QvisAbstractOpacityBar::drawFilledCurve
//
// Purpose: 
//   This method draws a filled curve into the image, blending the pixels if
//   necessary.
//
// Arguments: 
//   curve : The curve to draw.
//   nc    : The length of the curve array.
//   cc    : The curve color.
//   opac  : The curve's opacity. If < 1. then the pixels get blended.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 18 14:08:16 PST 2008
//
// Modifications:
//   Brad Whitlock, Mon Dec 27 15:31:59 PST 2010
//   I made it call drawFilledCurveWithSelection.
//
// ****************************************************************************

void
QvisAbstractOpacityBar::drawFilledCurve(float *curve, bool *mask, int nc,
    const QColor &cc, float opac)
{
    float range[2] = {0.f, 1.f};
    drawFilledCurveWithSelection(curve, 0, nc, 
                                 cc, opac,
                                 cc, 0.f,
                                 cc, false,
                                 range, 0.f, 1.f);
}

// ****************************************************************************
// Method: QvisAbstractOpacityBar::drawFilledCurveWithSelection
//
// Purpose: 
//   This method draws a filled curve into the image, blending the pixels if
//   necessary.
//
// Arguments: 
//   curve           : The curve to draw.
//   mask            : A mask for the curve so we can turn off different bins.
//                     This can be NULL.
//   nc              : The length of the curve array.
//   curveOn         : The curve fill color when it is on.
//   curveOnOpacity  : The curve's opacity when it is on. If < 1. then the pixels
//                     get blended. Zero opacity pixels do not get drawn.
//   curveOff        : The curve fill color when it is off.
//   curveOffOpacity : The curve's opacity when it is off. If < 1. then the pixels
//                     get blended. Zero opacity pixels do not get drawn.
//   binLines        : The color for bin lines.
//   drawBinLines    : Draw the bin lines.
//   range           : The total range for the opacity bar.
//   minval          : The minimum range value that we'll draw.
//   maxval          : The maximum range value that we'll draw.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 27 15:32:58 PST 2010
//
// Modifications:
//   
// ****************************************************************************

void
QvisAbstractOpacityBar::drawFilledCurveWithSelection(float *curve, bool *mask, int nc,
    const QColor &curveOn,  float curveOnOpacity,
    const QColor &curveOff, float curveOffOpacity,
    const QColor &binLines, bool  drawBinLines,
    float range[2], float minval, float maxval)
{
    int w = contentsRect().width();
    int h = contentsRect().height();
    QRgb BL = binLines.rgb();

    for (int x = 0; x < w; x++)
    {
        float tx = float(x) / float(w-1);
        int   cx = qMin((nc-1), int(tx * nc));
        float yval = curve[cx];
        //qDebug("tx=%g, cx=%d, yval=%g", tx, cx, yval);

        // Determine whether we will need to draw the column
        bool drawColumn = true;
        if(mask != NULL)
            drawColumn = mask[cx];
        else
        {
            // Restrict the columns we draw to a range.
            float xval = (1.f-tx)*range[0] + tx*range[1];
            if(xval < minval || xval > maxval)
                drawColumn = false;
        }

        // Determine the proper color and opacity to use for the column
        // based on whether we're drawing it.
        QRgb color;
        float opacity;
        if(drawColumn)
        {
            color = curveOn.rgb();
            opacity = curveOnOpacity; 
        }
        else
        {
            color = curveOff.rgb();
            opacity = curveOffOpacity; 
        }

        // Draw the column
        if(opacity == 1.f)
        {
            // Draw full opacity.
            for (int y = 0; y < h; y++)
            { 
                float yval2 = 1 - float(y)/float(h-1);
                if (yval2 <= yval)
                    image->setPixel(x, y, color); 
            }
        }
        else if(opacity > 0.f)
        {
            // Blend the column with the background.
            for (int y = 0; y < h; y++)
            { 
                float yval2 = 1 - float(y)/float(h-1);
                if (yval2 <= yval)
                {
                    QRgb p = image->pixel(x, y);
                    int r = int((1.f - opacity)*float(qRed(p))   + opacity*float(qRed(color)));
                    int g = int((1.f - opacity)*float(qGreen(p)) + opacity*float(qGreen(color)));
                    int b = int((1.f - opacity)*float(qBlue(p))  + opacity*float(qBlue(color)));
                    image->setPixel(x,y, qRgb(r,g,b));
                }
            }
        }

        if(drawBinLines)
        {
            int yv = int((1.-yval) * (h-1));
            yv = qMin(yv, h-1);
            image->setPixel(x, yv, BL);
        }
    }

    // Draw the bin dividers
    if(drawBinLines)
    {
        for(int i = 1; i < histTextureSize+1; ++i)
        {
            float t = float(i) / float(histTextureSize);
            int x = int(t * w);
            if(x < w)
            {
                for (int y = 0; y < h; y++)
                    image->setPixel(x,y, BL);
            }
        }
    }
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
//    Brad Whitlock, Thu Dec 18 11:07:13 PST 2008
//    Added code to draw the background and a histogram.
//
//    Brad Whitlock, Mon Dec 27 15:39:24 PST 2010
//    Use histogramColor for the solid histogram color.
//
// ****************************************************************************

void
QvisAbstractOpacityBar::paintEvent(QPaintEvent *e)
{
    QFrame::paintEvent(e);

    int w = contentsRect().width();
    int h = contentsRect().height();
    if(ensureImageExists(w, h))
    {
        drawColorBackground();

        if(histTexture != 0)
        {
            if(backgroundColorControlPoints)
                drawFilledCurve(histTexture, histTextureMask, histTextureSize, QColor(0,0,0), 0.8f);
            else
                drawFilledCurve(histTexture, histTextureMask, histTextureSize, histogramColor, 1.f);
        }
        drawOpacities();
    }

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
//    Brad Whitlock, Thu Dec 18 14:09:40 PST 2008
//    I changed how the image gets invalidated.
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

    imageDirty();
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
