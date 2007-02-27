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

#include "KFListView.h"
#include "KFListViewItem.h"

#include <ViewerProxy.h>

#include <qpopupmenu.h>
#include <qpainter.h>
#include <qcursor.h>
#include <math.h>

#define clamp01(v) QMAX(0,QMIN(1,v))

static const char *interp0_xpm[] = {
"32 16 3 1",
"       c None",
".      c #000000",
"+      c #FFFFFF",
"                                ",
"                        +++++   ",
"                        +++++.  ",
"               ++++++++++++++.  ",
"               ++++++++++++++.  ",
"                ........+++++.  ",
"                         .....  ",
"                                ",
"                                ",
"   +++++                        ",
"   +++++++++++++.               ",
"   +++++++++++++.               ",
"   +++++.........               ",
"   +++++.                       ",
"    .....                       ",
"                                "};

static const char *interp1_xpm[] = {
"32 16 3 1",
"       c None",
".      c #000000",
"+      c #FFFFFF",
"                                ",
"                        +++++   ",
"                        +++++.  ",
"                      +++++++.  ",
"                    +++++++++.  ",
"                  ++++..+++++.  ",
"                ++++...  .....  ",
"              ++++...           ",
"            ++++...             ",
"   +++++  ++++...               ",
"   +++++++++...                 ",
"   +++++++...                   ",
"   +++++...                     ",
"   +++++.                       ",
"    .....                       ",
"                                "};

static const char *interp3_xpm[] = {
"32 16 3 1",
"       c None",
".      c #000000",
"+      c #FFFFFF",
"                                ",
"                        +++++   ",
"                        +++++.  ",
"                        +++++.  ",
"                        +++++.  ",
"           +++          +++++.  ",
"         +++.+++       ++.....  ",
"        ++... .++     ++..      ",
"       ++..     +++ +++..       ",
"   +++++..       .+++...        ",
"   +++++.                       ",
"   +++++.                       ",
"   +++++.                       ",
"   +++++.                       ",
"    .....                       ",
"                                "};

// ****************************************************************************
//  Constructor:  KFListView::KFListView
//
//  Arguments:
//    p          the parent
//    n          the name
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2003
//
//  Modifications:
//    Jeremy Meredith, Fri Jan 31 09:37:50 PST 2003
//    Added mouse tracking.
//
//    Jeremy Meredith, Fri Jan 31 15:19:17 PST 2003
//    Prevent scrollbar from appearing.
//
// ****************************************************************************
KFListView::KFListView(QWidget *p, const char *n) : QListView(p,n), GUIBase()
{
    //clearWFlags(Qt::WNorthWestGravity | Qt::WRepaintNoErase);
    clearWFlags(Qt::WNorthWestGravity);
    // WHY DOESN'T THIS WORK? ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    selecteditem = NULL;
    currentIndex = 0;
    nframes=1;
    snap = true;
    interpMenu = new QPopupMenu(this, "interpMenu");
    interpMenu->insertItem(interp0_xpm, INTERP_CONSTANT,  0);
    interpMenu->insertItem(interp1_xpm, INTERP_LINEAR,    1);
    interpMenu->insertItem(interp3_xpm, INTERP_CUBIC,     2);
    // QT 3.0: WE WANT THIS: setResizeMode(QListView::LastColumn);

    mousedown = false;
    setMouseTracking(true);

    setHScrollBarMode(QScrollView::AlwaysOff);
}

// ****************************************************************************
//  Method:  KFListView::SetNFrames
//
//  Purpose:
//    Sets the new number of frames for the widget
//
//  Arguments:
//    n          the new number of frames
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2003
//
// ****************************************************************************
void
KFListView::SetNFrames(int n)
{
    nframes = n;
}

// ****************************************************************************
//  Method:  KFListView::GetNFrames
//
//  Purpose:
//    Returns the number of frames
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2003
//
// ****************************************************************************
int
KFListView::GetNFrames()
{
    return nframes;
}

// ****************************************************************************
//  Method:  KFListView::kfstep
//
//  Purpose:
//    Returns the distance in pixels between successive frames
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2003
//
// ****************************************************************************
double
KFListView::kfstep()
{
    return double(columnWidth(1))/double(GetNFrames());
}

// ****************************************************************************
//  Method:  KFListView::kfwidth
//
//  Purpose:
//    Returns the width of the area of column 1 between timesteps 0..N
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2003
//
// ****************************************************************************
double
KFListView::kfwidth()
{
    return columnWidth(1) - kfstep();
}

// ****************************************************************************
//  Method:  KFListView::kfstart
//
//  Purpose:
//    Returns the x coodinate offset for the first frame
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2003
//
// ****************************************************************************
double
KFListView::kfstart()
{
    return kfstep() / 2.;
}

// ****************************************************************************
//  Method:  KFListView::paintEmptyArea
//
//  Purpose:
//    Overrides base class.  Paints the empty area of the list view
//
//  Arguments:
//    p          the painter to use
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2003
//
//  Modifications:
//    Jeremy Meredith, Fri Jan 31 15:19:01 PST 2003
//    Fixed graphical glitch.
//
// ****************************************************************************
void
KFListView::paintEmptyArea(QPainter *p, const QRect &rect)
{
    // Set up clipping areas
    bool clip = p->hasClipping();
    const QRegion whole = clip ? p->clipRegion() : rect;
    QRegion right = whole.intersect(QRect(columnWidth(0),0,width()-columnWidth(0),height()));

    // Draw the left half
    QListView::paintEmptyArea(p,rect);

    // Fill the background
    p->setClipRegion(right);
    p->fillRect(rect,QColor(192,192,192));

    // Draw the current time
    if (currentIndex >= 0)
    {
        QPen pen(QColor(0,0,255));
        pen.setWidth(3);
        p->setPen(pen);
        int x=int((float(currentIndex+0.5)*float(columnWidth(1)))/float(GetNFrames()));
        x += columnWidth(0);
        if (x >= rect.left()  &&  x < rect.left()+rect.width())
        {
            p->drawLine(x,QMAX(0,rect.top()),
                        x,QMIN(height(),rect.top()+rect.height()));
        }
    }

    // Draw the vertical lines
    p->setPen(QColor(224,224,224));
    for (int i=0; i<GetNFrames(); i++)
    {
        int x=int((float(i+0.5)*float(columnWidth(1)))/float(GetNFrames()));
        x += columnWidth(0);
        if (x < rect.left()  ||  x >= rect.left()+rect.width())
            continue;
        p->drawLine(x,QMAX(0,rect.top()),
                    x,QMIN(height(),rect.top()+rect.height()));
    }

    // reset the clipper
    if (clip)
        p->setClipRegion(whole);
    else
        p->setClipping(false);
}

// ****************************************************************************
//  Method:  KFListView::x2time
//
//  Purpose:
//    Converts an x coordinate to a floating point time value
//
//  Arguments:
//    x          the x coordinate
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2003
//
// ****************************************************************************
double
KFListView::x2time(int x)
{
    x -= columnWidth(0)+1;
    x -= int(kfstart());
    return double(x) / kfwidth();
}

// ****************************************************************************
//  Method:  KFListView::x2i
//
//  Purpose:
//    Converts an x coordinate to a frame number
//
//  Arguments:
//    x          the x coordinate
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2003
//
// ****************************************************************************
double
KFListView::x2i(int x)
{
    return time2i(x2time(x));
}

// ****************************************************************************
//  Method:  KFListView::time2i
//
//  Purpose:
//    Converts a floating point time value to a frame number
//
//  Arguments:
//    t          the time
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2003
//
// ****************************************************************************
int
KFListView::time2i(double t)
{
    t *= (GetNFrames()-1);
    return int(t+.5);
}

// ****************************************************************************
//  Method:  KFListView::i2time
//
//  Purpose:
//    Converts a frame number to a floating point time value
//
//  Arguments:
//    i          the frame number
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2003
//
// ****************************************************************************
double
KFListView::i2time(int i)
{
    if (GetNFrames() == 1)
        return 0;
    else
        return (double(i)/double(GetNFrames()-1));
}

// ****************************************************************************
//  Method:  KFListView::i2x
//
//  Purpose:
//    Converts a time step to a x coordinate
//
//  Arguments:
//    i          the frame number
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2003
//
// ****************************************************************************
int
KFListView::i2x(int i)
{
    return int((double(i+0.5)*double(columnWidth(1)))/double(GetNFrames()));
}

// ****************************************************************************
//  Method:  KFListView::GetCurrentIndex
//
//  Purpose:
//    Returns the current frame number
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2003
//
// ****************************************************************************
int
KFListView::GetCurrentIndex()
{
    return currentIndex;
}

// ****************************************************************************
//  Method:  KFListView::SelectNewItem
//
//  Purpose:
//    Selects a new item for either left- or right-mouse clicks for for
//    mouse-tracking pre-selection highlighting.
//
//  Arguments:
//    x,y        the mouse position
//    hint       true for pre-selection hinting through highlighting
//    del        true for selection for deletion
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 30, 2003
//
// ****************************************************************************
void
KFListView::SelectNewItem(int x, int y, bool hint, bool del)
{
    selecteditem = NULL;
    if (x > columnWidth(0))
    {
        KFListViewItem *item = (KFListViewItem*)itemAt(QPoint(x,y));
        if (item)
        {
            double  t    = x2time(x);
            double  tmin = x2time(x-5);
            double  tmax = x2time(x+5);
            if (item->Select(t, tmin, tmax, y-item->itemPos(), hint, del))
            {
                selecteditem = item;
            }
        }
    }
}

// ****************************************************************************
//  Method:  KFListView::contentsMousePressEvent
//
//  Purpose:
//    Handles mouse button presses
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2003
//
//  Modifications:
//    Jeremy Meredith, Fri Jan 31 09:39:30 PST 2003
//    Refactored some.  Added support for right-mouse clicks as well.
//
// ****************************************************************************
void
KFListView::contentsMousePressEvent(QMouseEvent *e)
{
    selecteditem = NULL;
    int x = e->x(),  y = e->y();
    if (e->button() == Qt::LeftButton || e->button() == Qt::RightButton)
    {
        mousedown = true;
        SelectNewItem(x,y, false, e->button() == Qt::RightButton);
    }
    QListView::contentsMousePressEvent(e);
}

// ****************************************************************************
//  Method:  KFListView::contentsMouseMoveEvent
//
//  Purpose:
//    Handles mouse motion
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2003
//
//  Modifications:
//    Jeremy Meredith, Fri Jan 31 09:39:43 PST 2003
//    Added support for mouse tracking for pre-selection highlight hinting.
//    Added support for deletion using the right mouse button.
//
// ****************************************************************************
void
KFListView::contentsMouseMoveEvent(QMouseEvent *e)
{
    int x = e->x(),  y = e->y();
    if (mousedown)
    {
        // This is the click-drag logic:
        if (selecteditem && e->state() == Qt::LeftButton)
        {
            double t = clamp01(x2time(e->x()));
            KFListViewItem *item = (KFListViewItem*)itemAt(QPoint(e->x(),e->y()));
            if (selecteditem->PointSelected())
            {
                if (snap)
                    selecteditem->SetNewPosSelectedPoint(i2time(time2i(t)));
                else
                    selecteditem->SetNewPosSelectedPoint(t);
                selecteditem->ShowSelection(item == selecteditem);
            }
            else if (selecteditem->EndptSelected())
            {
                if (snap)
                    selecteditem->SetNewPosSelectedEnd(i2time(time2i(t)));
                else
                    selecteditem->SetNewPosSelectedEnd(t);
                selecteditem->ShowSelection(item == selecteditem);
            }
            else if (selecteditem->InterpolantSelected())
            { 
                selecteditem->AddToInterpolantSelection(t);
                selecteditem->ShowSelection(item == selecteditem);
            }
            selecteditem->repaint();
        }
        if (selecteditem && e->state() == Qt::RightButton)
        {
            KFListViewItem *oldItem = selecteditem;
            int oldpos = time2i(oldItem->GetOldPosSelectedPoint());
            SelectNewItem(x,y, false, true);
            int newpos = selecteditem ? time2i(selecteditem->GetOldPosSelectedPoint()) : -1;
            if (oldItem != selecteditem || oldpos != newpos)
            {
                oldItem->Unselect();
                oldItem->repaint();
                if (selecteditem)
                {
                    selecteditem->Unselect();
                }
                selecteditem = NULL;
            }
        }
    }
    else
    {
        // We're doing mouse tracking here:
        if (selecteditem)
        {
            selecteditem->Unselect();
            selecteditem->repaint();
        }

        SelectNewItem(x,y, true, false);
        if (selecteditem)
            selecteditem->repaint();
    }

    //QListView::contentsMouseMoveEvent(e);
}

// ****************************************************************************
//  Method:  KFListView::contentsMouseReleaseEvent
//
//  Purpose:
//    Handles mouse button releases
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2003
//
//  Modifications:
//    Jeremy Meredith, Fri Jan 31 09:40:27 PST 2003
//    Added support for mouse tracking for pre-selection highlight hinting.
//    Added support for deletion using the right mouse button.
//    Hooked up most of the logic.
//
//    Jeremy Meredith, Tue Feb  4 17:50:02 PST 2003
//    Hooked up logic for the view keyframing information.
//
// ****************************************************************************
void
KFListView::contentsMouseReleaseEvent(QMouseEvent *e)
{
    int x = e->x(),  y = e->y();
    mousedown = false;

    //
    // Handle the mouse release after dragging
    //
    if (selecteditem && e->state() == Qt::LeftButton)
    {
        KFListViewItem *item = (KFListViewItem*)itemAt(QPoint(e->x(),e->y()));
        if (selecteditem == item)
        {
            if (item->InterpolantSelected())
            {
                int oldval = item->GetSelectedInterpolantsVal();
                for (int i=0; i<interpMenu->count(); i++)
                {
                    interpMenu->setItemChecked(i, i==oldval);
                }
                int newval = interpMenu->exec(QCursor::pos()-QPoint(16,10), oldval);
                if (newval >= 0)
                    item->SetSelectedInterpolantsVal(newval);
            }
            else if (item->PointSelected())
            {
                int oldpos = time2i(item->GetOldPosSelectedPoint());
                int newpos = time2i(item->GetNewPosSelectedPoint());
                if (item->GetStyle() == KFListViewItem::Style_Times)
                {
                    if (item->GetPlotId() >= 0)
                    {
                        GetViewerMethods()->MovePlotDatabaseKeyframe(item->GetPlotId(),
                                                         oldpos, newpos);
                    }
                    else
                    {
                        KFListViewItem *p = (KFListViewItem*)item->nextSibling();
                        while (p)
                        {
                            GetViewerMethods()->MovePlotDatabaseKeyframe(p->GetPlotId(),
                                                             oldpos, newpos);
                            p = (KFListViewItem*)p->nextSibling();
                        }
                    }
                }
                else if (item->IsView())
                {
                    GetViewerMethods()->MoveViewKeyframe(oldpos, newpos);
                }
                else
                {
                    GetViewerMethods()->MovePlotKeyframe(item->GetPlotId(), oldpos, newpos);
                }
            }
            else if (item->EndptSelected())
            {
                int beg = time2i(item->GetSelectedEndptBegin());
                int end = time2i(item->GetSelectedEndptEnd());
                GetViewerMethods()->SetPlotFrameRange(item->GetPlotId(),
                                          QMIN(beg,end), QMAX(beg,end));
            }
        }
        selecteditem->Unselect();
        selecteditem->repaint();
        selecteditem = NULL;
    }

    //
    // Handle the mouse release after right-clicking
    //
    if (selecteditem && e->state() == Qt::RightButton)
    {
        KFListViewItem *item = selecteditem;
        int oldpos = time2i(item->GetOldPosSelectedPoint());
        SelectNewItem(x,y, false, true);
        int newpos = selecteditem ? time2i(selecteditem->GetOldPosSelectedPoint()) : -1;
        if (item == selecteditem && oldpos == newpos)
        {
            if (item->PointSelected())
            {
                int pos = time2i(item->GetOldPosSelectedPoint());
                if (item->GetStyle() == KFListViewItem::Style_Times)
                {
                    if (item->GetPlotId() >= 0)
                    {
                        GetViewerMethods()->DeletePlotDatabaseKeyframe(item->GetPlotId(),
                                                           pos);
                    }
                    else
                    {
                        KFListViewItem *p = (KFListViewItem*)item->nextSibling();
                        while (p)
                        {
                            GetViewerMethods()->DeletePlotDatabaseKeyframe(p->GetPlotId(),
                                                               pos);
                            p = (KFListViewItem*)p->nextSibling();
                        }
                    }
                }
                else if (item->IsView())
                {
                    GetViewerMethods()->DeleteViewKeyframe(pos);
                }
                else
                {
                    GetViewerMethods()->DeletePlotKeyframe(item->GetPlotId(), pos);
                }
            }
        }
        if (selecteditem)
        {
            selecteditem->Unselect();
            selecteditem->repaint();
            selecteditem = NULL;
        }
    }
    QListView::contentsMouseReleaseEvent(e);

    //
    // Do the pre-selection higlighting again
    //
    SelectNewItem(x,y, true, false);
    if (selecteditem)
        selecteditem->repaint();
}

// ****************************************************************************
//  Method:  KFListView::resizeEvent
//
//  Purpose:
//    Handles resize events.
//
//  Arguments:
//    e          the resize event
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2003
//
// ****************************************************************************
void
KFListView::resizeEvent(QResizeEvent *e)
{
    setColumnWidth(1, e->size().width() - columnWidth(0) - 4);

    QListView::resizeEvent(e);
}

// ****************************************************************************
//  Method:  KFListView::timeChanged
//
//  Purpose:
//    Callback for when time is changed.
//
//  Arguments:
//    t          the new frame
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2003
//
// ****************************************************************************
void
KFListView::timeChanged(int t)
{
    currentIndex = t;
    viewport()->repaint(false);
}

// ****************************************************************************
//  Method:  KFListView::snapToFrameToggled
//
//  Purpose:
//    Callback for when someone toggles "snap to frame"
//
//  Arguments:
//    s          true if snap-to-frame is enabled
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2003
//
// ****************************************************************************
void
KFListView::snapToFrameToggled(bool s)
{
    snap = s;
}

// ****************************************************************************
//  Method:  KFListView::GetSnap
//
//  Purpose:
//    Returns true if snap-to-frame is enabled
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2003
//
// ****************************************************************************
bool
KFListView::GetSnap()
{
    return snap;
}
