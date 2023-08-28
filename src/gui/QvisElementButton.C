// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QvisElementButton.h>
#include <QvisPeriodicTableWidget.h>
#include <QvisElementSelectionWidget.h>
#include <QApplication>
#include <QBrush>
#include <QMenu>
#include <QPainter>
#include <QRect>
#include <QScreen>
#include <QStyle>
#include "AtomicProperties.h"

// Static members.
QvisElementSelectionWidget *QvisElementButton::sharedpopup = 0;
QvisElementButton::ElementButtonVector QvisElementButton::buttons;

// ****************************************************************************
// Method: QvisElementButton::QvisElementButton
//
// Purpose: 
//   This is the constructor for the QvisElementButton widget.
//
// Arguments:
//   parent : The parent of this widget.
//   name   : The name of this instance.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Notes: Taken largely from QvisColorButton
//
// Modifications:
//    Jeremy Meredith, Mon Feb 11 16:46:57 EST 2008
//    Changed to use the element selection widget instead of the
//    simple periodic table widget; the former was created to contain
//    both a periodic table widget and a "match any element" button
//    to allow wildcards.
//
//    Brad Whitlock, Tue Jun  3 14:43:47 PDT 2008
//    Qt 4.
//
// ****************************************************************************

QvisElementButton::QvisElementButton(QWidget *parent, const void *data) : 
    QPushButton(parent), number(-1)
{
    setText("*");

    // Create the button's color selection popup menu.
    if (sharedpopup == 0)
        sharedpopup = new QvisElementSelectionWidget(0, Qt::Popup);
    buttons.push_back(this);

    // Make the popup active when this button is clicked.
    connect(this, SIGNAL(pressed()), this, SLOT(popupPressed()));
}

// ****************************************************************************
// Method: QvisElementButton::~QvisElementButton
//
// Purpose: 
//   This is the destructor for the QvisElementButton class.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Notes: Taken largely from QvisColorButton
//
// Modifications:
//   
// ****************************************************************************

QvisElementButton::~QvisElementButton()
{
    // Remove the "this" pointer from the vector.
    size_t index = 0;
    bool notFound = true;
    for(size_t i = 0; i < buttons.size() && notFound; ++i)
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
        for(size_t i = index; i < buttons.size() - 1; ++i)
            buttons[i] = buttons[i + 1];
        buttons.pop_back();
    }
}


// ****************************************************************************
// Method: QvisElementButton::sizeHint
//
// Purpose: 
//   Returns the widget's preferred size.
//
// Returns:    The widget's preferred size.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Modifications:
//   
// ****************************************************************************

QSize
QvisElementButton::sizeHint() const
{
    return QSize(50, 25);
}

// ****************************************************************************
// Method: QvisElementButton::sizePolicy
//
// Purpose: 
//   Returns the widget's size policy. This widget cannot be resized.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Modifications:
//   
// ****************************************************************************

QSizePolicy
QvisElementButton::sizePolicy() const
{
    return QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

// ****************************************************************************
// Method: QvisElementButton::setElementNumber
//
// Purpose: 
//   Sets the button's element number
//
// Arguments:
//   e : The button's new element number
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Modifications:
//    Jeremy Meredith, Mon Feb 11 16:46:57 EST 2008
//    Allow a "-1" element number to mean "any element" wildcard.
//
//    Jeremy Meredith, Wed May 20 11:49:18 EDT 2009
//    MAX_ELEMENT_NUMBER now means the actual max element number, not the
//    total number of known elements in visit.  Added a fake "0" element
//    which means "unknown", and hydrogen now starts at 1.
//
//    Jeremy Meredith, Wed Aug 11 10:24:14 EDT 2010
//    Anything below "H" should use "*".  (Otherwise we might pick up
//    the "?" from the unknown element, for example.)
//
// ****************************************************************************

void
QvisElementButton::setElementNumber(int e)
{
    if (e >= -1 && e <= MAX_ELEMENT_NUMBER && number != e)
    {
        number = e;
        if (e < 1)
            setText("*");
        else
            setText(element_names[e]);

        if(isVisible())
            update();
    }
}

// ****************************************************************************
// Method: QvisElementButton::buttonColor
//
// Purpose: 
//   Gets the button's color.
//
// Returns:    The button's color.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Modifications:
//   
// ****************************************************************************

int
QvisElementButton::elementNumber() const
{
    return number;
}

// ****************************************************************************
// Method: QvisElementButton::popupPressed
//
// Purpose: 
//   This method is called when this button is clicked and it activates the
//   button's color popup menu.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Notes: Taken largely from QvisColorButton
//
// Modifications:
//   Kathleen Biagas, Wed Apr  5 13:04:35 PDT 2023
//   Replace obosolete desktop() with primaryScreen().
//   
// ****************************************************************************

void
QvisElementButton::popupPressed()
{
    if(isDown() && sharedpopup)
    {
        QPoint p1(mapToGlobal(rect().bottomLeft()));
        QPoint p2(mapToGlobal(rect().topRight()));
        QPoint buttonMiddle(p1.x() + ((p2.x() - p1.x()) >> 1),
                            p1.y() + ((p2.y() - p1.y()) >> 1));

        // Disconnect all other colorbuttons.
        for(size_t i = 0; i < buttons.size(); ++i)
        {
            disconnect(sharedpopup, SIGNAL(selectedElement(int)),
                       buttons[i], SLOT(elementSelected(int)));
        }

        // Connect this colorbutton to the popup menu.
        connect(sharedpopup, SIGNAL(selectedElement(int)),
                this, SLOT(elementSelected(int)));

        // Set the popup's initial color.
        sharedpopup->blockSignals(true);
        sharedpopup->setSelectedElement(number);
        sharedpopup->blockSignals(false);

        // Figure out a good place to popup the menu.
        int menuW = sharedpopup->sizeHint().width();
        int menuH = sharedpopup->sizeHint().height();
        int menuX = buttonMiddle.x();
        int menuY = buttonMiddle.y() - (menuH >> 1);

        // Fix the X dimension.
        if(menuX < 0)
           menuX = 0;
        else if(menuX + menuW > QApplication::primaryScreen()->geometry().width())
           menuX -= (menuW + 5);

        // Fix the Y dimension.
        if(menuY < 0)
           menuY = 0;
        else if(menuY + menuH > QApplication::primaryScreen()->geometry().height())
           menuY -= ((menuY + menuH) - QApplication::primaryScreen()->geometry().height());

        // Show the popup menu.         
        sharedpopup->move(menuX, menuY);
        sharedpopup->show();
        setDown(false);
    }
}

// ****************************************************************************
// Method: QvisElementButton::colorSelected
//
// Purpose: 
//   This method is called when a color has been selected from the color popup
//   menu. We emit the selectedColor signal here to inform widgets that may
//   be connected to this color button.
//
// Arguments:
//   c : The color that was selected, i.e. the button's new color.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Modifications:
//    Jeremy Meredith, Mon Feb 11 16:46:57 EST 2008
//    Allow a "-1" element number to mean "any element" wildcard.
//
//    Jeremy Meredith, Wed May 20 11:49:18 EDT 2009
//    MAX_ELEMENT_NUMBER now means the actual max element number, not the
//    total number of known elements in visit.  Added a fake "0" element
//    which means "unknown", and hydrogen now starts at 1.
//
//    Jeremy Meredith, Wed Aug 11 10:24:14 EDT 2010
//    Anything below "H" should use "*".  (Otherwise we might pick up
//    the "?" from the unknown element, for example.)
//
// ****************************************************************************

void
QvisElementButton::elementSelected(int element)
{
    if (element >= -1 && element <= MAX_ELEMENT_NUMBER)
    {
        number = element;
        if (element < 1)
            setText("*");
        else
            setText(element_names[element]);
        emit selectedElement(element);
    }
}

// ****************************************************************************
//  Method:  QvisElementButton::setHintedElements
//
//  Purpose:
//    Accept a list of atomic numbers that we will use for hinting.
//
//  Arguments:
//    elements   the list of atomic numbers
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 12, 2008
//
// ****************************************************************************

void
QvisElementButton::setHintedElements(const std::vector<int> &elements)
{
    sharedpopup->setHintedElements(elements);
}
