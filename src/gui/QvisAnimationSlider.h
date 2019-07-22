// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_ANIMATION_SLIDER_H
#define QVIS_ANIMATION_SLIDER_H
#include <QSlider>

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
//   Brad Whitlock, Fri May 30 09:48:45 PDT 2008
//   Qt 4.
//
// ****************************************************************************

class QvisAnimationSlider : public QSlider
{
    Q_OBJECT
public:
    QvisAnimationSlider(Qt::Orientation, QWidget *parent = 0);
    QvisAnimationSlider(QWidget *parent = 0);
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
