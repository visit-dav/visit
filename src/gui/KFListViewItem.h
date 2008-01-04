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

#ifndef KF_LIST_VIEW_ITEM_H
#define KF_LIST_VIEW_ITEM_H

#include <qlistview.h>
#include <vector>
class KFListView;

enum InterpolationType
{
    INTERP_CONSTANT,
    INTERP_LINEAR,
    INTERP_CUBIC
};

// ****************************************************************************
//  Class:  KFListViewItem
//
//  Purpose:
//    Keyframing listview item -- a row of the keyframe editor.
//
//  Programmer:  Jeremy Meredith
//  Creation:    May  8, 2002
//
//  Modifications:
//    Jeremy Meredith, Fri Jan 31 09:36:12 PST 2003
//    Added functionality for pre-selection highlighting, for deletion of
//    keyframe points, and added some other necessary infrastructure for
//    keyframing in general.
//
//    Jeremy Meredith, Tue Feb  4 17:50:39 PST 2003
//    Added code to keep track of if this is a view item.
//
// ****************************************************************************
class KFListViewItem : public QListViewItem
{
  public:
    enum Style 
    {
        Style_Atts,
        Style_Times,
        Style_Extents_and_Atts
    };

  public:
    KFListViewItem(QListView *l);
    KFListViewItem(QListViewItem *p);
    KFListViewItem(QListView *l,     QListViewItem *a);
    KFListViewItem(QListViewItem *p, QListViewItem *a);

    void SetPlotId(int i)  { plotid = i;   }
    int  GetPlotId()       { return plotid;}
    void SetIsView(bool b) { isview = b;   }
    bool IsView()          { return isview;}

    void SetStyle(Style s) { style = s;    }
    Style GetStyle()       { return style; }
    void SetBegin(double p);
    void SetEnd(double p);

    void Initialize();
    void AddPoint(double p);
    double GetOldPosSelectedPoint() { return positions[selectedpt]; }
    double GetNewPosSelectedPoint() { return newpos;                }
    void SetNewPosSelectedPoint(double p);
    void SetNewPosSelectedEnd(double p);
    void SetSelectedInterpolantsVal(int v);
    int  GetSelectedInterpolantsVal();
    double GetSelectedEndptBegin();
    double GetSelectedEndptEnd();
    KFListView *kfList();

    void DrawPoint(QPainter *p, int x, int y);
    void DrawExtentsBar(QPainter *p, int w, double p1, double p2,
                        const QColor &c1, const QColor &c2);

    virtual int width(const QFontMetrics &fm, const QListView *lv, int c) const;
    void Unselect();
    bool Select(double t, double tmin, double tmax, int y, bool hint, bool del);
    void ShowSelection(bool s);
    void AddToInterpolantSelection(double t);
    bool PointSelected();
    bool EndptSelected();
    bool InterpolantSelected();
  protected:
    virtual void paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int align);
  private:
    int plotid;
    bool isview;
    Style style;
    std::vector<double> positions;
    std::vector<int> interpolants;
    double beginpos;
    double endpos;

    bool selectionhidden;
    bool hintonly;
    bool deleting;

    int selectedend;
    int selectedpt;
    int selectedinterp1;
    int selectedinterp2;
    double newpos;
};


#endif
