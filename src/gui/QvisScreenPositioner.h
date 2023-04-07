// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_SCREEN_POSITIONER_H
#define QVIS_SCREEN_POSITIONER_H
#include <gui_exports.h>
#include <QWidget>

// ****************************************************************************
// Class: QvisScreenPositioner
//
// Purpose:
//   This widget allows the user to position a crosshairs that specifies a
//   screen position for annotations or other objects.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 1 14:10:17 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Jun  3 16:09:23 PDT 2008
//   Qt 4.
//
// ****************************************************************************

class GUI_API QvisScreenPositioner : public QWidget
{
    Q_OBJECT
public:
    QvisScreenPositioner(QWidget *parent = 0, Qt::WindowFlags = Qt::Widget);
    virtual ~QvisScreenPositioner();
    virtual QSize sizeHint () const;
    virtual QSize minimumSize() const;

    void setScreenSize(int xs, int ys);
    int  screenWidth() const       { return xScreenSize; };
    int  screenHeight() const      { return yScreenSize; };
    void setScreenPosition(int xp, int yp);
    void setScreenPosition(double xp, double yp);
    int  screenX() const           { return xPosition; }
    int  screenY() const           { return yPosition; }
    int  pageIncrement() const     { return pageIncrement_; }
    void setPageIncrement(int val) { pageIncrement_ = val; }    
signals:
    void screenPositionChanged(int newX, int newY);
    void screenPositionChanged(double newX, double newY);
    void intermediateScreenPositionChanged(int newX, int newY);
    void intermediateScreenPositionChanged(double newX, double newY);
public slots:
    void popupShow();
protected:
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void keyReleaseEvent(QKeyEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void paintEvent(QPaintEvent *);

    void sendNewScreenPosition();
    void setTempPositionFromWidgetCoords(int wx, int wy);
    void drawBox(QPainter &paint, const QRect &r,
                 const QColor &light, const QColor &dark, int lw);
    void drawLines(QPainter &paint);

    static const int minXScreenSize;
    static const int minYScreenSize;

    int xPosition;
    int yPosition;
    int xTempPosition;
    int yTempPosition;
    int xScreenSize;
    int yScreenSize;    
    int pageIncrement_;
    bool dragging;
    bool paging;
    bool drawFrame;
};

#endif
