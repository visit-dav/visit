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
#include <SpreadsheetTable.h>

#include <qpainter.h>
#include <qpalette.h>
#include <qstyle.h>
#include <qfontmetrics.h>

#include <avtLookupTable.h>
#include <vtkLookupTable.h>

// ****************************************************************************
// Method: SpreadsheetTable::SpreadsheetTable
//
// Purpose: 
//   Constructor for the SpreadsheetTable class.
//
// Arguments:
//   parent : The parent widget.
//   name   : This widget's name.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 15:32:53 PST 2007
//
// Modifications:
//   Gunther H. Weber, Fri Sep 14 11:40:18 PDT 2007
//   Changed focus style due to display problems of current cell when set
//   from pick attributes.
//   
// ****************************************************************************

SpreadsheetTable::SpreadsheetTable(QWidget *parent, const char *name) : 
    QTable(parent, name), formatString("%1.6f")
{
    lut = 0;
    renderInColor = true; //false;

    // Input data values.
    dataArray = 0;
    ghostArray = 0;
    dims[0] = dims[1] = dims[2] = 0;
    displayMode = SliceZ;
    sliceIndex = 0;

    setFocusStyle(QTable::FollowStyle);
}

// ****************************************************************************
// Method: SpreadsheetTable::~SpreadsheetTable
//
// Purpose: 
//   Destructor for the SpreadsheetTable class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 15:33:26 PST 2007
//
// Modifications:
//   
// ****************************************************************************

SpreadsheetTable::~SpreadsheetTable()
{
}

// ****************************************************************************
// Method: SpreadsheetTable::setLUT
//
// Purpose: 
//   Sets the LUT that should be used when we need to map data to colors.
//
// Arguments:
//   L  : The LUT to use.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 15:33:44 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
SpreadsheetTable::setLUT(avtLookupTable *L)
{
    lut = L;
}

// ****************************************************************************
// Method: SpreadsheetTable::setRenderInColor
//
// Purpose: 
//   Sets whether the table should render its text in color.
//
// Arguments:
//   ric : True for rendering in color; Otherwise false.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 15:34:15 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void 
SpreadsheetTable::setRenderInColor(bool ric)
{
    if(renderInColor != ric)
    {
        renderInColor = ric;
        update();
    }
}

// ****************************************************************************
// Method: SpreadsheetTable::setFormatString
//
// Purpose: 
//   Sets the format string to use for drawing text.
//
// Arguments:
//   fmt : The format string.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 15:34:46 PST 2007
//
// Modifications:
//   Gunther H. Weber, Thu Sep 27 11:37:18 PDT 2007
//   Adapt column width to fit displayed values
//   
// ****************************************************************************

void
SpreadsheetTable::setFormatString(const QString &fmt)
{
    if(fmt != formatString)
    {
        formatString = fmt;
        updateColumnWidths();
        update();
    }
}

// ****************************************************************************
// Method: SpreadsheetTable::setDataArray
//
// Purpose: 
//   Sets the data array and other parameters used to render the data in
//   the spreadsheet.
//
// Arguments:
//   arr : The data array to use for values.
//   ghosts : The data array to use for ghosting values.
//   d      : The i,j,k dimensions of the dataset.
//   dm     : The display mode (which way the data is sliced in the spreadsheet).
//   sliceindex : The index of the slice in the prescribed dimension
//   base_index : The base_index values to add to i,j,k when displaying the
//                indices that identify a cell or node.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 15:35:22 PST 2007
//
// Modifications:
//   Gunther H. Weber, Thu Sep 27 11:37:18 PDT 2007
//   Adapt column width to fit displayed values
//   
// ****************************************************************************

void
SpreadsheetTable::setDataArray(vtkDataArray *arr, vtkDataArray *ghosts,
    int d[3], DisplayMode dm, int sliceindex, int base_index[3])
{
    // The data arrays.
    dataArray = arr;
    ghostArray = ghosts;

    // The size of the data arrays.
    dims[0] = d[0];
    dims[1] = d[1];
    dims[2] = d[2];

    // The slice that this table will display.
    displayMode = dm;
    sliceIndex = sliceindex;

    if(displayMode == SliceX)
    {
        setNumCols(dims[2]);
        setNumRows(dims[1]);

        // Set the header labels appropriately.
        for(int k = 0; k < dims[2]; ++k)
        {
            QString label; label.sprintf("k=%d", k + base_index[2]);
            horizontalHeader()->setLabel(k, label);
        }
        for(int j = 0; j < dims[1]; ++j)
        {
            QString label; label.sprintf("j=%d", dims[1]-1-j + base_index[1]);
            verticalHeader()->setLabel(j, label);
        }
    }
    else if(displayMode == SliceY)
    {
        setNumCols(dims[2]);
        setNumRows(dims[0]);

        // Set the header labels appropriately.
        for(int i = 0; i < dims[0]; ++i)
        {
            QString label; label.sprintf("i=%d", dims[0]-1-i + base_index[0]);
            verticalHeader()->setLabel(i, label);
        }
        for(int k = 0; k < dims[2]; ++k)
        {
            QString label; label.sprintf("k=%d", k + base_index[2]);
            horizontalHeader()->setLabel(k, label);
        }
    }
    else if(displayMode == SliceZ)
    {
        setNumCols(dims[0]);
        setNumRows(dims[1]);

        // Set the header labels appropriately.
        for(int i = 0; i < dims[0]; ++i)
        {
            QString label; label.sprintf("i=%d", i + base_index[0]);
            horizontalHeader()->setLabel(i, label);
        }
        for(int j = 0; j < dims[1]; ++j)
        {
            QString label; label.sprintf("j=%d", dims[1]-1-j + base_index[1]);
            verticalHeader()->setLabel(j, label);
        }
    }
    else if(displayMode == UCDCell)
    {
        setNumCols(1);
        setNumRows(dims[1]);

        // Set the header labels appropriately.
        horizontalHeader()->setLabel(0, "cell value");
        for(int i = 0; i < dims[1]; ++i)
        {
            QString label; label.sprintf("%d", i + base_index[0]);
            verticalHeader()->setLabel(i, label);
        }
    }
    else if(displayMode == UCDNode)
    {
        setNumCols(1);
        setNumRows(dims[1]);

        // Set the header labels appropriately.
        horizontalHeader()->setLabel(0, "node value");
        for(int i = 0; i < dims[1]; ++i)
        {
            QString label; label.sprintf("%d", i + base_index[0]);
            verticalHeader()->setLabel(i, label);
        }
    }

    // Adjust column width
    QFontMetrics fm(font());
    QString lengthProbeString;
    lengthProbeString.sprintf(formatString, -1.11111111111111111111);
    for (int i=0; i<numCols(); ++i)
        setColumnWidth(i, fm.width(" AA=") + fm.width(lengthProbeString));
}

// ****************************************************************************
// Method: SpreadsheetTable
//
// Purpose: 
//   Disassociates the current data and ghost arrays from the spreadsheeet.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 15:37:38 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
SpreadsheetTable::clearDataArray()
{
    dataArray = 0;
    ghostArray = 0;
    selectNone();
}

// ****************************************************************************
// Method: SpreadsheetTable::selectedCellsAsText
//
// Purpose: 
//   Gets the selected cells as text.
//
// Returns:    The string containing the text for all selected cells of 
//             the table.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 22 10:47:33 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

QString
SpreadsheetTable::selectedCellsAsText() const
{
    QString txt;
    if(dataArray != 0)
    {
        int selId = 0;
        if(currentSelection() > -1)
            selId = currentSelection();
        else if(numSelections() > 0)
            selId = 0;
        else
            return txt;

        QTableSelection sel(selection(selId));

        QString s; 
        for(int row = sel.bottomRow(); row >= sel.topRow(); --row)
        {
            for(int col = sel.leftCol(); col <= sel.rightCol(); ++col)
            {
                int index = rowColToIndex(row, col);
                double value = 0.;
                if(index < dataArray->GetNumberOfTuples())
                    value = dataArray->GetTuple1(index);
                s.sprintf(formatString.latin1(), value);
                if(col > sel.leftCol())
                    txt += " ";
                txt += s;
            }
            txt += "\n";
        }
    }

    return txt;
}

// ****************************************************************************
// Method: SpreadsheetTable::selectedCellsSum
//
// Purpose: 
//   Sums the data values over the selected cells.
//
// Returns:    The sum of the values in the selected cells.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 22 12:28:57 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

double
SpreadsheetTable::selectedCellsSum() const
{
    double sum = 0.;

    if(dataArray != 0)
    {
        int selId = 0;
        if(currentSelection() > -1)
            selId = currentSelection();
        else if(numSelections() > 0)
            selId = 0;
        else
            return sum;

        QTableSelection sel(selection(selId));

        for(int row = sel.bottomRow(); row >= sel.topRow(); --row)
        {
            for(int col = sel.leftCol(); col <= sel.rightCol(); ++col)
            {
                int index = rowColToIndex(row, col);
                if(index < dataArray->GetNumberOfTuples())
                    sum += dataArray->GetTuple1(index);
            }
        }
    }

    return sum;
}

// ****************************************************************************
// Method: SpreadsheetTable::selectedCellsAverage
//
// Purpose: 
//   Average the values of the selected cells.
//
// Returns:    The average of the selected cells.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 22 12:29:26 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

double
SpreadsheetTable::selectedCellsAverage() const
{
    double avg = 0.;

    if(dataArray != 0)
    {
        int selId = 0;
        if(currentSelection() > -1)
            selId = currentSelection();
        else if(numSelections() > 0)
            selId = 0;
        else
            return avg;

        QTableSelection sel(selection(selId));
        double sum = 0.;
        int ncells = 0;
        for(int row = sel.bottomRow(); row >= sel.topRow(); --row)
        {
            for(int col = sel.leftCol(); col <= sel.rightCol(); ++col)
            {
                int index = rowColToIndex(row, col);
                if(index < dataArray->GetNumberOfTuples())
                    sum += dataArray->GetTuple1(index);
                ++ncells;
            }
        }
        if(ncells > 0)
           avg = sum / double(ncells);
    }

    return avg;
}

// ****************************************************************************
// Method: SpreadsheetTable::addSelectedCellLabel()
//
// Purpose: 
//   Add a label (pick letter) for a cell, which will be displayed if the
//   cell is selected.
//
// Arguments:
//   row : The row of the cell.
//   col : The column of the cell.
//   label: The label, i.e., pick letter
//
// Programmer: Gunther H. Weber
// Creation:   Fri Sep 14 11:44:31 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
SpreadsheetTable::addSelectedCellLabel(int row, int col, const std::string &label)
{
    // If there is already a previous pick for the same cell, replace the letter
    // for it with the new one ...
    for (std::list<SelectedCellLabel>::iterator it = selectedCellLabels.begin();
           it != selectedCellLabels.end(); ++it)
    {
        if ((it->row == row) && (it->col == col))
        {
            it->label=label;
            return;
        }

    } 
    // ... otherwise add new pick letter
    selectedCellLabels.push_back(SelectedCellLabel(row, col, label));
}

// ****************************************************************************
// Method: SpreadsheetTable::clearSelectedCellLabels()
//
// Purpose: 
//   Clear the list of cell labels (pick letters for cells).
//
// Programmer: Gunther H. Weber
// Creation:   Fri Sep 14 11:44:31 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
SpreadsheetTable::clearSelectedCellLabels()
{
    selectedCellLabels.clear();
}

// ****************************************************************************
// Method: SpreadsheetTable::setFont()
//
// Purpose: 
//   Update column widths after a font change
//
// Programmer: Gunther H. Weber
// Creation:   Thu Sep 27 13:37:40 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
SpreadsheetTable::setFont(QFont&f)
{
    QTable::setFont(f);
    updateColumnWidths();
}

// ****************************************************************************
// Method: SpreadsheetTable::rowColToIndex
//
// Purpose: 
//   Transforms row,col table index into an index into the dataset.
//
// Arguments:
//   row : The row of the cell.
//   col : The column of the cell.
//
// Returns:    The index into the dataset.
//
// Note:       This method accounts for different slice directions.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 15:38:05 PST 2007
//
// Modifications:
//   
// ****************************************************************************

int
SpreadsheetTable::rowColToIndex(int row, int col) const
{
    int index = 0;
    int nxny = dims[0] * dims[1];
 #define STRUCTURED_INDEX3(z,y,x) (((z)*nxny) + ((y)*dims[0]) + (x))
    if(displayMode == SliceX)
    {
        int i = sliceIndex;
        int j = dims[1]-1-row;
        int k = col;
        index = STRUCTURED_INDEX3(k, j, i);
    }
    else if(displayMode == SliceY)
    {
        int i = dims[0]-1-row;
        int j = sliceIndex;
        int k = col;
        index = STRUCTURED_INDEX3(k, j, i);
    }
    else if(displayMode == SliceZ)
    {
        int i = col;
        int j = dims[1]-1-row;
        int k = sliceIndex;
        index = STRUCTURED_INDEX3(k, j, i);
    }
    else
        index = row;

    return index;
}

// ****************************************************************************
// Method: SpreadsheetTable::displayValue
//
// Purpose: 
//   Returns the data value and ghost value for the specified cell.
//
// Arguments:
//   row : The cell row.
//   col : The cell column.
//   ghost : The return value for ghosting. True means the cell is ghost.
//
// Returns:    The data value at the cell.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 15:39:13 PST 2007
//
// Modifications:
//   
// ****************************************************************************

double
SpreadsheetTable::displayValue(int row, int col, bool &ghost) const
{
    int index = rowColToIndex(row, col);

    // Get the 
    ghost = false;
    if(ghostArray != 0)
    {
        // By convention, the ghost zones array contains unsigned char.
        const unsigned char *ghosts = (const unsigned char *)ghostArray->
            GetVoidPointer(0);
        if(index < ghostArray->GetNumberOfTuples())
            ghost = ghosts[index] > 0;
    }

    double value = 0.;
    if(index < dataArray->GetNumberOfTuples())
        value = dataArray->GetTuple1(index);

    return value;
}

// ****************************************************************************
// Method: SpreadsheetTable::paintCell
//
// Purpose: 
//   This method paints a cell using data directly from the VTK memory 
//   associated with the spreadsheet.
//
// Arguments:
//   p : The painter to use.
//   row : The row of the cell.
//   col : The column of the cell.
//   cr  : The cell rectangle.
//   selected : True if the cell is selected.
//   cg       : The color group to use when drawing the cell.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 15:40:16 PST 2007
//
// Modifications:
//   Gunther H. Weber, Fri Sep 14 11:41:13 PDT 2007
//   Draw background of current cell in different color from other selected
//   cells. Show pick letters for selected cells.
//   
// ****************************************************************************

void
SpreadsheetTable::paintCell(QPainter *p, int row, int col, 
                           const QRect &cr, bool selected,
                           const QColorGroup &cg)
{
    if (focusStyle() == SpreadSheet && selected &&
        row == currentRow() &&
        col == currentColumn() && (hasFocus() || viewport()->hasFocus()))
        selected = FALSE;

    int w = cr.width();
    int h = cr.height();
    int x2 = w - 1;
    int y2 = h - 1;

    if (dataArray != 0)
    {
        // Get the data value from the VTK memory
        bool ghost = false;
        double dataVal = displayValue(row, col, ghost);
        QString s; s.sprintf(formatString.latin1(), dataVal);

        if (selected)
        {
            for (std::list<SelectedCellLabel>::const_iterator it = selectedCellLabels.begin();
                    it != selectedCellLabels.end(); ++it)
            {
                if (row == it-> row && col == it->col)
                {
                    s = QString(it->label.c_str()) + QString("=") + s;
                }
            }
        }

        // Paint the background of the cell.
        if(ghost)
        {
            p->fillRect(0, 0, w, h, 
                selected ? cg.brush(QColorGroup::Highlight) : QColor(200,200,200));
        }
        else
        {
            bool current = (col == currentColumn() && row == currentRow());

            p->fillRect(0, 0, w, h, 
                selected ?
                    (current ? cg.brush(QColorGroup::Highlight) : // Selected and current
                     QColor(128,128,128)) : // Selected, not current
                    cg.brush(QColorGroup::Base)); // Not selected
        }


        // Determine the color of the text in the cell.
        if(ghost)
        {
            if(selected)
                p->setPen(cg.highlightedText());
            else
                p->setPen(cg.text());
        }
        else if(renderInColor && lut != 0)
        {
            // Map the value to a color.
            const unsigned char *rgb = lut->GetLookupTable()->MapValue(dataVal);
            QColor c(rgb[0], rgb[1], rgb[2]);
            p->setPen(c);
        }
        else if(selected)
            p->setPen(cg.highlightedText());
        else
            p->setPen(cg.text());

        // Draw the value according to the format string.
        p->drawText(2, 0, w - 4, h, Qt::AlignRight | Qt::AlignVCenter,
                    s);
    }
    else
    {
        // Paint the background of the cell.
        p->fillRect(0, 0, w, h, selected ? cg.brush(QColorGroup::Highlight) : cg.brush(QColorGroup::Base));
    }

    if (showGrid())
    {
        // Draw our lines
        QPen pen(p->pen());
        int gridColor = style().styleHint(QStyle::SH_Table_GridLineColor, this);
        if (gridColor != -1)
        {
            const QPalette &pal = palette();
            if (cg != colorGroup() &&
                cg != pal.disabled() &&
                cg != pal.inactive())
                p->setPen(cg.mid());
            else
                p->setPen((QRgb)gridColor);
        }
        else
            p->setPen(cg.mid());
        
        p->drawLine(x2, 0, x2, y2);
        p->drawLine(0, y2, x2, y2);
        p->setPen(pen);
    } 
}

// ****************************************************************************
// Method: SpreadsheetTable::selectAll
//
// Purpose: 
//   Selects all of the cells in the table.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 22 14:31:06 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
SpreadsheetTable::selectAll()
{
    selectNone();
    QTableSelection sel;
    sel.init(0, 0);
    sel.expandTo(numRows(), numCols());
    addSelection(sel);
}

// ****************************************************************************
// Method: SpreadsheetTable::selectNone
//
// Purpose: 
//   Removes all of the selections from the table.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 22 14:31:23 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
SpreadsheetTable::selectNone()
{
    // Remove the selections that may be on the table.
    for(int i = 0; i < numSelections(); ++i)
        removeSelection(i);
    horizontalHeader()->update();
    verticalHeader()->update();
    emit selectionChanged();
}

// ****************************************************************************
// Method: SpreadsheetTable::updateColumnWidths
//
// Purpose: 
//   Change table column width so that numeric values (and labels) displayed
//   in cells are not truncated
//
// Programmer: Gunther H. Weber
// Creation:   Thu Sep 27 13:43:05 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
SpreadsheetTable::updateColumnWidths()
{
    QFontMetrics fm(font());
    QString lengthProbeString;
    lengthProbeString.sprintf(formatString, -1.11111111111111111111);
    for (int i=0; i<numCols(); ++i)
        setColumnWidth(i, fm.width(" AA=") + fm.width(lengthProbeString));
}
