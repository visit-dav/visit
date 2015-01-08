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
*  This code file was originally inspired by the Mantid Project:
*  http://www.mantidproject.org/.
*
*****************************************************************************/

#include "QvisCollapsibleFrame.h"

#include <QVBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QFontMetrics>
#include <QPolygon>
#include <QTimer>

// ****************************************************************************
// Class: QvisTitleBar::QvisTitleBar
//
// Purpose:
//   Creates a title bar that goes across the frame
//
// Programmer: Allen Sanderson
// Creation:   23 July 2014
//
QvisTitleBar::QvisTitleBar(const QString& title, QWidget* parent) :
  QLabel(title, parent), state(Exposed)
{
    setFrameStyle(QFrame::WinPanel);
    setFrameShadow(QFrame::Raised);
}

// ****************************************************************************
// Class: QvisTitleBar::mousePressEvent
//
// Purpose:
//   Captures the mouse events for collapsing or expanding the frame.
//
// Programmer: Allen Sanderson
// Creation:   23 July 2014
//
void QvisTitleBar::mousePressEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        event->accept();

        if( state == Hidden )
            state = Exposed;
        else // if( state == Exposed )
          state = Hidden;

        // Send a signal out that will trigger the associated widget to be
        // exposed or hidden.
        emit showOrHide( state );
    }
    else
    {
        event->ignore();
    }
}

// ****************************************************************************
// Class: QvisTitleBar::paintEvent
//
// Purpose:
//   Creates arrow indicating whether the panel is collapsed or expanded.
//   This methos is called after a mouse event which will updates the state. 
//
// Programmer: Allen Sanderson
// Creation:   23 July 2014
//
void QvisTitleBar::paintEvent(QPaintEvent *event)
{
    // Update the label as normal.
    QLabel::paintEvent(event);

    QFontMetrics fm(this->font());

    int s = (fm.height() - 4);

    // Update the label to have a plus/minus sign and an arrow.
    if (s > 0)
    {
        QPainter painter(this);
        painter.setBrush(QBrush(QColor(Qt::black)));

        int x = this->width() - 2*s;
        int y = (this->height() - s) / 2;

        QPolygon triangle(3);
        
        if( state == Exposed )
        {
            painter.drawText(x - s, y + s*3/4, "-" );

            // Downward pointing arrow
            triangle.setPoint(0, x, y);
            triangle.setPoint(1, x + s, y);
            triangle.setPoint(2, x + s/2, y + s);
        }
        else // if( state == Hidden )
        {
            painter.drawText(x - s, y + s*3/4, "+" );
            
            // Left pointing arrow
            triangle.setPoint(0, x, y);
            triangle.setPoint(1, x, y + s);
            triangle.setPoint(2, x + s, y + s/2);
        }

        painter.drawPolygon(triangle);
    }
}

// ****************************************************************************
// Class: QvisTitleBar::setShow
//
// Purpose:
//   Manually sets the state to show and shows the associated widget
//
// Programmer: Allen Sanderson
// Creation:   23 July 2014
//
void QvisTitleBar::setShow()
{
    state = Exposed;
    emit showOrHide(state);
}

// ****************************************************************************
// Class: QvisTitleBar::setHide
//
// Purpose:
//   Manually sets the state to hide and hides the associated widget
//
// Programmer: Allen Sanderson
// Creation:   23 July 2014
//
void QvisTitleBar::setHide()
{
    state = Hidden;
    emit showOrHide(state);
}

// ****************************************************************************
// Class: QvisCollapsibleFrame::QvisCollapsibleFrame
//
// Purpose:
//   This class creates a titlebar (label) and a frame (layout) that contains
//   a widget that get shown or hidden. 
//
// Programmer: Allen Sanderson
// Creation:   23 July 2014
//
QvisCollapsibleFrame::QvisCollapsibleFrame(const QString& title,
                                           QWidget* parent) :
  QWidget(parent), widget(NULL)
{
    titleBar = new QvisTitleBar(title,this);

    layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    
    layout->addWidget( titleBar);

    // The titlebar will send a signal with state that the frame needs
    // to process, i.e. show or hide the associated widget.
    connect(titleBar, SIGNAL(showOrHide(QvisTitleBar::LayoutState)), this,
            SLOT(showOrHide(QvisTitleBar::LayoutState)));
}

// ****************************************************************************
// Class: QvisCollapsibleFrame::~QvisCollapsibleFrame
//
// Purpose:
//   This class is a label and a layout (panel) for widgets
//
// Programmer: Allen Sanderson
// Creation:   23 July 2014
//
QvisCollapsibleFrame::~QvisCollapsibleFrame()
{
    delete titleBar;
    delete layout;
}

// ****************************************************************************
// Class: QvisCollapsibleFrame::setWidget
//
// Purpose: Sets the widget that will be contained in the layout. Only
//   one widget per layout (panel).
//
// Programmer: Allen Sanderson
// Creation:   23 July 2014
//
void QvisCollapsibleFrame::setWidget(QWidget* w)
{
    widget = w;
    widget->setParent(this);
    
    // Add the widget to the layout.
    layout->addWidget(widget);
}

// ****************************************************************************
// Class: QvisCollapsibleFrame::showOrHide
//
// Purpose:
//   QT slot that shows or hides the widget associated with the frame (layout).
//
// Programmer: Allen Sanderson
// Creation:   23 July 2014
//
void QvisCollapsibleFrame::showOrHide(QvisTitleBar::LayoutState state)
{
    if (!widget)
        return;

    if (state == QvisTitleBar::Exposed)
        widget->show();
    else // if (state == QvisTitleBar::Hidden)
        widget->hide();

    // Now that the widget is exposed or hidden send another signal to
    // update the layout to be the right size.
    emit updateLayout();
}

// ****************************************************************************
// Class: QvisCollapsibleFrame::setShow
//
// Purpose:
//   Manually shows the associated widget and updates the titlebar.
//
// Programmer: Allen Sanderson
// Creation:   23 July 2014
//
void QvisCollapsibleFrame::setShow()
{
    titleBar->setShow();
    showOrHide( QvisTitleBar::Exposed );
}

// ****************************************************************************
// Class: QvisCollapsibleFrame::setHide
//
// Purpose:
//   Manually hides the associated widget and updates the titlebar.
//
// Programmer: Allen Sanderson
// Creation:   23 July 2014
//
void QvisCollapsibleFrame::setHide()
{
    titleBar->setHide();
    showOrHide( QvisTitleBar::Hidden );
}

// ****************************************************************************
// Class: QvisCollapsibleLayout::QvisCollapsibleLayout
//
// Purpose:
//   This class (QWidget) manages the collapsable frames using a layout
//
// Programmer: Allen Sanderson
// Creation:   23 July 2014
//
QvisCollapsibleLayout::QvisCollapsibleLayout(QWidget* parent) : QWidget(parent)
{
    layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    
    // Add the layout to the base widget.
    setLayout(layout);
}

// ****************************************************************************
// Class: QvisCollapsibleLayout::~QvisCollapsibleLayout
//
// Purpose:
//   This class creates layout for managing collapsable frames.
//
// Programmer: Allen Sanderson
// Creation:   23 July 2014
//
QvisCollapsibleLayout::~QvisCollapsibleLayout()
{
    delete layout;
}

// ****************************************************************************
// Class: QvisCollapsibleLayout::addFrame
//
// Purpose:
//   Adds a new collapsable frame to the layout.
//
// Programmer: Allen Sanderson
// Creation:   23 July 2014
//
QvisCollapsibleFrame* QvisCollapsibleLayout::addFrame(const QString& title,
                                                      QWidget* widget)
{
    QvisCollapsibleFrame *frame = new QvisCollapsibleFrame(title, this);
    frame->setWidget(widget);
    
    layout->addWidget(frame);
    
    // The frame will send a signal that the layout needs to be resized.
    connect(frame, SIGNAL(updateLayout()), this, SLOT(updateLayout()));
    
    // Return the frame so the user can manually set the state.
    return frame;
}

// ****************************************************************************
// Class: QvisCollapsibleLayout::updateLayout
//
// Purpose: QT slot that emits a signal via the timer which is
// enough of a delay for the window resizing to happen correctly.
// I.e. there is a timing issue.
//
// Programmer: Allen Sanderson
// Creation:   23 July 2014
//
void QvisCollapsibleLayout::updateLayout()
{
    if (layout->count() == 0)
        return;

    // Use the timer to set up the adjustWindowSize as it can not be
    // called directly.
    QTimer::singleShot(0, this, SLOT(adjustWindowSize()));

    emit adjustWindowSize();
}

// ****************************************************************************
// Class: QvisCollapsibleLayout::adjustWindowSize
//
// Purpose: QT slot that invalidates the layout then forces the parent
// window to resize after a show/hide event.
//
// Programmer: Allen Sanderson
// Creation:   23 July 2014
//
// Modifications:
//   Kathleen Biagas, Wed Aug 13 17:05:23 MST 2014
//   Hack to prevent windows above the plot (or operator) atts windows from
//   being resized.  Prevents main gui resize on Windows.
//
// ****************************************************************************

void
QvisCollapsibleLayout::adjustWindowSize()
{
    // NOTE: the layout invalidate and the adjust size calls must be
    // done as a signal via the QTimer that causes a slight delay
    // otherwise the resizing does not happen properly. I.e. there is
    // a timing issue.

    layout->invalidate();
  
    QWidget *parent = layout->parentWidget();
    while (parent)
    {
        parent->adjustSize();
        if (parent->windowTitle().toStdString().find("attributes") == std::string::npos)
            parent = parent->parentWidget();
        else
            break;
    }
}
