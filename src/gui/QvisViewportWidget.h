// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_VIEWPORT_WIDGET_H
#define QVIS_VIEWPORT_WIDGET_H

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
