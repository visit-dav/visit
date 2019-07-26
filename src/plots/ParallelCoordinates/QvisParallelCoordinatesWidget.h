// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_PARALLEL_COORDINATES_WIDGET_H
#define QVIS_PARALLEL_COORDINATES_WIDGET_H

#include <QWidget>

#include <vectortypes.h>

#define AXIS_LEFT_MARGIN        0.125
#define AXIS_RIGHT_MARGIN       0.125
#define AXIS_BOTTOM_MARGIN      0.30
#define AXIS_TOP_MARGIN         0.05

#define TICKS_PER_AXIS          9
#define AXIS_AND_TICK_WIDTH     2       // pixels
#define TICK_HALF_LENGTH        3       // pixels

#define DASHES_PER_AXIS         15
#define DASH_GAP_FRACTION       0.4


// ****************************************************************************
// Class: QvisParallelCoordinatesWidget
//
// Purpose:
//   This widget displays a simple thumbnail rendering of a ParallelCoordinates plot.
//   This thumbnail is used in that plot's wizard to aid the user in selecting
//   the initial set of axes that will appear in the plot.
//
// Note: This is intended to emulate the style of the QvisScatterWidget used
//       in the Scatter plot, which came first.
//
// Programmer: Jeremy Meredith
// Creation:   January 31, 2008
//
// Notes: initial implementation taken from Mark Blair's ParallelAxis plot.
//
// Modifications:
//    Cyrus Harrison, Mon Jul 21 08:33:47 PDT 2008
//    Initial Qt4 Port. 
//
// ****************************************************************************

class QvisParallelCoordinatesWidget : public QWidget
{
    Q_OBJECT
public:
    QvisParallelCoordinatesWidget(QWidget *parent);

    virtual     ~QvisParallelCoordinatesWidget();
    virtual      QSize sizeHint() const;
    virtual      QSizePolicy sizePolicy() const;
    
    void         setNumberOfAxes(int axisCount_);
    void         setAxisTitles(const stringVector&);
    void         redrawAllAxes(bool rightAxisNamed);

public slots:
    virtual void show();
    virtual void hide();
    
protected:
    virtual void paintEvent(QPaintEvent *e);
    virtual void resizeEvent(QResizeEvent *e);

    void         redrawScene(QPainter *painter);
    void         deleteBackingPixmap();

    QPixmap      *pixmap;
    bool          pixmapDirty;
    int           animationProgress;        // Might be useful eventually to
    bool          animationCountPositive;   // axis currently being selected.
    
    int           axisCount;
    bool          namedRightAxis;
    stringVector  axisTitles;

private:
    void          drawAxes(QPainter *painter);
    void          drawAxisTitles(QPainter *painter);
    void          drawDataCurves(QPainter *painter);

    int           axisBottomY;
    int           axisTopY;
    
    intVector     axesXPos;
    intVector     ticksYPos;
    intVector     dashesTopYPos;
    intVector     dashesBotYPos;
};

#endif
