/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

#ifndef QVIS_SCATTER_WIDGET_H
#define QVIS_SCATTER_WIDGET_H
#include <QWidget>
#include <mini3D.h>

class QTimer;

// ****************************************************************************
// Class: QvisScatterWidget
//
// Purpose:
//   This widget displays some simple 3D graphics that look like a scatter
//   plot. This widget is used in the wizard to illustrate some of the steps.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 13 14:45:08 PST 2004
//
// Modifications:
//   Brad Whitlock, Fri Aug  8 09:42:33 PDT 2008
//   Qt 4.
//
// ****************************************************************************

class QvisScatterWidget : public QWidget
{
    Q_OBJECT
public:
    QvisScatterWidget(QWidget *parent);
    virtual ~QvisScatterWidget();
    virtual QSize sizeHint() const;

    void setThreeD(bool threeD);
    void setHighlightAxis(bool highlight);
    void setColoredPoints(bool colored);

    void setAllowAnimation(bool allow);

public slots:
    virtual void show();
    virtual void hide();
protected slots:
    void handleTimer();
protected:
    virtual void paintEvent(QPaintEvent *e);
    virtual void resizeEvent(QResizeEvent *e);

    void redrawScene(QPainter *painter);
    void redrawScene2D(QPainter *painter);
    void redrawScene3D(QPainter *painter);
    void drawSpherePoints();
    void deleteBackingPixmap();

    void createSharedElements();
    void initializeArrow();
    void initializeSphere(m3d_complex_element &, int nx, int ny, float rad,
                          float r, float g, float b);

    QPixmap                   *pixmap;
    QTimer                    *timer;
    m3d_renderer               renderer;
    bool                       rendererCreated;
    bool                       pixmapDirty;
    int                        animationProgress;
    bool                       animationCountPositive;

    bool                       threeD;
    bool                       highlightedAxis;
    bool                       coloredPoints;
    bool                       allowAnimation;

    static bool                sharedElementsCreated;
    static m3d_complex_element sphere;
    static m3d_complex_element arrow;
};

#endif
