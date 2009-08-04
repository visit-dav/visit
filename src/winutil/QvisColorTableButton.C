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

#include <QvisColorTableButton.h>
#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QDesktopWidget>
#include <QMenu>

//
// Static members.
//

int           QvisColorTableButton::numInstances = 0;
QMenu        *QvisColorTableButton::colorTableMenu = 0;
QActionGroup *QvisColorTableButton::colorTableMenuActionGroup = 0;
QvisColorTableButton::ColorTableButtonVector QvisColorTableButton::buttons;
int         QvisColorTableButton::numColorTableNames = 0;
QString    *QvisColorTableButton::colorTableNames = 0;
bool        QvisColorTableButton::popupHasEntries = false;

// ****************************************************************************
// Method: QvisColorTableButton::QvisColorTableButton
//
// Purpose: 
//   Constructor for the QvisColorTableButton class.
//
// Arguments:
//   parent : The parent widget.
//   name   : The name to associate with this widget.
//
// Programmer: Brad Whitlock
// Creation:   Sat Jun 16 20:06:13 PST 2001
//
// Modifications:
//   Brad Whitlock, Thu Feb 14 13:38:42 PST 2002
//   Added code to count the number of instances.
//
//   Brad Whitlock, Fri May  9 11:23:57 PDT 2008
//   Qt 4.
//
// ****************************************************************************

QvisColorTableButton::QvisColorTableButton(QWidget *parent) :
    QPushButton(parent), colorTable("Default")
{
    // Increase the instance count.
    ++numInstances;

    // Create the button's color table popup menu.
    if(colorTableMenu == 0)
    {
        colorTableMenuActionGroup = new QActionGroup(0);

        colorTableMenu = new QMenu(0);
        colorTableMenuActionGroup->addAction(colorTableMenu->addAction("Default"));
        colorTableMenu->addSeparator();
    }
    buttons.push_back(this);

    // Make the popup active when this button is clicked.
    connect(this, SIGNAL(pressed()), this, SLOT(popupPressed()));

    setText(colorTable);
}

// ****************************************************************************
// Method: QvisColorTableButton::~QvisColorTableButton
//
// Purpose: 
//   This is the destructor for the QvisColorTableButton class.
//
// Programmer: Brad Whitlock
// Creation:   Sat Jun 16 20:06:57 PST 2001
//
// Modifications:
//   Brad Whitlock, Thu Feb 14 13:31:46 PST 2002
//   Deleted the popup menu if it exists.
//
// ****************************************************************************

QvisColorTableButton::~QvisColorTableButton()
{
    // Decrease the instance count.
    --numInstances;

    // Remove the "this" pointer from the vector.
    size_t index;
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

    if(numInstances == 0)
    {
        if(colorTableMenuActionGroup != 0)
        {
            delete colorTableMenuActionGroup;
            colorTableMenuActionGroup = 0;
        }

        // Delete the popup menu if it exists because it will not be deleted
        // unless we do it since it is a parentless widget.
        if(colorTableMenu != 0)
        {
            delete colorTableMenu;
            colorTableMenu = 0;
        }

        // Delete the color table names.
        if(colorTableNames != 0)
        {
            delete [] colorTableNames;
            colorTableNames = 0;
            numColorTableNames = 0;
        }
    }
}

// ****************************************************************************
// Method: QvisColorTableButton::sizeHint
//
// Purpose: 
//   Returns the widget's preferred size.
//
// Returns:    The widget's preferred size.
//
// Programmer: Brad Whitlock
// Creation:   Sat Jun 16 20:07:23 PST 2001
//
// Modifications:
//   
// ****************************************************************************

QSize
QvisColorTableButton::sizeHint() const
{
     return QSize(125, 40).expandedTo(QApplication::globalStrut());
}

// ****************************************************************************
// Method: QvisColorTableButton::sizePolicy
//
// Purpose: 
//   Returns the widget's size policy -- how allows itself to be resized.
//
// Returns:    The widget's size policy.
//
// Programmer: Brad Whitlock
// Creation:   Sat Jun 16 20:07:55 PST 2001
//
// Modifications:
//   
// ****************************************************************************

QSizePolicy
QvisColorTableButton::sizePolicy() const
{
    return QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

// ****************************************************************************
// Method: QvisColorTableButton::useDefaultColorTable
//
// Purpose: 
//   Tells the widget to use the default color table.
//
// Programmer: Brad Whitlock
// Creation:   Sat Jun 16 20:08:42 PST 2001
//
// Modifications:
//   Brad Whitlock, Tue Jan 17 11:41:44 PDT 2006
//   Added a tooltip so long color table names can be put in a tooltip.
//   
// ****************************************************************************

void
QvisColorTableButton::useDefaultColorTable()
{
    colorTable = QString("Default");
    setText(colorTable);
    setToolTip(colorTable);
}

// ****************************************************************************
// Method: QvisColorTableButton::setColorTable
//
// Purpose: 
//   Tells the widget to use a specified color table.
//
// Arguments:
//   ctName : The name of the color table to use.
//
// Programmer: Brad Whitlock
// Creation:   Sat Jun 16 20:09:09 PST 2001
//
// Modifications:
//   Brad Whitlock, Fri Feb 15 10:15:43 PDT 2002
//   Made it set the menu text to "Default" if no color table is found.
//
//   Brad Whitlock, Tue Jan 17 11:41:44 PDT 2006
//   Added a tooltip so long color table names can be put in a tooltip.
//
// ****************************************************************************

void
QvisColorTableButton::setColorTable(const QString &ctName)
{
    if(getColorTableIndex(ctName) != -1)
    {
        colorTable = ctName;
        setText(colorTable);
        setToolTip(colorTable);
    }
    else
    {
        QString def("Default");
        setText(def);
        setToolTip(def);
    }
}

// ****************************************************************************
// Method: QvisColorTableButton::getColorTable
//
// Purpose: 
//   Returns the name of the widget's color table.
//
// Returns:    The name of the widget's color table.
//
// Programmer: Brad Whitlock
// Creation:   Sat Jun 16 20:09:46 PST 2001
//
// Modifications:
//   
// ****************************************************************************

const QString &
QvisColorTableButton::getColorTable() const
{
    return colorTable;
}

//
// Qt slot functions.
//

// ****************************************************************************
// Method: QvisColorTableButton::popupPressed
//
// Purpose: 
//   This is a Qt slot function that pops up the color table popup menu when
//   the button is pressed.
//
// Programmer: Brad Whitlock
// Creation:   Sat Jun 16 20:10:16 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisColorTableButton::popupPressed()
{
    if(isDown() && colorTableMenu)
    {
        // If the popup menu does not have anything in it, fill it up.
        if(!popupHasEntries)
            regeneratePopupMenu();

        QPoint p1(mapToGlobal(rect().bottomLeft()));
        QPoint p2(mapToGlobal(rect().topRight()));
        QPoint buttonMiddle(p1.x() + ((p2.x() - p1.x()) >> 1),
                            p1.y() + ((p2.y() - p1.y()) >> 1));

        // Disconnect all other color table buttons.
        for(size_t i = 0; i < buttons.size(); ++i)
        {
            disconnect(colorTableMenuActionGroup, SIGNAL(triggered(QAction *)),
                       buttons[i], SLOT(colorTableSelected(QAction *)));
        }

        // Connect this colorbutton to the popup menu.
        connect(colorTableMenuActionGroup, SIGNAL(triggered(QAction *)),
                this, SLOT(colorTableSelected(QAction *)));

        // Figure out a good place to popup the menu.
        int menuW = colorTableMenu->sizeHint().width();
        int menuH = colorTableMenu->sizeHint().height();
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
        colorTableMenu->exec(QPoint(menuX, menuY));
        setDown(FALSE);
    }
}

// ****************************************************************************
// Method: QvisColorTableButton::colorTableSelected
//
// Purpose: 
//   This is a Qt slot function that is called when a color table has been
//   selected from the popup menu. The widget then emits a selectedColorTable
//   signal.
//
// Arguments:
//   index : The index of the color table chosen from the popup menu.
//
// Programmer: Brad Whitlock
// Creation:   Sat Jun 16 20:11:06 PST 2001
//
// Modifications:
//   Brad Whitlock, Tue Jan 17 11:41:44 PDT 2006
//   Added a tooltip so long color table names can be put in a tooltip.
//
//   Brad Whitlock, Fri May  9 11:39:40 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisColorTableButton::colorTableSelected(QAction *action)
{
    int index = colorTableMenuActionGroup->actions().indexOf(action);

    if(index == 0)
    {
        QString def("Default");
        emit selectedColorTable(true, def);
        setText(def);
        setToolTip(def);
    }
    else
    {
        emit selectedColorTable(false, colorTableNames[index - 1]);
        setText(colorTableNames[index-1]);
        setToolTip(colorTableNames[index-1]);
    }
}

//
// Static methods...
//

// ****************************************************************************
// Method: QvisColorTableButton::clearAllColorTables
//
// Purpose: 
//   This is a static method to clear all of the known color tables.
//
// Programmer: Brad Whitlock
// Creation:   Sat Jun 16 20:12:33 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisColorTableButton::clearAllColorTables()
{
    delete [] colorTableNames;
    colorTableNames = 0;
    numColorTableNames = 0;

    // Clear out the popup menu.
    popupHasEntries = false;
}

// ****************************************************************************
// Method: QvisColorTableButton::addColorTable
//
// Purpose: 
//   This is a static method that tells the widget about a new color table.
//
// Arguments:
//   ctName : The name of the new color table.
//
// Programmer: Brad Whitlock
// Creation:   Sat Jun 16 20:13:09 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisColorTableButton::addColorTable(const QString &ctName)
{
    QString *newList = new QString[numColorTableNames + 1];
    for(int i = 0; i < numColorTableNames; ++i)
        newList[i] = colorTableNames[i];
    newList[numColorTableNames] = ctName;
    if(colorTableNames != NULL)
        delete [] colorTableNames;
    colorTableNames = newList;
    ++numColorTableNames;
}

// ****************************************************************************
// Method: QvisColorTableButton::updateColorTableButtons
//
// Purpose: 
//   This is a static method that iterates through all instances of
//   QvisColorTableButton to make sure that the color table that they use is
//   a valid color table. This will also be used to update their color table
//   pixmaps -- someday.
//
// Programmer: Brad Whitlock
// Creation:   Sat Jun 16 20:13:46 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisColorTableButton::updateColorTableButtons()
{
    for(size_t i = 0; i < buttons.size(); ++i)
    {
        // If the color table that was being used by the button is no
        // longer in the list, make it use the default.
        if(getColorTableIndex(buttons[i]->getColorTable()) == -1)
        {
            buttons[i]->setText("Default");
            buttons[i]->setColorTable("Default");
        }
    }
}

// ****************************************************************************
// Method: QvisColorTableButton::getColorTableIndex
//
// Purpose: 
//   Returns the index of the specified color table in the internal color
//   table list. If the color table is not found, -1 is returned.
//
// Arguments:
//   ctName : The name of the color table to look for.
//
// Returns:    The index of the color table, or -1.
//
// Programmer: Brad Whitlock
// Creation:   Sat Jun 16 20:15:16 PST 2001
//
// Modifications:
//   
// ****************************************************************************

int
QvisColorTableButton::getColorTableIndex(const QString &ctName)
{
    int retval = -1;
    if(colorTableNames)
    {
        for(int i = 0; i < numColorTableNames; ++i)
        {
            if(colorTableNames[i] == ctName)
            {
                retval = i;
                break;
            }
        }
    }

    return retval;
}

// ****************************************************************************
// Method: QvisColorTableButton::regeneratePopupMenu
//
// Purpose: 
//   This method is called when the popup menu needs to be regenerated. This
//   happens when the color table list changes.
//
// Programmer: Brad Whitlock
// Creation:   Sat Jun 16 20:16:34 PST 2001
//
// Modifications:
//   Brad Whitlock, Fri May  9 11:21:28 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisColorTableButton::regeneratePopupMenu()
{
    // Remove all items and add the default.
    QList<QAction*> actions = colorTableMenuActionGroup->actions();
    for(int i = 0; i < actions.count(); ++i)
        colorTableMenuActionGroup->removeAction(actions[i]);
    colorTableMenu->clear();

    colorTableMenuActionGroup->addAction(colorTableMenu->addAction("Default"));
    colorTableMenu->addSeparator();
    
    // Add an item for each color table.
    for(int i = 0; i < numColorTableNames; ++i)
        colorTableMenuActionGroup->addAction(colorTableMenu->addAction(colorTableNames[i]));

    // Indicate that we've added choices to the menu.
    popupHasEntries = true;
}
