#ifndef QVIS_SPECTRUM_BAR_H
#define QVIS_SPECTRUM_BAR_H
#include <gui_exports.h>
#include <qwidget.h>

// Forward declarations.
class ControlPointList;
class QPainter;
class QPixmap;
class QTimer;

// ****************************************************************************
// Class: QvisSpectrumBar
//
// Purpose:
//   This class is the spectrum bar widget which is a widget used for editing
//   color tables. The color table is composed of colored control points that
//   can be moved to alter the appearance of the color table.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 3 09:58:26 PDT 2001
//
// Modifications:
//   Brad Whitlock, Fri Sep 7 12:45:57 PDT 2001
//   Added override of the paletteChange method.
//
//   Brad Whitlock, Wed Mar 13 08:43:40 PDT 2002
//   Added internal drawBox and drawArrow methods. Also consolidated the
//   widget's pixmaps into a single pixmap so I can set it as the widget's
//   background to reduce flicker.
//
// ****************************************************************************

class GUI_API QvisSpectrumBar : public QWidget
{
    Q_OBJECT
public:
    typedef enum {HorizontalTop,
                  HorizontalBottom,
                  VerticalLeft,
                  VerticalRight} ControlOrientation;

    QvisSpectrumBar(QWidget *parent, const char *name = 0);
    virtual ~QvisSpectrumBar();
    virtual QSize sizeHint() const;
    virtual QSizePolicy sizePolicy() const;

    void   addControlPoint(const QColor &color, float position = -1);
    bool   continuousUpdate() const;
    QColor controlPointColor(int index) const;
    float  controlPointPosition(int index) const;
    bool   equalSpacing() const;
    int    numControlPoints() const;
    int    activeControlPoint() const;
    bool   suppressUpdates() const;
    void   setSuppressUpdates(bool val);

    void   setOrientation(ControlOrientation orient);
    void   setContinuousUpdate(bool val);
    void   setControlPointColor(int index, const QColor &color);
    void   setControlPointPosition(int index, float position);
    bool   smoothing() const;

    unsigned char *getRawColors(int range);
    void   setRawColors(unsigned char *colors, int ncolors);

public slots:
    void   alignControlPoints();
    void   setEqualSpacing(bool val);
    void   setSmoothing(bool val);
    void   removeControlPoint();
    void   setEditMode(bool val);
signals:
    void   activeControlPointChanged(int index);
    void   selectColor(int index);
    void   selectColor(int index, const QPoint &pos);
    void   controlPointAdded(int index, const QColor &c, float position);
    void   controlPointColorChanged(int index, const QColor &c);
    void   controlPointMoved(int index, float position);
    void   controlPointRemoved(int index, const QColor &c, float position);
protected:
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void keyReleaseEvent(QKeyEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void paintEvent(QPaintEvent *e);
    virtual void resizeEvent(QResizeEvent *e);
    virtual void paletteChange(const QPalette &);

    QPoint controlPointLocation(int index) const;
    void   updateControlPoints();
    void   colorSelected(int index);
    void   deletePixmap();
    void   drawControls();
    void   drawControlPoint(QPainter *paint, const QBrush &top,
                            const QBrush &bottom, const QBrush &fore,
                            const QColor &sel, const QColor &cpt, int x, int y,
                            int w, int h, int shadow_thick,
                            ControlOrientation orient,
                            bool selected);
    void   drawBox(QPainter *paint, const QRect &r, const QColor &light,
                   const QColor &dark, int lw = 2);
    void   drawArrow(QPainter *p, bool down, int x, int y, int w, int h,
                     const QColorGroup &g);

    void   drawSpectrum();
    void   updateEntireWidget();

    void   moveControlPoint(int changeType);
    void   moveControlPointRedraw(int index, float pos, bool redrawSpectrum);
private slots:
    void   handlePaging();
private:
    QPixmap            *pixmap;
    QTimer             *timer;

    ControlOrientation orientation;
    int                margin;
    QRect              spectrumArea;
    QRect              controlsArea;
    QRect              slider;
    bool               b_smoothing;
    bool               b_equalSpacing;
    bool               b_sliding;
    bool               b_continuousUpdate;
    bool               b_suppressUpdates;
    int                paging_mode;
    bool               shiftApplied;

    ControlPointList   *controlPoints;
};

#endif
