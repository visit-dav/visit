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

#include <QvisPeriodicTableWidget.h>
#include <QCursor>
#include <QKeyEvent>
#include <QPainter>

#include "AtomicProperties.h"

static int periodic_table[10][18] = {
{  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2},
{  3,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  5,  6,  7,  8,  9, 10},
{ 11, 12,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 13, 14, 15, 16, 17, 18},
{ 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36},
{ 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54},
{ 55, 56,  0, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86},
{ 87, 88,  0,104,105,106,107,108,109,110,111,112,113,114,115,116,  0,  0},
{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
{  0,  0, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71,  0},
{  0,  0, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99,100,101,102,103,  0}
};

static int periodic_colors[10][18] = {
{  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2},
{  3,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  5,  5,  5,  5,  5,  2},
{  3,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  6,  5,  5,  5,  5,  2},
{  3,  4,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  6,  6,  5,  5,  5,  2},
{  3,  4,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  6,  6,  6,  5,  5,  2},
{  3,  4,  0,  7,  7,  7,  7,  7,  7,  7,  7,  7,  6,  6,  6,  6,  5,  2},
{  3,  4,  0,  7,  7,  7,  7,  7,  7,  7,  7,  7,  6,  6,  6,  6,  0,  0},
{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
{  0,  0,  7,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  0},
{  0,  0,  7,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  0}
};

static int element_type_colors[9][3] = {
{ 200, 200, 200 }, // 1: hydrogen
{ 100, 240, 240 }, // 2: noble gas
{ 255, 180,   0 }, // 3: alkali
{ 240, 240,   0 }, // 4: alkaline
{   0, 240,   0 }, // 5: nonmetals
{   0, 200, 200 }, // 6: poor metals
{ 255, 120, 160 }, // 7: transition metals
{ 255, 200, 120 }, // 8: lanthanides
{ 220, 180, 200 }, // 9: actinides
};


// ****************************************************************************
// Method: QvisPeriodicTableWidget::QvisPeriodicTableWidget
//
// Purpose: 
//   Constructor for the QvisPeriodicTableWidget class.
//
// Arguments:
//   parent : The parent widget to this object.
//   name   : The name of this object.
//   f      : The window flags. These control how window decorations are done.
//
// Programmer: Jeremy Meredith
// Creation:   August 11, 2006
//
// Modifications:
//    Jeremy Meredith, Mon Feb 11 15:21:46 EST 2008
//    Removed setting of ispopup to true; it's not for us to decide.
//
//    Jeremy Meredith, Tue Feb 12 14:00:26 EST 2008
//    Added support for hinting selectable elements.
//
//    Brad Whitlock, Tue Jun  3 14:22:03 PDT 2008
//    Qt 4.
//
// ****************************************************************************

QvisPeriodicTableWidget::QvisPeriodicTableWidget(QWidget *parent, 
    Qt::WindowFlags f) : QvisGridWidget(parent, f)
{
    numRows = 10;
    numColumns = 18;
    numGridSquares = numRows * numColumns;
    drawFrame = true;
    boxSizeValue = 24;
    boxPaddingValue = 2;
    setMinimumSize(minimumSize());
    hintedElements = new bool[numGridSquares];
    for (int i=0; i<numGridSquares; i++)
        hintedElements[i] = false;
}

// ****************************************************************************
// Method: QvisPeriodicTableWidget::~QvisPeriodicTableWidget
//
// Purpose: 
//   Destructor for the QvisPeriodicTableWidget class.
//
// Programmer: Jeremy Meredith
// Creation:   August 11, 2006
//
// Modifications:
//    Jeremy Meredith, Tue Feb 12 14:00:26 EST 2008
//    Added support for hinting selectable elements.
//   
// ****************************************************************************

QvisPeriodicTableWidget::~QvisPeriodicTableWidget()
{
    delete[] hintedElements;
}


// ****************************************************************************
// Method: QvisPeriodicTableWidget::setSelectedElement
//
// Purpose: 
//   Sets an element to be selected by element number.
//
// Programmer: Jeremy Meredith
// Creation:   August 11, 2006
//
// Modifications:
//    Jeremy Meredith, Mon Feb 11 16:46:57 EST 2008
//    Default to "-1" element number in case we didn't have anything selected.
//   
//    Jeremy Meredith, Wed May 20 11:50:45 EDT 2009
//    Fixed logic error.  Was not actually defaulting to -1.
//
// ****************************************************************************

void
QvisPeriodicTableWidget::setSelectedElement(int element)
{
    int index = -1;
    for (int i = 0; i < numGridSquares; i++)
    {
        if (indexToElement(i) == element)
        {
            index = i;
            break;
        }
    }
    setSelectedIndex(index);
}


// ****************************************************************************
// Method: QvisPeriodicTableWidget::keyPressEvent
//
// Purpose: 
//   This is the event handler for keypresses. It allows the user to operate
//   this widget with the keyboard.
//
// Arguments:
//   e : The key event.
//
// Programmer: Jeremy Meredith
// Creation:   August 11, 2006
//
// Modifications:
//   Brad Whitlock, Tue Jun  3 14:22:28 PDT 2008
//   Qt 4.
//
//   Jeremy Meredith, Wed May 20 11:49:18 EDT 2009
//   Added a fake "0" element which means "unknown", and hydrogen
//   now starts at 1, so we don't need to correct for 1-origin indices.
//
// ****************************************************************************

void 
QvisPeriodicTableWidget::keyPressEvent(QKeyEvent *e)
{
    int    column = activeIndex() % numColumns;
    int    row = activeIndex() / numColumns;

    // Handle the key strokes.
    switch(e->key())
    {
    case Qt::Key_Escape:
        // emit an empty color.
        emit selectedElement(indexToElement(activeIndex()));
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        setSelectedIndex(activeIndex());
        break;
    case Qt::Key_Left:
        if(column == 0)
            setActiveIndex(getIndex(row, numColumns - 1));
        else
            setActiveIndex(getIndex(row, column - 1));
        break;
    case Qt::Key_Right:
        if(column == numColumns - 1)
            setActiveIndex(getIndex(row, 0));
        else
            setActiveIndex(getIndex(row, column + 1));
        break;
    case Qt::Key_Up:
        if(row == 0)
            setActiveIndex(getIndex(numRows - 1, column));
        else
            setActiveIndex(getIndex(row - 1, column));
        break;
    case Qt::Key_Down:
        if(row == numRows - 1)
            setActiveIndex(getIndex(0, column));
        else
            setActiveIndex(getIndex(row + 1, column));
        break;
    }
}

// ****************************************************************************
// Method: QvisPeriodicTableWidget::drawItem
//
// Purpose: 
//   Draws the specified color box into the widget.
//
// Arguments:
//   paint : The painter to use.
//   index : The index of the color to draw.
//
// Programmer: Jeremy Meredith
// Creation:   August 11, 2006
//
// Modifications:
//    Jeremy Meredith, Tue Feb 12 14:00:26 EST 2008
//    Added support for hinting selectable elements.  The hinting method
//    that appeared to be the best combination between noticeable and
//    distracting, at least on my machine, was simply to boldface the font.
//
//    Brad Whitlock, Tue Jun  3 15:17:24 PDT 2008
//    Make the highlight more obvious on the active item.
//
//    Jeremy Meredith, Wed May 20 11:49:18 EDT 2009
//    Added a fake "0" element which means "unknown", and hydrogen
//    now starts at 1, so we don't need to correct for 1-origin indices.
//
// ****************************************************************************

void
QvisPeriodicTableWidget::drawItem(QPainter &paint, int index)
{
    if (index < 0)
        return;

    int element = indexToElement(index);
    if (element <= 0)
        return;

    if (element > MAX_ELEMENT_NUMBER)
        return;

    bool hint = hintedElements[index];

    int r = index / numColumns;
    int c = index % numColumns;
    int *colorvals = element_type_colors[periodic_colors[r][c]-1];
    
    // Get the location of the index'th color box.
    int x, y, boxWidth, boxHeight;
    getItemRect(index, x, y, boxWidth, boxHeight);

    QColor color(colorvals[0], colorvals[1], colorvals[2]);
    if(index == activeIndex())
        color = palette().color(QPalette::Highlight);
    paint.fillRect(x + 1, y + 1, boxWidth - 2, boxHeight - 2, color);
    drawBox(paint, QRect(x+1,y+1,boxWidth-2,boxHeight-2),
            color.light(125),
            color.dark(125), 1);
    QFont oldfont(paint.font());
    if (hint)
    {
        QFont newfont(oldfont);
        newfont.setBold(true);
        paint.setFont(newfont);
    }
    if(index == activeIndex())
        paint.setPen(palette().color(QPalette::HighlightedText));
    else
        paint.setPen(palette().color(QPalette::WindowText));
    paint.drawText(QRect(x,y,boxWidth,boxHeight),
                   Qt::AlignHCenter | Qt::AlignVCenter,
                   element_names[element]);
    if (hint)
    {
        paint.setFont(oldfont);
    }
}


// ****************************************************************************
// Method: QvisPeriodicTableWidget::isValidIndex
//
// Purpose: 
//   Returns true if a given index is actually valid.
//
// Programmer: Jeremy Meredith
// Creation:   August 11, 2006
//
// Modifications:
//    Jeremy Meredith, Wed May 20 11:49:18 EDT 2009
//    MAX_ELEMENT_NUMBER now means the actual max element number, not the
//    total number of known elements in visit.  Added a fake "0" element
//    which means "unknown", and hydrogen now starts at 1.
//   
// ****************************************************************************

bool
QvisPeriodicTableWidget::isValidIndex(int index) const
{
    return ((index >= 0) &&
            (index < numGridSquares) &&
            (indexToElement(index) <= MAX_ELEMENT_NUMBER) &&
            (indexToElement(index) > 1));
}

// ****************************************************************************
// Method: QvisPeriodicTableWidget::emitSelection
//
// Purpose: 
//   Emit any signals associated making a selection.
//
// Programmer: Jeremy Meredith
// Creation:   August 11, 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisPeriodicTableWidget::emitSelection()
{
    emit selectedElement(indexToElement(currentSelectedItem)-1);
}

// ****************************************************************************
//  Method:  QvisPeriodicTableWidget::indexToElement
//
//  Purpose:
//    Convert an index (e.g. from row/col) into an element number.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 11, 2006
//
// ****************************************************************************
int
QvisPeriodicTableWidget::indexToElement(int index) const
{
    int r = index / numColumns;
    int c = index % numColumns;
    int e = periodic_table[r][c];
    return e;
}


// ****************************************************************************
//  Method:  QvisPeriodicTableWidget::setHintedElements
//
//  Purpose:
//    Accept a list of atomic numbers that we will use for hinting.
//
//  Arguments:
//    elements   the list of atomic numbers
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 12, 2008
//
// ****************************************************************************

void
QvisPeriodicTableWidget::setHintedElements(const std::vector<int> &elements)
{
    for (int i=0; i<numGridSquares; i++)
    {
        hintedElements[i] = false;
        int element = indexToElement(i);
        if (element <= 0)
            continue;
        for (size_t j=0; j<elements.size(); j++)
        {
            if (elements[j] == element)
            {
                hintedElements[i] = true;
                break;
            }
        }
    }
}
