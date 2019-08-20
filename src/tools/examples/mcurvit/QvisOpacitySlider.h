// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_OPACITY_SLIDER_H
#define QVIS_OPACITY_SLIDER_H
#include <gui_exports.h>
#include <QAbstractSlider>

class QTimer;

// ****************************************************************************
// Class: QvisOpacitySlider
//
// Purpose:
//   This class defines a slider control, which is a simplified version of
//   QSlider that has a background gradient pixmap to indicate opacity and
//   is always drawn in windows style.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 7 12:10:01 PDT 2000
//
// Modifications:
//   Brad Whitlock, Thu Nov 13 10:01:21 PDT 2003
//   I added the internal pixmapWidth method.
//
//   Brad Whitlock, Tue Jun 28 12:11:36 PDT 2005
//   I made setGradientColor be a slot.
//
//   Brad Whitlock, Thu Jun  5 11:17:04 PDT 2008
//   Qt 4.
//
// ****************************************************************************

class GUI_API QvisOpacitySlider : public QAbstractSlider
{
    Q_OBJECT
    Q_PROPERTY(int pageStep READ pageStep WRITE setPageStep )
    Q_PROPERTY(int tickInterval READ tickInterval WRITE setTickInterval )
public:
    QvisOpacitySlider(QWidget *parent, const void *data = 0);
    QvisOpacitySlider(int minValue, int maxValue, int pageStep, int value,
                      QWidget *parent, const void *data=0);
    virtual ~QvisOpacitySlider();

    QRect        sliderRect() const;
    QSize        sizeHint() const;
    QSizePolicy  sizePolicy() const;
    QSize        minimumSizeHint() const;

    virtual void setTickInterval(int);
    int          tickInterval() const { return tickInt; }

public slots:
    virtual void setValue(int);
    virtual void setEnabled(bool);
    void         addStep();
    void         subtractStep();
    void         setGradientColor(const QColor &color);

signals:
    void    valueChanged(int value, const void *data);

protected:
    virtual void resizeEvent( QResizeEvent * );
    virtual void paintEvent( QPaintEvent * );

    virtual void keyPressEvent( QKeyEvent * );
    virtual void mousePressEvent( QMouseEvent * );
    virtual void mouseReleaseEvent( QMouseEvent * );
    virtual void mouseMoveEvent( QMouseEvent * );
    virtual void wheelEvent( QWheelEvent * );

    void    rangeChange();
    void    paletteChange(const QPalette &oldPalette);

    virtual int thickness() const;
    int     maximumSliderDragDistance() const;

    // Methods to draw the slider
    virtual void paintSlider(QPainter *, const QPalette&, const QRect &);
    void    drawTicks(QPainter *, const QPalette&, int, int, int=1) const;

    void    drawTicks(QPainter *,  int, int, int=1 ) const;
    void    drawSliderGroove(QPainter *p, int x, int y, int w, int h, int c);
    void    drawSlider(QPainter *p, int x, int y, int w, int h);
    void    createGradientImage();
    void    deleteGradientImage();
    int     imageWidth() const;
    int     textPadding() const;
    void    paintValueText(QPainter *, const QPalette&, int, int);

private slots:
    void    repeatTimeout();
    void    handle_valueChanged(int);

private:
    enum State { Idle, Dragging, TimingUp, TimingDown };

    void       addLine();
    void       subtractLine();
    void       addPage();
    void       subtractPage();

    void       init();
    int        positionFromValue(int) const;
    int        valueFromPosition(int) const;
    void       moveSlider(int);
    void       reallyMoveSlider(int);
    void       resetState();
    int        sliderLength() const;
    int        available() const;
    void       initTicks();

    QColor     gradientColor;
    QImage    *gradientImage;
    QTimer    *timer;
    int        sliderPos;
    int        sliderVal;
    int        clickOffset;
    State      state;
    int        tickOffset;
    int        tickInt;
    const void *userData;

private:    // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QvisOpacitySlider( const QvisOpacitySlider &);
    QvisOpacitySlider &operator=( const QvisOpacitySlider &);
#endif
};

#endif
