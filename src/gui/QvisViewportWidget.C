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
#include <QvisViewportWidget.h>
#include <QCursor>
#include <QGraphicsItem>
#include <QGraphicsSceneHoverEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPen>
#include <QColor>
#include <QMatrix>
#include <QPainter>

// ****************************************************************************
// Class: QViewportItem
//
// Purpose:
//   This class represents viewport objects that are drawn to the Graphics
//   scene.
//
//
// Programmer: Cyrus Harrison
// Creation:   Wed Nov  5 08:22:57 PST 2008
//
// Modifications:
//   
// ****************************************************************************

#include <iostream>
using namespace std;

class QViewportItem : public QGraphicsRectItem
{
public:
    QViewportItem (const QString &id, 
                   float llx, float lly,
                   float urx, float ury,
                   QvisViewportWidget *view);
    virtual ~QViewportItem();
    
    QString getId() const 
    { return id; }

    bool isBackground() const 
    { return background; }
    
    void mapRelativeToSize(const QSize &size,
                           float &llx,float &lly,float &urx,float &ury);

    void setRelativeToSize(const QSize &size,
                           float llx,float lly,float urx,float ury);

protected:
    
    void updateText(); 
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

    QString                  id;
    bool                     background;
    QvisViewportWidget      *view;
    QGraphicsSimpleTextItem *textItem;
    int                      resizeMode;
    QPointF                  prevPos;
    QRectF                   prevRect;

};


// ****************************************************************************
// Method: QViewportItem::QViewportItem
//
// Purpose:
//    Constructs a Viewport Graphics Item.
//
// Programmer: Cyrus Harrison
// Creation:   Wed Nov  5 08:22:57 PST 2008
//
// Modifications:
//   
// ****************************************************************************
QViewportItem ::QViewportItem(const QString &id, 
                              float llx, float lly,
                              float urx, float ury,
                              QvisViewportWidget *view) 
: QGraphicsRectItem(), id(id), background(false), view(view), resizeMode(0)
{
    QBrush brush(Qt::SolidPattern);
    brush.setColor(QColor(0,255,255));
    setBrush(brush);
    textItem = new QGraphicsSimpleTextItem(id,this);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable,true);
    setAcceptHoverEvents(true);
    if(llx == 0.0 && lly == 0.0 && urx == 1.0 && ury == 1.0)
    {
        background = true;
        setZValue(-1e32);
    }
    setRelativeToSize(view->size(),llx,lly,urx,ury);
}

// ****************************************************************************
// Method: QViewportItem::~QViewportItem
//
// Purpose:
//    Destructs a Viewport Graphics Item.
//
// Programmer: Cyrus Harrison
// Creation:   Wed Nov  5 08:22:57 PST 2008
//
// Modifications:
//   
// ****************************************************************************

QViewportItem::~QViewportItem()
{}

// ****************************************************************************
// Method: QViewportItem::mapRelativeToSize
//
// Purpose:
//    Gets the viewport item's size relative to input size.
//
// Programmer: Cyrus Harrison
// Creation:   Wed Nov  5 08:22:57 PST 2008
//
// Modifications:
//   
// ****************************************************************************        

void 
QViewportItem::mapRelativeToSize(const QSize &size,
                                 float &llx,float &lly,float &urx,float &ury)
{
    // get relative size to input
    float w = size.width();
    float h = size.height();

    llx = pos().x() / w;
    urx = llx + rect().width() / w;
    ury = (h - pos().y()) / h;
    lly = ury - rect().height() / h;
}

// ****************************************************************************
// Method: QViewportItem::setRelativeToSize
//
// Purpose:
//    Sets the viewport item's size with relative values to a given size.
//
// Programmer: Cyrus Harrison
// Creation:   Wed Nov  5 08:22:57 PST 2008
//
// Modifications:
//   
// ****************************************************************************        

void 
QViewportItem::setRelativeToSize(const QSize &size,
                                 float llx,float lly,float urx,float ury)
{
    // set relative to input
    float w = size.width();
    float h = size.height();
        
    float rel_w = urx - llx;
    float rel_h = ury - lly;

    setRect(0.0,0.0,rel_w * w,rel_h * h);
    setPos(llx* w, (1- ury) * h);
    updateText();
}

// ****************************************************************************
// Method: QViewportItem::setRelativeToSize
//
// Purpose:
//    Updates the viewport's name text based on the viewport item's size.
//
// Programmer: Cyrus Harrison
// Creation:   Wed Nov  5 08:22:57 PST 2008
//
// Modifications:
//   
// ****************************************************************************        

void
QViewportItem::updateText()
{
    QFontMetricsF fmet(textItem->font());
    float txt_w = fmet.width(id);
    float txt_h = fmet.height();
    if(txt_w > rect().width())
    {
        // See if we can shorten the name.
        QString name(id);
        QString number;
        QString numbers("0123456789");
        int i = name.length()-1;
        while(i >= 0 && numbers.contains(name[i]))
        {
            number = name.mid(i,1) + number;
            --i;
        }
        
        if(!number.isNull())
            name = number;

        textItem->setText(name);
        txt_w = fmet.width(name);
    }
    else
    {
        textItem->setText(id);
    }

    textItem->setPos(rect().width()/2.0- txt_w/2,
                     rect().height()/2.0- txt_h/2);
}    

// ****************************************************************************
// Method: QViewportItem::hoverLeaveEvent
//
// Purpose:
//    Called when the mouse leaves this item, resets the resize mode
//    and cursor.
//
// Programmer: Cyrus Harrison
// Creation:   Wed Nov  5 08:22:57 PST 2008
//
// Modifications:
//   
// ****************************************************************************        

void 
QViewportItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    view->setCursor(QCursor(Qt::ArrowCursor));
    resizeMode = 0;
}
   
// ****************************************************************************
// Method: QViewportItem::hoverMoveEvent
//
// Purpose:
//    Called when the mouse hovers over this item, sets the resize mode
//    and cursor.
//
// Programmer: Cyrus Harrison
// Creation:   Wed Nov  5 08:22:57 PST 2008
//
// Modifications:
//   
// ****************************************************************************        
 
void
QViewportItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    float x = event->pos().x();
    float y = event->pos().y();
    float w = rect().width();
    float h = rect().height();
       
    float dx = 3;
        
    resizeMode = 0;
    
    if( x - dx <= 0)
    {
        if( y - dx <=0)      // upper left corner
            resizeMode = 8;
        else if(y + dx >= h) // lower left corner
            resizeMode = 2;
        else                 // move left
            resizeMode = 1;
    }
    else if( x + dx >=w )
    {
        if( y - dx <=0)      // upper right corner
            resizeMode = 6;
        else if(y + dx >= h) // lower right corner
            resizeMode = 4;
        else                 // move right
            resizeMode = 5;
    }
    else if( y + dx >=h)     // move bottom
        resizeMode = 3;
    else if( y - dx <=0)     // move top
        resizeMode = 7;
      
    if(resizeMode == 1 || resizeMode == 5)       // left & right
        view->setCursor(QCursor(Qt::SizeHorCursor));
    else if(resizeMode == 2 || resizeMode == 6)  // bottom left & upper right
        view->setCursor(QCursor(Qt::SizeBDiagCursor));
    else if(resizeMode == 3 || resizeMode == 7) // top & bottom
        view->setCursor(QCursor(Qt::SizeVerCursor));
    else if(resizeMode == 4 || resizeMode == 8) // bottom right & upper left
        view->setCursor(QCursor(Qt::SizeFDiagCursor));
    else // no resize
        view->setCursor(QCursor(Qt::ArrowCursor));
 }

// ****************************************************************************
// Method: QViewportItem::mousePressEvent
//
// Purpose:
//    Handles selection (left button) and deletion (right button) mouse actions.
//
// Programmer: Cyrus Harrison
// Creation:   Wed Nov  5 08:22:57 PST 2008
//
// Modifications:
//   
// ****************************************************************************        

void
QViewportItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // select this viewport if left button is clicked
    // and it is not a background viewport
    if(event->button() == Qt::LeftButton && !isBackground())
    {
        prevPos  = pos();
        prevRect = rect();
        setSelected(true);
    }
    else if(event->button() == Qt::RightButton)
    {   
        // delete this viewport if the right button is cliced
        view->removeViewport(id);
    }
    else
    {
        resizeMode =0;
        QGraphicsRectItem::mousePressEvent(event);
    }
}

// ****************************************************************************
// Method: QViewportItem::mousePressEvent
//
// Purpose:
//    Handles moving and resizing of a viewport item.
//
// Programmer: Cyrus Harrison
// Creation:   Wed Nov  5 08:22:57 PST 2008
//
// Modifications:
//   
// ****************************************************************************        

void
QViewportItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    // only move if the viewport is selected
    if(!isSelected())
        return;

    // if we are simply moving the viewport, use
    // the base class mouse move event
    if(resizeMode == 0)
    {
        QGraphicsRectItem::mouseMoveEvent(event);
        view->viewportUpdated(this);
        return;
    }
        
    // move according to resize mode
    QPointF down = event->buttonDownScenePos(Qt::LeftButton);
    QPointF last = event->lastScenePos();
        
    float dx = down.x() - last.x();
    float dy = down.y() - last.y();
        
    float nx = prevPos.x();
    float ny = prevPos.y();
    float nw = prevRect.width();
    float nh = prevRect.height();
        
    switch(resizeMode)
    {
        case 1:
            // move left -dx
            nx -= dx;
            nw += dx;
            break;
        case 2:
            // move bottom left
            nx -= dx;
            nw += dx;
            nh -= dy;
            break;
        case 3:
            // move bottom -dy
            nh -=dy;
            break;
        case 4:
            // move bottom right
            nw -= dx;
            nh -= dy;
            break;
         case 5:
            // move right dx
            nw -= dx;
            break;
        case 6:
            // move top right
            ny -= dy;
            nw -= dx;
            nh += dy;
            break;
        case 7:
            // move top dy
            ny -= dy;
            nh += dy;
            break;
        case 8:
            // move top left
            nx -= dx;
            ny -= dy;
            nw += dx;
            nh += dy;
            break;
    };
        
    if(nw < 0)
    {
        nx += nw;
        nw = -1 * nw;
    }
    
    if(nh < 0)
    {
        ny += nh;
        nh = -1 * nh;
    }
        
    setPos(nx,ny);
    setRect(0,0,nw,nh);
    updateText();
        
    // signal change
    view->viewportUpdated(this);
}

// ****************************************************************************
// Method: QvisViewportWidget::QvisViewportWidget
//
// Purpose: 
//   Constructor
//
// Arguments:
//   minw    : The minimum widget width.
//   minh    : The minimum widget height.
//   parent  : The parent for the widget.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 14:16:21 PST 2006
//
// Modifications:
//   Cyrus Harrison, Fri Nov  7 15:42:13 PST 2008
//   Qt4 Refactor.
//
// ****************************************************************************

QvisViewportWidget::QvisViewportWidget(double aspect,
                                       int minw, int minh,
                                       QWidget *parent)
: QGraphicsView(parent), aspect(aspect), minW(minw), minH(minh), 
  prevSelected(""), 
  dragViewportOutline(false), 
  viewportOutline(0), 
  dragMouseStart(QPointF(0.0,0.0))
{
    init();
}

// ****************************************************************************
// Method: QvisViewportWidget::init
//
// Purpose: 
//   Initializes most of the members.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 14:18:03 PST 2006
//
// Modifications:
//   Cyrus Harrison, Fri Nov  7 15:42:13 PST 2008
//   Qt4 Refactor.
//
// ****************************************************************************

void
QvisViewportWidget::init()
{
    zValue = 0.0;
    // Set a min size.
    setMinimumSize(minW, minH);
    prevSize = size();

    // Create a scene with the right aspect ratio that will fit in the widget.
    int w = minW - 20;
    int h = int(aspect * w);
    if(h > minH)
    {
        h = minH - 20;
        w  = int(h / aspect);
    }
    
    scene = new QGraphicsScene(this);
    scene->setBackgroundBrush(Qt::white);
    setScene(scene);
    scene->setSceneRect(0.0,0.0,size().width()-1,size().height()-1);
    setMouseTracking(true);

    // Set the focus policy to StrongFocus. This means that the widget will
    // accept focus by tabbing and clicking.
    setFocusPolicy(Qt::StrongFocus);
    
    connect(scene,SIGNAL(selectionChanged()),
            this,SLOT(onSceneSelectionChanged()));
}

// ****************************************************************************
// Method: QvisViewportWidget::~QvisViewportWidget
//
// Purpose: 
//   Destructor
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 14:18:25 PST 2006
//
// Modifications:
//   Cyrus Harrison, Fri Nov  7 15:42:13 PST 2008
//   Qt4 Refactor.
//
// ****************************************************************************

QvisViewportWidget::~QvisViewportWidget()
{
    disconnect(scene,SIGNAL(selectionChanged()),
            this,SLOT(onSceneSelectionChanged()));

    delete scene;
}

// ****************************************************************************
// Method: QvisViewportWidget::sizeHint
//
// Purpose: 
//   Returns the desired size.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 14:18:51 PST 2006
//
// Modifications:
//   
// ****************************************************************************

QSize
QvisViewportWidget::sizeHint() const
{
    return QSize(minW,minH);
}

// ****************************************************************************
// Method: QvisViewportWidget::sizePolicy
//
// Purpose: 
//   Returns the desired size policy.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 14:18:51 PST 2006
//
// Modifications:
//   
// ****************************************************************************

QSizePolicy
QvisViewportWidget::sizePolicy() const
{
    return QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}


// ****************************************************************************
// Method: QvisViewportWidget::resizeEvent
//
// Purpose: 
//   Handle resize and make sure proper viewport aspects are retained.
//
// Programmer: Cyrus Harrison
// Creation:   Fri Nov  7 15:42:13 PST 2008
//
// Modifications:
//
// ****************************************************************************
void 
QvisViewportWidget::resizeEvent(QResizeEvent *event)
{
    float llx,lly,urx,ury;
    scene->setSceneRect(0,0,event->size().width()-1,event->size().height()-1);
    QGraphicsView::resizeEvent(event);
    QMapIterator<QString,QViewportItem*> itr(items);
    while(itr.hasNext())
    {
        itr.next();
        // get old relative rect
        itr.value()->mapRelativeToSize(prevSize,llx,lly,urx,ury);
        // set to new size
        itr.value()->setRelativeToSize(event->size(),llx,lly,urx,ury);
    }
    prevSize = event->size();
}

// ****************************************************************************
// Method: QvisViewportWidget::clear
//
// Purpose: 
//   Removes all of the viewports.
//
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 14:20:07 PST 2006
//
// Modifications:
//   Cyrus Harrison, Fri Nov  7 15:42:13 PST 2008
//   Qt4 Refactor.
//   
// ****************************************************************************

void
QvisViewportWidget::clear()
{
    disconnect(scene,SIGNAL(selectionChanged()),
            this,SLOT(onSceneSelectionChanged()));

    QStringList ids;
    QMapIterator<QString, QViewportItem*> itr(items);
    while(itr.hasNext())
    {
        itr.next();
        QString id = itr.key();
        ids.append(itr.key());
        delete itr.value();
    }
    items.clear();
    zValue = 0.0;

    connect(scene,SIGNAL(selectionChanged()),
            this,SLOT(onSceneSelectionChanged()));
    
    for(int i = 0; i < ids.size(); ++i)
    {
        emit viewportRemoved(ids[i]);
    }
}

// ****************************************************************************
// Method: QvisViewportWidget::getNumberOfViewports
//
// Purpose: 
//   Gets the number of viewports.
//
// Returns:    The number of viewports.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 14:20:35 PST 2006
//
// Modifications:
//   Cyrus Harrison, Fri Nov  7 15:42:13 PST 2008
//   Qt4 Refactor.
//
// ****************************************************************************

int
QvisViewportWidget::getNumberOfViewports() const
{
    return items.count();
}

// ****************************************************************************
// Method: QvisViewportWidget::getActiveViewportId
//
// Purpose: 
//   Returns the id of the active viewport or empty string.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 15:11:57 PST 2006
//
// Modifications:
//   Cyrus Harrison, Fri Nov  7 15:42:13 PST 2008
//   Qt4 Refactor.
//
// ****************************************************************************

QString
QvisViewportWidget::getActiveViewportId() const
{
    QMapIterator<QString, QViewportItem*> itr(items);
    while(itr.hasNext())
    {
        itr.next();
        if(itr.value()->isSelected())
            return itr.key();
    }
    return QString();
}

// ****************************************************************************
// Method: QvisViewportWidget::setActiveViewport
//
// Purpose: 
//   Sets the active viewport.
//
// Arguments:
//   id : The id of the viewport to make active.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 15:10:21 PST 2006
//
// Modifications:
//   Cyrus Harrison, Fri Nov  7 15:42:13 PST 2008
//   Qt4 Refactor.
//
// ****************************************************************************

void
QvisViewportWidget::setActiveViewport(const QString &id)
{
    if(id != getActiveViewportId())
    {
        QMapIterator<QString, QViewportItem*> itr(items);
        while(itr.hasNext())
        {
            itr.next();
            if(itr.key() == id)
            {
                activateItem(itr.value());
                break;
            }
        }
    }
}

// ****************************************************************************
// Method: QvisViewportWidget::getViewport
//
// Purpose: 
//   Get the viewport associated with the id.
//
// Arguments:
//   id : The id of the viewport to return.
//   ... : The viewport coordinates.
//
// Returns:    True on success; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 14:23:16 PST 2006
//
// Modifications:
//   Cyrus Harrison, Fri Nov  7 15:42:13 PST 2008
//   Qt4 Refactor.
//   
// ****************************************************************************

bool
QvisViewportWidget::getViewport(const QString &id, 
                                float &llx, float &lly,
                                float &urx, float &ury) const
{
    bool retval = false;
    
    QMapIterator<QString, QViewportItem*> itr(items);
    while(itr.hasNext())
    {
        itr.next();
        if(itr.key() == id)
        {
            itr.value()->mapRelativeToSize(size(),llx,lly,urx,ury);
            retval = true;
            break;
        }
    }
    return retval;
}


// ****************************************************************************
// Method: QvisViewportWidget::viewportUpdated
//
// Purpose: 
//   Used by Viewport Items to signal an update.
//
//
// Programmer: Cyrus Harrison
// Creation:   Wed Nov  5 13:51:03 PST 2008
//
// Modifications:
//   Cyrus Harrison, Fri Nov  7 15:42:13 PST 2008
//   Qt4 Refactor.
//
// ****************************************************************************

void
QvisViewportWidget::viewportUpdated(QViewportItem *item)
{
    // get values for selected viewport and emit viewportChanged signal
    float llx,lly,urx,ury;
    
    getRelativeSize(item,llx,lly,urx,ury);
    
    emit viewportChanged(item->getId(),llx,lly,urx,ury);
}


// ****************************************************************************
// Method: QvisViewportWidget::getNextId
//
// Purpose: 
//   Gets the id of the next viewport to be created.
//
// Returns:    The new viewport id.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 14:23:59 PST 2006
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 16:29:55 PDT 2008
//   Support for internationalization.
//   
//   Cyrus Harrison, Fri Nov  7 15:42:13 PST 2008
//   Qt4 Refactor.
//
// ****************************************************************************

QString
QvisViewportWidget::getNextId() const
{
    int index = 1;
    bool found;
    QString id;

    do
    {
        id.sprintf(" %d", index);
        id = tr("Viewport") + id;

        found = false;
        QMapIterator<QString, QViewportItem*> itr(items);
        while(itr.hasNext())
        {
            itr.next();
            if(itr.key() == id)
            {
                ++index;
                found = true;
                break;
            }
        }
    } while(found);

    return id;
}

// ****************************************************************************
// Method: QvisViewportWidget::addViewport
//
// Purpose: 
//   Adds a new viewport and returns its id.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 14:24:23 PST 2006
//
// Modifications:
//   Cyrus Harrison, Fri Nov  7 15:42:13 PST 2008
//   Qt4 Refactor.
//
// ****************************************************************************

QString
QvisViewportWidget::addViewport(float llx, float lly, 
                                float urx, float ury)
{
    return addViewport(getNextId(), llx, lly, urx, ury);
}

// ****************************************************************************
// Method: QvisViewportWidget::addViewport
//
// Purpose: 
//   Adds named viewport and returns its name.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 14:24:23 PST 2006
//
// Modifications:
//   Cyrus Harrison, Fri Nov  7 15:42:13 PST 2008
//   Qt4 Refactor.
//
// ****************************************************************************
QString
QvisViewportWidget::addViewport(const QString &id, 
    float llx, float lly, float urx, float ury)
{
    // See if the id is already in use. If it is then get a new id.
    QString the_id = id;

    if(items.contains(the_id))
        the_id = getNextId();
    
    // create new viewport
    QViewportItem *item = new QViewportItem(the_id,llx,lly,urx,ury,this);
    item->setZValue(zValue);
    scene->addItem(item);
    zValue +=1.0;
    
    items[the_id] = item;

    emit viewportAdded(the_id, llx, lly, urx, ury);
    // Make sure that the new item gets activated.
    activateItem(item);

    return the_id;
}

// ****************************************************************************
// Method: QvisViewportWidget::removeViewport
//
// Purpose: 
//   Removes the id'th viewport.
//
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 14:24:43 PST 2006
//
// Modifications:
//   Cyrus Harrison, Fri Nov  7 15:42:13 PST 2008
//   Qt4 Refactor.
//
// ****************************************************************************

void
QvisViewportWidget::removeViewport(const QString &id)
{
    QMapIterator<QString, QViewportItem*> itr(items);
    
    if(items.contains(id))
    {
        QViewportItem *item = items[id];
        items.remove(id);
        // note the id could actually be a const ref to the name inside
        // the item, so make sure to use it BEFORE deleting the item.
        emit viewportRemoved(id);
        delete item;
       
        //if we still have viewports left, activate the first one
        if(!items.isEmpty())
            activateItem(items.values()[0]);
    }
}

// ****************************************************************************
// Method: QvisViewportWidget::getRelativeSize
//
// Purpose: 
//   Sends any viewports shaped like the background to the back so we can
//   see the smaller viewports that should be drawn on top of them.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 2 12:13:41 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void QvisViewportWidget::getRelativeSize(QViewportItem *item,
                                         float &llx, float &lly,
                                         float &urx, float &ury)
{
    QSize curr_size = QSize((int)scene->width(),(int)scene->height());
    item->mapRelativeToSize(curr_size,llx,lly,urx,ury);
}

// ****************************************************************************
// Method: QvisViewportWidget::activateItem
//
// Purpose: 
//   Activates the specified viewport.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 14:26:51 PST 2006
//
// Modifications:
//   Cyrus Harrison, Fri Nov  7 15:42:13 PST 2008
//   Qt4 Refactor.
//
// ****************************************************************************

void
QvisViewportWidget::activateItem(QViewportItem *obj)
{
    if(obj == NULL)
        return;
        
    QMapIterator<QString, QViewportItem*> itr(items);
    while(itr.hasNext())
    {
        itr.next();
        itr.value()->setSelected(false);
    }
    if(!obj->isBackground())
        obj->setSelected(true);
    emit viewportActivated(obj->getId());
}


// ****************************************************************************
// Method: QvisViewportWidget::mousePressEvent
//
// Purpose: 
//   Handles mouse press events.
//
// Arguments:
//   e : The mouse press event.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 14:27:11 PST 2006
//
// Modifications:
//    Cyrus Harrison, Fri Nov  7 10:36:14 PST 2008
//    Qt4 Refactor.
//
// ****************************************************************************

void
QvisViewportWidget::mousePressEvent(QMouseEvent* e)
{
    // deselect all viewports!
    QMapIterator<QString,QViewportItem*> itr(items);
    while(itr.hasNext())
    {
        itr.next();
        itr.value()->setSelected(false);
    }
    
    // w/ right click delete, this could actually 
    // remove an item, so make sure not to use "itr"
    // after this
    QGraphicsView::mousePressEvent(e);
    
    // if no viewports or the "background viewport" is selected, start 
    // drawing of rubber band region for new viewport
    
    bool selected = false;
    
    QMapIterator<QString,QViewportItem*> itr2(items);
    while(itr2.hasNext() && ! selected)
    {
        itr2.next();
        QViewportItem *item = itr2.value();
        if(item->isSelected())
        {
            if(item->isBackground())
                item->setSelected(false);
            else
                selected = true;
        }
    }
    
    if(e->button() == Qt::LeftButton && !selected)
    {
        dragViewportOutline = true;
        dragMouseStart = e->posF();
    }
}

// ****************************************************************************
// Method: QvisViewportWidget::mouseMoveEvent
//
// Purpose: 
//   Handles mouse move events.
//
// Arguments:
//   e : The mouse event.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 14:27:11 PST 2006
//
// Modifications:
//    Cyrus Harrison, Fri Nov  7 10:36:14 PST 2008
//    Qt4 Refactor.
//
// ****************************************************************************

void
QvisViewportWidget::mouseMoveEvent(QMouseEvent* e)
{
    QGraphicsView::mouseMoveEvent(e);
    if(dragViewportOutline)
    {
        // create the outline object when first drag occurs
        if(!viewportOutline)
        {
            viewportOutline = new QGraphicsRectItem();
            viewportOutline->setBrush(Qt::NoBrush);
            viewportOutline->setPen(QPen(Qt::black));
            viewportOutline->setPos(dragMouseStart);
            // make sure the outline is drawn on top of all other items
            viewportOutline->setZValue(1e32);
            scene->addItem(viewportOutline);
        }
        
        // set rect
        float x = dragMouseStart.x();
        float y = dragMouseStart.y();
        float dx = e->posF().x() - x;
        float dy = e->posF().y() - y;
        
        float w = dx;
        float h = dy;
        
        // make sure to keep w/h positive
        
        if(dx < 0) 
        {
            x = x + w;
            w = -dx;
        }

        if(dy < 0)
        {
            y = y + h;
            h = -dy;
        }
        
        viewportOutline->setPos(x,y);
        viewportOutline->setRect(0.0,0.0,w,h);
    }
}

// ****************************************************************************
// Method: QvisViewportWidget::mouseReleaseEvent
//
// Purpose: 
//   Handles mouse release events.
//
// Arguments:
//   e : The mouse event.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 14:27:11 PST 2006
//
// Modifications:
//    Cyrus Harrison, Fri Nov  7 10:36:14 PST 2008
//    Qt4 Refactor.
//
// ****************************************************************************
void
QvisViewportWidget::mouseReleaseEvent(QMouseEvent* e)
{
    QGraphicsView::mouseReleaseEvent(e);
    
    if(dragViewportOutline)
    {
        if(viewportOutline)
        {
            // set a min size for creating a new viewport
            float min_dx = 4;
            if(viewportOutline->rect().width()  > min_dx && 
               viewportOutline->rect().height() > min_dx )
            {
                float w = scene->width();
                float h = scene->height();
        
                float llx = viewportOutline->pos().x() / w;
                float urx = llx + viewportOutline->rect().width() / w;
                float ury = (h - viewportOutline->pos().y()) / h;
                float lly = ury - viewportOutline->rect().height() / h;
                addViewport(llx,lly,urx,ury);
            }
            
            delete viewportOutline;
            viewportOutline = 0;
        }
        
        dragViewportOutline = false;
    }
    
}



// ****************************************************************************
// Method: QvisViewportWidget::keyPressEvent
//
// Purpose: 
//   Handles key press events.
//
// Arguments:
//   e : The key event.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 14:44:37 PST 2006
//
// Modifications:
//   Brad Whitlock, Wed Jun  4 11:06:34 PDT 2008
//   Qt 4.
//
//    Cyrus Harrison, Fri Nov  7 10:36:14 PST 2008
//    Qt4 Refactor.
//
// ****************************************************************************

void
QvisViewportWidget::keyPressEvent(QKeyEvent *e)
{
    QViewportItem *item = 0;
    QViewportItem *next = 0;
    
    QMapIterator<QString,QViewportItem*> itr(items);
    while(itr.hasNext())
    {
        itr.next();
        if(next == 0)
            next = itr.value();
        
        if(itr.value()->isSelected())
        {
            item = itr.value();
            if(itr.hasNext())
            {
                itr.next();
                next = itr.value();
            }
            break;
        }
    }
    
    if(item)
    {
        float dx = 0;
        float dy = 0;
        
        bool shiftApplied = false;
        if((e->modifiers() & Qt::ShiftModifier) > 0)
            shiftApplied = true;

        switch(e->key())
        {
            case Qt::Key_Left:
                dx = shiftApplied?-1:-10;
                dy = 0;
                break;
            case Qt::Key_Right:
                dx = shiftApplied?1:10;
                dy = 0;
                break;
            case Qt::Key_Up:
                dx = 0;
                dy = shiftApplied?-1:-10;
                break;
            case Qt::Key_Down:
                dx = 0;
                dy = shiftApplied?1:10;
                break;
            case Qt::Key_Space:
                if(next)
                    setActiveViewport(next->getId());
                break;
        }
        
        if(dx != 0 || dy != 0)
        {
            item->moveBy(dx,dy);
            viewportUpdated(item);
        }
    }
    else
    {
        QGraphicsView::keyPressEvent(e);
    }
}

// ****************************************************************************
// Method: QvisViewportWidget::onSceneSelectionChanged
//
// Purpose: 
//   Translates scene selection event to viewportActivated signal.
//
//
// Programmer: Cyrus Harrison
// Creation:   Fri Sep 29 14:44:37 PST 2006
//
// Modifications:
//
// ****************************************************************************

void QvisViewportWidget::onSceneSelectionChanged()
{
    bool found = false;
    QMapIterator<QString,QViewportItem*> itr(items);
    QString viewport_name = "";
    
    while(!found && itr.hasNext())
    {
        itr.next();
        bool sel = itr.value()->isSelected();
        if(sel)
        {
            viewport_name = itr.key();
            found = true;
        }
    }
    
    if(!found)
    {
        prevSelected = "";
    }
    else if(viewport_name != prevSelected)
    {
        emit viewportActivated(viewport_name);
        prevSelected = viewport_name;
    }
}
