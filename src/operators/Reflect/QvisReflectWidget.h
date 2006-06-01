/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#ifndef QVIS_REFLECT_WIDGET_H
#define QVIS_REFLECT_WIDGET_H
#include <qwidget.h>
#include <mini3D.h>

class QTimer;

// ****************************************************************************
// Class: QvisReflectWidget
//
// Purpose:
//   This is a 3D widget that allows us to set the octants for the reflect
//   operator.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 5 15:45:14 PST 2003
//
// Modifications:
//   Brad Whitlock, Mon Jun 23 16:49:01 PST 2003
//   I added a 2d interaction mode.
//
// ****************************************************************************

class QvisReflectWidget : public QWidget
{
    Q_OBJECT
public:
    QvisReflectWidget(QWidget *parent, const char *name=0);
    virtual ~QvisReflectWidget();
    virtual QSize sizeHint() const;
    virtual QSizePolicy sizePolicy() const;

    void setMode2D(bool val);
    bool getMode2D() const;
signals:
    void octantChanged(int);
    void valueChanged(bool *octants);
public slots:
    void setValues(bool *octants);
    void setOriginalOctant(int octant);
protected slots:
    void handleTimer();
protected:
    void drawOnOffActors(int n, float scale);
    void deleteBackingPixmap();
    void redrawScene(QPainter *painter);
    void redrawScene2D(QPainter *painter);
    void redrawScene3D(QPainter *painter);
    void setupAndDraw(QPainter *p);
    void setupCamera();
    void createSharedElements();
    void initializeAxes();
    void initializeAxes2D();
    void initializeArrow();
    void initializeSphere(m3d_complex_element &, int nx, int ny, float rad,
                          float r, float g, float b);
    void initializeCube(m3d_complex_element &, int nx, int ny,
                        float s, float r, float g, float b);
    void ScaleTranslateFromOriginToOctant(int octant, float s);

    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void paintEvent(QPaintEvent *e);
    virtual void resizeEvent(QResizeEvent *e);

    bool          mode2D;

    QPixmap      *pixmap;
    m3d_renderer  renderer;
    bool          rendererCreated;

    int           originOctant;
    bool          octantOn[8];

    QTimer       *timer;
    int           activeCamera;
    float         cameraInterpolant;
    bool          switchingCameras;

    static bool                sharedElementsCreated;
    static m3d_complex_element axes;
    static m3d_complex_element axes2D;
    static m3d_complex_element onCube;
    static m3d_complex_element offCube;
    static m3d_complex_element onSphere;
    static m3d_complex_element offSphere;
    static m3d_complex_element arrow;
};


#endif
