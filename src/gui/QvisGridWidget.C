/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

#include <QvisGridWidget.h>

#include <QMouseEvent>
#include <QPainter>
#include <QTimer>

// ****************************************************************************
// Method: QvisGridWidget::QvisGridWidget
//
// Purpose: 
//   Constructor for the QvisGridWidget class.
//
// Arguments:
//   parent : The parent widget to this object.
//   name   : The name of this object.
//   f      : The window flags. These control how window decorations are done.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 4 19:51:26 PST 2000
//
// Modifications:
//   Brad Whitlock, Thu Nov 21 17:13:29 PST 2002
//   Made boxSize and boxPadding values that can be set.
//
//   Jeremy Meredith, Tue Aug 29 12:42:24 EDT 2006
//   Refactored much of QvisColorGridWidget into this class.
//
//   Jeremy Meredith, Thu Aug 31 15:47:38 EDT 2006
//   Initialize isPopup.
//
//   Brad Whitlock, Mon Jun  2 16:29:14 PDT 2008
//   Qt 4.
//
// ****************************************************************************

QvisGridWidget::QvisGridWidget(QWidget *parent, Qt::WindowFlags f) : 
    QWidget(parent, f)
{
    numRows = 1;
    numColumns = 1;
    drawFrame = false;

    currentActiveItem = -1;
    currentSelectedItem = -1;

    numGridSquares = 0;

    boxSizeValue = 16;
    boxPaddingValue = 8;
    setMinimumSize(minimumSize());

    // Set the default size policy.
    setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,
        QSizePolicy::MinimumExpanding));

    isPopup = false;
    timer = NULL;
}

// ****************************************************************************
// Method: QvisGridWidget::~QvisGridWidget
//
// Purpose: 
//   Destructor for the QvisGridWidget class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 4 19:52:32 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Jun  2 16:29:48 PDT 2008
//   Qt 4.
//
// ****************************************************************************

QvisGridWidget::~QvisGridWidget()
{
}


// ****************************************************************************
// Method: QvisGridWidget::setIsPopup
//
// Purpose: 
//   Set whether or not this widget should function like its own popup.
//
// Programmer: Jeremy Meredith
// Creation:   August 11, 2006
//
// Modifications:
//   Jeremy Meredith, Thu Aug 31 15:48:05 EDT 2006
//   Only initialize timer and mouse tracking if we are a popup.
//   
// ****************************************************************************

void
QvisGridWidget::setIsPopup(bool ip)
{
    isPopup = ip;

    if (isPopup)
    {
        // Create the timer.
        timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(hide()));

        // Turn on mouse tracking for this widget so we can dispatch mouse
        // events to child widgets.
        setMouseTracking(true);
    }
}

// ****************************************************************************
// Method: QvisGridWidget::sizeHint
//
// Purpose: 
//   Returns the widget's favored size.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 4 19:52:56 PST 2000
//
// Modifications:
//   
// ****************************************************************************

QSize
QvisGridWidget::sizeHint() const
{
    return QSize(boxSizeValue * numColumns + boxPaddingValue * (numColumns + 1),
                 boxSizeValue * numRows + boxPaddingValue * (numRows + 1));
}

// ****************************************************************************
// Method: QvisGridWidget::minimumSize
//
// Purpose: 
//   Returns the minimum size for the widget.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 21 18:22:34 PST 2002
//
// Modifications:
//   
// ****************************************************************************

QSize
QvisGridWidget::minimumSize() const
{
    return QSize(boxSizeValue * numColumns + boxPaddingValue * (numColumns + 1),
                 boxSizeValue * numRows + boxPaddingValue * (numRows + 1));
}

//
// Properties.
//

void
QvisGridWidget::setBoxSize(int val)
{
    boxSizeValue = val;
    setMinimumSize(minimumSize());
}

void
QvisGridWidget::setBoxPadding(int val)
{
    boxPaddingValue = val;
    setMinimumSize(minimumSize());
}

int   
QvisGridWidget::boxSize() const
{
    return boxSizeValue;
}

int   
QvisGridWidget::boxPadding() const
{
    return boxPaddingValue;
}

// ****************************************************************************
// Method: QvisGridWidget::setFrame
//
// Purpose: 
//   Turns the frame around the widget on or off and causes the widget to
//   redraw itself accordingly.
//
// Arguments:
//   val : A boolean value indicating whether or not to draw the frame.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 5 10:35:25 PDT 2000
//
// Modifications:
//   Brad Whitlock, Mon Jun  2 16:30:17 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisGridWidget::setFrame(bool val)
{
    if(val != drawFrame)
    {
        drawFrame = val;

        if(isVisible())
        {
            // Make the widget redraw itself.
            update();
        }
    }
}

// ****************************************************************************
// Method: QvisGridWidget::rows
//
// Purpose: 
//   Returns the number of color rows.
//
// Returns:    Returns the number of color rows.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 4 19:57:44 PST 2000
//
// Modifications:
//   
// ****************************************************************************

int
QvisGridWidget::rows() const
{
    return numRows;
}

// ****************************************************************************
// Method: QvisGridWidget::columns
//
// Purpose: 
//   Returns the number of color columns.
//
// Returns:    Returns the number of color columns.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 4 19:57:44 PST 2000
//
// Modifications:
//   
// ****************************************************************************

int
QvisGridWidget::columns() const
{
    return numColumns;
}

// ****************************************************************************
// Method: QvisGridWidget::setSelectedIndex
//
// Purpose: 
//   Sets the selected color for the widget. If the new selected color differs
//   from the old one, the selected item signal is emitted.
//
// Arguments:
//   index : The index of the new selected color. 0..rows*cols.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 4 19:56:36 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Apr 26 11:49:58 PDT 2002
//   I fixed an error that cropped up on windows.
//
//   Brad Whitlock, Thu Nov 21 10:47:42 PDT 2002
//   I made it emit a signal that contains the color as well as the row and
//   column of the color that changed.
//
//   Brad Whitlock, Wed Feb 26 12:51:24 PDT 2003
//   I renamed the method and made it emit another selectedColor signal.
//
//   Jeremy Meredith, Tue Aug 29 12:42:24 EDT 2006
//   Refactored much of QvisColorGridWidget into this class.
//   Changed the signal emits to a pure virtual function defined
//   by the new concrete derived types.
//
//   Brad Whitlock, Mon Jun  2 16:31:01 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisGridWidget::setSelectedIndex(int index)
{
    if(index >= -1 && index < numGridSquares)
    {
        QRegion region = getItemRegion(currentSelectedItem);

        // Set the new value.
        currentSelectedItem = index;

        // If the selected color that we set is a real color, highlight
        // the new selected color.
        if(currentSelectedItem != -1)
            region = region + getItemRegion(currentSelectedItem);

        // Update the widget.
        if(isVisible() && !region.isEmpty())
            update(region);

        // emit the selectedItem signal.
        if(currentSelectedItem != -1)
        {
            emitSelection();
        }
    }
}

// ****************************************************************************
// Method: QvisGridWidget::selectedIndex
//
// Purpose: 
//   Returns the row and column of the selected color.
//
// Arguments:
//   row : The row of the selected color.
//   column : The column of the selected color.
//
// Returns:    The row and column of the selected color.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 21 10:54:04 PDT 2002
//
// Modifications:
//   Brad Whitlock, Wed Feb 26 12:47:46 PDT 2003
//   I renamed it and made it return the single color index.
//
// ****************************************************************************

int
QvisGridWidget::selectedIndex() const
{
    return currentSelectedItem;
}

// ****************************************************************************
// Method: QvisGridWidget::activeIndex
//
// Purpose: 
//   Returns the index of the active color.
//
// Returns:    Returns the index of the active color.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 4 19:57:44 PST 2000
//
// Modifications:
//   
// ****************************************************************************

int
QvisGridWidget::activeIndex() const
{
    return currentActiveItem;
}

// ****************************************************************************
// Method: QvisGridWidget::setActiveItem
//
// Purpose: 
//   Sets the active color. This is the color that is being considered but is
//   not yet selected.
//
// Arguments:
//   index : The index of the color to select. This is from 0..rows*cols.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 4 19:55:11 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Apr 26 11:49:20 PDT 2002
//   I fixed an error that cropped up on windows.
//
//   Brad Whitlock, Wed Feb 26 13:09:42 PST 2003
//   I made some internal interface changes.
//
//   Brad Whitlock, Mon Jun  2 16:31:16 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisGridWidget::setActiveIndex(int index)
{
    if(index >= -1 && index < numGridSquares)
    {
        QRegion region;

        // Get the current region
        if(activeIndex() != -1)
            region = getItemRegion(activeIndex());

        currentActiveItem = index;

        // Add the new region to the current region
        region = region + getItemRegion(activeIndex());

        // Update the widget.
        if(isVisible() && !region.isEmpty())
            update(region);
    }
}

// ****************************************************************************
// Method: QvisGridWidget::enterEvent
//
// Purpose: 
//   Turns on mouse tracking when the mouse enters this widget.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 4 20:00:48 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisGridWidget::enterEvent(QEvent *)
{
    // We've entered the widget, turn on mouse tracking.
    setMouseTracking(true);
}

// ****************************************************************************
// Method: QvisGridWidget::leaveEvent
//
// Purpose: 
//   Turns off mouse tracking when the mouse leaves this widget.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 4 20:01:13 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisGridWidget::leaveEvent(QEvent *)
{
    // We've left the widget, turn off mouse tracking.
    setMouseTracking(false);

    // Indicate that no color is active.
    setActiveIndex(-1);
}

// ****************************************************************************
// Method: QvisGridWidget::mouseMoveEvent
//
// Purpose: 
//   This method is called when the mouse is moved in the widget. Its job is to
//   determine whether or not the active color needs to be updated.
//
// Arguments:
//   e : The mouse event.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 4 20:01:46 PST 2000
//
// Modifications:
//   Brad Whitlock, Wed Feb 26 13:12:46 PST 2003
//   Internal interface changes.
//
// ****************************************************************************

void
QvisGridWidget::mouseMoveEvent(QMouseEvent *e)
{
    int index = getIndexFromXY(e->x(), e->y());

    // If we've moved the mouse to a new active color, unhighlight the old one
    // and highlight the new one.
    if(index != activeIndex())
    {
        setActiveIndex(index);
    }
}

// ****************************************************************************
// Method: QvisGridWidget::mouseReleaseEvent
//
// Purpose: 
//   This method is called when the mouse is clicked in the widget. It sets
//   the selected color based on the color that was clicked.
//
// Arguments:
//   e : The mouse event.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 4 20:02:43 PST 2000
//
// Modifications:
//   Brad Whitlock, Wed Feb 26 13:12:59 PST 2003
//   Internal interface changes.
//
//   Jeremy Meredith, Tue Aug 29 12:42:24 EDT 2006
//   Refactored much of QvisColorGridWidget into this class, and
//   made changes to support other kinds of derived types.
//
// ****************************************************************************

void
QvisGridWidget::mouseReleaseEvent(QMouseEvent *e)
{
    int index = getIndexFromXY(e->x(), e->y());

    // If a valid color index was returned, select the color.
    if(index != -1)
    {
        // Set the selected color.
        setSelectedIndex(index);
    }

    if (isPopup)
        hide();
}

// ****************************************************************************
// Method: QvisGridWidget::paintEvent
//
// Purpose: 
//   This method handles repainting the widget on the screen.
//
// Arguments:
//   e : The paint event.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 4 20:03:36 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Jun  2 16:32:34 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisGridWidget::paintEvent(QPaintEvent *e)
{
    // Set up a painter.
    QPainter paint(this);
    if(!e->region().isEmpty())
    {
        paint.setClipRegion(e->region());
        paint.setClipping(true);
    }

    // Draw the widget.
    drawItemArray(paint);
}

// ****************************************************************************
// Method: QvisGridWidget::resizeEvent
//
// Purpose: 
//   This method deletes the drawing pixmap so it will be redrawn in the new
//   size when the next paint event happens.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 4 20:04:15 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Jun  2 16:33:38 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisGridWidget::resizeEvent(QResizeEvent *)
{
    update();
}

// ****************************************************************************
// Method: QvisGridWidget::getIndexFromXY
//
// Purpose: 
//   Computes a color index given an x,y location in the widget.
//
// Arguments:
//   x : The x location.
//   y : The y location.
//
// Returns:   The color index given at the x,y location in the widget. 
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 4 20:04:59 PST 2000
//
// Modifications:
//   
// ****************************************************************************

int
QvisGridWidget::getIndexFromXY(int x, int y) const
{
    int index = -1;

    // See if the x,y coordinate is in the widget.
    if(QRect(0, 0, width(), height()).contains(QPoint(x, y)))
    {
        int boxWidth  = (width()  - boxPaddingValue) / numColumns;
        int boxHeight = (height() - boxPaddingValue) / numRows;

        int column = (x - boxPaddingValue) / boxWidth;
        int row = (y - boxPaddingValue) / boxHeight;
        index = getIndex(row, column);
    }

    return index;
}

// ****************************************************************************
// Method: QvisGridWidget::getIndex
//
// Purpose: 
//   Computes a color index given a row, column.
//
// Arguments:
//   row : The row of the color.
//   column : The column of the color.
//
// Returns:    The color index given the row,column.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 4 20:06:05 PST 2000
//
// Modifications:
//   
// ****************************************************************************

int
QvisGridWidget::getIndex(int row, int column) const
{
    return (row * numColumns) + column;
}

// ****************************************************************************
// Method: QvisGridWidget::getRowColumnFromIndex
//
// Purpose: 
//   Computes the row and column from a color index.
//
// Arguments:
//   index  : A color index.
//   row    : The row that contains the index.
//   column : The column that contains the index.
//
// Returns: The row and column of the index.   
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 21 10:45:26 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisGridWidget::getRowColumnFromIndex(int index, int &row, int &column) const
{
    row = index / numColumns;
    column = index % numColumns;
}

// ****************************************************************************
// Method: QvisGridWidget::getItemRect
//
// Purpose: 
//   Figures out the location of the color box in the widget for a specified
//   color index.
//
// Arguments:
//   index : The color index for which we want geometry.
//   x : A reference to an int in which we'll store the x value.
//   y : A reference to an int in which we'll store the y value.
//   w : A reference to an int in which we'll store the width value.
//   h : A reference to an int in which we'll store the height value.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 4 20:07:12 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisGridWidget::getItemRect(int index, int &x, int &y,
    int &w, int &h) const
{
    int column = index % numColumns;
    int row = index / numColumns;

    int boxWidth  = (width() - boxPaddingValue) / numColumns;
    int boxHeight = (height() -  boxPaddingValue) / numRows;

    // Figure out the x,y location.
    x = column * boxWidth + boxPaddingValue;
    y = row * boxHeight + boxPaddingValue;

    // Figure out the width, height.
    w = boxWidth - boxPaddingValue;
    h = boxHeight - boxPaddingValue;
}

// ****************************************************************************
// Method: QvisGridWidget::getItemRegion
//
// Purpose: 
//   Returns the region that would be drawn if we change the index'th item.
//
// Arguments:
//   index : the index of the item whose region we want.
//   
// Returns:    The region that would be drawn. The region is the itemRect()
//             adjusted with the box padding that separates items.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun  2 16:51:25 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QRegion
QvisGridWidget::getItemRegion(int index) const
{
    QRegion retval;

    if(isValidIndex(index))
    {
        // Get the location of the index'th color box.
        int x, y, boxWidth, boxHeight;
        getItemRect(index, x, y, boxWidth, boxHeight);

        // return the region that we would draw on.
        retval = QRegion(x - boxPaddingValue / 2, y - boxPaddingValue / 2,
            boxWidth + boxPaddingValue, boxHeight + boxPaddingValue);
    }

    return retval;
}

// ****************************************************************************
// Method: QvisGridWidget::drawItemArray
//
// Purpose: 
//   Draws all of the color boxes into the drawing pixmap.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 4 20:09:03 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Mar 11 11:29:27 PDT 2002
//   Rewrote so it does not use styles.
//
//   Brad Whitlock, Fri Apr 26 11:48:22 PDT 2002
//   I fixed an error that cropped up on windows.
//
//   Brad Whitlock, Wed Feb 26 13:00:03 PST 2003
//   I made it capable of drawing an incomplete row of colors.
//
//   Brad Whitlock, Thu Aug 21 15:36:18 PST 2003
//   I changed how the brush to draw the background is selected so it looks
//   better on MacOS X.
//
//   Brad Whitlock, Mon Jun  2 16:34:05 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisGridWidget::drawItemArray(QPainter &paint)
{
    if(drawFrame)
    {
        drawBox(paint, rect(), palette().color(QPalette::Light),
                palette().color(QPalette::Dark));
    }        

    // Draw all of the color boxes.
    int index = 0;
    for(int i = 0; i < numRows; ++i)
    {
        for(int j = 0; j < numColumns; ++j, ++index)
        {
            if(index < numGridSquares)
            {
                if(index == currentSelectedItem)
                    drawSelectedItem(paint, index);
                else if(index == activeIndex())
                    drawHighlightedItem(paint, index);
                else
                    drawItem(paint, index);
            }
        }
    }
}

// ****************************************************************************
// Method: QvisGridWidget::drawBox
//
// Purpose: 
//   Draws a highlighted box that looks like the edges of a button.
//
// Arguments:
//   paint : The painter used to draw.
//   r     : The bounding rectangle of the frame
//   light : The light color
//   dark  : The dark color
//   lw    : The width of the box.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 12 18:45:56 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisGridWidget::drawBox(QPainter &paint, const QRect &r,
    const QColor &light, const QColor &dark, int lw)
{
    int i;
    int X  = r.x();
    int X2 = r.x() + r.width() - 1;
    int Y  = r.y();
    int Y2 = r.y() + r.height() - 1;

    // Draw the highlight
    paint.setPen(QPen(light));
    for(i = 0; i < lw; ++i)
    {
        paint.drawLine(QPoint(X + i, Y + i), QPoint(X + i, Y2 - i));
        paint.drawLine(QPoint(X + i, Y + i), QPoint(X2 - i, Y + i));
    }

    // Draw the shadow
    paint.setPen(QPen(dark));
    for(i = 0; i < lw; ++i)
    {
        paint.drawLine(QPoint(X + i + 1, Y2 - i), QPoint(X2, Y2 - i));
        paint.drawLine(QPoint(X2 - i, Y + i + 1), QPoint(X2 - i, Y2));
    }
}

// ****************************************************************************
// Method: QvisGridWidget::drawHighlightedItem
//
// Purpose: 
//   Draws a highlighted color box.
//
// Arguments:
//   index : The index of the color box to draw.
//
// Returns:    The region that was covered by drawing.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 4 20:10:27 PST 2000
//
// Modifications:
//   Brad Whitlock, Tue Mar 12 18:48:55 PST 2002
//   Removed the style coding in favor of a custom drawing routine.
//
//   Brad Whitlock, Mon Jun  2 16:35:51 PDT 2008
//   Qt 4.
//
// ****************************************************************************

QRegion
QvisGridWidget::drawHighlightedItem(QPainter &paint, int index)
{
    QRegion retval;

    if(isValidIndex(index))
    {
        // Get the location of the index'th color box.
        int x, y, boxWidth, boxHeight;
        getItemRect(index, x, y, boxWidth, boxHeight);

        QRect r(x - boxPaddingValue / 2, y - boxPaddingValue / 2,
                boxWidth + boxPaddingValue, boxHeight + boxPaddingValue);

        // Draw a highlight.
        paint.fillRect(r, palette().color(QPalette::Highlight));

        // Draw the button and the color over the button.
        drawBox(paint, r, palette().color(QPalette::Light),
#ifdef Q_WS_MACX
                Qt::black
#else
                palette().color(QPalette::Shadow)
#endif
                );
        drawItem(paint, index);
 
        // return the region that we drew on.
        retval = QRegion(x - boxPaddingValue / 2, y - boxPaddingValue / 2,
            boxWidth + boxPaddingValue, boxHeight + boxPaddingValue);
    }

    return retval;
}

// ****************************************************************************
// Method: QvisGridWidget::drawSelectedItem
//
// Purpose: 
//   Draws a selected color box.
//
// Arguments:
//   index : The index of the color box to draw.
//
// Returns:    The region that was covered by drawing.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 4 20:10:27 PST 2000
//
// Modifications:
//   Brad Whitlock, Tue Mar 12 18:48:55 PST 2002
//   Removed the style coding in favor of a custom drawing routine.
//
//   Brad Whitlock, Fri Apr 26 11:37:12 PDT 2002
//   I fixed an error that cropped up on windows.
//
//   Brad Whitlock, Mon Jun  2 16:38:13 PDT 2008
//   Qt 4.
//
// ****************************************************************************

QRegion
QvisGridWidget::drawSelectedItem(QPainter &paint, int index)
{
    QRegion retval;

    if(isValidIndex(index))
    {
        // Get the location of the index'th color box.
        int x, y, boxWidth, boxHeight;
        getItemRect(index, x, y, boxWidth, boxHeight);

        QRect r(x - boxPaddingValue / 2, y - boxPaddingValue / 2,
                boxWidth + boxPaddingValue, boxHeight + boxPaddingValue);

        // Draw a sunken button.
        drawBox(paint, r,
#ifdef Q_WS_MACX
                Qt::black,
#else
                palette().color(QPalette::Dark),
#endif
                palette().color(QPalette::Light));

        // Draw the color over the button.
        drawItem(paint, index);

        // return the region that we drew on.
        retval = QRegion(x - boxPaddingValue / 2, y - boxPaddingValue / 2,
            boxWidth + boxPaddingValue, boxHeight + boxPaddingValue);
    }

    return retval;
}

// ****************************************************************************
// Method: QvisGridWidget::show
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
//    Jeremy Meredith, Thu Aug 31 15:47:12 EDT 2006
//    Added support for subclasses being popups.
//
//    Brad Whitlock, Mon Jun  2 16:44:46 PDT 2008
//    Qt 4.
//
// ****************************************************************************

void
QvisGridWidget::show()
{
    QWidget::show();
    if (isPopup && timer)
    {
        timer->setSingleShot(true);
        timer->start(15000);
    }
}

// ****************************************************************************
// Method: QvisGridWidget::hide()
//
// Purpose: 
//   This is a Qt slot function that hides the widget.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 26 15:00:47 PST 2001
//
// Modifications:
//    Jeremy Meredith, Thu Aug 31 15:47:12 EDT 2006
//    Added support for subclasses being popups.
//   
// ****************************************************************************

void
QvisGridWidget::hide()
{
    QWidget::hide();
    if (isPopup && timer)
        timer->stop();
}


// ****************************************************************************
//  Method:  QvisGridWidget::isValidIndex
//
//  Purpose:
//    Default check for a valid index.  Provided for
//    convenience of derived types.
//
//  Arguments:
//    index      the index to determine if it's valid
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 29, 2006
//
// ****************************************************************************

bool
QvisGridWidget::isValidIndex(int index) const
{
    return (index >= 0);
}

// ****************************************************************************
//  Method:  QvisGridWidget::emitSelection
//
//  Purpose:
//    Default action for emitting a selection.  Provided for
//    convenience of derived types.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 29, 2006
//
// ****************************************************************************

void
QvisGridWidget::emitSelection()
{
}

