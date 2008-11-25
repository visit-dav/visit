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
#ifndef QVIS_KEYFRAME_DRAWER_H
#define QVIS_KEYFRAME_DRAWER_H
#include <QObject>
#include <QColor>
#include <QList>
#include <QRect>

class QPainter;

// ****************************************************************************
// Class: QvisKeyframeDrawer
//
// Purpose:
//   This class draws keyframe controls used in the keyframe delegate and widgets.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 31 16:11:26 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

class QvisKeyframeDrawer : public QObject
{
public:
    QvisKeyframeDrawer(QObject *parent = 0);
    ~QvisKeyframeDrawer();

    void setNumFrames(int n);
    int  getNumFrames() const;
    void setCurrentIndex(int ci);

    void drawPlotRange(QPainter *, const QRect &, int start, int end, 
                       bool highlight=false, int activePoint=-1);
    void drawPoints(QPainter *, const QRect &, const QList<int> &, bool diamond=true,
                    int activePoint=-1);
    void drawTimeHandle(QPainter *p, const QRect &, int ci, bool=false);
    void drawBackground(QPainter *p, const QRect &);

    int xToFrame(int x) const;
    QRect handleRect(int id, int padding=0) const;

    // Colors used in the keyframe drawer
    QColor kfHandle;
    QColor kfHandleH;
    QColor kfBG;
    QColor kfVLine;
    QColor kfHLine;
    QColor kfTimeLine;
    QColor kfPlotRange;
    QColor kfPlotRangeH;
    QColor kfPlotRangeLines;
private:
    int   getX(int i) const;
    void  drawVerticalLines(QPainter *p, const QRect &r, int x0, int xn, const QColor &c1) const;

    QRect rect;
    int   numFrames;
    int   currentIndex;
};

#endif
