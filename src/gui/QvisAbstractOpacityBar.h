#ifndef QvisAbstractOpacityBar_H
#define QvisAbstractOpacityBar_H
#include <gui_exports.h>

#include <qframe.h>
class QPixmap;
class QImage;

// ****************************************************************************
//  Class:  QvisAbstractOpacityBar
//
//  Purpose:
//    Abstract base for an opacity map editor
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 30, 2001
//
// ****************************************************************************
class GUI_API QvisAbstractOpacityBar : public QFrame
{
    Q_OBJECT
  public:
                   QvisAbstractOpacityBar(QWidget *parent=NULL, const char *name=NULL);
    virtual       ~QvisAbstractOpacityBar();
    virtual float *getRawOpacities(int) = 0;

  protected:
    int            val2x(float);
    float          x2val(int);
    int            val2y(float);
    float          y2val(int);

    virtual void   paintEvent(QPaintEvent*);
    virtual void   resizeEvent(QResizeEvent*);
    virtual void   paintToPixmap(int,int) = 0;

    QPixmap       *pix;

  signals:
    void           mouseReleased();
};

#endif
