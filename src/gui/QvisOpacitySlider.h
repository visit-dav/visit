#ifndef QVIS_OPACITY_SLIDER_H
#define QVIS_OPACITY_SLIDER_H
#include <gui_exports.h>
#ifndef QT_H
#include "qwidget.h"
#include "qrangecontrol.h"
#endif

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
//   
// ****************************************************************************

class GUI_API QvisOpacitySlider : public QWidget, public QRangeControl
{
    Q_OBJECT
    Q_PROPERTY(int minValue READ minValue WRITE setMinValue )
    Q_PROPERTY(int maxValue READ maxValue WRITE setMaxValue )
    Q_PROPERTY(int lineStep READ lineStep WRITE setLineStep )
    Q_PROPERTY(int pageStep READ pageStep WRITE setPageStep )
    Q_PROPERTY(int value READ value WRITE setValue )
    Q_PROPERTY(int tickInterval READ tickInterval WRITE setTickInterval )
public:
    QvisOpacitySlider(QWidget *parent, const char *name=0,
                      const void *data = 0);
    QvisOpacitySlider(int minValue, int maxValue, int pageStep, int value,
                      QWidget *parent, const char *name=0, const void *data=0);
    virtual ~QvisOpacitySlider();

    QRect        sliderRect() const;
    QSize        sizeHint() const;
    QSizePolicy  sizePolicy() const;
    QSize        minimumSizeHint() const;

    virtual void setTickInterval(int);
    int          tickInterval() const { return tickInt; }

    void         setGradientColor(const QColor &color);

    int          minValue() const;
    int          maxValue() const;
    void         setMinValue(int);
    void         setMaxValue(int);
    int          lineStep() const;
    int          pageStep() const;
    void         setLineStep(int);
    void         setPageStep(int);
    int          value() const;

public slots:
    virtual void setValue(int);
    virtual void setEnabled(bool);
    void         addStep();
    void         subtractStep();

signals:
    void    valueChanged(int value);
    void    valueChanged(int value, const void *data);

protected:
    void    resizeEvent( QResizeEvent * );
    void    paintEvent( QPaintEvent * );

    void    keyPressEvent( QKeyEvent * );
    void    mousePressEvent( QMouseEvent * );
    void    mouseReleaseEvent( QMouseEvent * );
    void    mouseMoveEvent( QMouseEvent * );
    void    wheelEvent( QWheelEvent * );

    void    updateMask();
    void    valueChange();
    void    rangeChange();
    void    paletteChange(const QPalette &oldPalette);

    virtual int thickness() const;
    int     maximumSliderDragDistance() const;

    // Methods to draw the slider
    virtual void paintSlider(QPainter *, const QColorGroup&, const QRect &);
    void    drawTicks(QPainter *, const QColorGroup&, int, int, int=1) const;

    void    drawTicks(QPainter *,  int, int, int=1 ) const;
    void    drawSliderGroove(QPainter *p, int x, int y, int w, int h, QCOORD c);
    void    drawSliderMask(QPainter *p, int x, int y, int w, int h);
    void    drawSlider(QPainter *p, int x, int y, int w, int h);
    void    createGradientPixmap();
    void    deleteGradientPixmap();

private slots:
    void    repeatTimeout();

private:
    enum State { Idle, Dragging, TimingUp, TimingDown };

    void       init();
    int        positionFromValue(int) const;
    int        valueFromPosition(int) const;
    void       moveSlider(int);
    void       reallyMoveSlider(int);
    void       resetState();
    int        sliderLength() const;
    int        available() const;
    int        goodPart(const QPoint&) const;
    void       initTicks();

    QColor     gradientColor;
    QPixmap    *gradientPixmap;
    QTimer     *timer;
    QCOORD     sliderPos;
    int        sliderVal;
    QCOORD     clickOffset;
    State      state;
    bool       track;
    QCOORD     tickOffset;
    int        tickInt;
    const void *userData;

private:    // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QvisOpacitySlider( const QvisOpacitySlider &);
    QvisOpacitySlider &operator=( const QvisOpacitySlider &);
#endif
};

#endif
