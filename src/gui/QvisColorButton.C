#include <QvisColorButton.h>
#include <QvisColorSelectionWidget.h>
#include <qapplication.h>
#include <qbrush.h>
#include <qpainter.h>
#include <qpopupmenu.h>
#include <qstyle.h>

// Static members.
QvisColorSelectionWidget *QvisColorButton::popup = 0;
QvisColorButton::ColorButtonVector QvisColorButton::buttons;

// ****************************************************************************
// Method: QvisColorButton::QvisColorButton
//
// Purpose: 
//   This is the constructor for the QvisColorButton widget.
//
// Arguments:
//   parent : The parent of this widget.
//   name   : The name of this instance.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 5 17:01:15 PST 2000
//
// Notes:
//   All color buttons share the same popup menu for selecting colors. There
//   is a static member of this class called buttons which is a vector of
//   pointers to QvisColorbutton. When the color button wants to display the
//   shared popup menu, it disconnects all other color buttons that may be
//   connected to it and then connects "this" colorbutton. This ensures that
//   the popup menu can only be used by one color button at any one time.
//
// Modifications:
//   Brad Whitlock, Fri Oct 26 16:45:48 PST 2001
//   I made the popup menu have no parent since when it had a parent, it
//   had some strange behaviors.
//
// ****************************************************************************

QvisColorButton::QvisColorButton(QWidget *parent, const char *name,
    const void *data) : QButton(parent, name), color(255, 0, 0)
{
    // Initialize the user data.
    userData = data;

    // Create the button's color selection popup menu.
    if(popup == 0)
        popup = new QvisColorSelectionWidget(0, "popup", WType_Popup);
    buttons.push_back(this);

    // Make the popup active when this button is clicked.
    connect(this, SIGNAL(pressed()), this, SLOT(popupPressed()));
}

// ****************************************************************************
// Method: QvisColorButton::~QvisColorButton
//
// Purpose: 
//   This is the destructor for the QvisColorButton class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 5 17:02:58 PST 2000
//
// Modifications:
//   
// ****************************************************************************

QvisColorButton::~QvisColorButton()
{
    // Remove the "this" pointer from the vector.
    int i, index;
    bool notFound = true;
    for(i = 0; i < buttons.size() && notFound; ++i)
    {
        if(this == buttons[i])
        {
            notFound = false;
            index = i;
        }
    }

    // If the pointer was found, shift the pointers in the vector and pop the
    // last element.
    if(!notFound)
    {
        for(i = index; i < buttons.size() - 1; ++i)
            buttons[i] = buttons[i + 1];
        buttons.pop_back();
    }
}

// ****************************************************************************
// Method: QvisColorButton::setUserData
//
// Purpose: 
//   Sets the userData pointer.
//
// Arguments:
//   data : The new user data.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 6 13:55:11 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisColorButton::setUserData(const void *data)
{
    userData = data;
}

// ****************************************************************************
// Method: QvisColorButton::getUserData
//
// Purpose: 
//   Gets the current userData.
//
// Returns:    The current userData.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 6 13:55:44 PST 2000
//
// Modifications:
//   
// ****************************************************************************

const void *
QvisColorButton::getUserData() const
{
    return userData;
}

// ****************************************************************************
// Method: QvisColorButton::drawButton
//
// Purpose: 
//   This method is called to draw the button.
//
// Arguments:
//   paint : The paint device on which we're painting.
//
// Notes:
//   This was mostly taken from QPushButton.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 5 17:03:27 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Mar 8 16:45:36 PST 2002
//   Updated style support.
//
// ****************************************************************************

void
QvisColorButton::drawButton(QPainter *paint)
{
    int i;
    int X  = 0;
    int X2 = 0 + width() - 1;
    int Y  = 0;
    int Y2 = 0 + height() - 1;

    // Draw the highlight
    paint->setPen(QPen(colorGroup().light()));
    for(i = 0; i < 2; ++i)
    {
        paint->drawLine(QPoint(X + i, Y + i), QPoint(X + i, Y2 - i));
        paint->drawLine(QPoint(X + i, Y + i), QPoint(X2 - i, Y + i));
    }

    // Draw the shadow
    paint->setPen(QPen(colorGroup().dark()));
    for(i = 0; i < 2; ++i)
    {
        paint->drawLine(QPoint(X + i + 1, Y2 - i), QPoint(X2, Y2 - i));
        paint->drawLine(QPoint(X2 - i, Y + i + 1), QPoint(X2 - i, Y2));
    }

    // Draw the button.
    drawButtonLabel(paint);

    // Draw the focus
    if(hasFocus())
    {
        QRect r(4, 4, width() - 8, height() - 8);
#if QT_VERSION >= 300
        style().drawPrimitive(QStyle::PE_FocusRect, paint, r, colorGroup(),
                              QStyle::Style_HasFocus);
#else
        style().drawFocusRect(paint, r, colorGroup());
#endif
    }
}

// ****************************************************************************
// Method: QvisColorButton::drawButtonLabel
//
// Purpose: 
//   This method draws the insides of the button. In this case, it is a 
//   rectangle of a certain color.
//
// Arguments:
//   paint : The paint device on which we're painting.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 5 17:04:44 PST 2000
//
// Modifications:
//   Brad Whitlock, Wed Jan 30 13:59:26 PST 2002
//   Modified the code so it uses a checkerboard brush when the widget
//   is disabled.
//
// ****************************************************************************

void
QvisColorButton::drawButtonLabel(QPainter *paint)
{
    QBrush brush(color);

    if(!isEnabled())
        brush.setStyle(QBrush::Dense5Pattern);

    // Draw the color area.
    QRect r(2, 2, width() - 4, height() - 4);
    paint->fillRect(r, brush);
}

// ****************************************************************************
// Method: QvisColorButton::sizeHint
//
// Purpose: 
//   Returns the widget's preferred size.
//
// Returns:    The widget's preferred size.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 5 17:05:42 PST 2000
//
// Modifications:
//   
// ****************************************************************************

QSize
QvisColorButton::sizeHint() const
{
    return QSize(50, 25);
}

// ****************************************************************************
// Method: QvisColorButton::sizePolicy
//
// Purpose: 
//   Returns the widget's size policy. This widget cannot be resized.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 6 11:35:51 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

QSizePolicy
QvisColorButton::sizePolicy() const
{
    return QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

// ****************************************************************************
// Method: QvisColorButton::setButtonColor
//
// Purpose: 
//   Sets the button's color.
//
// Arguments:
//   c : The button's new color.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 5 17:06:16 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Mar 5 14:53:32 PST 2001
//   Added code to prevent updates if the new color is the same as the old
//   color.
//
// ****************************************************************************

void
QvisColorButton::setButtonColor(const QColor &c)
{
    if(color != c)
    {
        color = c;

        if(isVisible())
            update();
    }
}

// ****************************************************************************
// Method: QvisColorButton::buttonColor
//
// Purpose: 
//   Gets the button's color.
//
// Returns:    The button's color.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 5 17:06:43 PST 2000
//
// Modifications:
//   
// ****************************************************************************

const QColor &
QvisColorButton::buttonColor() const
{
    return color;
}

// ****************************************************************************
// Method: QvisColorButton::popupPressed
//
// Purpose: 
//   This method is called when this button is clicked and it activates the
//   button's color popup menu.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 5 17:07:08 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisColorButton::popupPressed()
{
    if(isDown() && popup)
    {
        QPoint p1(mapToGlobal(rect().bottomLeft()));
        QPoint p2(mapToGlobal(rect().topRight()));
        QPoint buttonMiddle(p1.x() + ((p2.x() - p1.x()) >> 1),
                            p1.y() + ((p2.y() - p1.y()) >> 1));

        // Disconnect all other colorbuttons.
        for(int i = 0; i < buttons.size(); ++i)
        {
            disconnect(popup, SIGNAL(selectedColor(const QColor &)),
                       buttons[i], SLOT(colorSelected(const QColor &)));
        }

        // Connect this colorbutton to the popup menu.
        connect(popup, SIGNAL(selectedColor(const QColor &)),
                this, SLOT(colorSelected(const QColor &)));

        // Set the popup's initial color.
        popup->blockSignals(true);
        popup->setSelectedColor(color);
        popup->blockSignals(false);

        // Figure out a good place to popup the menu.
        int menuW = popup->sizeHint().width();
        int menuH = popup->sizeHint().height();
        int menuX = buttonMiddle.x();
        int menuY = buttonMiddle.y() - (menuH >> 1);

        // Fix the X dimension.
        if(menuX < 0)
           menuX = 0;
        else if(menuX + menuW > QApplication::desktop()->width())
           menuX -= (menuW + 5);

        // Fix the Y dimension.
        if(menuY < 0)
           menuY = 0;
        else if(menuY + menuH > QApplication::desktop()->height())
           menuY -= ((menuY + menuH) - QApplication::desktop()->height());

        // Show the popup menu.         
        popup->move(menuX, menuY);
        popup->show();
        setDown(FALSE);
    }
}

// ****************************************************************************
// Method: QvisColorButton::colorSelected
//
// Purpose: 
//   This method is called when a color has been selected from the color popup
//   menu. We emit the selectedColor signal here to inform widgets that may
//   be connected to this color button.
//
// Arguments:
//   c : The color that was selected, i.e. the button's new color.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 5 17:07:59 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Oct 26 14:28:51 PST 2001
//   Removed the code to hide the popup since it knows to hide itself now.
//
// ****************************************************************************

void
QvisColorButton::colorSelected(const QColor &c)
{
    if(c.isValid())
    {
        // Set the button color to the new color and make it redraw.
        setButtonColor(c);
        if(isVisible())
            update();

        // Tell widgets connected to this button that a color was chosen.
        emit selectedColor(c);
        emit selectedColor(c, userData);
    }
}
