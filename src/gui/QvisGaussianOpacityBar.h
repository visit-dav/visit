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
// ****************************************************************************

class GUI_API QvisGaussianOpacityBar : public QvisAbstractOpacityBar
{
    Q_OBJECT
  public:
                  QvisGaussianOpacityBar(QWidget *parent=NULL, const char *name=NULL);
                 ~QvisGaussianOpacityBar();
    float        *getRawOpacities(int);
    int           getNumberOfGaussians();
    void          getGaussian(int, float*,float*,float*,float*,float*);
    void          setAllGaussians(int, float*);

  protected:
    void          mouseMoveEvent(QMouseEvent*);
    void          mousePressEvent(QMouseEvent*);
    void          mouseReleaseEvent(QMouseEvent*);
    void          paintToPixmap(int,int);
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
