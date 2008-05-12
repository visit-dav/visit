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

#include <QvisSubsetListViewItem.h>
#include <QvisSubsetListView.h>
#include <qheader.h>
#include <qpainter.h>
#include <qpen.h>
#include <qpixmapcache.h>

#include <icons/subsetturndown.xpm>

static const int BoxSize = 16;

// ****************************************************************************
// Method: QvisSubsetListViewItem::QvisSubsetListViewItem
//
// Purpose: 
//   This is a constructor for the QvisSubsetListViewItem class.
//
// Arguments:
//   parent : A pointer to the item's parent listview. This will make the new
//            item a top-level item.
//   text   : The text to be displayed for the item.
//   s      : The item's checked state.
//   id_    : The set id to associate with the item.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 5 09:34:03 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

QvisSubsetListViewItem::QvisSubsetListViewItem(QListView *parent,
    const QString &text, CheckedState s, int id_) :
    QCheckListItem(parent, text, QCheckListItem::CheckBox)
{
    state = s;
    itemId = id_;
    checkable = true;
}

// ****************************************************************************
// Method: QvisSubsetListViewItem::QvisSubsetListViewItem
//
// Purpose: 
//   This is a constructor for the QvisSubsetListViewItem class.
//
// Arguments:
//   parent : A pointer to the item's parent listview item. This will make the
//            new item a low-level item.
//   text   : The text to be displayed for the item.
//   s      : The item's checked state.
//   id_    : The collection id to associate with the item.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 5 09:34:03 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

QvisSubsetListViewItem::QvisSubsetListViewItem(QListViewItem *parent,
    const QString &text, CheckedState s, int id_) :
    QCheckListItem(parent, text, QCheckListItem::CheckBox)
{
    state = s;
    itemId = id_;
    checkable = true;
}

// ****************************************************************************
// Method: QvisSubsetListViewItem::~QvisSubsetListViewItem
//
// Purpose: 
//   This is the destructor for the QvisSubsetListViewItem class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 5 09:36:22 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

QvisSubsetListViewItem::~QvisSubsetListViewItem()
{
    // nothing
}

// ****************************************************************************
// Method: QvisSubsetListViewItem::compare(QListViewItem* i, int col, bool)
//
// Purpose: 
//   Compares this list view item to i using the column col in ascending order.
//
// Returns:
//   < 0 if this item is less than i, 0 if they are equal and > 0 if this item
//   is greater than i.
//
// Programmer: Gunther H. Weber
// Creation:   Mon May 12 14:55:15 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

int
QvisSubsetListViewItem::compare (QListViewItem* i, int col, bool ascending) const
{
    return itemId - static_cast<QvisSubsetListViewItem*>(i)->itemId;
}

// ****************************************************************************
// Method: QvisSubsetListViewItem::id
//
// Purpose: 
//   Returns the set or collection id associated with the item.
//
// Returns:    The set or collection id associated with the item.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 5 09:37:45 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

int
QvisSubsetListViewItem::id() const
{
    return itemId;
}

// ****************************************************************************
// Method: QvisSubsetListViewItem::setId
//
// Purpose: 
//   Sets the item's set or collection id.
//
// Arguments:
//   id_ : The new id.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 5 09:38:19 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetListViewItem::setId(int id_)
{
    itemId = id_;
}

// ****************************************************************************
// Method: QvisSubsetListViewItem::setState
//
// Purpose: 
//   Sets the checked state of the item.
//
// Arguments:
//   s : The new checked state.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 5 09:38:51 PDT 2001
//
// Modifications:
//   Brad Whitlock, Thu Jul 5 17:47:44 PST 2001
//   Added a check so the item is only repainted if the new state is
//   different from the old state.
//
// ****************************************************************************

void
QvisSubsetListViewItem::setState(CheckedState s)
{
    if(state != s)
    {
        state = s;
        // Redraw the item.
        repaint();
    }
}

// ****************************************************************************
// Method: QvisSubsetListViewItem::getState
//
// Purpose: 
//   Returns the item's checked state.
//
// Returns:    The item's checked state.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 5 09:39:17 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

CheckedState
QvisSubsetListViewItem::getState() const
{
    return state;
}

// ****************************************************************************
// Method: QvisSubsetListViewItem::setCheckable
//
// Purpose: 
//   Sets whether or not the item can be checked.
//
// Arguments:
//   val : Whether or not the item can be checked.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 5 09:39:44 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetListViewItem::setCheckable(bool val)
{
    checkable = val;
}

// ****************************************************************************
// Method: QvisSubsetListViewItem::getCheckable
//
// Purpose: 
//   Returns whether or not the item can be checked.
//
// Returns:    Whether or not the item can be checked.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 5 09:40:29 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

bool
QvisSubsetListViewItem::getCheckable() const
{
    return checkable;
}

// ****************************************************************************
// Method: QvisSubsetListViewItem::activate
//
// Purpose: 
//   This method is called when the item is clicked. Depending on what we
//   actually do to the item, this could set the state and/or cause the parent
//   listview to emit a checked signal.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 5 09:40:55 PDT 2001
//
// Modifications:
//   Hank Childs, Wed Aug  8 18:23:33 PDT 2001
//   Add support for old Qt (2.2.1).
//
//   Brad Whitlock, Mon Feb 24 13:46:34 PST 2003
//   I added support for turndowns.
//
// ****************************************************************************

void
QvisSubsetListViewItem::activate()
{
    QvisSubsetListView *lv = (QvisSubsetListView *)listView();

#if QT_VERSION >= 230
    if(listView() && !listView()->isEnabled() || !isEnabled())
#else
    if(listView() && !listView()->isEnabled())
#endif
        return;

    // If it not in "checkbox mode" get out.
    if(!checkable)
        return;

    QPoint pos;
    if(activatedPos(pos))
    {
        bool hasChildren = childCount() > 0;
        bool checked = false;

        // ignore clicks outside the box
        if(pos.x() < 0 || pos.x() >= (hasChildren ? 2 : 1) * BoxSize)
            return;

        if(hasChildren)
        {
            if(pos.x() <= BoxSize)
                setOpen(!isOpen());
            else
                checked = true;
        }
        else
            checked = true;

        if(checked)
        {
            // Make the state transition.
            if(state == NotChecked)
                setState(CompletelyChecked);
            else if(state == CompletelyChecked)
                setState(NotChecked);
            else if(state == PartiallyChecked)
                setState(CompletelyChecked);

            // Tell the parent, if there is one, to emit a checked signal.
            if(lv)
            {
                lv->emitChecked(this);
            }
        }
    }
}

// ****************************************************************************
// Method: QvisSubsetListViewItem::paintCell
//
// Purpose: 
//   This method draws the listview item.
//
// Arguments:
//   p      : The painter used to draw the item.
//   cg     : The color group that we're using.
//   column : The column of the listview.
//   width  : The width of the listview item.
//   height : The height of the listview item.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 5 09:42:18 PDT 2001
//
// Modifications:
//   Brad Whitlock, Mon Feb 24 13:50:34 PST 2003
//   I added turndowns.
//
// ****************************************************************************

void
QvisSubsetListViewItem::paintCell(QPainter *p, const QColorGroup &cg,
    int column, int width, int align)
{
    // Return if we do not have a valid painter.
    if(!p)
        return;

    // Draw the entire item in the base color.
    p->fillRect(0, 0, width, height(), cg.brush(QColorGroup::Base));

    // The rest is text, or for subclasses to change.
    if(column != 0)
    {
        QListViewItem::paintCell(p, cg, column, width, align);
        return;
    }

    QListView *lv = listView();
    if(!lv)
        return;

    int r = lv->itemMargin();
    int initialX = r;
    int x = r;

    if(checkable)
    {
        if(childCount() > 0)
        {
            x = BoxSize;
            r += x;
        }

        int y = (height() - BoxSize) / 2;
        int marg = lv->itemMargin();

        // Draw the empty check box.
#if QT_VERSION >= 230
        if(isEnabled())
            p->setPen(QPen(cg.text(), 2));
        else
            p->setPen(QPen(listView()->palette().color(QPalette::Disabled, QColorGroup::Text), 2));
#else
        p->setPen(QPen(listView()->palette().color(QPalette::Disabled, QColorGroup::Text), 2));
#endif
        p->drawRect(x+marg, y+2, BoxSize-4, BoxSize-4);
 
        // Draw the check mark
        if(state == CompletelyChecked)
        {
            QPointArray a(7*2);
            int i, xx, yy;
            ++x; ++y;
            xx = x+1+marg;
            yy = y+5;
            for (i=0; i<3; i++) {
                a.setPoint(2*i,   xx, yy);
                a.setPoint(2*i+1, xx, yy+2);
                xx++; yy++;
            }
            yy -= 2;
            for (i=3; i<7; i++) {
                a.setPoint(2*i,   xx, yy);
                a.setPoint(2*i+1, xx, yy+2);
                xx++; yy--;
            }
            p->drawLineSegments(a);
        }
        else if(state == PartiallyChecked)
        {
            p->drawLine(x+marg-1, y+2+BoxSize-4-1, x+marg+BoxSize-4-1, y+2-1);
        }

        if(childCount() > 0)
        {
            // Look for pixmaps in the pixmap cache.
            QPixmap *subset_td1 = QPixmapCache::find("subset_td1");
            QPixmap *subset_td2 = QPixmapCache::find("subset_td2");

            // If one or more pixmaps were not found, create them and add them
            // to the pixmap cache.
            if(subset_td1 == 0 || subset_td2 == 0)
            {
                QPixmap subset_td(subsetturndown_xpm);
                QPixmapCache::insert("subset_td1", subset_td);
                subset_td1 = QPixmapCache::find("subset_td1");

                QWMatrix m;
                m.translate(-BoxSize/2, -BoxSize/2);
                m.rotate(-90);
                m.translate(BoxSize/2, BoxSize/2);
                QPixmap subset_td2t(subset_td.xForm(m));
                QPixmapCache::insert("subset_td2", subset_td2t);
                subset_td2 = QPixmapCache::find("subset_td2");
            }

            // Draw the pixmap
            if(isOpen())
                p->drawPixmap(initialX, 2, *subset_td1);
            else
                p->drawPixmap(initialX, 2, *subset_td2);
        }

        // Prepare to call the base class's paintCell method.
        r += BoxSize + 4;
        p->translate(r, 0);
        p->setPen(QPen(cg.text()));
    }

    // We're done drawing the checked part of the item. Call the base
    // class's paintCell method to draw the rest of the item.
    QListViewItem::paintCell(p, cg, column, width - r, align);
}

// ****************************************************************************
// Method: QvisSubsetListViewItem::paintFocus
//
// Purpose: 
//   This method paints the focus rectangle around the listview item.
//
// Arguments:
//   p  : The painter used to draw the item.
//   cg : The colorgroup.
//   r  : The focus rectangle.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 5 09:44:08 PDT 2001
//
// Modifications:
//   Brad Whitlock, Fri Jul 30 12:04:50 PDT 2004
//   I fixed the size of the focus rectangle for when there is a turndown.
//
// ****************************************************************************

void
QvisSubsetListViewItem::paintFocus(QPainter *p, const QColorGroup &cg,
    const QRect &r)
{
    bool intersect = true;
    QListView *lv = listView();

    if(lv && lv->header()->mapToActual(0) != 0)
    {
        int xdepth = lv->treeStepSize() * ( depth() + ( lv->rootIsDecorated() ? 1 : 0) ) + lv->itemMargin();
        int p = lv->header()->cellPos( lv->header()->mapToActual(0) );
        xdepth += p;
        intersect = r.intersects( QRect( p, r.y(), xdepth - p + 1, r.height() ) );
    }
    
    if(checkable && intersect)
    {
        int turnDownSizeD2 = (childCount() > 0) ? ((BoxSize + 4)/2) : 0;
        QRect rect( r.x() + BoxSize + 5 + turnDownSizeD2,
                    r.y(),
                    r.width() - BoxSize - 5 - turnDownSizeD2,
                    r.height() );
        QListViewItem::paintFocus(p, cg, rect);
    }
    else
    {
        QListViewItem::paintFocus(p, cg, r);
    }
}
