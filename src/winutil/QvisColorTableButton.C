// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QvisColorTableButton.h>
#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QMenu>
#include <QPainter>
#include <QPixmap>

#include <ColorTableAttributes.h>
#include <ColorControlPointList.h>

#define ICON_NX 32
#define ICON_NY 16

//
// Static members.
//

int           QvisColorTableButton::numInstances = 0;
QMenu        *QvisColorTableButton::colorTableMenu = 0;
QActionGroup *QvisColorTableButton::colorTableMenuActionGroup = 0;
QvisColorTableButton::ColorTableButtonVector QvisColorTableButton::buttons;
QStringList  QvisColorTableButton::colorTableNames;
bool        QvisColorTableButton::popupHasEntries = false;
ColorTableAttributes *QvisColorTableButton::colorTableAtts = NULL;

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
//   Kathleen Biagas, Thu Jun 18 11:42:47 PDT 2020
//   Call 'setMenu' to allow QPushButton to control showing and placment of
//   colorTableMenu.  This will add an arrow to the button to indicate a menu
//   is attached. Connect QMenu's 'aboutToShow' instead of QPushButton's
//   'pressed' signal.
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
    setMenu(colorTableMenu);
    buttons.push_back(this);

    // Make the popup active when this button is clicked.
    connect(colorTableMenu, SIGNAL(aboutToShow()), this, SLOT(popupPressed()));

    setText(colorTable);
    setIconSize(QSize(ICON_NX,ICON_NY));
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
//   Justin Privitera, Thu Jun 16 18:01:49 PDT 2022
//   Removed mappedColorTableNames, so no need to clear it in the destructor.
//
// ****************************************************************************

QvisColorTableButton::~QvisColorTableButton()
{
    // Decrease the instance count.
    --numInstances;

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
        colorTableNames.clear();
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
    setIcon(QIcon());
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
//    Justin Privitera, Wed Aug  3 19:46:13 PDT 2022
//    Added logic to prevent CT from being changed when CT passed out of the 
//    tag filtering selection.
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
        setIcon(getIcon(ctName));
    }
    else
    {
        // if this color table was deleted
        if (colorTableAtts->GetColorTableIndex(ctName.toStdString()) == -1)
        {
            QString def("Default");
            setText(def);
            setToolTip(def);
            setIcon(QIcon());
        }
        // but if it was filtered, we don't want to do anything
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
//    Kathleen Biagas, Thu Jun 18 11:39:24 PDT 2020
//    Menu is now connected to this pushbutton via setMenu method, so it will
//    control placement. This slot is now called when aboutToShow signal is
//    triggered, so don't need to test for 'isDown'.
//
//    Kathleen Biagas, Fri Jun 19 12:06:22 PDT 2020
//    isDown is actually important, Added it back to the if-test.
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

        // Disconnect all other color table buttons.
        for(size_t i = 0; i < buttons.size(); ++i)
        {
            disconnect(colorTableMenuActionGroup, SIGNAL(triggered(QAction *)),
                       buttons[i], SLOT(colorTableSelected(QAction *)));
        }

        // Connect this colorbutton to the popup menu.
        connect(colorTableMenuActionGroup, SIGNAL(triggered(QAction *)),
                this, SLOT(colorTableSelected(QAction *)));
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
//   Kathleen Biagas, Mon Aug  4 15:54:14 PDT 2014
//   Handle grouping.
// 
//   Justin Privitera, Thu Jun 16 18:01:49 PDT 2022
//   Removed grouping.
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
        setIcon(QIcon());
    }
    else
    {
        QString ctName = colorTableNames.at(index-1);

        emit selectedColorTable(false, ctName);
        setText(ctName);
        setIcon(getIcon(ctName));
        setToolTip(ctName);
    }
}

//
// Static methods
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
//   Justin Privitera, Thu Jun 16 18:01:49 PDT 2022
//   Removed mappedColorTableNames.
//
// ****************************************************************************

void
QvisColorTableButton::clearAllColorTables()
{
    colorTableNames.clear();

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
//   Kathleen Biagas, Mon Aug  4 15:55:26 PDT 2014
//   colorTableNames now a QStringList, so append and sort.
//   Added mappedColorTableNames.
// 
//   Justin Privitera, Thu Jun 16 18:01:49 PDT 2022
//   No longer takes the category name.
//
// ****************************************************************************

void
QvisColorTableButton::addColorTable(const QString &ctName)
{
    colorTableNames.append(ctName);
    colorTableNames.sort();
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
//    Justin Privitera, Wed Aug  3 19:46:13 PDT 2022
//    Added logic to prevent CT from being changed when CT passed out of the 
//    tag filtering selection.
//   
// ****************************************************************************

void
QvisColorTableButton::updateColorTableButtons()
{
    for(size_t i = 0; i < buttons.size(); ++i)
    {
        // the CT that was being used by the button is no longer in the list
        if(getColorTableIndex(buttons[i]->getColorTable()) == -1)
        {
            // if deleted
            if (colorTableAtts->GetColorTableIndex(
                buttons[i]->getColorTable().toStdString()) == -1)
            {
                // then use the default
                buttons[i]->setText("Default");
                buttons[i]->setColorTable("Default");
            }
            // but if it was filtered, we do nothing
        }
        else
            buttons[i]->setIcon(getIcon(buttons[i]->text()));
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
//   Kathleen Biagas, Mon Aug  4 15:59:18 PDT 2014
//   Use the indexOf method for QStringList.
//
// ****************************************************************************

int
QvisColorTableButton::getColorTableIndex(const QString &ctName)
{
    return colorTableNames.indexOf(ctName);
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
//   Brad Whitlock, Wed Apr 25 13:32:01 PDT 2012
//   Add pixmaps of the color table.
//
//   Kathleen Biagas, Mon Aug  4 15:59:56 PDT 2014
//   Hangle grouping.
// 
//   Justin Privitera, Thu Jun 16 18:01:49 PDT 2022
//   Removed categories/grouping.
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
    for (int i = 0; i < colorTableNames.size(); ++i)
    {
        QAction *action = colorTableMenu->addAction(makeIcon(colorTableNames.at(i)), colorTableNames.at(i));
        colorTableMenuActionGroup->addAction(action);
    }

    // Indicate that we've added choices to the menu.
    popupHasEntries = true;
}

// ****************************************************************************
// Method: QvisColorTableButton::getIcon
//
// Purpose: 
//   This method gets the existing icon or makes one if necessary.
//
// Programmer: Brad Whitlock
// Creation:   Wed Apr 25 16:04:54 PDT 2012
//
// Modifications:
//
// ****************************************************************************

QIcon
QvisColorTableButton::getIcon(const QString &ctName)
{
    QList<QAction*> a = colorTableMenuActionGroup->actions();
    for(int i = 0; i < a.size(); ++i)
        if(a[i]->text() == ctName)
            return a[i]->icon();

    return makeIcon(ctName);
}

// ****************************************************************************
// Method: QvisColorTableButton::makeIcon
//
// Purpose: 
//   This method makes an icon from the color table definition.
//
// Programmer: Brad Whitlock
// Creation:   Wed Apr 25 16:04:54 PDT 2012
//
// Modifications:
//
// ****************************************************************************

QIcon
QvisColorTableButton::makeIcon(const QString &ctName)
{
    QIcon icon;
    const ColorControlPointList *cTable = NULL;
    if(colorTableAtts != NULL)
        cTable = colorTableAtts->GetColorControlPoints(ctName.toStdString());
    if(cTable != NULL)
    {
        QPixmap pix(ICON_NX, ICON_NY);
        unsigned char rgb[ICON_NX*3];
        cTable->GetColors(rgb, ICON_NX);
        QPainter paint(&pix);
        for(int ii = 0; ii < ICON_NX; ++ii)
        {
            paint.setPen(QPen(QColor((int)rgb[3*ii+0], (int)rgb[3*ii+1], (int)rgb[3*ii+2])));
            paint.drawLine(ii, 0, ii, ICON_NY-1);
        }

        icon = QIcon(pix);
    }

    return icon;
}

// ****************************************************************************
// Method: QvisColorTableButton::setColorTableAttributes
//
// Purpose: 
//   This method sets the color table attributes.
//
// Programmer: Brad Whitlock
// Creation:   Wed Apr 25 16:04:54 PDT 2012
//
// Modifications:
//
// ****************************************************************************

void
QvisColorTableButton::setColorTableAttributes(ColorTableAttributes *cAtts)
{
    colorTableAtts = cAtts;
}
