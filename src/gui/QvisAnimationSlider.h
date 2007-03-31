#ifndef QVIS_ANIMATION_SLIDER_H
#define QVIS_ANIMATION_SLIDER_H
#include <qslider.h>

// ****************************************************************************
// Class: QvisAnimationSlider
//
// Purpose:
//   Special purpose slider that does not emit valueChanged signals while
//   the slider is being paged.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 30 14:26:22 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class QvisAnimationSlider : public QSlider
{
    Q_OBJECT
public:
    QvisAnimationSlider(Orientation, QWidget *parent = 0, const char *name = 0);
    QvisAnimationSlider(QWidget *parent = 0, const char *name = 0);
    virtual ~QvisAnimationSlider();

    bool sliderIsDown() const { return the_sliderIsDown; };

signals:
    void sliderValueChanged(int);
    void sliderWasReleased();
protected slots:
    void handleValueChanged(int);
    void handleSliderReleased();
protected:
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);

    bool the_sliderIsDown;
    bool emittedSignal;
};

#endif
