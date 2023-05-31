// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <stdio.h>
#include <QvisScreenPositionEdit.h>
#include <QvisScreenPositioner.h>
#include <QvisTurnDownButton.h>

#include <QApplication>
#include <QLayout>
#include <QLineEdit>
#include <QPainter>
#include <QPushButton>
#include <QRect>
#include <QScreen>
#include <QStyle>
#include <QTimer>

// ****************************************************************************
// Method: QvisScreenPositionEdit::QvisScreenPositionEdit
//
// Purpose:
//   Constructor for the QvisScreenPositionEdit class.
//
// Arguments:
//   parent : The widget's parent.
//   name   : The widget's name
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 13:49:54 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Jun  3 16:12:06 PDT 2008
//   Qt 4.
//
//   Kathleen Biagas, Mon Nov 20 14:18:41 PST 2017
//   Add 'editingFinished' signal connection to the returnPressed slot.
//
// ****************************************************************************

QvisScreenPositionEdit::QvisScreenPositionEdit(QWidget *parent) :
    QWidget(parent)
{
    QHBoxLayout *hLayout = new QHBoxLayout(this);
    hLayout->setContentsMargins(0,0,0,0);
    hLayout->setSpacing(0);
    lineEdit = new QLineEdit(this);
    connect(lineEdit, SIGNAL(editingFinished()),
            this, SLOT(returnPressed()));
    connect(lineEdit, SIGNAL(returnPressed()),
            this, SLOT(returnPressed()));
    hLayout->addWidget(lineEdit);

    turnDown = new QvisTurnDownButton(this);
    connect(turnDown, SIGNAL(pressed()),
            this, SLOT(popup()));
    hLayout->addWidget(turnDown);

    lineEdit->setMinimumHeight(turnDown->height());

    screenPositionPopup = new QvisScreenPositioner(this, Qt::Popup);
    screenPositionPopup->setFixedSize(QSize(150,150));
    connect(screenPositionPopup,
            SIGNAL(intermediateScreenPositionChanged(double,double)),
            this, SLOT(updateText(double, double)));
    connect(screenPositionPopup,
            SIGNAL(screenPositionChanged(double,double)),
            this, SLOT(newScreenPosition(double, double)));

    popupTimer = new QTimer(this);
    connect(popupTimer, SIGNAL(timeout()),
            this, SLOT(closePopup()));

    setPosition(0.5f, 0.5f);
}

// ****************************************************************************
// Method: QvisScreenPositionEdit::~QvisScreenPositionEdit
//
// Purpose:
//   Destructor for the QvisScreenPositionEdit class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 13:50:35 PST 2003
//
// Modifications:
//
// ****************************************************************************

QvisScreenPositionEdit::~QvisScreenPositionEdit()
{
}

// ****************************************************************************
// Method: QvisScreenPositionEdit::setPosition
//
// Purpose:
//   Sets the screen position.
//
// Arguments:
//   x,y : The new screen position.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 13:50:53 PST 2003
//
// Modifications:
//
// ****************************************************************************

void
QvisScreenPositionEdit::setPosition(double x, double y)
{
    screenX = (x < 0.) ? 0. : ((x > 1.) ? 1. : x);
    screenY = (y < 0.) ? 0. : ((y > 1.) ? 1. : y);
    updateText(screenX, screenY);
    screenPositionPopup->setScreenPosition(screenX, screenY);
}

// ****************************************************************************
// Method: QvisScreenPositionEdit::getPosition
//
// Purpose:
//   Gets the screen position.
//
// Arguments:
//   x,y : The return coordinates.
//
// Returns:    True if the coordinates are valid, false otherwise.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 13:51:39 PST 2003
//
// Modifications:
//
// ****************************************************************************

bool
QvisScreenPositionEdit::getPosition(double &x, double &y)
{
    bool okay = false;
    double newX, newY;

    if((okay = getCurrentValues(&newX, &newY)) == true)
    {
        screenX = newX;
        screenY = newY;
    }

    x = screenX;
    y = screenY;
    return okay;
}

// ****************************************************************************
// Method: QvisScreenPositionEdit::getCurrentValues
//
// Purpose:
//   Gets the current values from the line edit.
//
// Arguments:
//   newX, newY : The return coordinates.
//
// Returns:    True if the coordinates are good; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 13:52:38 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Jun  3 16:17:48 PDT 2008
//   Qt 4.
//
// ****************************************************************************

bool
QvisScreenPositionEdit::getCurrentValues(double *newX, double *newY)
{
    QString temp(lineEdit->displayText().simplified());
    bool okay = !temp.isEmpty();
    if(okay)
        okay = (sscanf(temp.toStdString().c_str(), "%lg %lg", newX, newY) == 2);

    return okay;
}

//
// Qt slots
//

// ****************************************************************************
// Method: QvisScreenPositionEdit::updateText
//
// Purpose:
//   This is a Qt slot function that updates the text in the line edit.
//
// Arguments:
//   x,y : The new coordinate to display.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 13:53:39 PST 2003
//
// Modifications:
//  Kathleen Biagas, Jan 21, 2021
//  Replace QString.asprintf with QString.arg.
//
// ****************************************************************************

void
QvisScreenPositionEdit::updateText(double x, double y)
{
    QString tmp = QString("%1 %2").arg(x).arg(y);
    lineEdit->setText(tmp);
}

// ****************************************************************************
// Method: QvisScreenPositionEdit::newScreenPosition
//
// Purpose:
//   This is a Qt slot function that emits a screenPositionChanged signal
//   when the screen coordinate changes for whatever reason.
//
// Arguments:
//   x,y : The new screen coordinate.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 13:54:19 PST 2003
//
// Modifications:
//
// ****************************************************************************

void
QvisScreenPositionEdit::newScreenPosition(double x, double y)
{
    screenX = x;
    screenY = y;
    updateText(screenX, screenY);
    closePopup();

    emit screenPositionChanged(screenX, screenY);
}

// ****************************************************************************
// Method: QvisScreenPositionEdit::popup
//
// Purpose:
//   This is a Qt slot function that is called to popup the screen positioner
//   widget.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 13:55:18 PST 2003
//
// Modifications:
//   Kathleen Biagas, Wed Apr  5 13:04:35 PDT 2023
//   Replace obosolete desktop() with primaryScreen().
//
// ****************************************************************************

void
QvisScreenPositionEdit::popup()
{
    popupTimer->start(15000);

    // Figure out a good place to popup the menu.
    int menuW = screenPositionPopup->width();
    int menuH = screenPositionPopup->height();
    QPoint p(mapToGlobal(QPoint(0,0)));
    int menuX = p.x() + width() - menuW;
    int menuY = p.y() + height();

    // Fix the X dimension.
    if(menuX < 0)
        menuX = 0;
    else if(menuX + menuW > QApplication::primaryScreen()->geometry().width())
    {
        int extrapixels = QApplication::primaryScreen()->geometry().width() - menuX - menuW;
        menuX -= (extrapixels + 5);
    }

    // Fix the Y dimension.
    if(menuY - menuH < 0)
        menuY = menuY - height() - menuH;

    // Show the popup menu.
    screenPositionPopup->move(menuX, menuY);
    screenPositionPopup->popupShow();
}

// ****************************************************************************
// Method: QvisScreenPositionEdit::closePopup
//
// Purpose:
//   This is a Qt slot function that is called by a timer when the screen
//   positioner widget has been popped up for too long.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 13:55:47 PST 2003
//
// Modifications:
//
// ****************************************************************************

void
QvisScreenPositionEdit::closePopup()
{
    turnDown->setDown(false);
    popupTimer->stop();
    screenPositionPopup->hide();
}

// ****************************************************************************
// Method: QvisScreenPositionEdit::returnPressed
//
// Purpose:
//   This is a Qt slot function that is called when return is pressed in the
//   line edit.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 13:56:24 PST 2003
//
// Modifications:
//
// ****************************************************************************

void
QvisScreenPositionEdit::returnPressed()
{
    double newX, newY;
    if(getCurrentValues(&newX, &newY))
    {
        newScreenPosition(newX, newY);
        screenPositionPopup->setScreenPosition(screenX, screenY);
    }
}

