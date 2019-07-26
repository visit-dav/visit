// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_SPECTRUM_BAR_H
#define QVIS_SPECTRUM_BAR_H
#include <gui_exports.h>
#include <QWidget>

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
//   Brad Whitlock, Mon Jun  2 10:47:34 PDT 2008
//   Qt 4.
//
//   Jeremy Meredith, Fri Feb 20 15:03:25 EST 2009
//   Added alpha channel support.
//
//   Jeremy Meredith, Mon Feb 23 15:22:04 EST 2009
//   Removed the background pixmap; drawing with a real alpha
//   was expensive, and it was easy to fake it.
//
//   Brad Whitlock, Fri Apr 27 15:15:58 PDT 2012
//   Change smoothing from bool to enum. I also added more smoothing methods.
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

    enum SmoothingMethod
    {
        None,
        Linear,
        CubicSpline
    };

    QvisSpectrumBar(QWidget *parent);
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
    SmoothingMethod smoothing() const;

    unsigned char *getRawColors(int range);
    void   setRawColors(unsigned char *colors, int ncolors);

public slots:
    void   alignControlPoints();
    void   setEqualSpacing(bool val);
    void   setSmoothing(SmoothingMethod val);
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
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void paintEvent(QPaintEvent *e);
    virtual void resizeEvent(QResizeEvent *e);
    virtual void paletteChange(const QPalette &);

    QPoint controlPointLocation(int index) const;
    void   updateControlPoints();
    void   colorSelected(int index);
    void   drawControls(QPainter &);
    void   drawControlPoint(QPainter &paint, const QBrush &top,
                            const QBrush &bottom, const QBrush &fore,
                            const QColor &sel, const QColor &cpt, int x, int y,
                            int w, int h, int shadow_thick,
                            ControlOrientation orient,
                            bool selected);
    void   drawBox(QPainter &paint, const QRect &r, const QColor &light,
                   const QColor &dark, int lw = 2);
    void   drawArrow(QPainter &p, bool down, int x, int y, int w, int h,
                     const QPalette &pal);

    void   drawSpectrum(QPainter &);

    void   moveControlPoint(int changeType);
    void   moveControlPointRedraw(int index, float pos, bool redrawSpectrum);

    float  evalCubicSpline(float t, const float *allX, const float *allY, int n) const;
    void   getColorsCubicSpline(unsigned char *rgba, int ncolors) const;

private slots:
    void   handlePaging();
private:
    QTimer             *timer;

    ControlOrientation orientation;
    int                margin;
    QRect              spectrumArea;
    QRect              controlsArea;
    QRect              slider;
    SmoothingMethod    m_smoothing;
    bool               m_equalSpacing;
    bool               m_sliding;
    bool               m_continuousUpdate;
    bool               m_suppressUpdates;
    int                paging_mode;

    ControlPointList   *controlPoints;
};

#endif
