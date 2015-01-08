/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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

#include <QvisHistogram.h>

#include <QMouseEvent>
#include <QPainter>

#include <ColorControlPointList.h>

// ****************************************************************************
// Method: QvisHistogram::QvisHistogram
//
// Purpose: 
//   Constructor
//
// Arguments:
//   parent : The widget's parent.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 29 13:53:56 PST 2010
//
// Modifications:
//   
// ****************************************************************************

QvisHistogram::QvisHistogram(QWidget *parent) : QvisAbstractOpacityBar(parent)
{
    drawBinLines = false;

    totalRangeValid = false;
    totalRange[0] = 0.f;
    totalRange[1] = 1.f;

    selectedRange[0] = 0.f;
    selectedRange[1] = 1.f;

    selectionEnabled = true;
    selection = -1;
    tempRange[0] = 0.f;
    tempRange[1] = 0.f;

    // Make it brighter
    histogramColor = QColor(100,100,100);
}

// ****************************************************************************
// Method: QvisHistogram::~QvisHistogram
//
// Purpose: 
//   Destructor
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 29 13:54:15 PST 2010
//
// Modifications:
//   
// ****************************************************************************

QvisHistogram::~QvisHistogram()
{
}

void
QvisHistogram::setDrawBinLines(bool val)
{
    drawBinLines = val;
    imageDirty();
    update();
}


bool
QvisHistogram::getDrawBinLines() const
{
    return drawBinLines;
}

void
QvisHistogram::setSelectionEnabled(bool val)
{
    selectionEnabled = val;
}

bool
QvisHistogram::getSelectionEnabled() const
{
    return selectionEnabled;
}

// ****************************************************************************
// Method: QvisHistogram::getRawOpacities
//
// Purpose: 
//   Implemented to satisfy base class pure virtual.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 29 13:54:31 PST 2010
//
// Modifications:
//   
// ****************************************************************************

float *
QvisHistogram::getRawOpacities(int)
{
    return 0;
}

void
QvisHistogram::setBackgroundColorControlPoints(const ColorControlPointList *)
{
}

// ****************************************************************************
// Method: QvisHistogram::drawOpacities
//
// Purpose: 
//   Draw the hilighted portion of the histogram.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 29 13:54:58 PST 2010
//
// Modifications:
//   
// ****************************************************************************

void
QvisHistogram::drawOpacities()
{
    QColor curveOn(255, 255, 255);
    QColor curveOff(150, 150, 150);
    QColor binLines(75,75,75);
    if(!isEnabled())
    {
        curveOn = QColor(150,150,150);
        curveOff = QColor(100, 100, 100);
    }

    // Here we have been called from the base class after the histogram has
    // been drawn. We need to draw it again, taking into account the min/max
    // so we only overdraw a portion of it.
    if(histTexture != 0)
    {
        drawFilledCurveWithSelection(histTexture, histTextureMask, histTextureSize,
            curveOn, 1.f,
            curveOff, 1.f,
            binLines, drawBinLines,
            totalRange, selectedRange[0], selectedRange[1]);
    }

    // If we're selecting then draw the area we're selecting
    if(selectionEnabled && selection != -1)
    {
        QColor red(255,0,0), orange(255,128,50);
        float range[2];
        range[0] = (selection == 0) ? totalRange[0] : tempRange[1];
        range[1] = (selection == 0) ? tempRange[0] : totalRange[1];
        float *oneTexture = new float[histTextureSize];
        for(int i = 0; i < histTextureSize; ++i)
            oneTexture[i] = 1.f;
        drawFilledCurveWithSelection(oneTexture, 0, histTextureSize,
            red, 0.7f,
            red, 0.f,
            orange, drawBinLines,
            totalRange, range[0], range[1]);
        delete [] oneTexture;
    }
}

// ****************************************************************************
// Method: QvisHistogram::paintEvent
//
// Purpose: 
//   Draw the opacity bar and also some red bounds indicators.
//
// Arguments:
//   e : The paint event.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 29 13:55:27 PST 2010
//
// Modifications:
//   Brad Whitlock, Mon Nov  7 15:51:00 PST 2011
//   I added special case text.
//
// ****************************************************************************

void
QvisHistogram::paintEvent(QPaintEvent *e)
{
    QvisAbstractOpacityBar::paintEvent(e);

    QPainter p(this);

    // Draw selection lines
    if(selectionEnabled)
    {
        // Now draw some indicator lines.
        QColor darkred(175/(isEnabled()?1:2),0,0),
               brightred(255/(isEnabled()?1:2),0,0);

        int x;
        x = value2Pixel(selection == 0 ? tempRange[0] : selectedRange[0]);
        x = qMax(x, contentsRect().left());
        p.setPen(selection==0 ? brightred : darkred);
        p.drawLine(x, contentsRect().top(),
                   x, contentsRect().top() + contentsRect().height());

        x = value2Pixel(selection == 1 ? tempRange[1] : selectedRange[1]);
        x = qMin(x, contentsRect().x() + contentsRect().width() - 1);
        p.setPen(selection==1 ? brightred : darkred);
        p.drawLine(x, contentsRect().top(),
                   x, contentsRect().top() + contentsRect().height());
    }

    // If there is no data, draw "No data" string across the widget.
    if(histTexture == 0)
    {
        p.setPen(Qt::white);
        p.drawText(contentsRect().x(), contentsRect().y(),
                   contentsRect().width(), contentsRect().height(),
                   Qt::AlignCenter | Qt::AlignVCenter,
                   tr("No data"));
    }
    else
    {
        // Examine the histogram texture and look for the case where there 
        // are just a few values.
        float sum = 0.f;
        for(int i = 0; i < histTextureSize; ++i)
            sum += histTexture[i];
        p.setPen(Qt::white);
        if(sum == 0.f)
        {
            p.drawText(contentsRect().x(), contentsRect().y(),
                       contentsRect().width(), contentsRect().height(),
                       Qt::AlignCenter | Qt::AlignVCenter,
                       tr("No data"));
        }
        else if(sum == histTexture[0])
        {
            QString minstr(tr("minimum"));
            if(totalRangeValid)
                minstr = QString().setNum(totalRange[0]);
            p.drawText(contentsRect().x(), contentsRect().y(),
                       contentsRect().width(), contentsRect().height(),
                       Qt::AlignCenter | Qt::AlignVCenter,
                       tr("All values are %1").arg(minstr));
        }
        else if(sum == histTexture[histTextureSize-1])
        {
            QString maxstr(tr("maximum"));
            if(totalRangeValid)
                maxstr = QString().setNum(totalRange[1]);
            p.drawText(contentsRect().x(), contentsRect().y(),
                       contentsRect().width(), contentsRect().height(),
                       Qt::AlignCenter | Qt::AlignVCenter,
                       tr("All values are %1").arg(maxstr));
        }
        else if(sum == (histTexture[0] + histTexture[histTextureSize-1]))
        {
            QString minstr(tr("minimum"));
            if(totalRangeValid)
                minstr = QString().setNum(totalRange[0]);
            QString maxstr(tr("maximum"));
            if(totalRangeValid)
                maxstr = QString().setNum(totalRange[1]);
            p.drawText(contentsRect().x(), contentsRect().y(),
                       contentsRect().width(), contentsRect().height(),
                       Qt::AlignCenter | Qt::AlignVCenter,
                       tr("%1% of values are %2\n%3% of values are %4").
                       arg(histTexture[0] / sum).arg(minstr).
                       arg(histTexture[histTextureSize-1] / sum).arg(maxstr)
                      );
        }
    }

#if 0
    // draw some axis titles
    int fH = fontMetrics().boundingRect("Xj").height();
    p.drawText(contentsRect().left(), contentsRect().bottom(), "X axis");

    p.save();
    p.translate(contentsRect().left()+20, contentsRect().bottom());
    p.rotate(-90);
    p.drawText(0, 0, "Y axis");
    p.restore();

#endif
}

// ****************************************************************************
// Method: QvisHistogram::mousePressEvent
//
// Purpose: 
//   Find the closest bounds indicator and select it.
//
// Arguments:
//   e : The mouse event.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 29 13:56:00 PST 2010
//
// Modifications:
//   
// ****************************************************************************

void
QvisHistogram::mousePressEvent(QMouseEvent *e)
{
    if(!selectionEnabled)
        return;

    tempRange[0] = selectedRange[0];
    tempRange[1] = selectedRange[1];

    if(e->x() >= contentsRect().x() &&
       e->x() <= (contentsRect().x()+contentsRect().width()))
    {
        // Which edge is closer
        int dx0 = qAbs(value2Pixel(selectedRange[0]) - e->x());
        int dx1 = qAbs(value2Pixel(selectedRange[1]) - e->x());
        selection = (dx0 < dx1) ? 0 : 1;
        tempRange[selection] = pixel2Value(e->x());
    }
    else if(e->x() < contentsRect().x())
    {
        selection = 0;
        tempRange[0] = totalRange[0];
    }
    else
    {
        selection = 1;
        tempRange[1] = totalRange[1];
    }

    imageDirty();
    update();
}

// ****************************************************************************
// Method: QvisHistogram::mouseMoveEvent
//
// Purpose: 
//   Move the temporary bounds as we move the mouse.
//
// Arguments:
//   e : The mouse event.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 29 13:56:29 PST 2010
//
// Modifications:
//   
// ****************************************************************************

void
QvisHistogram::mouseMoveEvent(QMouseEvent *e)
{
    if(!selectionEnabled)
        return;

    if(selection == 0)
    {
        int minX = contentsRect().x();
        int maxX = value2Pixel(tempRange[1]);
        if(e->x() >= minX && e->x() <= maxX)
        {
            tempRange[selection] = pixel2Value(e->x());
            imageDirty();
            update();
        }
    }
    else if(selection == 1)
    {
        int minX = value2Pixel(tempRange[0]);
        int maxX = contentsRect().x() + contentsRect().width();
        if(e->x() >= minX && e->x() <= maxX)
        {
            tempRange[selection] = pixel2Value(e->x());
            imageDirty();
            update();
        }
    }
}

// ****************************************************************************
// Method: QvisHistogram::mouseReleaseEvent
//
// Purpose: 
//   Use the new selected range when we release the mouse. Emit a signal.
//
// Arguments:
//   e : the mouse event.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 29 13:56:56 PST 2010
//
// Modifications:
//   
// ****************************************************************************

void
QvisHistogram::mouseReleaseEvent(QMouseEvent *e)
{
    if(!selectionEnabled)
        return;

    selection = -1;
    selectedRange[0] = tempRange[0];
    selectedRange[1] = tempRange[1];
    imageDirty();
    update();

    emit selectedRangeChanged(selectedRange[0], selectedRange[1]);
}

// ****************************************************************************
// Method: QvisHistogram::value2Pixel
//
// Purpose: 
//   Translate a value to a pixel.
//
// Arguments:
//   selectedVal : The value to turn into a pixel.
//
// Returns:    Pixel location of selected value.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 29 13:57:36 PST 2010
//
// Modifications:
//   Brad Whitlock, Thu Oct 27 12:23:51 PDT 2011
//   Clamp values so we don't get bad pixel values.
//
// ****************************************************************************

int
QvisHistogram::value2Pixel(float selectedVal) const
{
    float t;
    if(totalRangeValid)
    {
        if(selectedVal < totalRange[0])
            selectedVal = totalRange[0];
        else if(selectedVal > totalRange[1])
            selectedVal = totalRange[1];

        t = (selectedVal - totalRange[0]) / (totalRange[1] - totalRange[0]);
    }
    else
    {
        if(selectedVal < selectedRange[0])
            selectedVal = selectedRange[0];
        else if(selectedVal > selectedRange[1])
            selectedVal = selectedRange[1];

        t = (selectedVal - selectedRange[0]) / (selectedRange[1] - selectedRange[0]);
    }
    return (contentsRect().width() * t) + contentsRect().x();
}

// ****************************************************************************
// Method: QvisHistogram::pixel2Value
//
// Purpose: 
//   Translate pixel location into value
//
// Arguments:
//   pixel : The pixel value to translate.
//
// Returns:    The value for the given pixel.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 29 13:58:07 PST 2010
//
// Modifications:
//   
// ****************************************************************************

float
QvisHistogram::pixel2Value(int pixel) const
{
    float t = float(pixel - contentsRect().x()) / float(contentsRect().width());
    float x;
    if(t < 0.f)
        x = totalRange[0];
    else if(t > 1.f)
        x = totalRange[1];
    else
        x = (1.f-t)*totalRange[0] + t*totalRange[1];
    return x;
}

// ****************************************************************************
// Method: QvisHistogram::getTotalRange
//
// Purpose: 
//   Get the total range.
//
// Arguments:
//   valid : Whether the total range is valid.
//   r0    : The min total range.
//   r1    : The max total range.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 29 13:58:45 PST 2010
//
// Modifications:
//   
// ****************************************************************************

void
QvisHistogram::getTotalRange(bool &valid, float &r0, float &r1) const
{
    valid = totalRangeValid;
    r0 = totalRange[0];
    r1 = totalRange[1];
}

// ****************************************************************************
// Method: QvisHistogram::setTotalRange
//
// Purpose: 
//   Set the total range.
//
// Arguments:
//   r0    : The min total range.
//   r1    : The max total range.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 29 13:58:45 PST 2010
//
// Modifications:
//   
// ****************************************************************************

void
QvisHistogram::setTotalRange(float r0, float r1)
{
    totalRangeValid = true;
    totalRange[0] = (r0 < r1) ? r0 : r1;
    totalRange[1] = (r0 < r1) ? r1 : r0;
 
    imageDirty();
    update();
}

// ****************************************************************************
// Method: invalidateTotalRange
//
// Purpose: 
//   Tell the widget that the total range is not valid.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 29 13:59:43 PST 2010
//
// Modifications:
//   
// ****************************************************************************

void
QvisHistogram::invalidateTotalRange()
{
    totalRangeValid = false;
    totalRange[0] = 0.f;
    totalRange[1] = 1.f;
}

// ****************************************************************************
// Method: QvisHistogram::getSelectedRange
//
// Purpose: 
//   Get the selected range.
//
// Arguments:
//   r0 : The min of the range.
//   r1 : The max of the range.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 29 14:00:00 PST 2010
//
// Modifications:
//   
// ****************************************************************************

void
QvisHistogram::getSelectedRange(float &r0, float &r1) const
{
    r0 = selectedRange[0];
    r1 = selectedRange[1];
}

// ****************************************************************************
// Method: QvisHistogram::setSelectedRange
//
// Purpose: 
//   Set the selected range.
//
// Arguments:
//   r0 : The min of the range.
//   r1 : The max of the range.
//
// Returns:    True if the range was changed. False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 29 14:00:00 PST 2010
//
// Modifications:
//   
// ****************************************************************************

bool
QvisHistogram::setSelectedRange(float r0, float r1)
{
    selectedRange[0] = (r0 < r1) ? r0 : r1;
    selectedRange[1] = (r0 < r1) ? r1 : r0;

    // If we had to reverse the order, return true.
    bool changed = (r0 > r1);

#if 0
    // I was clamping the range to the totalRange but this is not
    // a good thing because you may want selectedRange larger than
    // the totalRange because totalRange may be for 1 time state.
    if(totalRangeValid)
    {
        if(selectedRange[0] < totalRange[0])
        {
            selectedRange[0] = totalRange[0];
            changed = true;
        }
        if(selectedRange[0] > totalRange[1])
        {
            selectedRange[0] = totalRange[1];
            changed = true;
        }

        if(selectedRange[1] < totalRange[0])
        {
            selectedRange[1] = totalRange[0];
            changed = true;
        }
        if(selectedRange[1] > totalRange[1])
        {
            selectedRange[1] = totalRange[1];
            changed = true;
        }
    }
#endif

    imageDirty();
    update();

    return changed;
}


