// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_GAUSSIAN_OPACITY_BAR_H
#define QVIS_GAUSSIAN_OPACITY_BAR_H
#include <gui_exports.h>

#include <QvisAbstractOpacityBar.h>

class QPixmap;

// ****************************************************************************
//  Class:  QvisGaussianOpacityBar
//
//  Purpose:
//    Gaussian-max implementation of QvisAbstractOpacityBar
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 30, 2001
//
//  Modifications:
//    Jeremy Meredith, Mon Feb  7 10:37:27 PST 2005
//    Removed mouseReleased because it was already in the base class.
//
//    Brad Whitlock, Wed Jun  4 10:32:52 PDT 2008
//    Qt 4.
//
//    Brad Whitlock, Thu Dec 18 14:07:00 PST 2008
//    I changed the drawOpacities method.
//
// ****************************************************************************

class GUI_API QvisGaussianOpacityBar : public QvisAbstractOpacityBar
{
    Q_OBJECT
  public:
                  QvisGaussianOpacityBar(QWidget *parent=NULL);
                 ~QvisGaussianOpacityBar();
    float        *getRawOpacities(int);
    int           getNumberOfGaussians();
    void          getGaussian(int, float*,float*,float*,float*,float*);
    void          setAllGaussians(int, float*);
    void          removeAllGaussians();
    
  protected:
    virtual void  mouseMoveEvent(QMouseEvent*);
    virtual void  mousePressEvent(QMouseEvent*);
    virtual void  mouseReleaseEvent(QMouseEvent*);
    virtual void  drawOpacities();
    void          drawControlPoints();

  private:
    enum Mode     {modeNone, modeX, modeH, modeW, modeWR, modeWL, modeB};
    // encapsulation of gaussian parameters
    class Gaussian
    {
      public:
        float x;
        float h;
        float w;
        float bx;
        float by;
      public:
        Gaussian(float x_,float h_,float w_,float bx_,float by_) : x(x_),h(h_),w(w_),bx(bx_),by(by_) {};
        Gaussian() {};
        ~Gaussian() {};
    };

    // the list of gaussians
    int         ngaussian;
    Gaussian    gaussian[200];

    // the current interaction mode and the current gaussian
    Mode        currentMode;
    int         currentGaussian;

    // GUI interaction variables
    bool        mousedown;
    int         lastx;
    int         lasty;

    // helper functions
    bool findGaussianControlPoint(int,int, int*,Mode*);
    void removeGaussian(int);
    void addGaussian(float,float,float,float,float);
};

#endif
