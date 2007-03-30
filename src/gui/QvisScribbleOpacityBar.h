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
// ****************************************************************************

class GUI_API QvisScribbleOpacityBar : public QvisAbstractOpacityBar
{
    Q_OBJECT
public:
    QvisScribbleOpacityBar(QWidget *parent=NULL, const char *name=NULL);
    ~QvisScribbleOpacityBar();
    float *getRawOpacities(int);
    void   setRawOpacities(int,float*);

signals:
    void opacitiesChanged();

public slots:
    void makeTotallyZero();
    void makeLinearRamp();
    void makeTotallyOne();
    void smoothCurve();

protected:
    void mouseMoveEvent(QMouseEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void setValue(float,float);
    void setValues(int,int,int,int);
    void paintToPixmap(int,int);

private:
    int nvalues;
    float *values;
    bool mousedown;
    int lastx;
    int lasty;
};

#endif
