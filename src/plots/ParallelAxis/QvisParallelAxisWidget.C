/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <QvisParallelAxisWidget.h>
#include <math.h>
#include <qdrawutil.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpen.h>

#include <visitstream.h>

static double sampleYs[8][3] = {
    { 0.21, 0.55, 0.85 },
    { 0.32, 0.81, 0.61 },
    { 0.17, 0.77, 0.65 },
    { 0.93, 0.11, 0.57 },
    { 0.62, 0.13, 0.33 },
    { 0.71, 0.42, 0.26 },
    { 0.88, 0.56, 0.31 },
    { 0.48, 0.54, 0.63 }
};


// ****************************************************************************
// Method: QvisParallelAxisWidget::QvisParallelAxisWidget
//
// Purpose: 
//   Constructor for the QvisParallelAxisWidget class.
//
// Arguments:
//   parent : The parent of this widget.
//   name   : This widget's name.
//
// Note: This is intended to emulate the style of the QvisScatterWidget used
//       in the Scatter plot, which came first.
//
// Programmer: Mark Blair
// Creation:   Wed Jun 14 18:54:00 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

QvisParallelAxisWidget::QvisParallelAxisWidget(QWidget *parent, const char *name) : 
    QWidget(parent, name)
{
    axisCount = 2;
    namedRightAxis = false;

    axisTitles.clear();
    axisTitles.push_back(std::string("var1"));

    pixmap      = NULL;
    pixmapDirty = true;
}


// ****************************************************************************
// Method: QvisParallelAxisWidget::~QvisParallelAxisWidget
//
// Purpose: 
//   Destructor for the QvisParallelAxisWidget class.
//
// Note: This is intended to emulate the style of the QvisScatterWidget used
//       in the Scatter plot, which came first.
//
// Programmer: Mark Blair
// Creation:   Wed Jun 14 18:54:00 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

QvisParallelAxisWidget::~QvisParallelAxisWidget()
{
    deleteBackingPixmap();
}


// ****************************************************************************
// Method: QvisParallelAxisWidget::sizeHint
//
// Purpose: 
//   Returns the widget's preferred size.
//
// Returns:    The widget's preferred size.
//
// Note: This is intended to emulate the style of the QvisScatterWidget used
//       in the Scatter plot, which came first.
//
// Programmer: Mark Blair
// Creation:   Wed Jun 14 18:54:00 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

QSize
QvisParallelAxisWidget::sizeHint() const
{
    return QSize(250,250);
}


// ****************************************************************************
// Method: QvisParallelAxisWidget::sizePolicy
//
// Purpose: 
//   Returns the widget's size policy.
//
// Returns:    The widget's size policy.
//
// Note: This is intended to emulate the style of the QvisScatterWidget used
//       in the Scatter plot, which came first.
//
// Programmer: Mark Blair
// Creation:   Wed Jun 14 18:54:00 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

QSizePolicy
QvisParallelAxisWidget::sizePolicy() const
{
    return QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}


// ****************************************************************************
// Method: QvisParallelAxisWidget::deleteBackingPixmap
//
// Purpose: 
//   Deletes the backing pixmap.
//
// Note: This is intended to emulate the style of the QvisScatterWidget used
//       in the Scatter plot, which came first.
//
// Programmer: Mark Blair
// Creation:   Wed Jun 14 18:54:00 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisParallelAxisWidget::deleteBackingPixmap()
{
    if (pixmap != NULL)
    {
        delete pixmap;
        pixmap = NULL;
    }
}


// ****************************************************************************
// Method: QvisParallelAxisWidget::setNumberOfAxes
//
// Purpose: Sets the number of axes to be drawn in the scene.
//
// Arguments:
//   axisCount_ : Number of axes to draw in scene
//
// Programmer: Mark Blair
// Creation:   Wed Jun 14 18:54:00 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisParallelAxisWidget::setNumberOfAxes(int axisCount_)
{
    axisCount = axisCount_;
    pixmapDirty = true;
}


// ****************************************************************************
// Method: QvisParallelAxisWidget::setAxisTitles
//
// Purpose: Sets the titles (names) to be drawn at the bottoms of the axes when
//          the axes are drawn.
//
// Arguments:
//   axisTitles_: Titles (names) of axes to draw in scene
//
// Programmer: Mark Blair
// Creation:   Wed Jun 14 18:54:00 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisParallelAxisWidget::setAxisTitles(const std::string axisTitles_[])
{
    axisTitles.clear();
    
    for (int axisNum = 0; axisNum < axisCount; axisNum++)
    {
        axisTitles.push_back(axisTitles_[axisNum]);
    }
    
    pixmapDirty = true;
}


// ****************************************************************************
// Method: QvisParallelAxisWidget::redrawAllAxes
//
// Purpose: Redraws the scene with current number of axes and axis titles.
//
// Arguments:
//   rightAxisNamed : If true, right axis and its title will be drawn in the
//                    foreground color, as it would be for all the other axes.
//                    If false, right axis will be drawn in a different color
//                    and its title will be drawn as "?".
//
// Note: This is intended to emulate the style of the QvisScatterWidget used
//       in the Scatter plot, which came first.
//
// Programmer: Mark Blair
// Creation:   Wed Jun 14 18:54:00 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisParallelAxisWidget::redrawAllAxes(bool rightAxisNamed)
{
    namedRightAxis = rightAxisNamed;
    
    pixmapDirty = true;
    update();
}


// ****************************************************************************
// Method: QvisParallelAxisWidget::redrawScene
//
// Purpose: 
//   Redraws the scene.
//
// Arguments:
//   painter : The painter to use to redraw the scene.
//
// Note: This is intended to emulate the style of the QvisScatterWidget used
//       in the Scatter plot, which came first.
//
// Programmer: Mark Blair
// Creation:   Wed Jun 14 18:54:00 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisParallelAxisWidget::redrawScene(QPainter *painter)
{
    double sceneWidth = (double)width();
    double sceneHeight = (double)height();
    double leftAxisX = sceneWidth * AXIS_LEFT_MARGIN;
    double axisSpacing = (sceneWidth*(1.0-AXIS_LEFT_MARGIN-AXIS_RIGHT_MARGIN)) /
        (double)(axisCount-1);
    double tickSpacing = (sceneHeight*(1.0-AXIS_BOTTOM_MARGIN-AXIS_TOP_MARGIN)) /
        (double)(TICKS_PER_AXIS+1);
    
    QBrush backgroundBrush(QColor(255,255,255));
    qDrawShadePanel(painter,
        0, 0, width(), height(), colorGroup(), true, 2, &backgroundBrush);
        
    axisBottomY = (int)(sceneHeight*(1.0-AXIS_BOTTOM_MARGIN) + 0.5);
    axisTopY    = (int)(sceneHeight*AXIS_TOP_MARGIN + 0.5);
    
    axesXPos.clear();
    
    for (int axisNum = 0; axisNum < axisCount; axisNum++)
    {
        axesXPos.push_back((int)(leftAxisX + (double)axisNum*axisSpacing + 0.5));
    }
    
    ticksYPos.clear();
    
    for (int tickNum = 1; tickNum <= TICKS_PER_AXIS; tickNum++)
    {
        ticksYPos.push_back((int)(axisTopY + (double)tickNum*tickSpacing + 0.5));
    }
    
    double axisLen = (double)(axisBottomY - axisTopY);
    double dashAndGapLen = axisLen / ((double)DASHES_PER_AXIS - DASH_GAP_FRACTION);
    double dashTopYPos = (double)axisTopY;
    int dashLen = (int)(dashAndGapLen * (1.0-DASH_GAP_FRACTION));
        
    dashesTopYPos.clear(); dashesBotYPos.clear();
    
    for (int dashNum = 0; dashNum < DASHES_PER_AXIS; dashNum++)
    {
        dashesTopYPos.push_back((int)dashTopYPos);
        dashesBotYPos.push_back((int)dashTopYPos + dashLen);
        
        dashTopYPos += dashAndGapLen;
    }
    
    dashesBotYPos[DASHES_PER_AXIS-1] = axisBottomY;

    drawDataCurves(painter);
    drawAxes(painter);
    drawAxisTitles(painter);
}


// ****************************************************************************
// Method: QvisParallelAxisWidget::drawAxes
//
// Purpose: Draws currently selected coordinate axes with tick marks.  If new
//          axis is about to be selected, draws it on the far right in red.
//
// Programmer: Mark Blair
// Creation:   Wed Jun 14 18:54:00 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisParallelAxisWidget::drawAxes(QPainter *painter)
{
    int axisNum, axisX, tickNum, dashNum;

    painter->setPen(QPen(QColor(0,0,0), AXIS_AND_TICK_WIDTH));

    for (axisNum = 0; axisNum < axisCount; axisNum++)
    {
        axisX = axesXPos[axisNum];

        if ((!namedRightAxis && (axisNum == axisCount-1)) ||
            (axisNum >= axisTitles.size()))
        {
            for (dashNum = 0; dashNum < dashesTopYPos.size(); dashNum++)
            {
                painter->drawLine(axisX, dashesTopYPos[dashNum],
                                  axisX, dashesBotYPos[dashNum]);
            }
        }
        else
        {
            painter->drawLine(axisX, axisBottomY, axisX, axisTopY);
            
            for (tickNum = 0; tickNum < ticksYPos.size(); tickNum++)
            {
                painter->drawLine(axisX-TICK_HALF_LENGTH, ticksYPos[tickNum],
                                  axisX+TICK_HALF_LENGTH, ticksYPos[tickNum]);
            }
        }
    }
}


// ****************************************************************************
// Method: QvisParallelAxisWidget::drawAxisTitles
//
// Purpose: Draws titles (variable names) of currently selected coordinate
//          axes.  If new axis is about to be selected, draws "???" as its title.
//
// Programmer: Mark Blair
// Creation:   Wed Jun 14 18:54:00 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisParallelAxisWidget::drawAxisTitles(QPainter *painter)
{
    int charSpacing = (fontMetrics().height() * 2) / 3;
    int maxTitleChars = (int)((double)height()*AXIS_BOTTOM_MARGIN) / charSpacing;
    int axisNum, axisX, slashIndex, titleCharCount, charNum, charXPos, charYPos;
    std::string axisTitle, titleChar;

    painter->setPen(QPen(QColor(0,80,255), AXIS_AND_TICK_WIDTH));

    for (axisNum = 0; axisNum < axisCount; axisNum++)
    {
        if ((!namedRightAxis && (axisNum == axisCount-1)) ||
            (axisNum >= axisTitles.size()))
        {
            axisTitle = std::string("?");
            titleCharCount = 1;
        }
        else
        {
            axisTitle = axisTitles[axisNum];
            if ((slashIndex = axisTitle.find_first_of("/")) != std::string::npos)
                axisTitle = axisTitle.substr(slashIndex+1);
            titleCharCount =
            (axisTitle.length() > maxTitleChars) ? maxTitleChars : axisTitle.length();
        }
        
        axisX = axesXPos[axisNum];
        
        for (charNum = 0; charNum < titleCharCount; charNum++)
        {
            titleChar = axisTitle.substr(charNum, 1);
            charXPos = axisX - fontMetrics().width(QChar(*(axisTitle.c_str())))/2;
            charYPos = (charNum+1)*charSpacing + axisBottomY;

            painter->drawText(charXPos, charYPos, QString(titleChar.c_str()));
        }
    }
}


// ****************************************************************************
// Method: QvisParallelAxisWidget::drawDataCurves
//
// Purpose: Draws simulated data curves between currently selected coordinate
//          axes.
//
// Programmer: Mark Blair
// Creation:   Wed Jun 14 18:54:00 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisParallelAxisWidget::drawDataCurves(QPainter *painter)
{
    double axisHeight = (double)height() * (1.0-AXIS_BOTTOM_MARGIN-AXIS_TOP_MARGIN);
    int axisNum, segNum;
    int leftAxisX, leftAxisY, rightAxisX, rightAxisY;

    painter->setPen(QPen(QColor(128,128,128), 1));

    for (axisNum = 0; axisNum < axisCount-1; axisNum++)
    {
        if (axisNum > 7) break;   // Shouldn't happen

        leftAxisX = axesXPos[axisNum]; rightAxisX = axesXPos[axisNum+1];
        
        for (segNum = 0; segNum < 3; segNum++)
        {
            leftAxisY  = (int)(sampleYs[axisNum  ][segNum]*axisHeight + axisTopY);
            rightAxisY = (int)(sampleYs[axisNum+1][segNum]*axisHeight + axisTopY);

            painter->drawLine(leftAxisX, leftAxisY, rightAxisX, rightAxisY);
        }
    }
}


// ****************************************************************************
// Method: QvisParallelAxisWidget::paintEvent
//
// Purpose: 
//   This method is called when the widget needs to be repainted.
//
// Arguments:
//   e : The paint event to process.
//
// Note: This is intended to emulate the style of the QvisScatterWidget used
//       in the Scatter plot, which came first.
//
// Programmer: Mark Blair
// Creation:   Wed Jun 14 18:54:00 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisParallelAxisWidget::paintEvent(QPaintEvent *e)
{
    bool clipByRegion = true;

    // Draw the scene into the backing pixmap.
    bool needsPaint = pixmapDirty;

    if (pixmap == NULL)
    {
        needsPaint = true;
        pixmap = new QPixmap(width(), height());
    }

    if (needsPaint)
    {
        QPainter pixpaint(pixmap);
        redrawScene(&pixpaint);
        setBackgroundPixmap(*pixmap);
        clipByRegion = false;
        pixmapDirty = false;
    }

    // Blit the pixmap to the screen.
    QPainter paint(this);
    if (clipByRegion && !e->region().isEmpty() && !e->region().isNull())
        paint.setClipRegion(e->region());
    paint.drawPixmap(QPoint(0,0), *pixmap);
}


// ****************************************************************************
// Method: QvisParallelAxisWidget::resizeEvent
//
// Purpose: 
//   This method is called when the widget needs to be resized.
//
// Arguments:
//   e : The resize event.
//
// Note: This is intended to emulate the style of the QvisScatterWidget used
//       in the Scatter plot, which came first.
//
// Programmer: Mark Blair
// Creation:   Wed Jun 14 18:54:00 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisParallelAxisWidget::resizeEvent(QResizeEvent *e)
{
    deleteBackingPixmap();
    
    pixmapDirty = true;
    update();
}


//
// Qt slot functions
//

// ****************************************************************************
// Method: QvisParallelAxisWidget::show
//
// Purpose: 
//   This is a Qt slot function that is called when the widget is shown.
//
// Programmer: Mark Blair
// Creation:   Wed Jun 14 18:54:00 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisParallelAxisWidget::show()
{
    QWidget::show();
    // Maybe future animation code here.
}


// ****************************************************************************
// Method: QvisParallelAxisWidget::hide
//
// Purpose: 
//   This is a Qt slot function that is called when the widget is hidden.
//
// Programmer: Mark Blair
// Creation:   Wed Jun 14 18:54:00 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisParallelAxisWidget::hide()
{
    QWidget::hide();
    // Maybe future animation code here.
}
