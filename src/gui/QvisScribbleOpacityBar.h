// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_SCRIBBLE_OPACITY_BAR_H
#define QVIS_SCRIBBLE_OPACITY_BAR_H
#include <gui_exports.h>
#include <QvisAbstractOpacityBar.h>

class QPixmap;

// ****************************************************************************
//  Class:  QvisScribbleOpacityBar
//
//  Purpose:
//    Freeform implementation of OpacityBar
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 30, 2001
//
//  Modifications:
//     Brad Whitlock, Fri Apr 6 12:22:25 PDT 2001
//     Added some new slots to set some default opacity ramps.
//
//     Gunther Weber, Fri Apr  6 16:20:55 PDT 2007
//     Add support for an inverse linear ramp.
//
//     Brad Whitlock, Wed Jun  4 09:19:29 PDT 2008
//     Qt 4.
//
//     Brad Whitlock, Thu Dec 18 14:10:55 PST 2008
//     I changed the drawOpacities method.
//
// ****************************************************************************

class GUI_API QvisScribbleOpacityBar : public QvisAbstractOpacityBar
{
    Q_OBJECT
public:
    QvisScribbleOpacityBar(QWidget *parent);
    ~QvisScribbleOpacityBar();
    float *getRawOpacities(int);
    void   setRawOpacities(int,float*);

signals:
    void opacitiesChanged();

public slots:
    void makeTotallyZero();
    void makeLinearRamp();
    void makeInverseLinearRamp();
    void makeTotallyOne();
    void smoothCurve();
    void makeTent();

protected:
    virtual void mouseMoveEvent(QMouseEvent*);
    virtual void mousePressEvent(QMouseEvent*);
    virtual void mouseReleaseEvent(QMouseEvent*);
    void setValue(float,float);
    void setValues(int,int,int,int);
    virtual void drawOpacities();

private:
    int nvalues;
    float *values;
    bool mousedown;
    int lastx;
    int lasty;
};

#endif
