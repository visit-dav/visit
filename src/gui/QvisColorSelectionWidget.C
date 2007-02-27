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

#include <QvisColorSelectionWidget.h>
#include <QvisColorGridWidget.h>
#include <qapplication.h>
#include <qcolordialog.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qtimer.h>

#define COLOR_COLUMNS 8

// Static members.
const int QvisColorSelectionWidget::MAX_CUSTOM_COLORS = 8;
const unsigned char QvisColorSelectionWidget::colorComponents[] = {
0,  0,   0,    153, 51,   0,    51, 51,   0,    0, 51, 0,
0, 51, 102,      0,  0, 128,    51, 51, 153,    51, 51, 51,

128, 0,  0,    255, 102,  0,   128, 128, 0,    0, 128, 0,
0, 128, 128,     0,   0, 255,  102, 102, 153,  128, 128, 128,

255, 0, 0,     255, 153, 0,    153, 204, 0,    51, 153, 102,
51, 204, 204,  51, 102, 255,   128, 0, 128,    153, 153, 153,

255, 0, 255,   255, 204, 0,    255, 255, 0,    0, 255, 0,
0, 255, 255,   0, 204, 255,    153, 51, 102,   192, 192, 192,

255, 153, 204,  244, 204, 153,  255, 255, 153,  204, 255, 204,
204, 255, 255,  153, 204, 255,  204, 153, 255, 255, 255, 255
}; 

// ****************************************************************************
// Method: QvisColorSelectionWidget::QvisColorSelectionWidget
//
// Purpose: 
//   This is the constructor for the QvisColorSelectionWidget class.
//
// Arguments:
//   parent : The widget's parent.
//   name   : The name of the instance.
//   f      : The window flags.
//
// Note:       This widget acts as a layout for its children since it 
//   positions the children manually and dispatches mouse events to them.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 5 16:50:44 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Oct 26 14:08:32 PST 2001
//   Added a timer.
//
//   Brad Whitlock, Wed Feb 26 13:14:55 PST 2003
//   Made some changes because I changed QvisColorGridWidget.
//
// ****************************************************************************

QvisColorSelectionWidget::QvisColorSelectionWidget(QWidget *parent,
    const char *name, WFlags f) : QWidget(parent, name, f)
{
    // Initialize the standard colors
    QColor c[40];
    const unsigned char *colors = colorComponents;
    int i;
    for(i = 0; i < 40; ++i, colors += 3)
        c[i] = QColor((int)colors[0], (int)colors[1], (int)colors[2]);

    // Initialize the custom colors.
    numCustomColors = 0;
    QColor customColors[MAX_CUSTOM_COLORS];
    for(i = 0; i < MAX_CUSTOM_COLORS; ++i)
        customColors[i] = QColor(255, 255, 255);

    // Create the timer.
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(hide()));

    // Create the standard colors.
    standardColorGrid = new QvisColorGridWidget(this, "standardColors");
    standardColorGrid->setPaletteColors(c, 40, COLOR_COLUMNS);
    standardColorGrid->setFrame(true);
    standardColorGrid->move(0, 0);
    standardColorGrid->resize(standardColorGrid->sizeHint());
    connect(standardColorGrid, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(handleSelectedColor(const QColor &)));

    // Create the custom colors.
    customColorGrid = new QvisColorGridWidget(this, "customColors");
    customColorGrid->setPaletteColors(customColors, MAX_CUSTOM_COLORS,
         COLOR_COLUMNS);
    customColorGrid->setFrame(true);
    customColorGrid->move(0, standardColorGrid->sizeHint().height());
    customColorGrid->resize(customColorGrid->sizeHint());
    connect(customColorGrid, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(handleSelectedColor(const QColor &)));

    // Create the "More colors..." button
    moreColorsButton = new QPushButton("More colors ...", this, 
        "moreColorsButton");
    moreColorsButton->move(0, standardColorGrid->sizeHint().height() + 
        customColorGrid->sizeHint().height());
    moreColorsButton->resize(200, moreColorsButton->sizeHint().height());
    connect(moreColorsButton, SIGNAL(clicked()),
            this, SLOT(getCustomColor()));

    // Turn on mouse tracking for this widget so we can dispatch mouse
    // events to child widgets.
    setMouseTracking(true);
}

// ****************************************************************************
// Method: QvisColorSelectionWidget::~QvisColorSelectionWidget
//
// Purpose: 
//   This is the destructor for the QvisColorSelectionWidget class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 5 16:53:38 PST 2000
//
// Modifications:
//   
// ****************************************************************************

QvisColorSelectionWidget::~QvisColorSelectionWidget()
{
    // nothing here
}

// ****************************************************************************
// Method: QvisColorSelectionWidget::sizeHint
//
// Purpose: 
//   Calculates the widget's preferred size.
//
// Returns:    The widget's preferred size.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 5 16:54:00 PST 2000
//
// Modifications:
//   
// ****************************************************************************

QSize
QvisColorSelectionWidget::sizeHint() const
{
    QSize s1 = standardColorGrid->sizeHint();
    QSize s2 = customColorGrid->sizeHint();
    QSize s3 = moreColorsButton->sizeHint();

    // Find the largest width.
    int maxWidth = s1.width();
    if(s2.width() > maxWidth)
        maxWidth = s3.width();
    if(s3.width() > maxWidth)
        maxWidth = s3.width();

    return QSize(maxWidth, s1.height() + s2.height() + s3.height());
}

// ****************************************************************************
// Method: QvisColorSelectionWidget::setSelectedColor
//
// Purpose: 
//   Tries to select the specified color in both color grids.
//
// Arguments:
//   color : The color to select.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 5 16:56:25 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisColorSelectionWidget::setSelectedColor(const QColor &color)
{
    // Set the selected color in both of the color grids.
    standardColorGrid->setSelectedColor(color);
    customColorGrid->setSelectedColor(color);
}

// ****************************************************************************
// Method: QvisColorSelectionWidget::mouseMoveEvent
//
// Purpose: 
//   This method is called in response to mouse movement in this widget. This
//   method's job is to dispatch the event to the proper child widget.
//
// Arguments:
//   e : The mouse event.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 5 16:57:13 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisColorSelectionWidget::mouseMoveEvent(QMouseEvent *e)
{
    QRect standardRect(standardColorGrid->x(), standardColorGrid->y(),
                       standardColorGrid->width(),
                       standardColorGrid->height());
    QRect customRect(customColorGrid->x(), customColorGrid->y(),
                     customColorGrid->width(), customColorGrid->height());

    // If the mouse is in the color grids, forward the event to them.
    if(standardRect.contains(QPoint(e->x(), e->y())))
    {
        QApplication::sendEvent(standardColorGrid, e);
    }
    else if(customRect.contains(QPoint(e->x(), e->y())))
    {
        // Create a new mouse event that has the coordinates relative to
        // the custom color grid widget.
        QPoint newPoint(e->x(), e->y() - standardColorGrid->height());
        QMouseEvent me(QMouseEvent::MouseMove, newPoint, e->button(),
                       e->state());
        QApplication::sendEvent(customColorGrid, &me);
    }
}

// ****************************************************************************
// Method: QvisColorSelectionWidget::mouseReleaseEvent
//
// Purpose: 
//   This method is called in response to the mouse button being released in
//   this widget. This method's job is to dispatch the event to the proper
//   child widget.
//
// Arguments:
//   e : The mouse event.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 5 16:57:13 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisColorSelectionWidget::mouseReleaseEvent(QMouseEvent *e)
{
    QRect standardRect(standardColorGrid->x(), standardColorGrid->y(),
                       standardColorGrid->width(), standardColorGrid->height());
    QRect customRect(customColorGrid->x(), customColorGrid->y(),
                     customColorGrid->width(), customColorGrid->height());
    QRect buttonRect(moreColorsButton->x(), moreColorsButton->y(),
                     moreColorsButton->width(), moreColorsButton->height());

    // If the mouse is in the color grids, forward the event to them.
    if(standardRect.contains(e->pos()))
    {
        QApplication::sendEvent(standardColorGrid, e);
    }
    else if(customRect.contains(e->pos()))
    {
        // Create a new mouse event that has the coordinates relative to
        // the custom color grid widget.
        QPoint newPoint(e->x(), e->y() - standardColorGrid->height());
        QMouseEvent me(QMouseEvent::MouseButtonRelease, newPoint, e->button(),
                       e->state());

        QApplication::sendEvent(customColorGrid, &me);
    }
    else if(buttonRect.contains(e->pos()))
    {
        // Pretend that the button was clicked.
        getCustomColor();
    }
}

// ****************************************************************************
// Method: QvisColorSelectionWidget::getCustomColor
//
// Purpose: 
//   This is a Qt slot function that is called when the "More colors..." button
//   is clicked. It brings up the Qt color dialog to choose a special color.
//   If a valid color is chosen, it is added to the custom colors and the
//   selectedColor signal is emitted.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 5 16:59:03 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Oct 26 14:21:16 PST 2001
//   Added code to hide the widget.
//
//   Brad Whitlock, Wed Feb 26 13:16:27 PST 2003
//   I changed the code to shift the colors in the color grid.
//
// ****************************************************************************

void
QvisColorSelectionWidget::getCustomColor()
{
    // Hide the widget
    hide();
    
    // Get an initial color for the color dialog.
    QColor initialColor = customColorGrid->selectedColor();
    if(!initialColor.isValid())
    {
        initialColor = standardColorGrid->selectedColor();
        if(!initialColor.isValid())
        {
            initialColor = QColor(255,255,255);
        }
    }

    // Get a new color using the Qt color dialog.
    QColor newColor = QColorDialog::getColor(initialColor);

    // If we have a valid new color, then put it in the custom colors.
    if(newColor.isValid())
    {
        int i;
        QColor newCustomColors[MAX_CUSTOM_COLORS];

        // Shift the palette colors to make room for the new one.
        for(i = 0; i < MAX_CUSTOM_COLORS - 1; ++i)
            newCustomColors[i + 1] = customColorGrid->paletteColor(i);
        newCustomColors[0] = newColor;

        // Set the new palette into the custom colors.
        customColorGrid->setPaletteColors(newCustomColors, MAX_CUSTOM_COLORS,
                                          COLOR_COLUMNS);
        if(numCustomColors < MAX_CUSTOM_COLORS)
            ++numCustomColors;

        // Tell other widgets that a color was selected.
        emit selectedColor(newColor);
    }
}

// ****************************************************************************
// Method: QvisColorSelectionWidget::show
//
// Purpose: 
//   This is a Qt slot function that shows the widget.
//
// Note:       Starts a timer to hide the widget.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 26 14:29:39 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisColorSelectionWidget::show()
{
    QWidget::show();
    timer->start(15000, true);
}

// ****************************************************************************
// Method: QvisColorSelectionWidget::hide()
//
// Purpose: 
//   This is a Qt slot function that hides the widget.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 26 15:00:47 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisColorSelectionWidget::hide()
{
    QWidget::hide();
    timer->stop();
}

// ****************************************************************************
// Method: QvisColorSelectionWidget::handleSelectedColor
//
// Purpose: 
//   This is a Qt slot function that is called when a color is selected using
//   the color grids.
//
// Arguments:
//   color : The selected color.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 26 15:01:19 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisColorSelectionWidget::handleSelectedColor(const QColor &color)
{
    // Make sure the widget hides.
    hide();

    // Emit the color changed signal.
    emit selectedColor(color);
}
