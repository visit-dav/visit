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

#ifndef KF_LIST_VIEW_H
#define KF_LIST_VIEW_H

#include <GUIBase.h>
#include <qlistview.h>
class KFListViewItem;
class QPopupMenu;

// ****************************************************************************
//  Class:  KFListView
//
//  Purpose:
//    The main listview for the keyframe editor.  (the big grid)
//
//  Programmer:  Jeremy Meredith
//  Creation:    May  8, 2002
//
//  Modifications:
//    Jeremy Meredith, Fri Jan 31 09:37:17 PST 2003
//    Added some infrastructure necessary for keyframing.
//
// ****************************************************************************
class KFListView : public QListView, public GUIBase
{
    Q_OBJECT
  public:
    KFListView(QWidget *p, const char *n);
    double kfstep();
    double kfwidth();
    double kfstart();
    virtual void paintEmptyArea(QPainter *p, const QRect &rect);
    double x2time(int x);
    double x2i(int x);
    int time2i(double t);
    double i2time(int i);
    int i2x(int i);
    int GetCurrentIndex();
    bool GetSnap();
    int GetNFrames();
    void SetNFrames(int);
    void SelectNewItem(int x, int y, bool hint, bool del);
    virtual void contentsMousePressEvent(QMouseEvent *e);
    virtual void contentsMouseMoveEvent(QMouseEvent *e);
    virtual void contentsMouseReleaseEvent(QMouseEvent *e);
    virtual void resizeEvent(QResizeEvent *e);
  public slots:
    void timeChanged(int);
    void snapToFrameToggled(bool);
  private:
    int nframes;
    bool snap;
    KFListViewItem *selecteditem;
    int     currentIndex;
    QPopupMenu *interpMenu;
    bool mousedown;
};


#endif
