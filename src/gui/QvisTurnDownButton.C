#include <QvisTurnDownButton.h>
#include <qpainter.h>
#include <qstyle.h>

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
//   
// ****************************************************************************

QvisTurnDownButton::QvisTurnDownButton(QWidget *parent, const char *name) : 
    QPushButton(parent, name)
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
// ****************************************************************************

void
QvisTurnDownButton::drawButtonLabel(QPainter *painter)
{
    int x = 0;
    int y = 0;
    int w = width();
    int h = height();

    if(style().inherits("QMotifStyle"))
    {
        x = y = 2;
        w -= 4;
        h -= 4;
    }

#if QT_VERSION >= 300
    QRect r(x,y,w,h);
    style().drawPrimitive(QStyle::PE_ArrowDown, painter, r, colorGroup(), QStyle::Style_Enabled);    
#else
    style().drawArrow(painter, Qt::DownArrow, isDown(),
         x, y, w, h,
         colorGroup(), isEnabled());
#endif
}
