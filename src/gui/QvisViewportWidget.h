#ifndef QVIS_VIEWPORT_WIDGET_H
#define QVIS_VIEWPORT_WIDGET_H
/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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
#include <qwidget.h>
#include <qcanvas.h>
#include <qmap.h>

// ****************************************************************************
// Class: QvisViewportWidget
//
// Purpose:
//   Displays viewports and lets the user move them around and resize them.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 28 13:27:10 PST 2006
//
// Modifications:
//   
// ****************************************************************************

class QvisViewportWidget : public QWidget
{
    Q_OBJECT
public:
    QvisViewportWidget(double _aspect, int minw, int minh,
                       QWidget *parent = 0, const char *name = 0);
    QvisViewportWidget(QWidget *parent = 0, const char *name = 0);

    virtual ~QvisViewportWidget();

    virtual QSize sizeHint() const;
    virtual QSizePolicy sizePolicy() const;

    int  getNumberOfViewports() const;
    bool getViewport(const QString &, float &llx, float &lly, float &urx, float &ury) const;
    QString getActiveViewportId() const;

    void setContinuousUpdates(bool);
    bool continuousUpdates() const;

public slots:
    void clear();
    QString addViewport(const QString &, float llx, float lly, float urx, float ury);
    QString addViewport(float llx, float lly, float urx, float ury);
    void removeViewport(const QString &);
    void setActiveViewport(const QString &);
    void setAspect(double);
    void sendBackgroundToBack();
signals:
    void viewportAdded(const QString &, float llx, float lly, float urx, float ury);
    void viewportChanged(const QString &, float llx, float lly, float urx, float ury);
    void viewportRemoved(const QString &);
    void viewportActivated(const QString &);
protected:
    void init();

    QPoint canvasOrigin() const;

    virtual void paintEvent(QPaintEvent *);

    virtual void mouseMoveEvent(QMouseEvent* e);
    virtual void mousePressEvent(QMouseEvent* e);
    virtual void mouseReleaseEvent(QMouseEvent* e);

    virtual void keyPressEvent(QKeyEvent *);
    virtual void keyReleaseEvent(QKeyEvent *e);

    QString getNextId() const;
    QString findViewportId(QCanvasItem *) const;
    void activateItem(QCanvasItem *);
    void bringToFront(QCanvasItem *);
    void sendToBack(QCanvasItem *);

    struct ViewportInfo
    {
        QString id;
        float   lower_left[2];
        float   upper_right[2];
    };
    typedef QMap<QCanvasItem*, ViewportInfo> QCanvasItemViewportInfoMap;

    QCanvasItemViewportInfoMap viewportMap;
    int                     minW, minH;
    double                  aspect;
    double                  zValue;
    QCanvas                *canvas;
    QCanvasItem            *item;
    QCanvasItem            *createitem;
    bool                    mouseDown;
    QPoint                  moving_start;
    int                     cursorResizeMode;
    bool                    doContinuousUpdates;
    bool                    shiftApplied;
};

#endif
