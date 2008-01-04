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

#include "KFBase.h"
#include "KFListViewItem.h"
#include "KFListView.h"

#include <float.h>
#include <math.h>
#include <qpainter.h>
#include <qpointarray.h>

// ****************************************************************************
//  Constructors:  KFListViewItem::KFListViewItem
//
//  Purpose:
//    Constructors for the KF items.
//
//  Arguments:
//    l/p      The parent listview/listviewitem
//    a        An optional positional "after" item.
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2003
//
//  Modifications:
//    Jeremy Meredith, Fri Jan 31 09:31:49 PST 2003
//    Added plotid.
//
//    Jeremy Meredith, Tue Feb  4 17:50:58 PST 2003
//    Added isview.
//
// ****************************************************************************
KFListViewItem::KFListViewItem(QListView *l)     : QListViewItem(l)
{
    Initialize();
    style = Style_Atts;
    plotid = -1;
    isview = false;
}

KFListViewItem::KFListViewItem(QListViewItem *p) : QListViewItem(p)
{
    Initialize();
    style = Style_Atts;
    plotid = -1;
    isview = false;
}

KFListViewItem::KFListViewItem(QListView *l,     QListViewItem *a) : QListViewItem(l, a)
{
    Initialize();
    style = Style_Atts;
    plotid = -1;
    isview = false;
}

KFListViewItem::KFListViewItem(QListViewItem *p, QListViewItem *a) : QListViewItem(p, a)
{
    Initialize();
    style = Style_Atts;
    plotid = -1;
    isview = false;
}

// ****************************************************************************
//  Method:  KFListViewItem::Initialize
//
//  Purpose:
//    Reset the contents of the item.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2003
//
//  Modifications:
//    Jeremy Meredith, Fri Jan 31 09:32:00 PST 2003
//    Added "hintonly" for pre-selection highlighting.
//
// ****************************************************************************
void
KFListViewItem::Initialize()
{
    hintonly = true;
    selectedend = -1;
    selectedpt = -1;
    selectedinterp1 = -1;
    selectedinterp2 = -1;
    selectionhidden=false;
    positions.clear();
    interpolants.clear();
    beginpos = 0;
    endpos   = 1;
}

// ****************************************************************************
//  Method:  KFListViewItem::AddPoint
//
//  Purpose:
//    Add a point at the given position
//
//  Arguments:
//    p          the position
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2003
//
// ****************************************************************************
void
KFListViewItem::AddPoint(double p)
{
    positions.push_back(p);
    if (positions.size() >= 2)
        interpolants.push_back(INTERP_CONSTANT);
}

// ****************************************************************************
//  Method:  KFListViewItem::SetBegin
//
//  Purpose:
//    Sets the beginning point of the extents
//
//  Arguments:
//    p          the position
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2003
//
// ****************************************************************************
void
KFListViewItem::SetBegin(double p)
{
    beginpos = p;
}

// ****************************************************************************
//  Method:  KFListViewItem::SetEnd
//
//  Purpose:
//    Sets the beginning point of the extents
//
//  Arguments:
//    p          the position
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2003
//
// ****************************************************************************
void
KFListViewItem::SetEnd(double p)
{
    endpos = p;
}

// ****************************************************************************
//  Method:  KFListViewItem::SetNewPosSelectedPoint
//
//  Purpose:
//    Sets a new position for the selected point
//
//  Arguments:
//    p          the position
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 31, 2003
//
// ****************************************************************************
void
KFListViewItem::SetNewPosSelectedPoint(double p)
{
    newpos = p;
}

// ****************************************************************************
//  Method:  KFListViewItem::SetNewPosSelectedEnd
//
//  Purpose:
//    Sets a new position for the selected endpoint of the extents
//
//  Arguments:
//    p          the position
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 31, 2003
//
// ****************************************************************************
void
KFListViewItem::SetNewPosSelectedEnd(double p)
{
    newpos = p;
}

// ****************************************************************************
//  Method:  KFListViewItem::SetSelectedInterpolantsVal
//
//  Purpose:
//    Sets the style of the selected interpolants
//
//  Arguments:
//    v          the new style
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2003
//
// ****************************************************************************
void
KFListViewItem::SetSelectedInterpolantsVal(int v)
{
    for (int i  = QMIN(selectedinterp1,selectedinterp2);
             i <= QMAX(selectedinterp1,selectedinterp2);
         i++)
    {
        interpolants[i] = v;
    }
}

// ****************************************************************************
//  Method:  KFListViewItem::GetSelectedInterpolantsVal
//
//  Purpose:
//    Returns the (approximate) style of the selected interpolants
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2003
//
// ****************************************************************************
int
KFListViewItem::GetSelectedInterpolantsVal()
{
    // just pick one -- this is only for a hint, anyway
    return interpolants[selectedinterp1];
}

// ****************************************************************************
//  Method:  KFListViewItem::GetSelectedEndptBegin
//
//  Purpose:
//    Returns the beginning position of the selected extents.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 30, 2003
//
// ****************************************************************************
double
KFListViewItem::GetSelectedEndptBegin()
{
    return (selectedend==0) ? newpos : beginpos;
}

// ****************************************************************************
//  Method:  KFListViewItem::GetSelectedEndptEnd
//
//  Purpose:
//    Returns the end position of the selected extents.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 30, 2003
//
// ****************************************************************************
double
KFListViewItem::GetSelectedEndptEnd()
{
    return (selectedend==1) ? newpos : endpos;
}

// ****************************************************************************
//  Method:  KFListViewItem::PointSelected
//
//  Purpose:
//    Return true if any points are selected
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2003
//
// ****************************************************************************
bool
KFListViewItem::PointSelected()
{
    return selectedpt != -1;
}

// ****************************************************************************
//  Method:  KFListViewItem::EndptSelected
//
//  Purpose:
//    Returns true if either extents endpoint is selected
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2003
//
// ****************************************************************************
bool
KFListViewItem::EndptSelected()
{
    return selectedend != -1;
}

// ****************************************************************************
//  Method:  KFListViewItem::InterpolantSelected
//
//  Purpose:
//    Returns true if any interpolants are selected
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2003
//
// ****************************************************************************
bool
KFListViewItem::InterpolantSelected()
{
    return (selectedinterp1 != -1 && selectedinterp2 != -1);
}

// ****************************************************************************
//  Method:  KFListViewItem::kfList
//
//  Purpose:
//    Returns the enclosing listview as a KFListView object.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2003
//
// ****************************************************************************
KFListView *
KFListViewItem::kfList()
{
    return (KFListView*)listView();
}

// ****************************************************************************
//  Method:  KFListViewItem::width
//
//  Purpose:
//    Overrides base class width() determination.
//
//  Arguments:
//    fm         font metrics
//    lv         the enclosing listview
//    c          the column number
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2003
//
// ****************************************************************************
int
KFListViewItem::width(const QFontMetrics &fm, const QListView *lv, int c) const
{
    int col0width = QListViewItem::width(fm,lv,c);

    if (c == 0)
        return col0width;

    //return lv->width() - col0width - 2;
    return 0;
}

// ****************************************************************************
//  Method:  KFListViewItem::Unselect
//
//  Purpose:
//    Unselect all selected objects
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2003
//
//  Modifications:
//    Jeremy Meredith, Fri Jan 31 09:33:07 PST 2003
//    Unhide any selections on an unselect.
//
// ****************************************************************************
void
KFListViewItem::Unselect()
{
    selectionhidden = true;
    selectedend = -1;
    selectedpt = -1;
    selectedinterp1 = -1;
    selectedinterp2 = -1;
}

// ****************************************************************************
//  Method:  KFListViewItem::ShowSelection
//
//  Purpose:
//    Shows/hides the current selection.
//
//  Arguments:
//    s          true to show, false to hide
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2003
//
// ****************************************************************************
void
KFListViewItem::ShowSelection(bool s)
{
    selectionhidden = !s;
}

// ****************************************************************************
//  Method:  KFListViewItem::Select
//
//  Purpose:
//    Find the nearest object within the item and select it.  Return true
//    if anything was selected.
//
//  Arguments:
//    t          the nearest time value
//    tmin       the lowest possible time value to select
//    tmax       the highest possible time value to select
//    y          the y coordinate of the mouse within the item
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2003
//
//  Modifications:
//    Jeremy Meredith, Fri Jan 31 09:33:43 PST 2003
//    Disabled interpolant selction for now. Added code to prevent
//    selection of anything but control points for deletion actions.
//    Fixed a couple small logic errors.
//
// ****************************************************************************
bool
KFListViewItem::Select(double t, double tmin, double tmax, int y,
                       bool hint, bool del)
{
    int i;
    newpos = t;

    hintonly = hint;
    deleting = del;

    selectedpt      = -1;
    selectedinterp1 = -1;
    selectedinterp2 = -1;
    selectedend     = -1;

    int    minend   = -1;
    double minedist = DBL_MAX;
    if (style == Style_Extents_and_Atts && !deleting)
    {
        double distbeg = fabs(t - beginpos);
        double distend = fabs(t - endpos);
        if (tmin < beginpos && tmax > beginpos && distbeg < minedist)
        {
            minedist = distbeg;
            minend = 0;
        }
        if (tmin < endpos && tmax > endpos && distend < minedist)
        {
            minedist = distend;
            minend = 1;
        }
        if (isOpen())
        {
            selectedend = minend;
            selectionhidden=false;
            return (minend != -1);
        }
    }

    int    minpt    = -1;
    double minpdist = DBL_MAX;
    for (i=0; i<positions.size(); i++)
    {
        double p = positions[i];
        if (p > tmin && p < tmax)
        {
            double dist = fabs(p - t);
            if (dist < minpdist)
            {
                minpdist = dist;
                minpt    = i;
            }
        }
    }

    int    mininterp = -1;
    double minidist  = DBL_MAX;
#ifdef DISABLE_INTEPOLATION_MODIFICATION_FOR_NOW
    if (style != Style_Times && !deleting)
    {
        for (i=0; i<interpolants.size(); i++)
        {
            double p0 =  positions[i  ];
            double p1 = (positions[i  ]+.5/double(kfList()->GetNFrames()));
            double p2 = (positions[i+1]-.5/double(kfList()->GetNFrames()));
            double p3 =  positions[i+1];
            double dist = QMIN(fabs(t - p1),  fabs(p2 - t));
            if (t>p0 && t<p3 && dist<minidist)
            {
                minidist  = dist;
                mininterp = i;
            }
        }
    }
#endif

    if (minidist < minpdist && minidist < minedist)
    {
        selectedinterp1 = mininterp;
        selectedinterp2 = mininterp;
        selectionhidden=false;
        return (mininterp!=-1);
    }
    else
    {
        if (minend != -1 && 
            (minpt==-1 || y<(height()/4) || y>(height()*3/4)))
        {
            selectedend = minend;
            selectionhidden=false;
            return true;
        }
        else if (minpt != -1)
        {
            selectedpt  = minpt;
            selectionhidden=false;
            return true;
        }
        else
        {
            return false;
        }
    }
}

// ****************************************************************************
//  Method:  KFListViewItem::AddToInterpolantSelection
//
//  Purpose:
//    Adds more interpolants to the selected interpolants list
//
//  Arguments:
//    t          the time value to find interpolants at
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2003
//
// ****************************************************************************
void
KFListViewItem::AddToInterpolantSelection(double t)
{
    int    mininterp = -1;
    double minidist  = DBL_MAX;
    for (int i=0; i<interpolants.size(); i++)
    {
        double p = (positions[i]+positions[i+1])/2.;
        double dist = fabs(p - t);
        if (dist < minidist)
        {
            minidist  = dist;
            mininterp = i;
        }
    }

    if (mininterp >= 0)
    {
        selectedinterp2 = mininterp;
    }
}

// ****************************************************************************
//  Method:  KFListViewItem::DrawPoint
//
//  Purpose:
//    Draws a point as either an attributes or a timestep glyph.
//
//  Arguments:
//    p          a QPainter to use
//    x,y        the local coordinates of the point center
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2003
//
// ****************************************************************************
void
KFListViewItem::DrawPoint(QPainter *p, int x, int y)
{
    QPointArray pa(4);
    if (style == Style_Atts || style == Style_Extents_and_Atts)
    {
        pa.setPoint(0, x,y-5);
        pa.setPoint(1, x-5,y);
        pa.setPoint(2, x,y+5);
        pa.setPoint(3, x+5,y);
    }
    else
    {
        pa.setPoint(0, x-2,y-5);
        pa.setPoint(1, x-2,y+5);
        pa.setPoint(2, x+2,y+5);
        pa.setPoint(3, x+2,y-5);
    }
    p->drawPolygon(pa);
}

// ****************************************************************************
//  Method:  KFListViewItem::DrawExtentsBar
//
//  Purpose:
//    Draws the extents bar with a given color and endpoints
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 30, 2003
//
// ****************************************************************************
void
KFListViewItem::DrawExtentsBar(QPainter *p, int w, double p1, double p2,
                               const QColor &c1, const QColor &c2)
{
    int h=height();
    int x1=int(kfList()->kfstart() + kfList()->kfwidth()*p1);
    int x2=int(kfList()->kfstart() + kfList()->kfwidth()*p2);
    int y=h/2;

    // draw the big bar
    p->fillRect(x1,y-6,x2-x1,13, c1);

    p->fillRect(x1-2,y-8,5,17, c1);
    p->fillRect(x2-2,y-8,5,17, c1);

    // draw current time marker
    int x=kfList()->i2x(kfList()->GetCurrentIndex());
    if (x >= x1 && x <= x2)
    {
        QPen pen(QColor(c1.red()/2,c1.green()/2,(c1.blue()+255)/2));
        pen.setWidth(3);
        p->setPen(pen);
        p->drawLine(x,y-6,x,y+7);
    }

    // draw the tick marks
    p->setPen(c2);
    int i;
    for (i=0; i<kfList()->GetNFrames(); i++)
    {
        int x=int((double(i+0.5)*double(w))/double(kfList()->GetNFrames()));
        if (x >= x1 && x <= x2)
        {
            p->drawLine(x,y-6,x,y+6);
        }
    }
}

// ****************************************************************************
//  Method:  KFListViewItem::paintCell
//
//  Purpose:
//    Overrides base class.  Used to paint the keyframe objects.
//
//  Arguments:
//    p          the painter to use
//    cg         the colorgroup to use
//    column     the column to paint
//    w          the width of the column
//    align      another argument that I don't know what it does.  It's not
//               documented for Qt 2.3.0, so I really can't find out either.
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2003
//
//  Modifications:
//    Jeremy Meredith, Fri Jan 31 09:35:20 PST 2003
//    Added code to draw differently for pre-selection highlighting and for
//    deleting actions.  Rearranged some stuff.
//
// ****************************************************************************
void
KFListViewItem::paintCell(QPainter *p, const QColorGroup &cg,
                          int column, int w, int align)
{
    if (column == 0)
    {
        QListViewItem::paintCell(p,cg,column,w,align);
        return;
    }

    int h = height();

    //cerr << "painting item in column " << column  << "  width=" << w << " height="<<height() << endl;

    p->setClipping(false);
    p->fillRect(0,0,w,h,gray2);

    // draw current time marker
    QPen pen(QColor(0,0,255));
    pen.setWidth(3);
    p->setPen(pen);
    int x=kfList()->i2x(kfList()->GetCurrentIndex());
    p->drawLine(x,0,x,h);

    // draw horizontal line
    p->setPen(gray3);
    p->drawLine(int(kfList()->kfstart()),h/2,int(kfList()->kfstart()+kfList()->kfwidth()),h/2);
    // draw time ticks
    p->setPen(gray1);
    int i;
    for (i=0; i<kfList()->GetNFrames(); i++)
    {
        int x=int((double(i+0.5)*double(w))/double(kfList()->GetNFrames()));
        p->drawLine(x,0,x,h);
    }

    // draw extents bars
    if (style == Style_Extents_and_Atts)
    {
        if (selectedend == -1 || selectionhidden)
            DrawExtentsBar(p,w, beginpos, endpos,
                           QColor(50,180,80), QColor(100,200,100));
        else if (hintonly)
            DrawExtentsBar(p,w, beginpos, endpos,
                           QColor(100,200,100), QColor(150,255,150));
        else
        {
            DrawExtentsBar(p,w, beginpos, endpos,
                           QColor(145,190,145), QColor(200,255,200));
            if (selectedend == 0)
                DrawExtentsBar(p,w, newpos, endpos,
                               QColor(150,255,150), QColor(255,255,255));
            if (selectedend == 1)
                DrawExtentsBar(p,w, beginpos, newpos,
                               QColor(150,255,150), QColor(255,255,255));
        }
    }

    if (style != Style_Extents_and_Atts || !isOpen())
    {
        // draw interpolants
        for (i=0; i<interpolants.size(); i++)
        {
            int x1=int(kfList()->kfstart()+kfList()->kfwidth()*positions[i]);
            int x2=int(kfList()->kfstart()+kfList()->kfwidth()*positions[i+1]);
            int y=h/2;
            if (i >= QMIN(selectedinterp1,selectedinterp2) &&
                i <= QMAX(selectedinterp1,selectedinterp2) &&
                !selectionhidden)
            {
                if (hintonly)
                    p->fillRect(x1,y-2,x2-x1,5, QColor(215,215,215));
                else
                    p->fillRect(x1,y-2,x2-x1,5, QColor(255,255,255));
            }
            else if (interpolants[i] != INTERP_CONSTANT)
            {
                QColor color;
                if (interpolants[i] == INTERP_LINEAR)
                    color = QColor(128,128,128);
                else if (interpolants[i] == INTERP_CUBIC)
                    color = QColor(0,255,0);

                p->fillRect(x1,y-2,x2-x1,5,color);
            }
        }

        // draw position markers
        p->setPen(black);
        p->setBrush(black);
        for (i=0; i<positions.size(); i++)
        {
            int x=int(kfList()->kfstart()+kfList()->kfwidth()*positions[i]);
            int y=h/2;
            if (selectedpt != i || selectionhidden)
            {
                DrawPoint(p,x,y);
            }
            else
            {
                if (hintonly || deleting)
                {
                    p->setBrush(QColor(90,90,110));
                    DrawPoint(p,x,y);
                    p->setBrush(black);
                }
                else
                {
                    p->setBrush(gray2);
                    DrawPoint(p,x,y);
                    p->setBrush(black);
                }
            }
        }
        if (selectedpt >= 0 && !hintonly && !selectionhidden && !deleting)
        {
            int x=int(kfList()->kfstart()+kfList()->kfwidth()*newpos);
            int y=h/2;
            p->setBrush(white);
            DrawPoint(p,x,y);
        }
        if (selectedpt >= 0 && deleting)
        {
            QPen xpen(QColor(255,0,0));
            xpen.setWidth(3);
            p->setPen(xpen);
            int x=int(kfList()->kfstart()+kfList()->kfwidth()*positions[selectedpt]);
            int y=h/2;
            p->drawLine(x-6,y-6,x+7,y+7);
            p->drawLine(x-6,y+7,x+7,y-6);
        }
    }

    p->setClipping(true);
}
