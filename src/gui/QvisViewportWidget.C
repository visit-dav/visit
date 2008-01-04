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
#include <QvisViewportWidget.h>
#include <qcursor.h>
#include <qpen.h>
#include <qcolor.h>
#include <qwmatrix.h>
#include <qpainter.h>

// ****************************************************************************
// Class: QCanvasViewport
//
// Purpose:
//   This class represents viewport objects that are drawn to the canvas.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 14:15:42 PST 2006
//
// Modifications:
//   
// ****************************************************************************

class QCanvasViewport : public QCanvasRectangle
{
public:
    QCanvasViewport(QvisViewportWidget *W, const QString &_id, int x, int y,
        int w, int h, QCanvas *canvas) : QCanvasRectangle(x, y, w, h, canvas)
    {
        id = _id;
        viewportWidget = W;
    }

    virtual ~QCanvasViewport()
    {
    }

    int rtti () const { return RTTI; }
    static const int RTTI;
protected:
    QString id;
    QvisViewportWidget *viewportWidget;

    // Override shape drawing so we can draw text in the viewport.
    void drawShape(QPainter &painter)
    {
        QString name(id);

        // If the string is too wide for the box then use just the number.
        int W = rect().width();
        W = (W < 0) ? -W : W;
        if(viewportWidget->fontMetrics().boundingRect(name).width() > W)
        {
            // See if we can shorten the name.
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
        }

        if(isActive())
        {
            QBrush newBrush(Qt::SolidPattern);
            newBrush.setColor(viewportWidget->colorGroup().highlight());
            painter.setBrush(newBrush);
            QCanvasRectangle::drawShape(painter);

            painter.setPen(viewportWidget->colorGroup().highlightedText());
            painter.drawText(rect(),
                             Qt::AlignHCenter | Qt::AlignVCenter,
                             name);
            painter.setPen(pen());
        }
        else
        {
            QCanvasRectangle::drawShape(painter);

            painter.drawText(rect(),
                             Qt::AlignHCenter | Qt::AlignVCenter,
                             name);
        }
    }
};

const int QCanvasViewport::RTTI = 123456789;


// ****************************************************************************
// Method: QvisViewportWidget::QvisViewportWidget
//
// Purpose: 
//   Constructor
//
// Arguments:
//   _aspect : A floating point number (height/width) that describes the shape
//             of the viewport.
//   minw    : The minimum widget width.
//   minh    : The minimum widget height.
//   parent  : The parent for the widget.
//   name    : The name of the widget.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 14:16:21 PST 2006
//
// Modifications:
//   
// ****************************************************************************

QvisViewportWidget::QvisViewportWidget(double _aspect, int minw, int minh, 
    QWidget *parent, const char *name) : QWidget(parent,name, 
    Qt::WRepaintNoErase|Qt::WResizeNoErase), viewportMap(), moving_start()
{
    minW = minw;
    minH = minh;
    aspect = _aspect; // height / width

    init();
}

QvisViewportWidget::QvisViewportWidget(QWidget *parent, const char *name) :
    QWidget(parent,name, Qt::WRepaintNoErase|Qt::WResizeNoErase), 
    viewportMap(), moving_start()
{
    minW = 200;
    minH = 200;
    aspect = 1.; // height / width

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
//   
// ****************************************************************************

void
QvisViewportWidget::init()
{
    item = 0;
    createitem = 0;
    mouseDown = false;
    canvas = 0;
    doContinuousUpdates = false;
    shiftApplied = false;
    zValue = 0.;

    // Set a min size.
    setMinimumSize(minW, minH);

    // Create a canvas with the right aspect ratio that will fit in the widget.
    int w = minW - 20;
    int h = int(aspect * w);
    if(h > minH)
    {
        h = minH - 20;
        w  = int(h / aspect);
    }
    canvas = new QCanvas(w, h);
    canvas->setBackgroundColor(QColor(255,255,255));

    // Turn on mouse tracking.
    cursorResizeMode = -1;
    setMouseTracking(true);

    // Set the focus policy to StrongFocus. This means that the widget will
    // accept focus by tabbing and clicking.
    setFocusPolicy(StrongFocus);
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
//   
// ****************************************************************************

QvisViewportWidget::~QvisViewportWidget()
{
    delete canvas;
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
// Method: QvisViewportWidget::setContinuousUpdates
//
// Purpose: 
//   Sets whether signals will be emitted when moving or resizing viewports.
//
// Arguments:
//   val : The new update flag.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 14:19:33 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisViewportWidget::setContinuousUpdates(bool val)
{
    doContinuousUpdates = val;
}

bool
QvisViewportWidget::continuousUpdates() const
{
    return doContinuousUpdates;
}

// ****************************************************************************
// Method: QvisViewportWidget::clear
//
// Purpose: 
//   Removes all of the viewports.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 14:20:07 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisViewportWidget::clear()
{
    QCanvasItemViewportInfoMap::Iterator it = viewportMap.begin();
    for(; it != viewportMap.end(); ++it)
    {
        //qDebug("viewportRemoved(%d);", it.data().id);
        QString id = it.data().id;

        //qDebug("deleting 0x%08p\n", it.key());
        delete it.key();

        emit viewportRemoved(id);
    }
    viewportMap.clear();
    zValue = 0.;
    update();
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
//   
// ****************************************************************************

int
QvisViewportWidget::getNumberOfViewports() const
{
    return viewportMap.count();
}

// ****************************************************************************
// Method: QvisViewportWidget::getActiveViewportId
//
// Purpose: 
//   Returns the id of the active viewport or -1.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 15:11:57 PST 2006
//
// Modifications:
//   
// ****************************************************************************

QString
QvisViewportWidget::getActiveViewportId() const
{
    QCanvasItemViewportInfoMap::ConstIterator it = viewportMap.begin();
    for(; it != viewportMap.end(); ++it)
    {
        if(it.key()->isActive())
            return it.data().id;
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
//   
// ****************************************************************************

void
QvisViewportWidget::setActiveViewport(const QString &id)
{
    if(id != getActiveViewportId())
    {
        QCanvasItemViewportInfoMap::ConstIterator it = viewportMap.begin();
        for(; it != viewportMap.end(); ++it)
        {
            if(it.data().id == id)
            {
                activateItem(it.key());
                update();

                //qDebug("activateViewport(%d);", id);
                emit viewportActivated(id);
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
//   
// ****************************************************************************

bool
QvisViewportWidget::getViewport(const QString &id, 
    float &llx, float &lly, float &urx, float &ury) const
{
    bool retval = false;
    QCanvasItemViewportInfoMap::ConstIterator it = viewportMap.begin();
    for(; it != viewportMap.end(); ++it)
    {
        if(it.data().id == id)
        {
            llx = it.data().lower_left[0];
            lly = it.data().lower_left[1];
            urx = it.data().upper_right[0];
            ury = it.data().upper_right[1];

            retval = true;
            break;
        }
    }

    return retval;
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
        id.sprintf("Viewport %d", index);

        found = false;
        QCanvasItemViewportInfoMap::ConstIterator it = viewportMap.begin();
        for(; it != viewportMap.end(); ++it)
        {
            if(it.data().id == id)
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
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 14:24:23 PST 2006
//
// Modifications:
//   
// ****************************************************************************

QString
QvisViewportWidget::addViewport(float llx, float lly, float urx, float ury)
{
    return addViewport(getNextId(), llx, lly, urx, ury);
}

QString
QvisViewportWidget::addViewport(const QString &id, 
    float llx, float lly, float urx, float ury)
{
    int rx = int(llx * canvas->width());
    int ry = int((1. - ury) * canvas->height());
    int w = int((urx - llx) * canvas->width());
    int h = int((ury - lly) * canvas->height());

    // See if the id is already in use. If it is then get a new id.
    QString newid(id);
    QCanvasItemViewportInfoMap::ConstIterator it = viewportMap.begin();
    for(; it != viewportMap.end(); ++it)
    {
        if(it.data().id == newid)
        {
            newid = getNextId();
            break;
        }
    }

    // Create a new canvas object for the new viewport.
    QCanvasViewport *v = new QCanvasViewport(this, id, rx, ry, w, h, canvas);
    QBrush brush(Qt::SolidPattern);
    brush.setColor(QColor(0,255,255));
    v->setBrush(brush);
    v->setZ(zValue);
    v->show();

    zValue = zValue + 1.;

    // Add a new item to the viewport map.
    ViewportInfo info;
    info.lower_left[0] = llx;
    info.lower_left[1] = lly;
    info.upper_right[0] = urx;
    info.upper_right[1] = ury;
    info.id = id;
    viewportMap[v] = info;

    // Make sure that the new item gets activated.
    activateItem(v);

    // update the widget.
    update();

    //qDebug("viewportAdded(%d, %g, %g, %g, %g);", info.id, llx, lly, urx, ury);
    emit viewportAdded(info.id, llx, lly, urx, ury);

    //qDebug("activateViewport(%d);", info.id);
    emit viewportActivated(info.id);

    return info.id;
}

// ****************************************************************************
// Method: QvisViewportWidget::removeViewport
//
// Purpose: 
//   Removes the id'th viewport.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 14:24:43 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisViewportWidget::removeViewport(const QString &id)
{
    QCanvasItemViewportInfoMap::Iterator it = viewportMap.begin();
    for(; it != viewportMap.end(); ++it)
    {
        if(it.data().id == id)
        {
            delete it.key();
            viewportMap.erase(it);

            // Make the first item active.
            QString newId;
            it = viewportMap.begin();
            if(it != viewportMap.end())
            {
                activateItem(it.key());
                newId = it.data().id;
            }

            // update the widget.
            update();

            //qDebug("viewportRemoved(%d);", id);
            emit viewportRemoved(id);

            // Tell clients that a new viewport is active.
            if(!newId.isNull())
                emit viewportActivated(newId);

            break;
        }
    }
}

// ****************************************************************************
// Method: QvisViewportWidget::sendBackgroundToBack
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

void
QvisViewportWidget::sendBackgroundToBack()
{
    bool needUpdate = false;
    QCanvasItemViewportInfoMap::Iterator it = viewportMap.begin();
    for(; it != viewportMap.end(); ++it)
    {
        if(it.data().lower_left[0] == 0. &&
           it.data().lower_left[1] == 0. &&
           it.data().upper_right[0] == 1. &&
           it.data().upper_right[1] == 1.)
        {
            sendToBack(it.key());
            needUpdate = true;
        }
    }

    if(needUpdate)
        update();
}

// ****************************************************************************
// Method: QvisViewportWidget::paintEvent
//
// Purpose: 
//   Handles paint events.
//
// Arguments:
//   pe : The paint event.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 14:25:05 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisViewportWidget::paintEvent(QPaintEvent *e)
{
    // Iterate through the objects and move and scale them.
    QPainter paint(this);

    // Figure out the matrix needed to make the canvas draw in the center
    // of the widget.
    QWMatrix m;
    QPoint co(canvasOrigin());
    m.translate(co.x(), co.y());
    paint.setWorldMatrix(m, false);

    // Draw the canvas.
    canvas->drawArea(canvas->rect(), &paint, true);
}

// ****************************************************************************
// Method: QvisViewportWidget::canvasOrigin
//
// Purpose: 
//   Returns the origin of the canvas within the widget.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 14:25:25 PST 2006
//
// Modifications:
//   
// ****************************************************************************

QPoint
QvisViewportWidget::canvasOrigin() const
{
    int tx = int((width() - canvas->width()) / 2);
    int ty = int((height() - canvas->height()) / 2);
    return QPoint(tx, ty);
}

// ****************************************************************************
// Method: QvisViewportWidget::findViewportId
//
// Purpose: 
//   Finds the viewport
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 14:25:45 PST 2006
//
// Modifications:
//   
// ****************************************************************************

QString
QvisViewportWidget::findViewportId(QCanvasItem *val) const
{
    QString ret;
    QCanvasItemViewportInfoMap::ConstIterator it = viewportMap.find(val);
    if(it != viewportMap.end())
        ret = it.data().id;
 
    return ret;
}

// ****************************************************************************
// Method: QvisViewportWidget::activateItem
//
// Purpose: 
//   Activates the specified viewport.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 14:26:51 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisViewportWidget::activateItem(QCanvasItem *obj)
{
    if(obj != 0)
    {
        // Turn off all other items.
        QCanvasItemList l=canvas->allItems();
        for (QCanvasItemList::Iterator it=l.begin(); it!=l.end(); ++it)
            (*it)->setActive(false);

        obj->setActive(true);
    }
}

// ****************************************************************************
// Method: QvisViewportWidget::setAspect
//
// Purpose: 
//   Sets the aspect of the canvas used to show the viewports.
//
// Arguments:
//   newAspect : The aspect ratio used to show the canvas.
//
// Returns:    
//
// Note:       Aspect is height / width.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 2 11:26:20 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisViewportWidget::setAspect(double newAspect)
{
    if(aspect == newAspect)
        return;

    aspect = newAspect;

    // Save the values from the old map and clear it out, deleting its
    // viewport objects.
    QCanvasItemViewportInfoMap oldMap(viewportMap);
    QCanvasItemViewportInfoMap::Iterator it = viewportMap.begin();
    QString activeId;
    for(; it != viewportMap.end(); ++it)
    {
        if(it.key()->isActive())
            activeId = it.data().id;
        delete it.key();
    }
    viewportMap.clear();

    // Create a new canvas
    delete canvas;
    int w = width() - 20;
    int h = int(aspect * w);
    if(h > height())
    {
        h = height() - 20;
        w  = int(h / aspect);
    }
    canvas = new QCanvas(w, h);
    canvas->setBackgroundColor(QColor(255,255,255));

    // Now, add new viewports to the canvas.
    setUpdatesEnabled(false);
    blockSignals(true);
    for(it = oldMap.begin(); it != oldMap.end(); ++it)
    {
        addViewport(it.data().id,
            it.data().lower_left[0],
            it.data().lower_left[1],
            it.data().upper_right[0],
            it.data().upper_right[1]);
    }
    for(it = viewportMap.begin(); it != viewportMap.end(); ++it)
    {
        if(it.data().id == activeId)
        {
            activateItem(it.key());
            break;
        }
    }
    blockSignals(false);
    setUpdatesEnabled(true);

    update();    
}

// ****************************************************************************
// Method: QvisViewportWidget::sendToBack
//
// Purpose: 
//   Sends the specified viewport to the back
//
// Arguments:
//   obj : The viewport canvas item.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 2 11:53:31 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisViewportWidget::sendToBack(QCanvasItem *obj)
{
    double minZ = 1.e10;
    QCanvasItemViewportInfoMap::ConstIterator q = viewportMap.begin();
    for(; q != viewportMap.end(); ++q)
    {
        if(q.key()->z() < minZ)
            minZ = q.key()->z();
    }
    if(obj->z() > minZ)
    {
        for(q = viewportMap.begin(); q != viewportMap.end(); ++q)
        {
            q.key()->setZ(q.key()->z() + 1.);
        }
        obj->setZ(minZ);
        //qDebug("Lowering");
    }
}

// ****************************************************************************
// Method: QvisViewportWidget::bringToFront
//
// Purpose: 
//   Brings the specified viewport to the front.
//
// Arguments:
//   obj : The viewport canvas item.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 2 11:53:00 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisViewportWidget::bringToFront(QCanvasItem *obj)
{
    // Raise the item. We have to give it the
    // highest z value to do so. That means we need to search
    // canvas items and give this item the highest z value.
    double maxZ = -1.e10;
    QCanvasItemViewportInfoMap::ConstIterator q = viewportMap.begin();
    for(; q != viewportMap.end(); ++q)
    {
        if(q.key()->z() > maxZ)
            maxZ = q.key()->z();
    }
    // Set the new max Z.
    maxZ = maxZ + 1.;
    obj->setZ(maxZ);
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
//   
// ****************************************************************************

void
QvisViewportWidget::mousePressEvent(QMouseEvent* e)
{
    if((e->button() & QMouseEvent::LeftButton) != 0)
    {
        //qDebug("press left button");
        mouseDown = true;
    }
    else if((e->button() & QMouseEvent::MidButton) != 0)
    {
        //qDebug("press middle button");
    }
    else if((e->button() & QMouseEvent::RightButton) != 0)
    {
        //qDebug("press right button");
    }
    else
    {
        //qDebug("no button");
        item = 0;
        mouseDown = false;
        return;
    }

    // Get the coordinate of the point in the canvas.
    QPoint p(e->pos() - canvasOrigin());
    //QPoint p(e->pos());

    // If we have no cursor resize mode then look for the item under
    // the cursor.
    if(cursorResizeMode == -1)
    {
        QCanvasItemList l=canvas->collisions(p);
        for (QCanvasItemList::Iterator it=l.begin(); it!=l.end(); ++it)
        {
            bool returnOkay = true;

            if((e->button() & Qt::LeftButton) != 0)
            {
                // See if the viewport that we clicked on is the background.
                bool backgroundViewport = false;
                QCanvasItemViewportInfoMap::ConstIterator obj;
                if((obj = viewportMap.find(*it)) != viewportMap.end())
                {
                    backgroundViewport = obj.data().lower_left[0] == 0. &&
                        obj.data().lower_left[1] == 0. &&
                        obj.data().upper_right[0] == 1. &&
                        obj.data().upper_right[1] == 1.;
                }
              
                // If it is not the background viewport then remember it
                // so we can move it.
                if(!backgroundViewport)
                {
                    item = *it;
                    moving_start = p;
                }
                else
                {
                    // Send the viewport to the back since it's the 
                    // background.
                    sendToBack(*it);
                    item = 0;

                    // This lets us draw new viewports on top of the
                    // background viewport.
                    returnOkay = false;
                }

                // Make sure that the item gets activated.
                activateItem(*it);

                // Redraw the widget
                update();

                QString id = findViewportId(*it);
                if(!id.isNull())
                {
                    //qDebug("activateViewport(%d);", id);
                    emit viewportActivated(id);
                }
            }
            else if((e->button() & Qt::MidButton) != 0)
            {
                item = 0;

                if((e->state() & Qt::ShiftButton) != 0)
                {
                    // Send to back.
                    sendToBack(*it);
                }
                else
                {
                    // Raise the item that we clicked on.
                    bringToFront(*it);
                }

                // Make sure that the item gets activated.
                activateItem(*it);

                // Redraw the widget
                update();

                QString id = findViewportId(*it);
                if(!id.isNull())
                {
                    //qDebug("activateViewport(%d);", id);
                    emit viewportActivated(id);
                }
            }
            else if((e->button() & Qt::RightButton) != 0)
            {
                removeViewport(findViewportId(*it));
                item = 0;
            }

            if(returnOkay)
                return;
        }

        // We did not click on any existing viewport so create a new one.
        item = 0;
        if((e->button() & QMouseEvent::LeftButton) != 0)
        {
            createitem = new QCanvasRectangle(p.x(), p.y(), 2, 2, canvas);
            createitem->setZ(zValue);
            zValue = zValue + 1;
            createitem->show();
            bringToFront(createitem);

            setCursor(QCursor(Qt::ArrowCursor));

            moving_start = p;
            cursorResizeMode = 2;

            // Show the new viewport.
            update();
        }
    }
    else
    {
        moving_start = p;

        // We're doing the initial button press for a resize operation.
        // Make sure that we make the viewport active.
        QString id = findViewportId(item);
        if(!id.isNull())
        {
            activateItem(item); 

            // Redraw the widget
            update();

            //qDebug("activateViewport(%d);", id);
            emit viewportActivated(id);
        }
    }
}

#define MIN_VAL(A, B) ((A < B) ? A : B)
#define MAX_VAL(A, B) ((A > B) ? A : B)

void
QRectToViewportCoords(QCanvas *canvas, const QRect &r,
    float &llx, float &lly, float &urx, float &ury)
{
    int x0 = r.x();
    int y0 = r.y();
    int x1 = x0 + r.width();
    int y1 = y0 + r.height();

    float fx0 = float(x0) / float(canvas->width());
    float fy0 = float(y0) / float(canvas->height());

    float fx1 = float(x1) / float(canvas->width());
    float fy1 = float(y1) / float(canvas->height());

    float miny  = MIN_VAL(fy0, fy1);
    float maxy = MAX_VAL(fy0, fy1);

    llx = MIN_VAL(fx0, fx1);
    lly = 1. - maxy;
    urx = MAX_VAL(fx0, fx1);
    ury = 1. - miny;
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
//   
// ****************************************************************************

void
QvisViewportWidget::mouseReleaseEvent(QMouseEvent* e)
{
    //qDebug("mouse release");
    mouseDown = false;
    setCursor(QCursor(Qt::ArrowCursor));
    cursorResizeMode = -1;

    // Get the coordinate of the point in the canvas.
    //QPoint p(worldMatrixInverse().map(e->pos()));

    if(createitem != 0)
    {
        QCanvasRectangle *cr = (QCanvasRectangle *)createitem;
        float llx, lly, urx, ury;
        QRectToViewportCoords(canvas, cr->rect(), llx, lly, urx, ury);

        // Remove the current item.
        delete createitem;
        createitem = 0;

        // Only create the viewport if it is large enough. This size 
        // criterion lets us click on the background viewport without
        // creating a new viewport if we happen to move the mouse a little.
        if(urx - llx > 0.02 &&
           ury - lly > 0.02)
        {
            // Add a new viewport.
            addViewport(llx, lly, urx, ury);
        }
        else
            update();
    }
    else if(item != 0)
    {
        // Look for the item in the viewportMap and update it
        QCanvasItemViewportInfoMap::Iterator it;
        it = viewportMap.find(item);
        if(it != viewportMap.end())
        {
            //qDebug("viewportChanged(%d, %g, %g, %g, %g);", 
                   //it.data().id, 
                   //it.data().lower_left[0], it.data().lower_left[1],
                   //it.data().upper_right[0], it.data().upper_right[1]);

            emit viewportChanged(it.data().id, 
                   it.data().lower_left[0], it.data().lower_left[1],
                   it.data().upper_right[0], it.data().upper_right[1]);
        }
    }

    item = 0;
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
//   
// ****************************************************************************

void
QvisViewportWidget::mouseMoveEvent(QMouseEvent* e)
{
    // Get the coordinate of the point in the canvas.
    QPoint p(e->pos() - canvasOrigin());
    //QPoint p(e->pos());

    if(mouseDown)
    {
        QCanvasItem *itemOfInterest = (createitem != 0) ? createitem : item;
        if (itemOfInterest != 0)
        {
            if(cursorResizeMode == -1)
            {
                itemOfInterest->moveBy(p.x() - moving_start.x(),
                                       p.y() - moving_start.y());
            }
            else if(cursorResizeMode == 0)
            {
                //qDebug("resize BL");
                QCanvasRectangle *cr = (QCanvasRectangle *)itemOfInterest;
                int W = cr->width() + moving_start.x() - p.x();
                int H = p.y() - cr->rect().y();
                cr->move(p.x(), cr->rect().y());
                cr->setSize(W, H);
            }
            else if(cursorResizeMode == 1)
            {
                //qDebug("resize B");
                QCanvasRectangle *cr = (QCanvasRectangle *)itemOfInterest;
                int H = p.y() - cr->rect().y();
                cr->setSize(cr->width(), H);
            }
            else if(cursorResizeMode == 2)
            {
                //qDebug("resize BR");
                QCanvasRectangle *cr = (QCanvasRectangle *)itemOfInterest;
                int W = p.x() - cr->rect().x();
                int H = p.y() - cr->rect().y();
                cr->setSize(W, H);
            }
            else if(cursorResizeMode == 3)
            {
                //qDebug("resize R");
                QCanvasRectangle *cr = (QCanvasRectangle *)itemOfInterest;
                int W = p.x() - cr->rect().x();
                cr->setSize(W, cr->height());
            }
            else if(cursorResizeMode == 4)
            {
                //qDebug("resize TR");
                QCanvasRectangle *cr = (QCanvasRectangle *)itemOfInterest;
                int W = p.x() - cr->rect().x();
                int H = cr->height() + moving_start.y() - p.y();
                cr->move(cr->rect().x(), p.y());
                cr->setSize(W, H);
            }
            else if(cursorResizeMode == 5)
            {
                //qDebug("resize T");
                QCanvasRectangle *cr = (QCanvasRectangle *)itemOfInterest;
                cr->move(cr->rect().x(), p.y());
                int H = cr->height() + moving_start.y() - p.y();
                cr->setSize(cr->width(), H);
            }
            else if(cursorResizeMode == 6)
            {
                //qDebug("resize TL");
                QCanvasRectangle *cr = (QCanvasRectangle *)itemOfInterest;
                int W = cr->width() + moving_start.x() - p.x();
                int H = cr->height() + moving_start.y() - p.y();
                cr->move(p.x(), p.y());
                cr->setSize(W, H);
            }
            else if(cursorResizeMode == 7)
            {
                //qDebug("resize L");
                QCanvasRectangle *cr = (QCanvasRectangle *)itemOfInterest;
                cr->move(p.x(), cr->rect().y());
                int W = cr->width() + moving_start.x() - p.x();
                cr->setSize(W, cr->height());
            }

            // Look for the item in the viewportMap and update it
            QCanvasItemViewportInfoMap::Iterator it;
            it = viewportMap.find(item);
            if(it != viewportMap.end())
            {
                QCanvasRectangle *cr = (QCanvasRectangle *)item;
                float llx, lly, urx, ury;
                QRectToViewportCoords(canvas, cr->rect(), llx, lly, urx, ury);

                it.data().lower_left[0] = llx;
                it.data().lower_left[1] = lly;
                it.data().upper_right[0] = urx;
                it.data().upper_right[1] = ury;

                if(doContinuousUpdates)
                {
                    //qDebug("viewportChanged(%d, %g, %g, %g, %g);", 
                           //it.data().id, llx, lly, urx, ury);
                    emit viewportChanged(it.data().id, llx, lly, urx, ury);
                }
            }

            moving_start = p;
            update();
        }
    }
    else if(canvas->rect().contains(p))
    {
        // Compare the point against all of the rectangle canvas items
        // so we can adjust the cursor to resize if necessary.
        bool cursorSet = false;
        QCanvasItemList l = canvas->collisions(p);
        for (QCanvasItemList::Iterator it=l.begin(); it!=l.end(); ++it)
        {
            if((*it)->rtti() == QCanvasRectangle::RTTI ||
               (*it)->rtti() == QCanvasViewport::RTTI)
            {
                QCanvasRectangle *cr = (QCanvasRectangle *)*it;
                QRect r(cr->rect());
                int lp = 2;
                int W = r.width();
                int H = r.height();
                QRect left(r.x() - lp, r.y() - lp,
                           2*lp+1, H+2*lp);
                QRect right(r.x() + W - lp, r.y() - lp,
                            2*lp+1, H+2*lp);
                // Reverse top,bottom so the cursor resize will think we have 
                // 0,0 in the lower left.
                QRect top(r.x() - lp, r.y() - lp, W+2*lp, 1+2*lp);
                QRect bottom(r.x() - lp, r.y() + H - lp, W+2*lp, 1+2*lp);

                int L = left.contains(p)?1:0;
                int R = right.contains(p)?1:0;
                int B = bottom.contains(p)?1:0;
                int T = top.contains(p)?1:0;
                int index = (L<<3) | (R<<2) | (B<<1) | T;
                const int cursor_lookup[] = {-1,5,1,-1,3,4,2,-1,7,6,0,-1,-1,-1,-1,-1};

                if(cursor_lookup[index] == 0 || cursor_lookup[index] == 4)
                {
                    setCursor(QCursor(Qt::SizeBDiagCursor));
                    cursorSet = true;
                    cursorResizeMode = cursor_lookup[index];
                    item = *it;
                    break;
                }
                else if(cursor_lookup[index] == 1 || cursor_lookup[index] == 5)
                {
                    setCursor(QCursor(Qt::SizeVerCursor));
                    cursorSet = true;
                    cursorResizeMode = cursor_lookup[index];
                    item = *it;
                    break;
                }
                else if(cursor_lookup[index] == 2 || cursor_lookup[index] == 6)
                {
                    setCursor(QCursor(Qt::SizeFDiagCursor));
                    cursorSet = true;
                    cursorResizeMode = cursor_lookup[index];
                    item = *it;
                    break;
                }
                else if(cursor_lookup[index] == 3 || cursor_lookup[index] == 7)
                {
                    setCursor(QCursor(Qt::SizeHorCursor));
                    cursorSet = true;
                    cursorResizeMode = cursor_lookup[index];
                    item = *it;
                    break;
                }
            }
        }

        if(!cursorSet)
        {
            // We're not resizing but we could still be over something
            // that we want to move.
            if(cursorResizeMode == -1)
            {
                for (QCanvasItemList::Iterator it=l.begin(); it!=l.end(); ++it)
                {
                    if((*it)->rtti() == QCanvasRectangle::RTTI ||
                       (*it)->rtti() == QCanvasViewport::RTTI)
                    {
                        QCanvasRectangle *cr = (QCanvasRectangle *)*it;
                        QRect r(cr->rect());
                        if(r.contains(p))
                        {
                            setCursor(QCursor(Qt::PointingHandCursor));
                            cursorSet = true;
                            break;
                        }
                    }
                }
            }

            if(!cursorSet)
                setCursor(QCursor(Qt::ArrowCursor));

            cursorResizeMode = -1;
        }
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
//   
// ****************************************************************************

void
QvisViewportWidget::keyPressEvent(QKeyEvent *e)
{
    // Figure the currently selected viewport.
    QCanvasItem *viewport = 0;
    QCanvasItemList l=canvas->allItems();
    for (QCanvasItemList::Iterator it=l.begin(); it!=l.end(); ++it)
    {
        if((*it)->isActive())
        {
            viewport = *it;
            break;
        }
    }

    if(viewport != 0)
    {
        QPoint vec;
        bool move = false;

        switch(e->key())
        {
        case Key_Shift:
            //qDebug("key shift");
            shiftApplied = true;
            break;
        case Key_Left:
            //qDebug("key left");
            move = true;
            vec = QPoint(shiftApplied?-1:-10,0);
            break;
        case Key_Right:
            //qDebug("key right");
            move = true;
            vec = QPoint(shiftApplied?1:10,0);
            break;
        case Key_Up:
            //qDebug("key up");
            move = true;
            vec = QPoint(0, shiftApplied?-1:-10);
            break;
        case Key_Down:
            //qDebug("key down");
            move = true;
            vec = QPoint(0,shiftApplied?1:10);
            break;
        case Key_Space:
            //qDebug("key space");
            // Find the next item in the list and make it active
            if(viewportMap.begin() != viewportMap.end())
            {
                QCanvasItemViewportInfoMap::Iterator it = viewportMap.find(viewport);
                if(it != viewportMap.end())
                {
                    // Go to the next one.
                    QString currentId = it.data().id;
                    ++it;
                    if(it == viewportMap.end())
                        it = viewportMap.begin();
                    QString nextId = it.data().id;
                    if(currentId != nextId)
                    {
                        activateItem(it.key());
                        update();
                        //qDebug("activateViewport(%d);", nextId);
                        emit viewportActivated(nextId);
                    }
                }
            }
        }

        if(move)
        {
            QCanvasItemViewportInfoMap::Iterator it = viewportMap.find(viewport);
            if(it != viewportMap.end())
            {
                QCanvasRectangle *cr = (QCanvasRectangle *)viewport;
                cr->moveBy(vec.x(), vec.y());

                float llx, lly, urx, ury;
                QRectToViewportCoords(canvas, cr->rect(), llx, lly, urx, ury);

                it.data().lower_left[0] = llx;
                it.data().lower_left[1] = lly;
                it.data().upper_right[0] = urx;
                it.data().upper_right[1] = ury;

                update();

                //qDebug("viewportChanged(%d, %g, %g, %g, %g);", 
                       //it.data().id, llx, lly, urx, ury);
                emit viewportChanged(it.data().id, llx, lly, urx, ury);
            }
        }
    }
}

// ****************************************************************************
// Method: QvisViewportWidget::keyReleaseEvent
//
// Purpose: 
//   Handles key releases.
//
// Arguments:
//   e : The key event.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 15:03:52 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisViewportWidget::keyReleaseEvent(QKeyEvent *e)
{
    if(e->key() == Key_Shift)
        shiftApplied = false;
}
