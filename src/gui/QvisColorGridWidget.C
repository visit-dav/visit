/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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

#include <QvisColorGridWidget.h>
#include <qcursor.h>
#include <qpainter.h>
#include <qpixmap.h>

// ****************************************************************************
// Method: QvisColorGridWidget::QvisColorGridWidget
//
// Purpose: 
//   Constructor for the QvisColorGridWidget class.
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
// ****************************************************************************

QvisColorGridWidget::QvisColorGridWidget(QWidget *parent, const char *name,
    WFlags f) : QvisGridWidget(parent, name, f)
{
    paletteColors = 0;

    // Set the default size policy.
    setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,
        QSizePolicy::MinimumExpanding));
}

// ****************************************************************************
// Method: QvisColorGridWidget::~QvisColorGridWidget
//
// Purpose: 
//   Destructor for the QvisColorGridWidget class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 4 19:52:32 PST 2000
//
// Modifications:
//   
// ****************************************************************************

QvisColorGridWidget::~QvisColorGridWidget()
{
    if(paletteColors)
       delete [] paletteColors;
}


// ****************************************************************************
// Method: QvisColorGridWidget::setSelectedColor
//
// Purpose: 
//   Sets the selected color for the widget. If the new selected color differs
//   from the old one, the selectedColor signal is emitted.
//
// Arguments:
//   color : The color to select.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 4 19:56:36 PST 2000
//
// Modifications:
//   Brad Whitlock, Wed Feb 26 12:50:49 PDT 2003
//   Changed the name of a method that gets called.
//
// ****************************************************************************

void
QvisColorGridWidget::setSelectedColor(const QColor &color)
{
    // Figure out the index of the color. If it is not in the palette, we'll
    // end up unselecting the currently selected color.
    int index = -1;
    bool notFound = true;
    for(int i = 0; i < numGridSquares && notFound; ++i)
    {
        if(color == paletteColors[i])
        {
            index = i;
            notFound = false;
        }
    }

    // Set the selected color.
    setSelectedIndex(index);
}

// ****************************************************************************
// Method: QvisColorGridWidget::selectedColor
//
// Purpose: 
//   Returns the selected color.
//
// Returns:    Returns the index of the selected color.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 4 19:59:16 PST 2000
//
// Modifications:
//   
// ****************************************************************************

QColor
QvisColorGridWidget::selectedColor() const
{
    QColor retval;

    if(currentSelectedItem != -1)
        retval = paletteColors[currentSelectedItem];

    return retval;
}

// ****************************************************************************
// Method: QvisColorGridWidget::paletteColor
//
// Purpose: 
//   Returns the color at the specified row and column.
//
// Returns:    The color at the specified row and column.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 5 13:16:12 PST 2000
//
// Modifications:
//   Brad Whitlock, Wed Feb 26 12:47:06 PDT 2003
//   Made it take a single argument.
//
// ****************************************************************************

QColor
QvisColorGridWidget::paletteColor(int index) const
{
    QColor retval;

    if(index >= 0 && index < numGridSquares)
    {
        retval = paletteColors[index];
    }

    return retval;
}

// ****************************************************************************
// Method: QvisColorGridWidget::setPaletteColors
//
// Purpose: 
//   Sets the widget's palette colors. These are the colors from which the
//   user can select.
//
// Arguments:
//   c    : The array of QColors to copy. Must be rows*columns elements long.
//   rows : The number of rows to draw.
//   cols : The number of columns to draw.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 4 19:53:25 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Nov 22 15:27:27 PST 2002
//   I fixed a bug that prevented the widget from updating correctly.
//
//   Brad Whitlock, Wed Feb 26 12:54:04 PDT 2003
//   I made it take a number of colors instead of row and column. Then I added
//   a suggested columns default argument to set the number of columns that
//   we'd like to use.
//
// ****************************************************************************

void
QvisColorGridWidget::setPaletteColors(const QColor *c, int nColors,
    int suggestedColumns)
{
    if(c != 0 && nColors > 0)
    {
        if(paletteColors)
            delete [] paletteColors;

        // Copy the color array.
        numGridSquares = nColors;
        paletteColors = new QColor[numGridSquares];
        for(int i = 0; i < numGridSquares; ++i)
            paletteColors[i] = c[i];

        // Figure out the number of rows and columns.
        numColumns = suggestedColumns;
        if(numColumns < 1)
            numColumns = 6;
        numRows = nColors / numColumns;
        if(numRows < 1)
            numRows = 1;
        if(numRows * numColumns < nColors)
            ++numRows;

        // Adjust the active and selected colors if necessary.
        currentActiveItem = -1;
        if(currentSelectedItem >= numGridSquares)
            currentSelectedItem = -1;

        // Make the widget repaint if it is visible.
        if(isVisible())
        {
            delete drawPixmap;
            drawPixmap = 0;
            update();
        }
        else if(drawPixmap)
        {
            delete drawPixmap;
            drawPixmap = 0;
        }
    }
}

// ****************************************************************************
// Method: QvisColorGridWidget::setPaletteColor
//
// Purpose: 
//   Sets the color of an individual color box and updates the widget.
//
// Arguments:
//   color : The new color.
//   index : The index of the color we're changing.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 5 12:43:58 PDT 2000
//
// Modifications:
//   Brad Whitlock, Fri Apr 26 11:36:42 PDT 2002
//   I fixed an error that cropped up on Windows.
//
//   Brad Whitlock, Wed Feb 26 12:53:07 PDT 2003
//   I made it take a single index argument instead of row and column.
//
// ****************************************************************************

void
QvisColorGridWidget::setPaletteColor(const QColor &color, int index)
{
    if(index >= 0 && index < numGridSquares)
    {
        // If the colors are different, update the widget.
        if(color != paletteColors[index])
        {
            QRegion region;

            // Replace the color
            paletteColors[index] = color;

            // Redraw the color in the appropriate manner.
            if(index == currentSelectedItem)
                region = drawSelectedItem(0, index);
            else if(index == activeIndex())
                region = drawHighlightedItem(0, index);
            else
            {
                int x, y, w, h;
                getItemRect(index, x, y, w, h);
                region = QRegion(x, y, w, h);

                if(drawPixmap)
                {
                    QPainter paint(drawPixmap);
                    drawItem(paint, index);
                }
            }

            // Repaint the region that was changed.
            if(isVisible())
                repaint(region);
            else if(drawPixmap)
            {
                delete drawPixmap;
                drawPixmap = 0;
            }
        }
    }
}

// ****************************************************************************
// Method: QvisColorGridWidget::containsColor
//
// Purpose: 
//   Searches the color palette for a specified color.
//
// Returns:    Whether or not the color is in the palette.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 5 16:41:47 PST 2000
//
// Modifications:
//   
// ****************************************************************************

bool
QvisColorGridWidget::containsColor(const QColor &color) const
{
    bool notFound = true;

    for(int i = 0; i < numGridSquares && notFound; ++i)
    {
        if(color == paletteColors[i])
            notFound = false;
    }

    return !notFound;
}

// ****************************************************************************
// Method: QvisColorGridWidget::keyPressEvent
//
// Purpose: 
//   This is the event handler for keypresses. It allows the user to operate
//   this widget with the keyboard.
//
// Arguments:
//   e : The key event.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 4 19:59:53 PST 2000
//
// Modifications:
//   Brad Whitlock, Wed Feb 26 13:10:56 PST 2003
//   Made some internal interface changes.
//
// ****************************************************************************

void 
QvisColorGridWidget::keyPressEvent(QKeyEvent *e)
{
    QColor temp;
    int    column = activeIndex() % numColumns;
    int    row = activeIndex() / numColumns;

    // Handle the key strokes.
    switch(e->key())
    {
    case Key_Escape:
        // emit an empty color.
        emit selectedColor(temp);
        break;
    case Key_Return:
    case Key_Enter:
        setSelectedIndex(activeIndex());
        break;
    case Key_Left:
        if(column == 0)
            setActiveIndex(getIndex(row, numColumns - 1));
        else
            setActiveIndex(getIndex(row, column - 1));
        break;
    case Key_Right:
        if(column == numColumns - 1)
            setActiveIndex(getIndex(row, 0));
        else
            setActiveIndex(getIndex(row, column + 1));
        break;
    case Key_Up:
        if(row == 0)
            setActiveIndex(getIndex(numRows - 1, column));
        else
            setActiveIndex(getIndex(row - 1, column));
        break;
    case Key_Down:
        if(row == numRows - 1)
            setActiveIndex(getIndex(0, column));
        else
            setActiveIndex(getIndex(row + 1, column));
        break;
    }
}

// ****************************************************************************
// Method: QvisColorGridWidget::mousePressEvent
//
// Purpose: 
//   This method is called when the mouse is clicked in the widget. It sets
//   the selected color based on the color that was clicked.
//
// Arguments:
//   e : The mouse event.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 21 11:07:54 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisColorGridWidget::mousePressEvent(QMouseEvent *e)
{
    if(e->button() == RightButton)
    {
        int index = getIndexFromXY(e->x(), e->y());

        // If a valid color index was returned, select the color.
        if(index != -1)
        {
            // Set the selected color.
            setSelectedIndex(index);

            // Emit a signal that allows us to activate a menu.
            int row, column;
            QPoint center(e->x(), e->y());
            getRowColumnFromIndex(currentSelectedItem, row, column);
            emit activateMenu(selectedColor(), row, column,
                              mapToGlobal(center));
        }
    }
}

// ****************************************************************************
// Method: QvisColorGridWidget::drawColor
//
// Purpose: 
//   Draws the specified color box into the drawing pixmap.
//
// Arguments:
//   paint : The painter to use.
//   index : The index of the color to draw.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 4 20:09:29 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Apr 26 11:47:44 PDT 2002
//   I fixed an error that cropped up on windows.
//
// ****************************************************************************

void
QvisColorGridWidget::drawItem(QPainter &paint, int index)
{
    if(index >= 0)
    {
        // Get the location of the index'th color box.
        int x, y, boxWidth, boxHeight;
        getItemRect(index, x, y, boxWidth, boxHeight);

        paint.setPen(colorGroup().dark());
        paint.drawRect(x, y, boxWidth, boxHeight);
        paint.fillRect(x + 1, y + 1, boxWidth - 2, boxHeight - 2,
                       paletteColors[index]);
    }
}

// ****************************************************************************
//  Method:  QvisColorGridWidget::emitSelection
//
//  Purpose:
//    Emit any signals associated with making a selection.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 11, 2006
//
// ****************************************************************************
void
QvisColorGridWidget::emitSelection()
{
    emit selectedColor(paletteColors[currentSelectedItem]);
    emit selectedColor(paletteColors[currentSelectedItem],
                       currentSelectedItem);
    int row, column;
    getRowColumnFromIndex(currentSelectedItem, row, column);
    emit selectedColor(paletteColors[currentSelectedItem], row, column);
}
