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

#include <QvisElementButton.h>
#include <QvisPeriodicTableWidget.h>
#include <QvisElementSelectionWidget.h>
#include <qapplication.h>
#include <qbrush.h>
#include <qpainter.h>
#include <qpopupmenu.h>
#include <qstyle.h>
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
// ****************************************************************************

QvisElementButton::QvisElementButton(QWidget *parent, const char *name,
    const void *data) : QPushButton(parent, name), number(-1)
{
    setText("*");

    // Create the button's color selection popup menu.
    if (sharedpopup == 0)
        sharedpopup = new QvisElementSelectionWidget(0,"sharedpopup",WType_Popup);
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
// Method: QvisElementButton::setButtonColor
//
// Purpose: 
//   Sets the button's color.
//
// Arguments:
//   c : The button's new color.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Modifications:
//    Jeremy Meredith, Mon Feb 11 16:46:57 EST 2008
//    Allow a "-1" element number to mean "any element" wildcard.
//
// ****************************************************************************

void
QvisElementButton::setElementNumber(int e)
{
    if (e >= -1 && e < MAX_ELEMENT_NUMBER && number != e)
    {
        number = e;
        if (e == -1)
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
        for(int i = 0; i < buttons.size(); ++i)
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
        else if(menuX + menuW > QApplication::desktop()->width())
           menuX -= (menuW + 5);

        // Fix the Y dimension.
        if(menuY < 0)
           menuY = 0;
        else if(menuY + menuH > QApplication::desktop()->height())
           menuY -= ((menuY + menuH) - QApplication::desktop()->height());

        // Show the popup menu.         
        sharedpopup->move(menuX, menuY);
        sharedpopup->show();
        setDown(FALSE);
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
// ****************************************************************************

void
QvisElementButton::elementSelected(int element)
{
    if (element >= -1 && element < MAX_ELEMENT_NUMBER)
    {
        number = element;
        if (element == -1)
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
