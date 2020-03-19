// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QvisAbstractOpacityBar_H
#define QvisAbstractOpacityBar_H
#include <gui_exports.h>

#include <QFrame>
class QImage;
class ColorControlPointList;

// ****************************************************************************
//  Class:  QvisAbstractOpacityBar
//
//  Purpose:
//    Abstract base for an opacity map editor
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 30, 2001
//
//  Modifications:
//    Gunther Weber, Fri Apr  6 16:04:52 PDT 2007
//    Added support for painting in the color spectrum.
//
//    Brad Whitlock, Fri May 30 09:32:22 PDT 2008
//    Qt 4.
//
//    Brad Whitlock, Thu Dec 18 10:55:02 PST 2008
//    I added histogram textures.
//
//    Brad Whitlock, Mon Dec 27 15:34:08 PST 2010
//    I added some knobs that alter how histograms are drawn.
//
// ****************************************************************************

class GUI_API QvisAbstractOpacityBar : public QFrame
{
    Q_OBJECT
public:
                   QvisAbstractOpacityBar(QWidget *parent=NULL);
    virtual       ~QvisAbstractOpacityBar();
    virtual float *getRawOpacities(int) = 0;
    void           setBackgroundColorControlPoints(const ColorControlPointList *ccp);

    void           setHistogramTexture(const float *t, int ts);
    void           setHistogramTexture(const float *t, const bool *tm, int ts);

signals:
    void           mouseReleased();

protected:
    int            val2x(float);
    float          x2val(int);
    int            val2y(float);
    float          y2val(int);

    void           drawColorBackground();
    void           drawFilledCurve(float *curve, bool *mask, int nc, const QColor &cc, float opac);
    void           drawFilledCurveWithSelection(float *curve, bool *mask, int nc,
                                          const QColor &curveOn,  float curveOnOpacity,
                                          const QColor &curveOff, float curveOffOpacity,
                                          const QColor &binLines, bool  drawBinLines,
                                          float range[2], float minval, float maxval);
    void           imageDirty();

    virtual void   paintEvent(QPaintEvent*);
    virtual void   resizeEvent(QResizeEvent*);
    virtual void   drawOpacities() = 0;

    QImage        *image;
    const ColorControlPointList
                  *backgroundColorControlPoints;
    float         *histTexture;
    bool          *histTextureMask;
    int            histTextureSize;
    QColor         histogramColor;

private:
    bool           ensureImageExists(int,int);
};

#endif
