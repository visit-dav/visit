/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
