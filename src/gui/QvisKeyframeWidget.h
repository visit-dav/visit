// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_KEYFRAME_WIDGET_H
#define QVIS_KEYFRAME_WIDGET_H
#include <QWidget>
#include <QMap>

class QvisKeyframeDrawer;

// ****************************************************************************
// Class: QvisKeyframeWidget
//
// Purpose:
//   This widget is used in the keyframe window's tree view to edit the locations
//   of keyframes interactively. The widget is created and managed by the
//   keyframe tree view's delegate.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 14:22:08 PST 2008
//
// Modifications:
//   
// ****************************************************************************

class QvisKeyframeWidget : public QWidget
{
    Q_OBJECT
public:
    QvisKeyframeWidget(QWidget *parent);
    virtual ~QvisKeyframeWidget();

    void setDrawer(QvisKeyframeDrawer *);
    QvisKeyframeDrawer *drawer();

    enum { Point_Diamond, Point_Bar, Point_Time };
    void setPointStyle(int);

    void setData(const QMap<int,int> &);
    void getData(QMap<int,int> &) const;

signals:
    void commit(QWidget *);

protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);

private:
    QMap<int,int> idToIndex;
    int pointStyle;
    int movingPoint;
    QvisKeyframeDrawer *d;
};

#endif
