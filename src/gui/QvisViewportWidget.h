#ifndef QVIS_VIEWPORT_WIDGET_H
#define QVIS_VIEWPORT_WIDGET_H
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
#include <QWidget>
#include <QGraphicsView>
#include <QMap>

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
//   Cyrus Harrison, Fri Nov  7 15:42:13 PST 2008
//   Qt4 Refactor.
//
// ****************************************************************************

class QViewportItem;

class QvisViewportWidget : public QGraphicsView
{
    Q_OBJECT
public:
    QvisViewportWidget(double aspect, int minw, int minh, QWidget *parent = 0);
    virtual ~QvisViewportWidget();

    virtual QSize       sizeHint() const;
    virtual QSizePolicy sizePolicy() const;

    int     getNumberOfViewports() const;
    bool    getViewport(const QString &, float &llx, float &lly, 
                                      float &urx, float &ury) const;
    QString getActiveViewportId() const;
    void    viewportUpdated(QViewportItem *item);
    
public slots:
    QString addViewport(const QString &, float llx, float lly,
                                         float urx, float ury);
    QString addViewport(float llx, float lly,float urx, float ury);
    void    removeViewport(const QString &);
    void    setActiveViewport(const QString &);
    void    clear();

signals:
    void viewportAdded(const QString &, float left, float bottom,
                                        float right, float top);
    void viewportChanged(const QString &, float left, float bottom,
                                          float right, float top);
    void viewportRemoved(const QString &);
    void viewportActivated(const QString &);
protected:
    void     init();
    QSize    prevSize;
    
    virtual void mousePressEvent(QMouseEvent* e);
    virtual void mouseMoveEvent(QMouseEvent* e);
    virtual void mouseReleaseEvent(QMouseEvent* e);
    virtual void keyPressEvent(QKeyEvent *);
    
    void    getRelativeSize(QViewportItem *item, 
                            float &llx, float &lly, float &urx, float &ury);

    void    getRelativeSize(const QPointF &pos, const QRectF &rect,
                            float &llx, float &lly,float &urx, float &ury);
    
                                                 
    void    resizeEvent(QResizeEvent *event);
    
    QString getNextId() const;
    
                        
    void    activateItem(QViewportItem *);
    void    bringToFront(QViewportItem *);
    void    sendToBack(QViewportItem *);
    
    QMap<QString,QViewportItem*> items;
    
    int                     minW, minH;
    double                  aspect;
    double                  zValue;
    QGraphicsScene         *scene;
    QString                 prevSelected;
    
    bool                    dragViewportOutline;
    QPointF                 dragMouseStart;
    QGraphicsRectItem      *viewportOutline;
    
private slots:
    void onSceneSelectionChanged();
    
};

#endif
