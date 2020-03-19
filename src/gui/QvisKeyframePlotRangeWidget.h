// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_KEYFRAME_PLOT_RANGE_WIDGET_H
#define QVIS_KEYFRAME_PLOT_RANGE_WIDGET_H
#include <QWidget>

class QvisKeyframeDrawer;

// ****************************************************************************
// Class: QvisKeyframePlotRangeWidget
//
// Purpose:
//   This widget is used in the keyframe window's treeview to edit the start
//   and end locations of plots.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 14:24:11 PST 2008
//
// Modifications:
//   
// ****************************************************************************

class QvisKeyframePlotRangeWidget : public QWidget
{
    Q_OBJECT
public:
    QvisKeyframePlotRangeWidget(QWidget *parent);
    virtual ~QvisKeyframePlotRangeWidget();

    void setDrawer(QvisKeyframeDrawer *);
    QvisKeyframeDrawer *drawer();

    void setNFrames(int n);
    void setCurrentIndex(int ci);

    void setData(int start, int end);
    void getData(int &start, int &end) const;

signals:
    void commit(QWidget *);

protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);

private:
    int endpoints[2];
    int movingPoint;
    QvisKeyframeDrawer *d;
};

#endif
