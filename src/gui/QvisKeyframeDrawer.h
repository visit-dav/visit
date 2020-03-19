// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
