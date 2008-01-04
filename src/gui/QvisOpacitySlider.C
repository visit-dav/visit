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

#include <QvisOpacitySlider.h>
#include <qbitmap.h>
#include <qcolor.h>
#include <qdrawutil.h>
#include <qimage.h>
#include <qpainter.h>
#include <qpalette.h>
#include <qstyle.h>
#include <qtimer.h>

// some static constants.
static const int thresholdTime = 500;
static const int repeatTime    = 100;
static int sliderStartVal = 0;

// ****************************************************************************
// Method: QvisOpacitySlider::QvisOpacitySlider
//
// Purpose: 
//   Constructor for the QvisOpacitySlider class.
//
// Arguments:
//   parent : The widget's parent.
//   name   : The widget's name.
//   data   : User data that is emitted with the valueChanged signal.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 12:14:41 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

QvisOpacitySlider::QvisOpacitySlider(QWidget *parent, const char *name,
    const void *data) : QWidget(parent, name)
{
    init();
    userData = data;
}

// ****************************************************************************
// Method: QvisOpacitySlider::QvisOpacitySlider
//
// Purpose: 
//   Constructor for the QvisOpacitySlider class.
//
// Arguments:
//   minValue : The minimum slider value.
//   maxValue : The maximum slider value.
//   step     : The page step value.
//   value    : The initial value.
//   parent   : The widget's parent.
//   name     : The widget's name.
//   data     : User data that is emitted with the valueChanged signal.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 12:14:41 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

QvisOpacitySlider::QvisOpacitySlider(int minValue, int maxValue, int pageStep,
    int value, QWidget *parent, const char *name, const void *data) :
    QWidget(parent, name), QRangeControl(minValue, maxValue, 1, pageStep,
                                         value)
{
    init();
    sliderVal = value;
    userData = data;
}

// ****************************************************************************
// Method: QvisOpacitySlider::~QvisOpacitySlider
//
// Purpose: 
//   Destructor for the QvisOpacitySlider class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 12:18:17 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

QvisOpacitySlider::~QvisOpacitySlider()
{
    deleteGradientPixmap();
}

// ****************************************************************************
// Method: QvisOpacitySlider::init
//
// Purpose: 
//   Initializes certain object attributes.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 12:18:36 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisOpacitySlider::init()
{
    gradientPixmap = 0;
    timer = 0;
    sliderPos = 0;
    sliderVal = 0;
    clickOffset = 0;
    state = Idle;
    tickInt = 0;
    gradientColor = colorGroup().background();
    setFocusPolicy(TabFocus);
    initTicks();
}

// ****************************************************************************
// Method: QvisOpacitySlider::initTicks
//
// Purpose: 
//   Does what's needed when someone changes the tickmark status.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 12:19:28 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisOpacitySlider::initTicks()
{
    tickOffset = height() - thickness();
}

// ****************************************************************************
// Method: QvisOpacitySlider::positionFromValue
//
// Purpose: 
//   Calculates slider position corresponding to a value.
//
// Arguments:
//   value : The value for which to calculate a position.
//
// Returns:    Slider position.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 12:20:07 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

int
QvisOpacitySlider::positionFromValue(int value) const
{
    return QRangeControl::positionFromValue(value, available());
}

// ****************************************************************************
// Method: QvisOpacitySlider::available
//
// Purpose: 
//   Returns the available space in which the slider can move.
//
// Returns:    The available space in which the slider can move
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 12:21:34 PDT 2000
//
// Modifications:
//   Brad Whitlock, Thu Nov 13 09:49:31 PDT 2003
//   I made it use pixmapWidth.
//
// ****************************************************************************

int
QvisOpacitySlider::available() const
{
    return pixmapWidth() - sliderLength();
}

// ****************************************************************************
// Method: QvisOpacitySlider::valueFromPosition
//
// Purpose: 
//   Calculates value corresponding to slider position.
//
// Returns:    Value corresponding to slider position.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 12:22:34 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

int
QvisOpacitySlider::valueFromPosition(int position) const
{
    return QRangeControl::valueFromPosition(position, available());
}

// ****************************************************************************
// Method: QvisOpacitySlider::rangeChange
//
// Purpose: 
//   Implements the virtual QRangeControl function.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 12:23:15 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisOpacitySlider::rangeChange()
{
    int newPos = positionFromValue(value());
    if(newPos != sliderPos)
        reallyMoveSlider( newPos );
}

// ****************************************************************************
// Method: QvisOpacitySlider::paletteChange
//
// Purpose: 
//   This method is called when the palette changes. The slider uses this
//   opportunity to redraw with its new gradient.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 6 14:55:16 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisOpacitySlider::paletteChange(const QPalette &)
{
    deleteGradientPixmap();
    update();
}

// ****************************************************************************
// Method: QvisOpacitySlider::valueChange
//
// Purpose: 
//   Implements the virtual QRangeControl function.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 12:23:57 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisOpacitySlider::valueChange()
{
    if(sliderVal != value())
    {
        int newPos = positionFromValue(value());
        sliderVal = value();
        reallyMoveSlider(newPos);
    }

    // emit the valueChanged signals.
    emit valueChanged(value());
    emit valueChanged(value(), userData);
}

// ****************************************************************************
// Method: QvisOpacitySlider::sliderRect
//
// Purpose: 
//   Returns the slider handle rectangle.
//
// Returns:    The slider handle rectangle.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 12:26:38 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

QRect
QvisOpacitySlider::sliderRect() const
{
    return QRect(sliderPos, tickOffset, sliderLength(), thickness());
}

// ****************************************************************************
// Method: QvisOpacitySlider::reallyMoveSlider
//
// Purpose: 
//   Performs the actual moving of the slider.
//
// Arguments:
//   newPos : The slider's new position.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 12:28:25 PDT 2000
//
// Modifications:
//   Brad Whitlock, Thu Nov 13 10:32:26 PDT 2003
//   I made it repaint the text value.
//
// ****************************************************************************

void 
QvisOpacitySlider::reallyMoveSlider(int newPos)
{
    QRect oldR = sliderRect();
    sliderPos = newPos;
    QRect newR = sliderRect();

    // Since sliderRect isn't virtual, I know that oldR and newR
    // are the same size.
    if(oldR.left() < newR.left())
        oldR.setRight(QMIN(oldR.right(), newR.left()));
    else
        oldR.setLeft(QMAX(oldR.left(), newR.right()));

    // If we're moving the slider, we have to update the text.
    int pmw = pixmapWidth();
    QRect newTextR(pmw, 0, width() - pmw, height());

    repaint(oldR);
    repaint(newR, FALSE);
    repaint(newTextR);

    if(autoMask())
        updateMask();
}

// ****************************************************************************
// Method: QvisOpacitySlider::sliderLength
//
// Purpose: 
//   Returns the width of the slider handle.
//
// Returns:    The width of the slider handle.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 12:38:54 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

int
QvisOpacitySlider::sliderLength() const
{
   return 11;
}

// ****************************************************************************
// Method: QvisOpacitySlider::maximumSliderDragDistance
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 12:39:24 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

int
QvisOpacitySlider::maximumSliderDragDistance() const
{
    return 20;
}

// ****************************************************************************
// Method: QvisOpacitySlider::paintSlider
//
// Purpose: 
//   Paints the slider button.
//
// Arguments:
//   p : The painter used to paint the slider.
//   r : The slider rectangle.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 12:27:21 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisOpacitySlider::paintSlider( QPainter *p, const QColorGroup &,
    const QRect &r )
{
    QPoint bo = p->brushOrigin();
    p->setBrushOrigin(r.topLeft());

    drawSlider(p, r.x(), r.y(), r.width(), r.height());
    p->setBrushOrigin(bo);
}

// ****************************************************************************
// Method: QvisOpacitySlider::drawSliderMask
//
// Purpose: 
//   Draws the slider mask.
//
// Arguments:
//   p : The painter used to draw.
//   x : The x location of the mask.
//   y : The y location of the mask.
//   w : The mask's width.
//   h : The mask's height.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 12:39:43 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisOpacitySlider::drawSliderMask(QPainter *p, int x, int y, int w, int h)
{
    int x1 = x;
    int x2 = x+w-1;
    int y1 = y;
    int y2 = y+h-1;

    y1 = y1 + w/2;

    QPointArray a;
    a.setPoints(5, x1,y1, x1 + w/2, y1 - w/2, x2,y1, x2,y2, x1,y2);

    p->setBrush(color1);
    p->setPen(color1);
    p->drawPolygon( a );
}

// ****************************************************************************
// Method: QvisOpacitySlider::drawSliderGroove
//
// Purpose: 
//   Draws the groove on which the slider handle slides.
//
// Arguments:
//   p      : The painter with which to draw.
//   x      : The groove's x location.
//   x      : The groove's x location.
//   width  : The groove's width.
//   height : The groove's height.
//   c      : A coordinate.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 12:37:18 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisOpacitySlider::drawSliderGroove(QPainter *p, int x, int y, int w, int,
    QCOORD c)
{
    qDrawWinPanel(p, x, y + c - 2,  w, 4, colorGroup(), TRUE);
    p->setPen(colorGroup().shadow());
    p->drawLine(x+1, y + c - 1, x + w - 3, y + c - 1);
}

// ****************************************************************************
// Method: QvisOpacitySlider::drawSlider
//
// Purpose: 
//   Draws the slider handle.
//
// Arguments:
//   p      : The painter used to do the drawing.
//   x      : The x position of the slider handle.
//   y      : The y position of the slider handle.
//   width  : The width of the slider handle.
//   height : The height of the slider handle.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 12:32:38 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisOpacitySlider::drawSlider(QPainter *p, int x, int y, int w, int h)
{
    // Get some colors from the widget's colorgroup.
    const QColor c0 = colorGroup().shadow();
    const QColor c1 = colorGroup().dark();
    const QColor c3 = colorGroup().midlight();
    const QColor c4 = colorGroup().light();

    int x1 = x;
    int x2 = x+w-1;
    int y1 = y;
    int y2 = y+h-1;
    int d = 0;
    y1 = y1 + w/2;
    d =  (w + 1) / 2 - 1;

    QPointArray a;
    a.setPoints(5, x1,y1, x1,y2, x2,y2, x2,y1, x1+d,y1-d);

    QBrush oldBrush = p->brush();
    p->setBrush(colorGroup().brush(QColorGroup::Button));
    p->setPen(NoPen);
    p->drawRect(x1, y1, x2-x1+1, y2-y1+1);
    p->drawPolygon(a);
    p->setBrush(oldBrush);

    p->setPen(c3);
    p->drawLine(x1+1, y1+1, x1+1, y2);
    p->setPen(c4);
    p->drawLine(x1, y1, x1, y2);
    p->setPen(c0);
    p->drawLine( x2, y1, x2, y2);
    p->setPen(c1);
    p->drawLine( x2-1, y1+1, x2-1, y2-1);
    p->setPen(c0);
    p->drawLine(x1, y2, x2, y2);
    p->setPen(c1);
    p->drawLine(x1+1, y2-1, x2-1, y2-1);
    p->setPen(c4);
    p->drawLine(x1, y1, x1+d, y1-d);
    p->setPen(c0);
    d = w - d - 1;
    p->drawLine(x2, y1, x2-d, y1-d);
    --d;
    p->setPen(c3);
    p->drawLine(x1+1, y1, x1+1+d, y1-d);
    p->setPen(c1);
    p->drawLine(x2-1, y1, x2-1-d, y1-d);
}

// ****************************************************************************
// Method: QvisOpacitySlider::drawTicks
//
// Purpose: 
//   Draws the slider's tick marks.
//
// Arguments:
//   p    : The painter used to draw.
//   dist : The distance from the edge of the widget.
//   w    : The number of pixels between the ticks.
//   i    : The tick interval
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 13:13:53 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisOpacitySlider::drawTicks(QPainter *p, int dist, int w, int i) const
{
    drawTicks(p, colorGroup(), dist, w, i);
}

// ****************************************************************************
// Method: QvisOpacitySlider::drawTicks
//
// Purpose: 
//   Draws the slider's tick marks.
//
// Arguments:
//   p    : The painter used to draw.
//   g    : The color group containing common GUI colors.
//   dist : The distance from the edge of the widget.
//   w    : The number of pixels between the ticks.
//   i    : The tick interval
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 13:13:53 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisOpacitySlider::drawTicks( QPainter *p, const QColorGroup& g, int dist,
    int w, int i) const
{
    p->setPen( g.foreground() );
    int v = minValue();
    int fudge = sliderLength() / 2 + 1;
    while(v <= maxValue() + 1)
    {
        int pos = positionFromValue(v) + fudge;
        p->drawLine( pos, dist, pos, dist + w );
        v += i;
    }
}

// ****************************************************************************
// Method: QvisOpacitySlider::textPadding
//
// Purpose: 
//   Returns the distance from the slider to the text.
//
// Returns:    The distance from the slider to the text.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 13 10:23:56 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

int
QvisOpacitySlider::textPadding() const
{
    return fontMetrics().width("0") / 2;
}

// ****************************************************************************
// Method: QvisOpacitySlider::pixmapWidth
//
// Purpose: 
//   Returns the width of the pixmap area, which is the width of the whole
//   widget minus the width if the text that we want to display.
//
// Returns:    The width of the pixmap area.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 13 09:45:41 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

int
QvisOpacitySlider::pixmapWidth() const
{
    return width() - fontMetrics().width("100%") - textPadding();
}

// ****************************************************************************
// Method: QvisOpacitySlider::paintValueText
//
// Purpose: 
//   Draws the value text at the specified location.
//
// Arguments:
//   p : The painter to use to draw the text.
//   x : The x location at which to draw the text.
//   h : The vertical height that we have to work with.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 13 10:19:22 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisOpacitySlider::paintValueText(QPainter *p, const QColorGroup &cg, int x,
    int h)
{
    // Create the text that we have to display.
    int v = (state == Dragging) ? (valueFromPosition(sliderPos)) : value();
    float t = float(v - minValue()) / float(maxValue() - minValue());
    QString txt; txt.sprintf("%d%%", int(t * 100.f));

    // Figure out the y offset.
    int dy = h - fontMetrics().height();
    int y = (h - dy / 2);

    // Set the brush and draw the text.
    p->setPen(cg.text());
    p->drawText(x + textPadding(), y, txt);
}

// ****************************************************************************
// Method: QvisOpacitySlider::createGradientPixmap
//
// Purpose: 
//   Creates the opacity gradient pixmap that is used as the slider's 
//   background pixmap.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 12:30:35 PDT 2000
//
// Modifications:
//   Brad Whitlock, Wed Jan 30 14:16:41 PST 2002
//   Added code to draw checkerboard over the gradient if the widget
//   is disabled.
//
//   Brad Whitlock, Thu Aug 21 17:31:30 PST 2003
//   Added code to do alpha blending with a pixmap background so it looks
//   like it's supposed to look on MacOS X.
//
//   Brad Whitlock, Thu Nov 13 09:47:15 PDT 2003
//   I made it use a smaller width so we can display the percent.
//
// ****************************************************************************

void
QvisOpacitySlider::createGradientPixmap()
{
    // Create the pixmap.
    int w = pixmapWidth();
    int h = height() - tickOffset;
    gradientPixmap = new QPixmap(w, h);

    QBrush brush(colorGroup().brush(QColorGroup::Background));
    QPainter paint(gradientPixmap);

    if(brush.pixmap())
    {
        // Paint the background into the pixmap.
        paint.fillRect(0, 0, w, h, colorGroup().brush(QColorGroup::Background));
        QImage img = gradientPixmap->convertToImage();
        if(!img.isNull())
        {
            for(int i = 0; i < w; ++i)
            {
                float t = float(i) / float(w - 1);
                float omt = 1.f - t;
                int   rc, gc, bc;
                for(int j = 0; j < h; ++j)
                {
                    // Alpha blend with the pixel that's there already.
                    QRgb p = img.pixel(i, j);
                    rc = int(omt * float(qRed(p)) + t * float(gradientColor.red()));
                    rc = (rc > 255) ? 255 : rc;
                    gc = int(omt * float(qGreen(p)) + t * float(gradientColor.green()));
                    gc = (gc > 255) ? 255 : gc;
                    bc = int(omt * float(qBlue(p)) + t * float(gradientColor.blue()));
                    bc = (bc > 255) ? 255 : bc;
                    img.setPixel(i, j, qRgb(rc, gc, bc));
                }
            }

            if(!gradientPixmap->convertFromImage(img))
            {
                qDebug("QvisOpacitySlider::createGradientPixmap: "
                       "Could not convert the image to a pixmap!");
            }
        }
        else
        {
            qDebug("QvisOpacitySlider::createGradientPixmap: "
                   "Could not create the initial image from the pixmap.");
        }
    }
    else
    {
        // Now draw the color gradient into the pixmap.
        float invWidth = 1.0 / ((float)w);
        float deltaRed = gradientColor.red() - colorGroup().background().red();
        float deltaGreen = gradientColor.green() - colorGroup().background().green();
        float deltaBlue = gradientColor.blue() - colorGroup().background().blue();

        deltaRed *= invWidth;
        deltaGreen *= invWidth;
        deltaBlue *= invWidth;

        float red = (float)colorGroup().background().red();
        float green = (float)colorGroup().background().green();
        float blue = (float)colorGroup().background().blue();

        for(int i = 0; i < w; ++i)
        {
            int currentRed = (int)red;
            int currentGreen = (int)green;
            int currentBlue = (int)blue;

            // Clamp values just in case they went out of range.
            if(currentRed < 0)
                currentRed = 0;
            else if(currentRed > 255)
                currentRed = 255;

            if(currentGreen < 0)
                currentGreen = 0;
            else if(currentGreen > 255)
                currentGreen = 255;

            if(currentBlue < 0)
                currentBlue = 0;
            else if(currentBlue > 255)
                currentBlue = 255;

            QColor tempColor(currentRed, currentGreen, currentBlue);
            paint.setPen(tempColor);
            paint.drawLine(i, 0, i, h);

            red += deltaRed;
            green += deltaGreen;
            blue += deltaBlue;
        }
    }

    // If the widget is disabled then draw a checkerboard over it.
    if(!isEnabled())
    {
        QBrush brush2(colorGroup().background());
        brush2.setStyle(QBrush::Dense6Pattern);
        paint.fillRect(QRect(0, 0, w, h), brush2);
    }
}

// ****************************************************************************
// Method: QvisOpacitySlider::deleteGradientPixmap
//
// Purpose: 
//   Delete the gradient pixmap.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 12:31:26 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisOpacitySlider::deleteGradientPixmap()
{
    if(gradientPixmap != 0)
    {
        delete gradientPixmap;
        gradientPixmap = 0;
    }
}

// ****************************************************************************
// Method: QvisOpacitySlider::setGradientColor
//
// Purpose: 
//   Sets the color used to generate the gradient pixmap.
//
// Arguments:
//   color : The new gradient color.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 12:31:44 PDT 2000
//
// Modifications:
//   Brad Whitlock, Mon Mar 5 14:55:23 PST 2001
//   Added code to prevent updates if the new color is the same as the old
//   color.
//
// ****************************************************************************

void
QvisOpacitySlider::setGradientColor(const QColor &color)
{
    if(gradientColor != color)
    {
        gradientColor = color;
        deleteGradientPixmap();

        if(isVisible())
            update();
    }
}

// ****************************************************************************
// Method: QvisOpacitySlider::resizeEvent
//
// Purpose: 
//   Called when the widget needs to be resized.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 12:24:59 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisOpacitySlider::resizeEvent(QResizeEvent *)
{
    rangeChange();
    initTicks();

    // Delete the gradient pixmap so it will be regenerated before the next
    // paint event.
    deleteGradientPixmap();

    if(autoMask())
        updateMask();
}

// ****************************************************************************
// Method: QvisOpacitySlider::paintEvent
//
// Purpose: 
//   This method is called when the widget gets a paint event. It redraws the
//   widget.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 12:41:02 PDT 2000
//
// Modifications:
//   Brad Whitlock, Mon Mar 11 12:42:43 PDT 2002
//   Upgraded to Qt 3.0.
//
//   Brad Whitlock, Thu Aug 21 17:33:54 PST 2003
//   I fixed it so it looks better on MacOS X.
//
//   Brad Whitlock, Thu Nov 13 09:48:31 PDT 2003
//   I made the width smaller so we could display the percent.
//
// ****************************************************************************

void
QvisOpacitySlider::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    int mid = thickness()/2 + sliderLength() / 8;

    // Draw the gradient pixmap.
    if(gradientPixmap == 0)
        createGradientPixmap();
    p.drawPixmap(0, tickOffset, *gradientPixmap);

    // Draw the groove on which the slider slides.    
    drawSliderGroove(&p, 0, tickOffset, pixmapWidth(), thickness(), mid);

    // Figure out the interval between the tick marks.
    int interval = tickInt;
    if(interval <= 0)
    {
        interval = lineStep();
        if(positionFromValue(interval) - positionFromValue(0) < 3)
            interval = pageStep();
    }

    // Draw the tick marks.
    p.fillRect(0, 0, pixmapWidth(), tickOffset,
               colorGroup().brush(QColorGroup::Background));
    p.fillRect(0, tickOffset + thickness(), pixmapWidth(), height(),
               colorGroup().brush(QColorGroup::Background));
    drawTicks(&p, colorGroup(), 0, tickOffset - 2, interval);

    // If this widget has focus, draw the focus rectangle.
    if(hasFocus())
    {
        QRect r;
        r.setRect( 0, tickOffset-1, pixmapWidth(), thickness()+2 );
        r = r.intersect(rect());
#if QT_VERSION >= 300
        style().drawPrimitive(QStyle::PE_FocusRect, &p, r, colorGroup(),
                              QStyle::Style_HasFocus);
#else
        style().drawFocusRect(&p, r, colorGroup());
#endif
    }

    // Draw the slider
    paintSlider(&p, colorGroup(), sliderRect());

    // Draw the value text.
    paintValueText(&p, colorGroup(), pixmapWidth(), height());
}

// ****************************************************************************
// Method: QvisOpacitySlider::updateMask
//
// Purpose: 
//   Reimplementation of QWidget::updateMask(). Draws the mask of the
//   slider when transparency is required.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 12:42:23 PDT 2000
//
// Modifications:
//   Brad Whitlock, Mon Mar 11 12:46:13 PDT 2002
//   Upgraded to Qt 3.0.
//
// ****************************************************************************

void
QvisOpacitySlider::updateMask()
{
    QBitmap bm( size() );
    bm.fill( color0 );

    QPainter p( &bm, this );
    QRect sliderR = sliderRect();
    QColorGroup g(color1, color1, color1, color1, color1, color1, color1, color1, color0);
    QBrush fill (color1);
    int mid = tickOffset + thickness()/2;
    mid += sliderLength() / 8;

    drawSliderMask(&p, sliderR.x(), sliderR.y(), sliderR.width(),
                   sliderR.height());

    int interval = tickInt;
    if(interval <= 0 )
    {
        interval = lineStep();
        if(positionFromValue(interval) - positionFromValue(0) < 3)
        interval = pageStep();
    }

    // Draw the tick marks
    drawTicks(&p, g, 0, tickOffset - 2, interval);

    setMask(bm);
}

// ****************************************************************************
// Method: QvisOpacitySlider::mousePressEvent
//
// Purpose: 
//   This method is called when the mouse is pressed in this widget.
//
// Arguments:
//   e : The mouse event.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 12:43:54 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisOpacitySlider::mousePressEvent(QMouseEvent *e)
{
    resetState();
    sliderStartVal = sliderVal;
    QRect r = sliderRect();

    if(e->button() == RightButton)
    {
        return;
    }
    else if(r.contains(e->pos()))
    {
        state = Dragging;
        clickOffset = (QCOORD)(goodPart( e->pos() ) - sliderPos);
//    emit sliderPressed();
    }
    else if(e->button() == MidButton)
    {
        int pos = goodPart(e->pos());
        moveSlider(pos - sliderLength() / 2);
        state = Dragging;
        clickOffset = sliderLength() / 2;
    }
    else if(e->pos().x() < r.left())
    {
        state = TimingDown;
        subtractPage();
        if(!timer)
            timer = new QTimer(this);
        connect( timer, SIGNAL(timeout()), SLOT(repeatTimeout()));
        timer->start( thresholdTime, TRUE );
    }
    else if(e->pos().x() > r.right())
    {
        state = TimingUp;
        addPage();
        if(!timer)
            timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), SLOT(repeatTimeout()));
        timer->start( thresholdTime, TRUE );
    }
}

// ****************************************************************************
// Method: QvisOpacitySlider::mouseMoveEvent
//
// Purpose: 
//   This method is called when the mouse is moved in this widget.
//
// Arguments:
//   e : The mouse move event.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 12:46:37 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisOpacitySlider::mouseMoveEvent(QMouseEvent *e)
{
    if(state != Dragging)
        return;

    QRect r = rect();
    int m = maximumSliderDragDistance();
    if(m >= 0)
    {
        r.setRect(r.x() - m, r.y() - 2*m/3,
                  r.width() + 2*m, r.height() + 3*m);
        if(!r.contains(e->pos()))
        {
            moveSlider( positionFromValue( sliderStartVal) );
            return;
        }
    }

    moveSlider(goodPart(e->pos()) - clickOffset );
}

// ****************************************************************************
// Method: QvisOpacitySlider::wheelEvent
//
// Purpose: 
//   This method is called when the widget gets a wheel event.
//
// Arguments:
//   e : The wheel event.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 12:48:10 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisOpacitySlider::wheelEvent(QWheelEvent * e)
{
    static float offset = 0;
    static QvisOpacitySlider* offset_owner = 0;
    if(offset_owner != this)
    {
        offset_owner = this;
        offset = 0;
    }
    offset += -e->delta()*QMAX(pageStep(),lineStep())/120;
    if(QABS(offset)<1)
        return;
    setValue( value() + int(offset) );
    offset -= int(offset);
}

// ****************************************************************************
// Method: QvisOpacitySlider::mouseReleaseEvent
//
// Purpose: 
//   This method is called when the mouse button is released in the widget.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 12:48:56 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisOpacitySlider::mouseReleaseEvent(QMouseEvent *)
{
    resetState();
}

// ****************************************************************************
// Method: QvisOpacitySlider::moveSlider
//
// Purpose: 
//   Moves the left (or top) edge of the slider to position pos. Performs
//   snapping.
//
// Arguments:
//   pos : The slider's new position.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 12:51:00 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisOpacitySlider::moveSlider(int pos)
{
    int newPos = QMIN(available(), QMAX( 0, pos));
    int newVal = valueFromPosition(newPos);
//    if ( sliderVal != newVal ) {
//    sliderVal = newVal;
//    emit sliderMoved( sliderVal );
//    }

    newPos = positionFromValue( newVal );

    if(sliderPos != newPos)
        reallyMoveSlider(newPos);
}

// ****************************************************************************
// Method: QvisOpacitySlider::resetState
//
// Purpose: 
//   Resets all state information and stops the timer
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 12:52:13 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisOpacitySlider::resetState()
{
    if(timer )
    {
        timer->stop();
        timer->disconnect();
    }

    switch(state)
    {
    case TimingUp:
    case TimingDown:
        break;
    case Dragging:
        setValue(valueFromPosition(sliderPos));
//        emit sliderReleased();
        break;
    case Idle:
        break;
    }

    state = Idle;
}

// ****************************************************************************
// Method: QvisOpacitySlider::keyPressEvent
//
// Purpose: 
//   Called when the widget needs to respond to a key press.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 12:53:45 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisOpacitySlider::keyPressEvent(QKeyEvent *e)
{
    switch(e->key())
    {
    case Key_Left:
        subtractLine();
        break;
    case Key_Right:
        addLine();
        break;
    case Key_Prior:
        subtractPage();
        break;
    case Key_Next:
        addPage();
        break;
    case Key_Home:
        setValue( minValue() );
        break;
    case Key_End:
        setValue( maxValue() );
        break;
    default:
        e->ignore();
    }
}

// ****************************************************************************
// Method: QvisOpacitySlider::setValue
//
// Purpose: 
//   Makes QRangeControl::setValue() available as a slot.
//
// Arguments:
//   value : The slider's new value.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 12:55:39 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisOpacitySlider::setValue(int value)
{
    QRangeControl::setValue(value);
}

// ****************************************************************************
// Method: QvisOpacitySlider::setEnabled
//
// Purpose: 
//   Sets the widget's enabled state and causes it to repaint if necessary.
//
// Arguments:
//   val : The new enabled state.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 31 09:25:55 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisOpacitySlider::setEnabled(bool val)
{
    if(isEnabled() != val)
        deleteGradientPixmap();

    QWidget::setEnabled(val);
}

// ****************************************************************************
// Method: QvisOpacitySlider::addStep
//
// Purpose: 
//   Moves the slider one pageStep() upwards.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 13:20:52 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisOpacitySlider::addStep()
{
    addPage();
}

// ****************************************************************************
// Method: QvisOpacitySlider::subtractStep
//
// Purpose: 
//   Moves the slider one pageStep() downwards.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 13:21:16 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisOpacitySlider::subtractStep()
{
    subtractPage();
}

// ****************************************************************************
// Method: QvisOpacitySlider::repeatTimeout
//
// Purpose: 
//   Waits for autorepeat.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 13:21:35 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisOpacitySlider::repeatTimeout()
{
    ASSERT(timer);
    timer->disconnect();
    if(state == TimingDown)
        connect(timer, SIGNAL(timeout()), SLOT(subtractStep()));
    else if ( state == TimingUp )
        connect(timer, SIGNAL(timeout()), SLOT(addStep()));
    timer->start(repeatTime, FALSE);
}

// ****************************************************************************
// Method: QvisOpacitySlider::goodPart
//
// Purpose: 
//   Returns the x coordinate of p.
//
// Arguments:
//   p : The point of interest.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 13:22:21 PST 2000
//
// Modifications:
//   
// ****************************************************************************

int
QvisOpacitySlider::goodPart(const QPoint &p) const
{
    return p.x();
}

// ****************************************************************************
// Method: QvisOpacitySlider::sizeHint
//
// Purpose: 
//   Returns the widget's preferred size.
//
// Returns:    The widget's preferred size.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 13:20:15 PST 2000
//
// Modifications:
//   
// ****************************************************************************

QSize
QvisOpacitySlider::sizeHint() const
{
    constPolish();
    const int length = 150;
    int thick = 16;
    const int tickSpace = 5;

    thick += tickSpace;
    thick += sliderLength() / 4;

    return QSize(length, thick);
}

// ****************************************************************************
// Method: QvisOpacitySlider::minimumSizeHint
//
// Purpose: 
//   Returns the widget's minimum size.
//
// Returns:    The widget's minimum size.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 13:19:43 PST 2000
//
// Modifications:
//   
// ****************************************************************************

QSize
QvisOpacitySlider::minimumSizeHint() const
{
    QSize s = sizeHint();
    s.setWidth(sliderLength());

    return s;
}

// ****************************************************************************
// Method: QvisOpacitySlider::sizePolicy
//
// Purpose: 
//   Returns the widget's size policy.
//
// Returns:    The widget's size policy.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 13:17:36 PST 2000
//
// Modifications:
//   
// ****************************************************************************

QSizePolicy
QvisOpacitySlider::sizePolicy() const
{
    return QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
}

// ****************************************************************************
// Method: QvisOpacitySlider::thickness
//
// Purpose: 
//   Returns the number of pixels to use for the business part of the
//    slider (i.e. the non-tickmark portion). The remaining space is shared
//    equally between the tickmark regions. This function and  sizeHint()
//    are closely related; if you change one, you almost certainly
//    have to change the other.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 13:18:07 PST 2000
//
// Modifications:
//   
// ****************************************************************************

int
QvisOpacitySlider::thickness() const
{
    int space = height();
    int thick = 6 + sliderLength() / 4;
    space -= thick;

    if(space > 0)
    thick += (space * 2) / 3;

    return thick;
}

// ****************************************************************************
// Method: QvisOpacitySlider::setTickInterval
//
// Purpose: 
//   Sets a new tick interval.
//
// Arguments:
//   i : The new tick interval.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 13:18:58 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisOpacitySlider::setTickInterval(int i)
{
    tickInt = QMAX(0, i);
    update();
    if(autoMask())
        updateMask();
}

// ****************************************************************************
// Method: QvisOpacitySlider::minValue
//
// Purpose: 
//   Returns the slider's min value.
//
// Returns:    The slider's min value.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 13:23:58 PST 2000
//
// Modifications:
//   
// ****************************************************************************

int
QvisOpacitySlider::minValue() const
{
    return QRangeControl::minValue();
}

// ****************************************************************************
// Method: QvisOpacitySlider::maxValue
//
// Purpose: 
//   Returns the slider's max value.
//
// Returns:    The slider's max value.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 13:24:34 PST 2000
//
// Modifications:
//   
// ****************************************************************************

int
QvisOpacitySlider::maxValue() const
{
    return QRangeControl::maxValue();
}

// ****************************************************************************
// Method: QvisOpacitySlider::setMinValue
//
// Purpose: 
//   Sets the slider's minimum value.
//
// Arguments:
//   i : The slider's new min value.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 13:25:12 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisOpacitySlider::setMinValue(int i)
{
    setRange(i, maxValue());
}

// ****************************************************************************
// Method: QvisOpacitySlider::setMaxValue
//
// Purpose: 
//   Sets the slider's max value.
//
// Arguments:
//   i : The slider's new max value.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 13:26:03 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void QvisOpacitySlider::setMaxValue( int i )
{
    setRange( minValue(), i );
}

// ****************************************************************************
// Method: QvisOpacitySlider::lineStep
//
// Purpose: 
//   Returns the slider's linestep.
//
// Returns:    The slider's linestep.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 13:28:08 PST 2000
//
// Modifications:
//   
// ****************************************************************************

int
QvisOpacitySlider::lineStep() const
{
    return QRangeControl::lineStep();
}

// ****************************************************************************
// Method: QvisOpacitySlider::pageStep
//
// Purpose: 
//   Returns the slider's pagestep.
//
// Returns:    The slider's pagestep.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 13:28:36 PST 2000
//
// Modifications:
//   
// ****************************************************************************

int QvisOpacitySlider::pageStep() const
{
    return QRangeControl::pageStep();
}

// ****************************************************************************
// Method: QvisOpacitySlider::setLineStep
//
// Purpose: 
//   Sets the slider's line step.
//
// Arguments:
//   i : The new line step.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 13:29:15 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisOpacitySlider::setLineStep(int i)
{
    setSteps(i, pageStep());
}

// ****************************************************************************
// Method: QvisOpacitySlider::setPageStep
//
// Purpose: 
//   Sets the slider's page step.
//
// Arguments:
//   i : The new page step.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 13:29:48 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisOpacitySlider::setPageStep(int i)
{
    setSteps(lineStep(), i);
}

// ****************************************************************************
// Method: QvisOpacitySlider::value
//
// Purpose: 
//   Returns the slider's current value.
//
// Returns:    The slider's current value.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 13:30:31 PST 2000
//
// Modifications:
//   
// ****************************************************************************

int
QvisOpacitySlider::value() const
{
    return QRangeControl::value();
}
