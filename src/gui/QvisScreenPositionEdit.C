#include <stdio.h>
#include <QvisScreenPositionEdit.h>
#include <QvisScreenPositioner.h>
#include <QvisTurnDownButton.h>

#include <qapplication.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <qstyle.h>
#include <qtimer.h>

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
//   
// ****************************************************************************

QvisScreenPositionEdit::QvisScreenPositionEdit(QWidget *parent,
    const char *name) : QWidget(parent, name)
{
    QHBoxLayout *hLayout = new QHBoxLayout(this);
    lineEdit = new QLineEdit(this, "lineEdit");
    connect(lineEdit, SIGNAL(returnPressed()),
            this, SLOT(returnPressed()));
    hLayout->addWidget(lineEdit);

    turnDown = new QvisTurnDownButton(this, "turnDown");
    connect(turnDown, SIGNAL(pressed()),
            this, SLOT(popup()));
    hLayout->addWidget(turnDown);

    screenPositionPopup = new QvisScreenPositioner(this, "screenPositionPopup",
        WType_Popup);
    screenPositionPopup->setFixedSize(QSize(150,150));
    connect(screenPositionPopup,
            SIGNAL(intermediateScreenPositionChanged(float,float)),
            this, SLOT(updateText(float, float)));
    connect(screenPositionPopup,
            SIGNAL(screenPositionChanged(float,float)),
            this, SLOT(newScreenPosition(float, float)));

    popupTimer = new QTimer(this, "popupTimer");
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
QvisScreenPositionEdit::setPosition(float x, float y)
{
    screenX = (x < 0.f) ? 0.f : ((x > 1.f) ? 1.f : x);
    screenY = (y < 0.f) ? 0.f : ((y > 1.f) ? 1.f : y);
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
QvisScreenPositionEdit::getPosition(float &x, float &y)
{
    bool okay = false;
    float newX, newY;

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
//   
// ****************************************************************************

bool
QvisScreenPositionEdit::getCurrentValues(float *newX, float *newY)
{
    QString temp(lineEdit->displayText().simplifyWhiteSpace());
    bool okay = !temp.isEmpty();
    if(okay)
        okay = (sscanf(temp.latin1(), "%g %g", newX, newY) == 2);

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
//   
// ****************************************************************************

void
QvisScreenPositionEdit::updateText(float x, float y)
{
    QString tmp;
    tmp.sprintf("%g %g", x, y);
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
QvisScreenPositionEdit::newScreenPosition(float x, float y)
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
    else if(menuX + menuW > QApplication::desktop()->width())
    {
        int extrapixels = QApplication::desktop()->width() - menuX - menuW;
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
    float newX, newY;
    if(getCurrentValues(&newX, &newY))
    {
        newScreenPosition(newX, newY);
        screenPositionPopup->setScreenPosition(screenX, screenY);
    }
}

