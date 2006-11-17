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

#include <QvisSequenceView.h>
#include <qcursor.h>
#include <qdrawutil.h>
#include <qfontmetrics.h>
#include <qpainter.h>

// ****************************************************************************
// Method: QvisSequenceView::QvisSequenceView
//
// Purpose: 
//   Constructor for the QvisSequenceView class.
//
// Arguments:
//   parent : The widget's parent.
//   name   : The name of the widget.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:55:38 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

QvisSequenceView::QvisSequenceView(QWidget *parent, const char *name) : 
    QGridView(parent, name)
{
    nRowsExist = 1;
    setNumRows(nRowsExist);
    nColsExist = 1;
    setNumCols(nColsExist);

    QRect r(fontMetrics().boundingRect("XVIEWPORT_1X"));
    setCellWidth(r.width());
    setCellHeight(r.height() * 5);

    srcRow = srcCol = -1;
    dropSiteIndex = -1;
}

// ****************************************************************************
// Method: QvisSequenceView::~QvisSequenceView
//
// Purpose: 
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:56:14 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

QvisSequenceView::~QvisSequenceView()
{
}

// ****************************************************************************
// Method: QvisSequenceView::clear
//
// Purpose: 
//   Clears out all of the sequences.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:56:33 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisSequenceView::clear()
{
    nRowsExist = 1;
    setNumRows(nRowsExist);
    nColsExist = 1;
    setNumCols(nColsExist);

    sequencesPerViewport.clear();
    dropSites.clear();
    srcRow = srcCol = dropSiteIndex = -1;

    update();
}

// ****************************************************************************
// Method: QvisSequenceView::getNumberOfSequencesInViewport
//
// Purpose: 
//   Get the number of sequences mapped to a viewport.
//
// Arguments:
//   vpt : The name of the viewport.
//
// Returns:    The number of sequences mapped to the viewport.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:57:09 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

int
QvisSequenceView::getNumberOfSequencesInViewport(const QString &vpt) const
{
    int nseq = -1;
    QStringQSequenceDataListMap::ConstIterator it;
    it = sequencesPerViewport.find(vpt);
    if(it != sequencesPerViewport.end())
        nseq = it.data().size();

    return nseq;
}

// ****************************************************************************
// Method: QvisSequenceView::getSequenceInViewport
//
// Purpose: 
//   Get a sequence in a viewport.
//
// Arguments:
//   vpName : The viewport name.
//   index  : The index of the sequence in the viewport.
//   name   : Return value for the sequence name.
//   t      : Return value for the sequence type.
//
// Returns:    True on success; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:57:41 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
QvisSequenceView::getSequenceInViewport(const QString &vpName, int index,
    QString &name, int &t) const
{
    QSequenceData info;
    bool ret;
    if((ret = getSequenceInformation(vpName, index, info)) == true)
    {
        name = info.name; 
        t = info.seqType;
    }
    return ret;
}

// ****************************************************************************
// Method: QvisSequenceView::addViewport
//
// Purpose: 
//   Adds a new viewport.
//
// Arguments:
//   viewportName : The name of the new viewport.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:59:19 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisSequenceView::addViewport(const QString &viewportName)
{
    if(sequencesPerViewport.count() > 0)
    {
        ++nRowsExist;
        setNumRows(nRowsExist);
    }

    sequencesPerViewport[viewportName] = QSequenceDataList();
}

// ****************************************************************************
// Method: QvisSequenceView::addSequenceToViewport
//
// Purpose: 
//   Adds a new sequence to the viewport.
//
// Arguments:
//   vpt     : The name of the viewport.
//   seqName : The name of the new sequence.
//   seqType : The type of sequence.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:59:50 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisSequenceView::addSequenceToViewport(const QString &vpt, 
    const QString &seqName, int seqType)
{
    addSequenceToViewport(vpt, seqName, QPixmap(), seqType);
}

// ****************************************************************************
// Method: QvisSequenceView::addSequenceToViewport
//
// Purpose: 
//   Adds a new sequence to the viewport.
//
// Arguments:
//   vpt     : The name of the viewport.
//   seqName : The name of the new sequence.
//   pix     : The sequence's pixmap.
//   seqType : The type of sequence.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 11:59:50 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisSequenceView::addSequenceToViewport(const QString &vpt, 
    const QString &seqName, const QPixmap &pix, int seqType)
{
    QStringQSequenceDataListMap::Iterator it;
    it = sequencesPerViewport.find(vpt);
    if(it == sequencesPerViewport.end())
    {
        sequencesPerViewport[vpt] = QSequenceDataList();
        it = sequencesPerViewport.find(vpt);
    }

    // Save info about this sequence.
    QSequenceData newSequence;
    newSequence.name = seqName;
    newSequence.pixmap = pix;
    newSequence.seqType = seqType;
    it.data().push_back(newSequence);

    if(it.data().size()+1 > nColsExist)
    {
        ++nColsExist;
        setNumCols(nColsExist);
    }

    update();

    // Emit something.
}

// ****************************************************************************
// Method: QvisSequenceView::insertSequenceInViewport
//
// Purpose: 
//   Inserts a sequence into the viewport.
//
// Arguments:
//   vpt     : The viewport name.
//   index   : the index at which to insert.
//   seqInfo : Sequence information.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 12:01:38 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisSequenceView::insertSequenceInViewport(const QString &vpt, int index,
    const QvisSequenceView::QSequenceData &seqInfo)
{
    // Find the viewport.
    QStringQSequenceDataListMap::Iterator it;
    it = sequencesPerViewport.find(vpt);
    if(it == sequencesPerViewport.end())
    {
        sequencesPerViewport[vpt] = QSequenceDataList();
        it = sequencesPerViewport.find(vpt);
    }

    // Insert the sequence into the viewport's list at the right place.
    QSequenceDataList::Iterator pos = it.data().begin();
    for(int i = 0; i < index && i < it.data().size(); ++i)
        ++pos;
    it.data().insert(pos, seqInfo);

    // Increase the number of columns if needed.
    if(it.data().size()+1 > nColsExist)
    {
        ++nColsExist;
        setNumCols(nColsExist);
    }

    update();

    // Emit something.
}

// ****************************************************************************
// Method: QvisSequenceView::getViewportName
//
// Purpose: 
//   Get the name of a viewport at the specified row in the table.
//
// Arguments:
//   row  : The row for which we want the viewport name.
//   name : Return value for the viewport name.
//
// Returns:    True on success; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 12:02:32 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
QvisSequenceView::getViewportName(int row, QString &name) const
{
    QStringQSequenceDataListMap::ConstIterator it = sequencesPerViewport.begin();
    for(int i = 0; i < row; ++i)
        ++it;
    bool retval = it != sequencesPerViewport.end();
    if(retval)
        name = it.key();
    return retval;
}

// ****************************************************************************
// Method: QvisSequenceView::getSequenceInformation
//
// Purpose: 
//   Get the sequence information at the specified index in the viewport.
//
// Arguments:
//   vpName : The viewport name.
//   index  : The index in the viewport.
//   info   : Return value for the sequence information.
//
// Returns:    True on success; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 12:03:45 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
QvisSequenceView::getSequenceInformation(const QString &vpName, int index,
     QSequenceData &info) const
{
    QStringQSequenceDataListMap::ConstIterator it = sequencesPerViewport.find(vpName);
    bool retval = it != sequencesPerViewport.end();
    if(retval)
    {
        retval = false;
        for(int i = 0; i < it.data().size(); ++i)
        {
            if(i == index)
            {
                retval = true;
                info = it.data()[i];
                break;
            }
        }
    }

    return retval;
}

// ****************************************************************************
// Method: QvisSequenceView::removeSequence
//
// Purpose: 
//   Remove the named sequence.
//
// Arguments:
//   name : The name of the sequence to remove.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 12:04:57 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisSequenceView::removeSequence(const QString &name)
{
    QStringQSequenceDataListMap::Iterator it = sequencesPerViewport.begin();
    for(; it != sequencesPerViewport.end(); ++it)
    {
        // Erase the item from the list.
        QSequenceDataList::Iterator dit = it.data().begin();
        for(; dit != it.data().end(); ++dit)
        {
            if((*dit).name == name)
            {
                it.data().erase(dit);
                return;
            }
        }
    }
}

// ****************************************************************************
// Method: QvisSequenceView::drawContents
//
// Purpose: 
//   Draws the contents for the widget's scroll area.
//
// Arguments:
//   p     : The painter to use.
//   clipx : The x clip location.
//   clipy : The y clip location.
//   clipw : The width of the clip rectangle.
//   cliph : The height of the clip rectangle.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 12:05:20 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisSequenceView::drawContents(QPainter *p, int clipx, int clipy, 
    int clipw, int cliph)
{
    QGridView::drawContents(p, clipx, clipy, clipw, cliph);

    drawDropSitesThatIntersect(p, QRect(clipx, clipy, clipw, cliph));
}

// ****************************************************************************
// Method: QvisSequenceView::paintCell
//
// Purpose: 
//   Paints a cell in the grid.
//
// Arguments:
//   p   : The painter to use.
//   row : The row of the cell.
//   col : The col of the cell.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 12:06:57 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisSequenceView::paintCell(QPainter *p, int row, int col)
{
    QRect r(cellRect());

    if(col == 0)
    {
        // Make it draw like a button.
        QBrush b(colorGroup().brush(QColorGroup::Button));
        qDrawWinButton(p, r.x(), r.y(), r.width(), r.height(), colorGroup(),
                       false, &b);
    }

    if(srcRow == row && srcCol == col)
    {
        // Draw the highlight rectangle.
        QBrush b(colorGroup().brush(QColorGroup::Highlight));
        p->fillRect(r, b);
    }

    drawCellContents(p, row, col);

    if(col != 0)
        p->drawLine(r.x(), r.y()+r.height()-1, r.x()+r.width(),r.y()+r.height()-1);
}

// ****************************************************************************
// Method: QvisSequenceView::drawCellContents
//
// Purpose: 
//   Draws the cell's contents.
//
// Arguments:
//   p   : The painter to use.
//   row : The row of the cell.
//   col : The col of the cell.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 12:07:39 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisSequenceView::drawCellContents(QPainter *p, int row, int col)
{
    drawCellContentsAt(p, cellRect(), row, col);
}

// ****************************************************************************
// Method: QvisSequenceView::drawCellContentsAt
//
// Purpose: 
//   Draws the contents of a cell.
//
// Arguments:
//   p   : The painter to use.
//   r   : The cell's rectangle.
//   row : The row of the cell.
//   col : The col of the cell.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 12:09:10 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisSequenceView::drawCellContentsAt(QPainter *p, const QRect &r, int row, int col)
{
    // Draw the text, etc.
    QString itemText;
    QPixmap pixmap;
    int ct = -1;
    if(!getCellInformation(row, col, itemText, pixmap, ct))
        return;

    QRect textRect(r);

    // Figure out the cell text. (taken from QIconView).
    QString tmpText = "...";
    int i = 0;
    while ( fontMetrics().width( tmpText + itemText[i] ) < textRect.width() &&
            i < itemText.length())
        tmpText += itemText[ i++ ];
    tmpText.remove( (uint)0, 3 );
    if(i < itemText.length())
        tmpText += "...";
    itemText = tmpText;

    if(!pixmap.isNull())
    {
        int px = r.x() + (r.width() - pixmap.width()) / 2;
        int py = r.y() + 1;
        p->drawPixmap(px, py, pixmap);

        int ty = py + pixmap.height() - 3;
        int th = fontMetrics().height();
        if(ty + th > r.y() + r.height())
            ty = r.y() + r.height() - th - 2;

        textRect = QRect(r.x(), ty, r.width(), th);
    }

    if(srcRow == row && srcCol == col)
    {
        // Draw the highlighted text.
        QPen oldPen(p->pen());
        QPen hpen(colorGroup().color(QColorGroup::HighlightedText));
        p->setPen(hpen);
        p->drawText(textRect,
                    Qt::AlignHCenter | Qt::AlignVCenter,
                    itemText);
        p->setPen(oldPen);
    }
    else
    {
        p->drawText(textRect,
                    Qt::AlignHCenter | Qt::AlignVCenter,
                    itemText);
    }
}

// ****************************************************************************
// Method: QvisSequenceView::drawDropSitesThatIntersect
//
// Purpose: 
//   Draws drop sites that intersect the clip rectangle.
//
// Arguments:
//   p    : The painter to use.
//   clip : The clip rectangle.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 12:10:02 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisSequenceView::drawDropSitesThatIntersect(QPainter *p, const QRect &clip)
{
    // Draw the inactive drop sites.
    if(dropSites.size() > 0)
    {
        QPen oldPen(p->pen());

        // Draw the inactive drop sites.
        QPen newPen(QColor(200,200,200));
        newPen.setWidth(2);
        p->setPen(newPen);
        for(int i = 0; i < dropSites.size(); ++i)
        {
            if(clip.intersects(dropSites[i].site) && i != dropSiteIndex)
            {
                int x = dropSites[i].site.x();
                int y = dropSites[i].site.y();
                int w = dropSites[i].site.width();
                int h = dropSites[i].site.height();
                p->moveTo(x,   y+3);
                p->lineTo(x,   y);
                p->lineTo(x+w, y);
                p->lineTo(x+w, y+3);

                y += h;
                p->moveTo(x,   y-3);
                p->lineTo(x,   y);
                p->lineTo(x+w, y);
                p->lineTo(x+w, y-3);
            }
        }

        // Draw the active drop site.
        newPen.setColor(colorGroup().color(QColorGroup::Highlight));
        p->setPen(newPen);
        if(dropSiteIndex != -1)
        {
            if(clip.intersects(dropSites[dropSiteIndex].site))
            {
                int x = dropSites[dropSiteIndex].site.x();
                int y = dropSites[dropSiteIndex].site.y();
                int w = dropSites[dropSiteIndex].site.width();
                int h = dropSites[dropSiteIndex].site.height();
                p->moveTo(x,   y+3);
                p->lineTo(x,   y);
                p->lineTo(x+w, y);
                p->lineTo(x+w, y+3);

                y += h;
                p->moveTo(x,   y-3);
                p->lineTo(x,   y);
                p->lineTo(x+w, y);
                p->lineTo(x+w, y-3);

                p->fillRect(x+2, y-h+2, w-4, h-4, colorGroup().brush(QColorGroup::Highlight));
            }
        }

        p->setPen(oldPen);
    }
}

// ****************************************************************************
// Method: QvisSequenceView::drawCellContentsOverlay
//
// Purpose: 
//   Draws the cell contents as an "overlay" while we move the mouse.
//
// Arguments:
//   p   : The painter to use.
//   r   : The cell's rectangle.
//   row : The row of the cell.
//   col : The col of the cell.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 12:10:57 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisSequenceView::drawCellContentsOverlay(QPainter *p, const QRect &r, int row, int col)
{
    for(int j = 0; j < numRows(); ++j)
        for(int i = 0; i < numCols(); ++i)
        {
            if(cellGeometry(j, i).intersects(r))
            {
                QWMatrix m;
                m.translate(i * cellWidth(), j * cellHeight());
                p->setWorldMatrix(m, false);

                // Some other way of clearing thecell would be good here.
                p->fillRect(0, 0, cellWidth(), cellHeight(), QBrush(QColor(255,255,255)));

                // Draw the cell contents.
                paintCell(p, j, i);
            }
        }

    //
    // Draw the drop sites that intersect the cells that were redrawn because
    // they were under the moving cell image.
    //
    p->setWorldMatrix(QWMatrix(), false);

    bool *dsDrawn = new bool[dropSites.size()+1];
    for(int j = 0; j < dropSites.size()+1; ++j)
        dsDrawn[j] = false;
    for(int j = 0; j < numRows(); ++j)
        for(int i = 0; i < numCols(); ++i)
        {
            for(int ds = 0; ds < dropSites.size(); ++ds)
            {
                QRect cg(cellGeometry(j, i));

                if(!dsDrawn[ds] &&
                   cg.intersects(dropSites[ds].site))
                {
                    drawDropSitesThatIntersect(p, cg);
                    dsDrawn[ds] = true;
                }
            }
        }
    delete [] dsDrawn;

    //
    // Draw the cell contents at the mouse location so we can move it around.
    //
    int tmpRow = srcRow;
    int tmpCol = srcCol;
    srcRow = srcCol = -1;
    drawCellContentsAt(p, r, row, col);
    srcRow = tmpRow;
    srcCol = tmpCol;
}

// ****************************************************************************
// Method: QvisSequenceView::getCellInformation
//
// Purpose: 
//   Get the cell information for cell row,col.
//
// Arguments:
//   row : The row of the cell that we want.
//   col : The col of the cell that we want.
//   txt : Return value for the cell text.
//   pix : Return value for the cell pixmap.
//   t   : Return value for the cell's sequence type.
//
// Returns:    True on success; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 12:12:10 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
QvisSequenceView::getCellInformation(int row, int col, QString &txt,
    QPixmap &pix, int &t) const
{
    QStringQSequenceDataListMap::ConstIterator it;
    int index = 0;
    for(it = sequencesPerViewport.begin(); it != sequencesPerViewport.end(); ++it, ++index)
    {
        t = -1;
        if(index == row)
        {
            for(int i = 0; i < it.data().size(); ++i)
            {
                if(i+1 == col)
                {
                    // It's a sequence.
                    txt = it.data()[i].name;
                    pix = it.data()[i].pixmap;
                    t = it.data()[i].seqType;
                    return true;
                }
            }

            pix = QPixmap();

            // It's a viewport
            if(col == 0)
            {
                txt = it.key();
                t = -1;
                return true;
            }

            txt = "";
            t = -1;
            return false;
        }
    }

    return false;
}

// ****************************************************************************
// Method: QvisSequenceView::contentsMousePressEvent
//
// Purpose: 
//   This method is called when the mouse is pressed in the widget.
//
// Arguments:
//   e : The mouse event to handle.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 12:13:41 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisSequenceView::contentsMousePressEvent(QMouseEvent *e)
{
    //qDebug("Mouse press (%d, %d)", e->pos().x(), e->pos().y());

    srcCol = columnAt(e->pos().x());
    if(srcCol > 0)
    {
        int t;
        QString txt;
        QPixmap pix;

        srcRow = rowAt(e->pos().y());

        // If we can't get cell information then it's an empty cell and we
        // should not allow clicking on it.
        if(!getCellInformation(srcRow, srcCol, txt, pix, t))
        {
            srcCol = srcRow = -1;
            return;
        }

        updateCell(srcRow, srcCol);

        // We have a highlighted cell so let's figure out the drop targets
        // and draw them.
        QStringQSequenceDataListMap::ConstIterator vpt = sequencesPerViewport.begin();
        dropSites.clear();
        dropSiteIndex = -1;

        const int yoff = 5;
        const int dropWidth = 8;
        int dropH = cellHeight() - 2 * yoff;
        for(int row = 0; row < nRowsExist; ++row, ++vpt)
        {
            DropData info;
            info.name = vpt.key();

            // Create drop sites for this viewport list.
            if(vpt.data().size() == 0)
            {
                int x0 = cellWidth();
                int y0 = row * cellHeight() + yoff;

                info.site = QRect(x0, y0, dropWidth, dropH);
                info.index = 0;
                dropSites.push_back(info);
            }
            else if(vpt.data().size() == 1)
            {
                if(row != srcRow)
                {
                    int x0 = cellWidth();
                    int y0 = row * cellHeight() + yoff;
                    info.site = QRect(x0, y0, dropWidth, dropH);
                    info.index = 0;
                    dropSites.push_back(info);

                    x0 += cellWidth() - dropWidth;
                    info.site = QRect(x0, y0, dropWidth*2, dropH);
                    info.index = 1;
                    dropSites.push_back(info);
                }
            }
            else
            {
                if((srcRow == row && srcCol > 1) ||
                   (srcRow != row))
                {
                    int x0 = cellWidth();
                    int y0 = row * cellHeight() + yoff;
                    info.site = QRect(x0, y0, dropWidth, dropH);
                    info.index = 0;
                    dropSites.push_back(info);
                }

                for(int i = 0; i < vpt.data().size(); ++i)
                {
                    if(srcRow == row && ((srcCol-1) == i || srcCol == i+2))
                    {
                        ; // nothing
                    }
                    else
                    {
                        int x0 = (i+2) * cellWidth() - dropWidth;
                        int y0 = row * cellHeight() + yoff;
                        info.site = QRect(x0, y0, dropWidth*2, dropH);
                        info.index = i+1;
                        dropSites.push_back(info);
                    }
                }
            }
        }

        // Update the widget so the drop sites are marked.
        updateContents();
    }
    else
        srcCol = -1;
}

// ****************************************************************************
// Method: QvisSequenceView::contentsMouseMoveEvent
//
// Purpose: 
//   This method is called when the mouse is moved in the widget.
//
// Arguments:
//   e : The mouse event to handle.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 12:14:15 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisSequenceView::contentsMouseMoveEvent(QMouseEvent *e)
{
    if(srcCol != -1 && srcRow != -1)
    {
        //qDebug("Mouse move (%d, %d)", e->pos().x(), e->pos().y());

        // Figure out the new drop site index.
        int newDropIndex = -1;
        for(int i = 0; i < dropSites.size(); ++i)
        {
            if(dropSites[i].site.contains(e->pos()))
            {
                newDropIndex = i;
                break;
            }
        }
        if(newDropIndex != dropSiteIndex)
        {
            dropSiteIndex = newDropIndex;
            updateContents();
        }

        QPoint p(e->pos());
        int x = p.x();
        int y = p.y();
        QRect r(x - cellWidth()/2, y - cellHeight()/2, cellWidth(), cellHeight());

        // If the user has gone outside of the cells then repaint the outlying area.
        QRect right(numCols() * cellWidth(), 0, visibleWidth(), visibleHeight());
        QRect bottom(0, numRows() * cellHeight(), visibleWidth(), visibleHeight());
        if(right.intersects(r))
            repaintContents(right, true);
        if(bottom.intersects(r))
            repaintContents(bottom, true);

        // Draw the moving cell.
        QPainter paint(viewport());
        drawCellContentsOverlay(&paint, r, srcRow, srcCol);
    }
}

// ****************************************************************************
// Method: QvisSequenceView::contentsMouseReleaseEvent
//
// Purpose: 
//   This method is called when the mouse is released in the widget.
//
// Arguments:
//   e : The mouse event.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 12:14:42 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisSequenceView::contentsMouseReleaseEvent(QMouseEvent *e)
{
    //qDebug("Mouse release (%d, %d)", e->pos().x(), e->pos().y());

    if(srcRow != -1 && srcCol != -1)
    {
        int r = srcRow;
        int c = srcCol;
        srcRow = srcCol = -1;
        updateCell(r,c);
    
        if(dropSiteIndex != -1)
        {
            // We're dropping in a valid drop site. We must move
            // the QSequenceData from srcRow,srcCol into the 
            // the location specified by the drop site.

            // Get the sequence data that we're moving.
            QString viewportName;
            if(getViewportName(r, viewportName))
            {
                //qDebug("MouseRelease: source viewport %s", viewportName.latin1());

                QSequenceData seqInfo;
                if(getSequenceInformation(viewportName, c-1, seqInfo))
                {
                    //qDebug("MouseRelease: source sequence %s,%d", 
                    //       seqInfo.name.latin1(), seqInfo.seqType);

                    //qDebug("MouseRelease: Want to insert at %s,%d", 
                    //       dropSites[dropSiteIndex].name.latin1(),
                    //       dropSites[dropSiteIndex].index);

                    //qDebug("Before");
                    //printContents();

                    // Determine the viewport that contains the sequence
                    // to be removed.
                    bool found = false;
                    QString fromViewport;
                    QStringQSequenceDataListMap::Iterator it;
                    for(it = sequencesPerViewport.begin();
                        it != sequencesPerViewport.end() && !found; ++it)
                    {
                        QSequenceDataList::Iterator dit;

                        // Try and find the name of the viewport that
                        // contains the sequence that we're moving.
                        if(!found)
                        {
                            for(dit = it.data().begin();
                                dit != it.data().end() && !found; ++dit)
                            {
                                if((*dit).name == seqInfo.name)
                                {
                                    fromViewport = it.key();
                                    found = true;
                                }
                            }
                        }
                    }

                    removeSequence(seqInfo.name);

                    // Now let's insert the data into the list for another viewport.
                    insertSequenceInViewport(dropSites[dropSiteIndex].name,
                                             dropSites[dropSiteIndex].index,
                                             seqInfo);

                    // Now that we've updated the viewport sequences, get
                    // a list of the sequences both for the viewport from
                    // which the sequence was removed and for the viewport
                    // to which the sequence was added.
                    QStringList fromViewportSequences;
                    QStringList toViewportSequences;
                    for(it = sequencesPerViewport.begin();
                        it != sequencesPerViewport.end(); ++it)
                    {
                        QSequenceDataList::Iterator dit;

                        if(it.key() == fromViewport)
                        {
                            for(dit = it.data().begin();
                                dit != it.data().end(); ++dit)
                            {
                                fromViewportSequences.push_back((*dit).name);
                            }
                        }

                        if(it.key() == dropSites[dropSiteIndex].name)
                        {
                            for(dit = it.data().begin();
                                dit != it.data().end(); ++dit)
                            {
                                toViewportSequences.push_back((*dit).name);
                            }
                        }
                    }

                    // Tell the client about the changes to the sequence lists.
                    if(fromViewport == dropSites[dropSiteIndex].name)
                    {
                        // Only one viewport list was involved.
                        emit updatedMapping(fromViewport,
                                            fromViewportSequences);
                    }
                    else
                    {
                        // Two viewport lists were involved.
                        emit updatedMapping(fromViewport,
                                            fromViewportSequences,
                                            dropSites[dropSiteIndex].name,
                                            toViewportSequences);
                    }

                    //qDebug("After");
                    //printContents();
                }
            }
        }

        dropSiteIndex = -1;
        dropSites.clear();
        updateContents();
    }
}

// ****************************************************************************
// Method: QvisSequenceView::printContents
//
// Purpose: 
//   Prints the widget contents for debugging.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 12:15:13 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisSequenceView::printContents() const
{
    QStringQSequenceDataListMap::ConstIterator it = sequencesPerViewport.begin();
    for(; it != sequencesPerViewport.end(); ++it)
    {
        qDebug("Viewport: %s", it.key().latin1());

        for(int i = 0; i < it.data().size(); ++i)
        {
            qDebug("\tsequence: %s, %d", it.data()[i].name.latin1(), it.data()[i].seqType);
        }
    }
}
