#ifndef KF_TIME_SLIDER_H
#define KF_TIME_SLIDER_H

#include <qframe.h>
class KFListView;

// ****************************************************************************
//  Class:  KFTimeSlider
//
//  Purpose:
//    The time slider for the keyframe editor.
//
//  Programmer:  Jeremy Meredith
//  Creation:    May  8, 2002
//
// ****************************************************************************
class KFTimeSlider : public QFrame
{
    Q_OBJECT
  public:
                 KFTimeSlider(QWidget *parent=NULL, const char *name=NULL,
                              KFListView *lv_=NULL);
                ~KFTimeSlider();
    void         setValue(int);
    void         setNSteps(int);
    void         updateSize();
  protected:
    void         paintEvent(QPaintEvent*);
    void         resizeEvent(QResizeEvent*);
    void         mouseMoveEvent(QMouseEvent*);
    void         mousePressEvent(QMouseEvent*);
    void         mouseReleaseEvent(QMouseEvent*);
    int          val2x(int);
    int          x2val(int);

  signals:
    void         valueChanged(int);
    void         mouseReleased();

  private:
    KFListView  *lv;
    int          nsteps;
    int          value;
};


#endif
