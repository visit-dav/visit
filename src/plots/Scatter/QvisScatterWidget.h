// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_SCATTER_WIDGET_H
#define QVIS_SCATTER_WIDGET_H
#include <QWidget>
#include <mini3D.h>

class QTimer;

// ****************************************************************************
// Class: QvisScatterWidget
//
// Purpose:
//   This widget displays some simple 3D graphics that look like a scatter
//   plot. This widget is used in the wizard to illustrate some of the steps.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 13 14:45:08 PST 2004
//
// Modifications:
//   Brad Whitlock, Fri Aug  8 09:42:33 PDT 2008
//   Qt 4.
//
// ****************************************************************************

class QvisScatterWidget : public QWidget
{
    Q_OBJECT
public:
    QvisScatterWidget(QWidget *parent);
    virtual ~QvisScatterWidget();
    virtual QSize sizeHint() const;

    void setThreeD(bool threeD);
    void setHighlightAxis(bool highlight);
    void setColoredPoints(bool colored);

    void setAllowAnimation(bool allow);

public slots:
    virtual void show();
    virtual void hide();
protected slots:
    void handleTimer();
protected:
    virtual void paintEvent(QPaintEvent *e);
    virtual void resizeEvent(QResizeEvent *e);

    void redrawScene(QPainter *painter);
    void redrawScene2D(QPainter *painter);
    void redrawScene3D(QPainter *painter);
    void drawSpherePoints();
    void deleteBackingPixmap();

    void createSharedElements();
    void initializeArrow();
    void initializeSphere(m3d_complex_element &, int nx, int ny, float rad,
                          float r, float g, float b);

    QPixmap                   *pixmap;
    QTimer                    *timer;
    m3d_renderer               renderer;
    bool                       rendererCreated;
    bool                       pixmapDirty;
    int                        animationProgress;
    bool                       animationCountPositive;

    bool                       threeD;
    bool                       highlightedAxis;
    bool                       coloredPoints;
    bool                       allowAnimation;

    static bool                sharedElementsCreated;
    static m3d_complex_element sphere;
    static m3d_complex_element arrow;
};

#endif
