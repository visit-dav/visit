/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

#ifndef QVIS_LIGHTING_WIDGET_H
#define QVIS_LIGHTING_WIDGET_H
#include <gui_exports.h>
#include <qwidget.h>
#include <mini3D.h>

// ****************************************************************************
// Class: QvisLightingWidget
//
// Purpose:
//   This class uses the mini3D renderer to draw a 3D representation of the
//   lighting being used for plots.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 16:30:31 PST 2001
//
// Modifications:
//   Brad Whitlock, Mon Mar 3 13:39:52 PST 2003
//   I updated the code to use the new m3d_renderer class instead of using
//   global m3d functions.
//
// ****************************************************************************

class GUI_API QvisLightingWidget : public QWidget
{
    Q_OBJECT
public:
    QvisLightingWidget(QWidget *parent, const char *name = 0);
    virtual ~QvisLightingWidget();
    virtual QSize sizeHint() const;
    virtual QSizePolicy sizePolicy() const;

    void setPreviewMode(bool val);
    void clearLights();
    void addLight(const double position[3], const QColor &c, int type);
    void setLightColor(const QColor &c);
    void setLightPosition(const double position[3]);
    void setLightType(int type);
signals:
    void lightMoved(double x, double y, double z);
protected:
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void paintEvent(QPaintEvent *e);
    virtual void resizeEvent(QResizeEvent *e);

    void createSharedElements();
    void deleteBackingPixmap();
    void redrawScene(QPainter *painter);
    void setupAndDraw(QPainter *painter);

    void initializeCube();
    void initializeSphere();
    void initializeLitecone();
    void initializeLitearrow();
    void initializeLitebulb();

    QPixmap    *pixmap;
    bool        previewMode;
    bool        rendererCreated;
    bool        mouseDown;
    bool        needsRedrawPixmap;
    float       lastX;
    float       lastY;

    // Some internal state for lights.
    int         numberOfLights;

    m3d_light   lights[8];
    matrix4     view;
    matrix4     view2;
    matrix4     proj;

    m3d_renderer renderer;

    static bool sharedElementsCreated;
    static m3d_complex_element litearrow;
    static m3d_complex_element litecone;
    static m3d_complex_element litebulb;
    static m3d_complex_element sphere;
    static m3d_complex_element cube;
};


#endif
