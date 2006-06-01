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

#include <QvisPlotListBoxItem.h>
#include <PlotPluginManager.h>
#include <PlotPluginInfo.h>
#include <OperatorPluginManager.h>
#include <OperatorPluginInfo.h>
#include <QualifiedFilename.h>

#include <qiconset.h>
#include <qpainter.h>
#include <qpixmapcache.h>

#include <icons/subset.xpm>

#define YICON_SPACING 3

//
// Statics
//
QPixmap *QvisPlotListBoxItem::subsetIcon = 0;

// ****************************************************************************
// Method: QvisPlotListBoxItem::QvisPlotListBoxItem
//
// Purpose: 
//   Constructor for QvisPlotListBoxItem.
//
// Arguments:
//   plot    : The Plot object that this element displays.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 11 11:39:33 PDT 2000
//
// Modifications:
//   Brad Whitlock, Tue Apr 8 12:11:08 PDT 2003
//   I made it store the plot information and I made it look up icons that
//   it will need in order to paint the item.
//
// ****************************************************************************

QvisPlotListBoxItem::QvisPlotListBoxItem(const QString &prefix_, const Plot &p)
    : QListBoxItem(), plot(p), prefix(prefix_), clickable()
{
    addClickableRects = true;

    // Tell the listbox that we'll use custom highlighting.
    setCustomHighlighting(true);

    // Set the string that we'll use when the item is not expanded.
    setText(GetDisplayString(plot, prefix));

    //
    // Set the plot's name and pixmap so that they are available when we
    // need to paint them.
    //
    QString key;
    PlotPluginManager *pMgr = PlotPluginManager::Instance();
    CommonPlotPluginInfo *plotInfo = pMgr->GetCommonPluginInfo(
        pMgr->GetEnabledID(plot.GetPlotType()));
    // Store the plot name
    plotName = QString(plotInfo->GetName());
    // Store the plot icon if it has one.
    GetPlotPixmap(plot.GetPlotType(), plotIcon);
    maxIconWidth  = plotIcon.width();
    maxIconHeight = plotIcon.height();

    //
    // Get the operator pixmaps so we can readily paint them.
    //
    if(plot.GetNumOperators() > 0)
    {
        // Allocate arrays of pixmaps and strings.
        operatorIcons = new QPixmap[plot.GetNumOperators()];
        operatorNames = new QString[plot.GetNumOperators()];

        // Get the pixmaps out of the pixmap cache.
        OperatorPluginManager *oMgr = OperatorPluginManager::Instance();
        for(int i = 0; i < plot.GetNumOperators(); ++i)
        {
            int operatorIndex = plot.GetOperator(i);
            CommonOperatorPluginInfo *info = oMgr->GetCommonPluginInfo(
                oMgr->GetEnabledID(operatorIndex));

            // Store the operator name
            operatorNames[i] = info->GetName();

            // Store the operator pixmap if it has one.
            GetOperatorPixmap(operatorIndex, operatorIcons[i]);

            // Find the maximum pixmap width and height.
            maxIconWidth  = QMAX(maxIconWidth, operatorIcons[i].width());
            maxIconHeight = QMAX(maxIconHeight, operatorIcons[i].height());
        }
    }
    else
    {
        operatorIcons = 0;
        operatorNames = 0;
    }

    //
    // If the class static subset pixmap has not been created, create it now.
    //
    if(subsetIcon == 0)
    {
        subsetIcon = new QPixmap(subset_xpm);
    }
}

// ****************************************************************************
// Method: QvisPlotListBoxItem::~QvisPlotListBoxItem
//
// Purpose: 
//   Destructor for QvisPlotListBoxItem.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 11 11:42:50 PDT 2000
//
// Modifications:
//   Brad Whitlock, Tue Apr 8 12:46:02 PDT 2003
//   Added code to delete the operatorIcons and operatorNames arrays.
//
// ****************************************************************************

QvisPlotListBoxItem::~QvisPlotListBoxItem()
{
    // Delete the operator pixmap array.
    delete [] operatorIcons;
    // Delete the operator name array.
    delete [] operatorNames;
}

// ****************************************************************************
// Method: QvisPlotListBoxItem::height
//
// Purpose: 
//   Returns the height of a line of text.
//
// Arguments:
//   lb : A pointer to the listbox that contains the item.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 11 11:44:54 PDT 2000
//
// Modifications:
//   Brad Whitlock, Wed May 7 19:09:18 PST 2003
//   I fixed it so it looks okay on Windows.
//
// ****************************************************************************

int
QvisPlotListBoxItem::height(const QListBox *lb) const
{
#if 0
    int h = lb ? lb->fontMetrics().lineSpacing() + 3 : 0;

    if(h < subsetIcon->width())
        h = subsetIcon->width() + 3;
    if(h < subsetIcon->height())
        h = subsetIcon->height() + 3;
#else
    int h = lb ? lb->fontMetrics().lineSpacing() : 0;
    if(h < subsetIcon->width())
        h = subsetIcon->width() + 2;
    if(h < subsetIcon->height())
        h = subsetIcon->height() + 2;
    h += 3;
#endif

    if(isExpanded())
    {
        int lineHeight = QMAX(maxIconHeight, lb->fontMetrics().lineSpacing());
        h += ((lineHeight + YICON_SPACING) * (plot.GetNumOperators() + 1) + YICON_SPACING);
    }

    return h;
}

// ****************************************************************************
// Method: QvisPlotListBoxItem::width
//
// Purpose: 
//   Returns the width of this line.
//
// Arguments:
//   lb : The listbox that contains the item.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 11 11:45:01 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

int
QvisPlotListBoxItem::width(const QListBox *lb) const
{
    return lb ? lb->fontMetrics().width(text()) + 6 : 0;
}

// ****************************************************************************
// Method: QvisPlotListBoxItem::paint
//
// Purpose: 
//   Draws the item in the listbox. It is drawn in different colors
//   depending on the state of the plot.
//
// Arguments:
//   painter : The QPainter object that we use for drawing.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 11 11:43:17 PDT 2000
//
// Modifications:
//   Brad Whitlock, Tue Apr 8 12:14:21 PDT 2003
//   I changed the method so it can draw plots expanded.
//
//   Brad Whitlock, Wed May 7 19:09:34 PST 2003
//   I fixed it so it looks okay on Windows.
//
//   Brad Whitlock, Fri Jun 20 10:42:58 PDT 2003
//   I made plots have "(hidden)" in their name when they are expanded and
//   hidden. I forgot to add that case when I added the code to draw
//   expanded plots.
//
//   Brad Whitlock, Thu Aug 21 17:45:49 PST 2003
//   I changed how the brush is created so it looks better on MacOS X.
//
// ****************************************************************************

void QvisPlotListBoxItem::paint(QPainter *painter)
{
    QPen backupPen(painter->pen());
    QFontMetrics fm = painter->fontMetrics();

    int x = 0;

    maxIconHeight = QMAX(maxIconHeight, fm.lineSpacing());

    // Draw the button background for the expand button and the subset button.
    int bw = fm.lineSpacing();
    if(bw < subsetIcon->width())
        bw = subsetIcon->width() + 2;
    if(bw < subsetIcon->height())
        bw = subsetIcon->height() + 2;
    int d = (bw + 2 - fm.ascent()) / 2;
    if(!selected())
    {
        painter->fillRect(0, 0, (bw << 1) + 3, bw + 1,
                          listBox()->colorGroup().brush(QColorGroup::Background));
    }
    else
    {
        // Draw the selection rectangle for the plot on the database line.
        painter->fillRect(0, 0, listBox()->width(), bw + 3, //fm.lineSpacing() + 3,
                          listBox()->colorGroup().brush(QColorGroup::Highlight));
    }

    // Create the points for the expanded button.
    QPointArray tri(3);
    if(isExpanded())
    {
        tri.setPoint(0, int(bw * 0.15f) + 1, int(bw * 0.4f) + 1);
        tri.setPoint(1, int(bw * 0.5f) + 1,  int(bw * 0.75f) + 1);
        tri.setPoint(2, int(bw * 0.85f) + 1, int(bw * 0.4f) + 1);
    }
    else
    {
        tri.setPoint(0, int(bw * 0.4f) + 1,  int(bw * 0.85) + 1);
        tri.setPoint(1, int(bw * 0.75f) + 1, int(bw * 0.5) + 1);
        tri.setPoint(2, int(bw * 0.4f) + 1,  int(bw * 0.15) + 1);
    }

    // Set the pen color for the arrow
    if(selected())
    {
        painter->setPen(listBox()->colorGroup().highlightedText());
        painter->setBrush(listBox()->colorGroup().brush(QColorGroup::HighlightedText));
    }
    else
    {
        painter->setPen(listBox()->colorGroup().text());
        painter->setBrush(listBox()->colorGroup().brush(QColorGroup::Text));
    }

    // Draw the expanded button
    painter->drawPolygon(tri, true);

    // Create some points of reference.
    int x0 = 0;
    int x1 = bw + 1;
    int x2 = bw + 2;
    int x3 = bw + 3;
    int x4 = (bw << 1) + 3;
    int x5 = (bw << 1) + 4;
    int y0 = 0;
    int y1 = bw + 1;
    int y2 = bw + 2;

    // Draw the subset button.
    painter->drawPixmap(x3 + 1, y0 + 1, *subsetIcon);

    // Draw the button outlines.
    if(!selected())
    {
        painter->setPen(listBox()->colorGroup().dark());
        painter->drawLine(x0, y1, x4, y1);
        painter->drawLine(x1, y1, x1, y0);
        painter->drawLine(x4, y1, x4, y0);
    }

    if(selected())
    {
        painter->setPen(listBox()->colorGroup().highlightedText());
        painter->drawLine(x2, y2, x2, y0);
        painter->drawLine(x5, y2, x5, y0);
    }
    else
    {
        painter->setPen(listBox()->colorGroup().shadow());
        painter->drawLine(x0, y2, x5, y2);
        painter->drawLine(x2, y2, x2, y0);
        painter->drawLine(x5, y2, x5, y0);
    }

    if(!selected())
    {
        painter->setPen(listBox()->colorGroup().light());
        painter->drawLine(x0, y0, x1, y0);
        painter->drawLine(x0, y0, x0, y1);
        painter->drawLine(x3, y0, x4, y0);
        painter->drawLine(x3, y0, x3, y1);
    }

    // Add a little space before we start drawing the text.
    x += x5 + 3;

    // Set the text color in the painter.
    setTextPen(painter, selected());

    //
    // If the item is expanded, we have lots of special drawing to do.
    //
    if(isExpanded())
    {
        // Figure out the database string to use.
        QString dbName;
        bool prefixIsNumeric = false;
        prefix.left(prefix.length() - 1).toInt(&prefixIsNumeric);
        if(prefixIsNumeric) 
        {
            // The prefix is a number so get the db name.
            QualifiedFilename name(plot.GetDatabaseName());
            dbName = prefix + QString(name.filename.c_str());
        }
        else
        {
            // The prefix is a database name.
            dbName = prefix;
        }
        int expandX = x + fm.width("9") / 2;
        int expandY = y2 + 2;

        // Draw the database name and the variable.
        dbName += QString(" - ");
        dbName += QString(plot.GetPlotVar().c_str());
        int textY = fm.ascent() + fm.leading()/2;
        painter->drawText(x, textY + d, dbName);

        //
        // Draw the operators.
        //
        int iconX = expandX + 10;
        int iconY = expandY + YICON_SPACING;
        int textX = iconX + maxIconWidth + 3;
        textY += fm.lineSpacing();
        int maxTextWidth = 0;
        int i;
        for(i = 0; i < plot.GetNumOperators(); ++i)
        {
            if(plot.GetActiveOperator() == i)
            {
                // Draw a selection rectangle for the operator
                painter->fillRect(iconX - YICON_SPACING,
                                  iconY - YICON_SPACING,
                                  listBox()->width(),
                                  maxIconHeight + 2 * YICON_SPACING,
                                  listBox()->colorGroup().highlight());
            }

            // Set the text color in the painter.
            setTextPen(painter, plot.GetActiveOperator() == i);

            // Draw the icon if there is one.
            if(!operatorIcons[i].isNull())
            {
                int icondX = (maxIconWidth  - operatorIcons[i].width()) / 2;
                int icondY = (maxIconHeight - operatorIcons[i].height()) / 2;
                int thisIconY = iconY + icondY;
                int thisIconX = iconX + icondX;
                painter->drawPixmap(thisIconX, thisIconY, operatorIcons[i]);
            }

            // Figure out where the text should go.
            QRect textSize(fm.boundingRect(operatorNames[i]));
            int textHeight = textSize.height();
            maxTextWidth = QMAX(maxTextWidth, textSize.width());
            int textdY = (maxIconHeight - textHeight) / 2;
            int thisTextY = iconY + textHeight + textdY;
            painter->drawText(textX, thisTextY, operatorNames[i]);

            iconY += (maxIconHeight + YICON_SPACING);
        }

        //
        // Draw the plot name.
        //
        if(!plotIcon.isNull())
        {
            int icondX = (maxIconWidth  - plotIcon.width()) / 2;
            int icondY = (maxIconHeight - plotIcon.height()) / 2;
            int thisIconY = iconY + icondY;
            int thisIconX = iconX + icondX;
            painter->drawPixmap(thisIconX, thisIconY, plotIcon);
        }
        QRect textSize(fm.boundingRect(plotName));
        int textHeight = textSize.height();
        maxTextWidth = QMAX(maxTextWidth, textSize.width());
        int textdY = (maxIconHeight - textHeight) / 2;
        int thisTextY = iconY + textHeight + textdY;
        setTextPen(painter, false);
        if(plot.GetHiddenFlag())
            painter->drawText(textX, thisTextY, plotName + " (hidden)");
        else
            painter->drawText(textX, thisTextY, plotName);
        // Make the text and icon clickable.
        QRect textRect(iconX, iconY,
                       listBox()->width(), maxIconHeight);
        AddClickableRectangle(plot.GetPlotType(), textRect, PlotAttributes);

        //
        // Draw the tree lines.
        //
        QPen dotPen;
        dotPen.setColor(listBox()->colorGroup().text());
        dotPen.setWidth(2);
        painter->setPen(dotPen);

        // Reset iconY to its initial value.
        iconY = expandY + YICON_SPACING;
        // Draw the vertical line down.
        int bottomY = iconY - 1 + int((float(plot.GetNumOperators()) + 0.5f) *
                      (maxIconHeight + YICON_SPACING));
        painter->drawLine(expandX, expandY, expandX, bottomY);
        // Draw the horizontal lines across.
        int lineY = iconY + maxIconHeight / 2;
        for(i = 0; i < plot.GetNumOperators(); ++i)
        {
            if(operatorIcons[i].isNull())
                painter->drawLine(expandX, lineY, textX - 3, lineY);
            else
                painter->drawLine(expandX, lineY, iconX - 3, lineY);
            lineY += (maxIconHeight + YICON_SPACING);
        }
        if(plotIcon.isNull())
            painter->drawLine(expandX, lineY, textX - 3, lineY);
        else
            painter->drawLine(expandX, lineY, iconX - 3, lineY);

        //
        // Draw the up and down boxes.
        //
        iconY = expandY + YICON_SPACING;
        int buttonX = textX + maxTextWidth + 10;
        int buttonY = iconY + maxIconHeight / 2;
        int buttonSize = textHeight * 7 / 5;
        for(i = 0; i < plot.GetNumOperators(); ++i)
        {
            if(plot.GetNumOperators() > 1)
            {
                if(i == 0)
                {
                    QRect r(buttonX, buttonY - buttonSize / 2, buttonSize, buttonSize);
                    AddClickableRectangle(i, r, Promote);
                    drawUpDownButton(painter, r, false);
                }
                else if(i == plot.GetNumOperators() - 1)
                {
                    QRect r(buttonX + buttonSize + 3, buttonY - buttonSize / 2,
                            buttonSize, buttonSize);
                    AddClickableRectangle(i, r, Demote);
                    drawUpDownButton(painter, r, true);
                }
                else
                {
                    QRect r(buttonX, buttonY - buttonSize / 2, buttonSize, buttonSize);
                    AddClickableRectangle(i, r, Promote);
                    drawUpDownButton(painter, r, false);
                    QRect r2(buttonX + buttonSize + 3, buttonY - buttonSize / 2,
                             buttonSize, buttonSize);
                    AddClickableRectangle(i, r2, Demote);
                    drawUpDownButton(painter, r2, true);
                }
            }

            QRect delRect(buttonX + 3 * (buttonSize + 3), buttonY - buttonSize / 2,
                          buttonSize, buttonSize);
            AddClickableRectangle(i, delRect, Delete);
            drawDeleteButton(painter, delRect);

            buttonY += (maxIconHeight + YICON_SPACING);
        }

        //
        // Now that the higher priority clickable rectangles have been added,
        // add some more for clicking the active operator.
        //
        iconY = expandY + YICON_SPACING;
        for(i = 0; i < plot.GetNumOperators(); ++i)
        {
            QRect operatorRect(iconX - YICON_SPACING,
                               iconY - YICON_SPACING,
                               listBox()->width(),
                               maxIconHeight + 2 * YICON_SPACING);
            AddClickableRectangle(i, operatorRect, ActiveOperator);
            iconY += (maxIconHeight + YICON_SPACING);
        }

        // prevent us from adding more clickable rectangles.
        addClickableRects = false;
    }
    else
        painter->drawText(x, fm.ascent() + d + fm.leading()/2, text());

    painter->setPen(backupPen);
}

// ****************************************************************************
// Method: QvisPlotListBoxItem::setTextPen
//
// Purpose: 
//   Sets the text pen according to the state of the plot and the highlightText
//   argument.
//
// Arguments:
//   painter       : The painter for which we're changing the pen color.
//   highlightText : Whether we should use the highlighted text color.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 12:57:12 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisPlotListBoxItem::setTextPen(QPainter *painter, bool highlightText) const
{
    if(plot.GetHiddenFlag())
        painter->setPen(Qt::gray);
    else if(plot.GetStateType() == Plot::NewlyCreated)
        painter->setPen(Qt::green);
    else if(plot.GetStateType() == Plot::Pending)
        painter->setPen(Qt::yellow);
    else if(plot.GetStateType() == Plot::Error)
        painter->setPen(Qt::red);
    else if(highlightText)
        painter->setPen(listBox()->colorGroup().highlightedText());
    else
        painter->setPen(listBox()->colorGroup().text());
}

// ****************************************************************************
// Method: QvisPlotListBoxItem::drawButtonSurface
//
// Purpose: 
//   Draws a button with no contents in the specified rectangle.
//
// Arguments:
//   painter : The painter to use to draw the button.
//   r       : The rectangle in which to draw the button.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 10 15:28:04 PST 2003
//
// Modifications:
//   Brad Whitlock, Fri Aug 22 09:19:17 PDT 2003
//   I changed how the brush is selected so it looks good on MacOS X.
//
// ****************************************************************************

void
QvisPlotListBoxItem::drawButtonSurface(QPainter *painter, const QRect &r) const
{
    // Draw the button background.
    painter->fillRect(r.x() + 1, r.y() + 1, r.width() - 3, r.height() - 3,
                      listBox()->colorGroup().brush(QColorGroup::Background));

    // Draw the highlights.
    int x0 = r.x();
    int x1 = r.x() + r.width() - 1;
    int y0 = r.y();
    int y1 = r.y() + r.height() - 1;
    painter->setPen(listBox()->colorGroup().shadow());
    painter->drawLine(x0, y1, x1, y1);
    painter->drawLine(x1, y1, x1, y0);
    painter->setPen(listBox()->colorGroup().dark());
    painter->drawLine(x0 + 1, y1 - 1, x1 - 1, y1 - 1);
    painter->drawLine(x1 - 1, y1 - 2, x1 - 1, y0 + 1);
    painter->setPen(listBox()->colorGroup().light());
    painter->drawLine(x0, y0, x1 - 1, y0);
    painter->drawLine(x0, y0, x0, y1 - 1);
}

// ****************************************************************************
// Method: QvisPlotListBoxItem::drawUpDownButton
//
// Purpose: 
//   Draws an up/down button in the specified rectangle.
//
// Arguments:
//   painter : The painter to use to draw the button.
//   r       : The rectangle in which to draw the button.
//   up      : Whether the arrow points up.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 10 15:28:56 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisPlotListBoxItem::drawUpDownButton(QPainter *painter, const QRect &r, 
    bool up) const
{
    // Draw the button surface.
    drawButtonSurface(painter, r);

    // Create the points for the arrow.
    int x = r.x() + 1;
    int y = r.y() + 1;
    int w = r.width() - 3;
    int h = r.height() - 3;
    QPointArray tri(3);
    if(up)
    {
        tri.setPoint(0, x+int(w * 0.15f), y+int(h * 0.65f));
        tri.setPoint(1, x+int(w * 0.5f),  y+int(h * 0.3f));
        tri.setPoint(2, x+int(w * 0.85f), y+int(h * 0.65f));
    }
    else
    {
        tri.setPoint(0, x+int(w * 0.15f), y+int(h * 0.4f));
        tri.setPoint(1, x+int(w * 0.5f),  y+int(h * 0.75f));
        tri.setPoint(2, x+int(w * 0.85f), y+int(h * 0.4f));
    }

    // Draw the arrow
    painter->setPen(listBox()->colorGroup().text());
    painter->setBrush(QBrush(listBox()->colorGroup().text()));
    painter->drawPolygon(tri, true);
}

// ****************************************************************************
// Method: QvisPlotListBoxItem::drawDeleteButton
//
// Purpose: 
//   Draws a delete button in the specified rectangle.
//
// Arguments:
//   painter : The painter in which to draw the button.
//   r       : The rectangle in which to draw the button.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 10 15:30:26 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisPlotListBoxItem::drawDeleteButton(QPainter *painter, const QRect &r) const
{
    // Draw the button surface.
    drawButtonSurface(painter, r);

    // Draw a red arrow
    QPen pen(Qt::red);
    pen.setWidth(2);
    painter->setPen(pen);
    painter->drawLine(r.x() + 3, r.y() + 3,
                      r.x() + r.width() - 4, r.y() + r.height() - 4);
    painter->drawLine(r.x() + 3, r.y() + r.height() - 4,
                      r.x() + r.width() - 4, r.y() + 3);
}

// ****************************************************************************
// Method: QvisPlotListBoxItem::clicked
//
// Purpose: 
//   This method checks a clicked position against any significant things in
//   the item. If anything significant is clicked, return a value other than
//   -1.
//
// Arguments:
//   pos           : The location of the click.
//   doubleClicked : Whether we're calling this routine in response to a 
//                   double mouse click.
//   id            : The int in which to return an identifier.
//
// Returns:    -1 if nothing was clicked or other values if something was
//             clicked.
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 8 16:32:59 PST 2003
//
// Modifications:
//   Brad Whitlock, Wed Jun 18 13:00:04 PST 2003
//   I made single clicking work for the Subset button.
//
// ****************************************************************************

int
QvisPlotListBoxItem::clicked(const QPoint &pos, bool doubleClicked, int &id)
{
    // Draw the button background for the expand button and the subset button.
    int bw = listBox()->fontMetrics().lineSpacing();
    if(bw < subsetIcon->width())
        bw = subsetIcon->width() + 2;
    if(bw < subsetIcon->height())
        bw = subsetIcon->height() + 2;

    QRect expandButton(1, 1, bw, bw);
    if(expandButton.contains(pos))
    {
        plot.SetExpandedFlag(!plot.GetExpandedFlag());
        return 0;
    }

    QRect subsetButton(bw + 4, 1, bw, bw);
    if(subsetButton.contains(pos))
    {
        return 1;
    }

    // Only consider the other options if the item is expanded.
    int retval = -1;
    if(isExpanded())
    {
        bool clickedSomething = false;
        for(int i = 0; i < clickable.size() && !clickedSomething; ++i)
        {
            if(clickable[i].r.contains(pos))
            {
                id = clickable[i].id;
                switch(clickable[i].type)
                {
                case PlotAttributes:
                    if(doubleClicked)
                        return 2;
                    clickedSomething = true;
                    break;
                case OperatorAttributes:
                    if(doubleClicked)
                        return 3;
                    clickedSomething = true;
                    break;
                case Promote:
                    return 4;
                case Demote:
                    return 5;
                case Delete:
                    return 6;
                case ActiveOperator:
                    plot.SetActiveOperator(id);
                    clickedSomething = true;
                    if(doubleClicked)
                    {
                        id = plot.GetOperator(id);
                        return 3;
                    }
                    else
                        return 0;
                default:
                    break;
                }
            }
        }

        if(pos.y() < bw && doubleClicked)
        {
            // Activate the plot attributes.
            id = plot.GetPlotType();
            return 2;
        }         
    }
    else if(doubleClicked)
    {
        // Activate the plot attributes.
        id = plot.GetPlotType();
        return 2;
    }

    return retval;
}

// ****************************************************************************
// Method: QvisPlotListBoxItem::AddClickableRectangle
//
// Purpose: 
//   Adds a rectangle to the list of clickable rectangles so that we can check
//   it when the user clicks on the item.
//
// Arguments:
//   id   : identifying information like plot or operator index.
//   r    : The rectangle of interest.
//   type : The type of clickable rectangle.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 10 15:32:08 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisPlotListBoxItem::AddClickableRectangle(int id, const QRect &r,
    ClickObjectType type)
{
    if(addClickableRects)
         clickable.push_back(ClickableRectangle(id, r, type));
}

// ****************************************************************************
// Method: QvisPlotListBoxItem::GetOperatorPixmap
//
// Purpose: 
//   Gets the specified operator pixmap or creates one if necessary.
//
// Arguments:
//   operatorType : The operator type for which we're getting the pixmap.
//   pm           : The return pixmap.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 10 15:33:37 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisPlotListBoxItem::GetOperatorPixmap(int operatorType, QPixmap &pm)
{
    OperatorPluginManager *oMgr = OperatorPluginManager::Instance();
    GUIOperatorPluginInfo *info = oMgr->GetGUIPluginInfo(
        oMgr->GetEnabledID(operatorType));

    QString key; key.sprintf("operator_icon_%s", info->GetName());
    if(!QPixmapCache::find(key, pm))
    {
        if(info->XPMIconData())
        {
            QIconSet icon(QPixmap(info->XPMIconData()));
            pm = icon.pixmap(QIconSet::Small, QIconSet::Normal);
            QPixmapCache::insert(key, pm);
        }
    }
}

// ****************************************************************************
// Method: QvisPlotListBoxItem::GetPlotPixmap
//
// Purpose: 
//   Gets the specified plot pixmap or creates one if necessary.
//
// Arguments:
//   plotType : The plot type for which we're getting the pixmap.
//   pm       : The return pixmap.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 10 15:33:37 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisPlotListBoxItem::GetPlotPixmap(int plotType, QPixmap &pm)
{
    PlotPluginManager *pMgr = PlotPluginManager::Instance();
    GUIPlotPluginInfo *info = pMgr->GetGUIPluginInfo(
        pMgr->GetEnabledID(plotType));

    QString key; key.sprintf("plot_icon_%s", info->GetName());
    if(!QPixmapCache::find(key, pm))
    {
        if(info->XPMIconData())
        {
            QIconSet icon(QPixmap(info->XPMIconData()));
            pm = icon.pixmap(QIconSet::Small, QIconSet::Normal);
            QPixmapCache::insert(key, pm);
        }
    }
}

// ****************************************************************************
// Method: QvisPlotListBoxItem::GetDisplayString
//
// Purpose: 
//   Adds the applied operators to the plot variable and returns the
//   resulting QString.
//
// Arguments:
//   plot : The Plot for which we're creating the plot variable.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 25 12:48:30 PDT 2000
//
// Modifications:
//    Brad Whitlock, Mon Mar 26 12:47:22 PDT 2001
//    Changed the code so it gets the plot name from the plugin manager.
//
//    Jeremy Meredith, Thu Jul 26 03:29:49 PDT 2001
//    Added support for operator plugins.
//
//    Jeremy Meredith, Thu Jul 26 09:53:57 PDT 2001
//    Renamed plugin info to include the word "plot".
//
//    Jeremy Meredith, Fri Sep 28 13:55:53 PDT 2001
//    Made plugin managers key off of ID instead of index.
//
// ****************************************************************************

QString
QvisPlotListBoxItem::GetDisplayString(const Plot &plot, const QString &prefix)
{
    // Get a pointer to the plot plugin manager.
    PlotPluginManager *plotPluginManager = PlotPluginManager::Instance();
    CommonPlotPluginInfo *plotInfo = NULL;
    plotInfo = plotPluginManager->GetCommonPluginInfo(
                  plotPluginManager->GetEnabledID(plot.GetPlotType()));
    QString plotTypeName(plotInfo->GetName());

    // Get a pointer to the operator plugin manager.
    OperatorPluginManager *operatorPluginManager = OperatorPluginManager::Instance();

    // Create the plot variable.
    int i;
    QString plotVar;
    for(i = plot.GetNumOperators(); i > 0; --i)
    {
        CommonOperatorPluginInfo *operatorInfo = 
            operatorPluginManager->GetCommonPluginInfo(
                  operatorPluginManager->GetEnabledID(plot.GetOperator(i-1)));
        plotVar += QString(operatorInfo->GetName());
        plotVar += QString("(");
    }
    plotVar += QString(plot.GetPlotVar().c_str());
    for(i = plot.GetNumOperators(); i > 0; --i)
        plotVar += QString(")");

    // Create the display string
    QString temp;
    temp.sprintf("%s%s - %s%s", prefix.latin1(),
        plotTypeName.latin1(), plotVar.latin1(),
        plot.GetHiddenFlag() ? " (hidden)" : "");

    return temp;
}

// ****************************************************************************
// Method: QvisPlotListBoxItem::isExpanded
//
// Purpose: 
//   Returns whether the item is expanded.
//
// Returns:    True if the item is expanded; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 17 09:28:36 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

bool
QvisPlotListBoxItem::isExpanded() const
{
    return plot.GetExpandedFlag();
}

// ****************************************************************************
// Method: QvisPlotListBoxItem::activeOperatorIndex
//
// Purpose: 
//   Returns the active operator index.
//
// Returns:    The active operator index.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 17 09:29:06 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

int
QvisPlotListBoxItem::activeOperatorIndex() const
{
    return plot.GetActiveOperator();
}

//
// ClickableRectangle class.
//

QvisPlotListBoxItem::ClickableRectangle::ClickableRectangle() : r()
{
    id = 0;
    type = QvisPlotListBoxItem::Expand;
}

QvisPlotListBoxItem::ClickableRectangle::ClickableRectangle(int i,
    const QRect &rect, QvisPlotListBoxItem::ClickObjectType t) : r(rect)
{
    id = i;
    type = t;
}

QvisPlotListBoxItem::ClickableRectangle::ClickableRectangle(
    const QvisPlotListBoxItem::ClickableRectangle &obj) : r(obj.r)
{
    type = obj.type;
    id = obj.id;
}

QvisPlotListBoxItem::ClickableRectangle::~ClickableRectangle()
{
}

void
QvisPlotListBoxItem::ClickableRectangle::operator = ( 
    const QvisPlotListBoxItem::ClickableRectangle &obj)
{
    r = obj.r;
    type = obj.type;
    id = obj.id;
}
