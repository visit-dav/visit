// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QvisTurnDownButton.h>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionHeader>

// ****************************************************************************
// Method: QvisTurnDownButton::QvisTurnDownButton
//
// Purpose: 
//   Constructor for the QvisTurnDownButton class.
//
// Arguments:
//   parent : The widget's parent.
//   name   : The widget's name.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 13:57:16 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Jun  3 16:13:20 PDT 2008
//   Qt 4.
//
// ****************************************************************************

QvisTurnDownButton::QvisTurnDownButton(QWidget *parent) : 
    QPushButton(parent)
{
    setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum));
}

// ****************************************************************************
// Method: QvisTurnDownButton::~QvisTurnDownButton
//
// Purpose: 
//   Destructor for the QvisTurnDownButton class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 13:57:59 PST 2003
//
// Modifications:
//   
// ****************************************************************************

QvisTurnDownButton::~QvisTurnDownButton()
{
}

// ****************************************************************************
// Method: QvisTurnDownButton::drawButtonLabel
//
// Purpose: 
//   Draws the arrow instead of a button label.
//
// Arguments:
//   painter : The painter to use to draw the arrow.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 13:58:19 PST 2003
//
// Modifications:
//   Brad Whitlock, Mon Mar 6 14:52:07 PST 2006
//   Added Qt 3 implementation.
//
//   Brad Whitlock, Tue Jun  3 16:24:44 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisTurnDownButton::paintEvent(QPaintEvent *e)
{
    QPushButton::paintEvent(e);

    QPainter paint(this);
    QPolygon tri(3);
    tri.setPoint(0, width()/2, height()*2/3);
    tri.setPoint(1, width()-4, height()*1/3);
    tri.setPoint(2, 4, height()*1/3);
    paint.setBrush(palette().text());
    paint.drawConvexPolygon(tri);
}
