// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QvisNoDefaultColorTableButton.h>
#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QMenu>
#include <QPainter>
#include <QPixmap>
#include <QRect>
#include <QScreen>
#include <DebugStream.h>

#include <ColorTableAttributes.h>
#include <ColorControlPointList.h>

#define ICON_NX 32
#define ICON_NY 16

const int DISC = 0;
const int CONT = 1;

//
// Static members.
//

int           QvisNoDefaultColorTableButton::numInstances = 0;
QvisNoDefaultColorTableButton::ColorTableButtonVector QvisNoDefaultColorTableButton::buttons;
QActionGroup *QvisNoDefaultColorTableButton::colorTableMenuActionGroup[2] = {0, 0};
QMenu        *QvisNoDefaultColorTableButton::colorTableMenu[2] = {0, 0};
QStringList   QvisNoDefaultColorTableButton::colorTableNames[2];
bool          QvisNoDefaultColorTableButton::popupHasEntries[2] = {false, false};
ColorTableAttributes *QvisNoDefaultColorTableButton::colorTableAtts = NULL;

// ****************************************************************************
// Method: QvisNoDefaultColorTableButton::QvisNoDefaultColorTableButton
//
// Purpose: 
//   Constructor for the QvisNoDefaultColorTableButton class.
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
//   Justin Privitera, Wed Jul 13 15:24:42 PDT 2022
//   Added `discrete` boolean argument to set the buttonType to 
//   discrete or continuous. Used it to index into arrays.
//
// ****************************************************************************

QvisNoDefaultColorTableButton::QvisNoDefaultColorTableButton(QWidget *parent, 
    bool discrete) : QPushButton(parent), colorTable(""), 
    buttonType(discrete)
{
    // Increase the instance count.
    ++numInstances;

    // Create the button's color table popup menu.
    if (colorTableMenu[buttonType] == 0)
    {
        colorTableMenuActionGroup[buttonType] = new QActionGroup(0);
        colorTableMenu[buttonType] = new QMenu(0);
    }
    buttons.push_back(this);

    // Make the popup active when this button is clicked.
    connect(this, SIGNAL(pressed()), this, SLOT(popupPressed()));

    setText(colorTable);
    setIconSize(QSize(ICON_NX,ICON_NY));
}

// ****************************************************************************
// Method: QvisNoDefaultColorTableButton::~QvisNoDefaultColorTableButton
//
// Purpose: 
//   This is the destructor for the QvisNoDefaultColorTableButton class.
//
// Programmer: Brad Whitlock
// Creation:   Sat Jun 16 20:06:57 PST 2001
//
// Modifications:
//   Brad Whitlock, Thu Feb 14 13:31:46 PST 2002
//   Deleted the popup menu if it exists.
// 
//   Justin Privitera, Thu Jun 16 18:01:49 PDT 2022
//   Removed mappedColorTableNames.
// 
//   Justin Privitera, Wed Jul 13 15:24:42 PDT 2022
//   Deleted only the static pieces of the button that belong to the button 
//   being deleted.
//
// ****************************************************************************

QvisNoDefaultColorTableButton::~QvisNoDefaultColorTableButton()
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

    if (colorTableMenuActionGroup[buttonType] != 0)
    {
        delete colorTableMenuActionGroup[buttonType];
        colorTableMenuActionGroup[buttonType] = 0;
    }
    // Delete the popup menu if it exists because it will not be deleted
    // unless we do it since it is a parentless widget.
    if (colorTableMenu[buttonType] != 0)
    {
        delete colorTableMenu[buttonType];
        colorTableMenu[buttonType] = 0;
    }
    // Delete the color table names.
    colorTableNames[buttonType].clear();
}

// ****************************************************************************
// Method: QvisNoDefaultColorTableButton::sizeHint
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
//    Kathleen Biagas, Mon May 22 12:41:37 PDT 2023
//    Removed use of QApplication::globalStrut, as it is a no-op (deprecated)
//    in Qt5 and removed in Qt6.
//
// ****************************************************************************

QSize
QvisNoDefaultColorTableButton::sizeHint() const
{
     return QSize(125, 40);
}

// ****************************************************************************
// Method: QvisNoDefaultColorTableButton::sizePolicy
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
QvisNoDefaultColorTableButton::sizePolicy() const
{
    return QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}


// ****************************************************************************
// Method: QvisNoDefaultColorTableButton::setColorTable
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
//   Justin Privitera, Wed Jul 13 15:24:42 PDT 2022
//   Using buttonType to index into the array vars.
// 
//   Justin Privitera, Wed Jul 20 14:15:34 PDT 2022
//   Added guard to prevent rare index out of bounds error caused by using
//   specific tags and searching simultaneously.
// 
//    Justin Privitera, Wed Aug  3 19:46:13 PDT 2022
//    Added logic to prevent CT from being changed when CT passed out of the 
//    tag filtering selection.
// 
//    Justin Privitera, Fri Sep  2 16:46:21 PDT 2022
//    Logic was added to ensure no desync with the color table atts and to
//    react to color tables outside the filtering selection.
// 
//   Justin Privitera, Wed Sep  6 11:52:18 PDT 2023
//   Fixed bug where the button could have no CT in it when using a config or
//   session file.
//
// ****************************************************************************

void
QvisNoDefaultColorTableButton::setColorTable(const QString &ctName)
{
debug1 << "QvisNoDefaultColorTableButton::setColorTable" << endl;
debug1 <<"    ctName: " << ctName.toStdString() << endl;
    // Is the color table in our local list?
    if (getColorTableIndex(ctName, buttonType) != -1)
    {
        colorTable = ctName;
        setText(colorTable);
        setToolTip(colorTable);
        setIcon(getIcon(colorTable));
    }
    // While the CT is not in our local list, our list is not empty
    else if (colorTableNames[buttonType].size() > 0)
    {
        // if this color table was deleted
        if (colorTableAtts->GetColorTableIndex(ctName.toStdString()) == -1)
        {
            // TODO how could this logic ever be hit?
            if (buttonType == CONT)
                colorTableAtts->SetDefaultContinuous(colorTableNames[buttonType][0].toStdString());
            else
                colorTableAtts->SetDefaultDiscrete(colorTableNames[buttonType][0].toStdString());
            colorTable = colorTableNames[buttonType][0];
            setText(colorTable);
            setToolTip(colorTable);
            setIcon(getIcon(colorTable));
        }
        else
        {
            colorTable = ctName;
            setText(colorTable);
            setToolTip(colorTable);
            setIcon(getIcon(colorTable));
            // TODO is adding the above 4 lines here to this case going to break things?
        }
        
    }
    // The color table is not in our list of color tables because our list is empty...
    // so we can't make any assumptions about its type and must check it at the door
    else if (colorTableAtts->GetColorControlPoints(ctName.toStdString())->GetDiscreteFlag() == buttonType) 
    {
        colorTable = ctName;
        setText(colorTable);
        setToolTip(colorTable);
        setIcon(getIcon(colorTable));
    }
debug1 << "QvisNoDefaultColorTableButton::setColorTable ... done" << endl;
}

// ****************************************************************************
// Method: QvisNoDefaultColorTableButton::getColorTable
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
QvisNoDefaultColorTableButton::getColorTable() const
{
    return colorTable;
}

// ****************************************************************************
// Method: QvisNoDefaultColorTableButton::getButtonType
//
// Purpose: 
//   Returns the button type (continuous or discrete).
//
// Programmer: Justin Privitera
// Creation:   Wed Jul 13 14:48:42 PDT 2022
//
// Modifications:
//   
// ****************************************************************************

const int
QvisNoDefaultColorTableButton::getButtonType() const
{
    return buttonType;
}

//
// Qt slot functions.
//

// ****************************************************************************
// Method: QvisNoDefaultColorTableButton::popupPressed
//
// Purpose: 
//   This is a Qt slot function that pops up the color table popup menu when
//   the button is pressed.
//
// Programmer: Brad Whitlock
// Creation:   Sat Jun 16 20:10:16 PST 2001
//
// Modifications:
//    Justin Privitera, Wed Jul 13 15:19:47 PDT 2022
//    Index into all the vars since they are now arrays.
//   
//    Kathleen Biagas, Wed Apr  5 13:04:35 PDT 2023
//    Replace obosolete desktop() with primaryScreen().
//
// ****************************************************************************

void
QvisNoDefaultColorTableButton::popupPressed()
{
    if (isDown() && colorTableMenu[buttonType])
    {
        // If the popup menu does not have anything in it, fill it up.
        if (!popupHasEntries[buttonType])
            regeneratePopupMenu();

        QPoint p1(mapToGlobal(rect().bottomLeft()));
        QPoint p2(mapToGlobal(rect().topRight()));
        QPoint buttonMiddle(p1.x() + ((p2.x() - p1.x()) >> 1),
                            p1.y() + ((p2.y() - p1.y()) >> 1));

        // Disconnect all other color table buttons.
        for(size_t i = 0; i < buttons.size(); ++i)
            disconnect(colorTableMenuActionGroup[buttonType], SIGNAL(triggered(QAction *)),
                       buttons[i], SLOT(colorTableSelected(QAction *)));
        // Connect this colorbutton to the popup menu.
        connect(colorTableMenuActionGroup[buttonType], SIGNAL(triggered(QAction *)),
                this, SLOT(colorTableSelected(QAction *)));

        // Figure out a good place to popup the menu.
        int menuW = colorTableMenu[buttonType]->sizeHint().width();
        int menuH = colorTableMenu[buttonType]->sizeHint().height();
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
        colorTableMenu[buttonType]->exec(QPoint(menuX, menuY));
        setDown(false);
    }
}

// ****************************************************************************
// Method: QvisNoDefaultColorTableButton::colorTableSelected
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
//   Removed categories/grouping.
// 
//   Justin Privitera, Wed Jul 13 15:19:47 PDT 2022
//   Index into the vars since they are now arrays.
//
// ****************************************************************************

void
QvisNoDefaultColorTableButton::colorTableSelected(QAction *action)
{
    int index = colorTableMenuActionGroup[buttonType]->actions().indexOf(action);
    QString ctName = colorTableNames[buttonType].at(index);

    emit selectedColorTable(ctName);
    setText(ctName);
    setIcon(getIcon(ctName));
    setToolTip(ctName);
}

//
// Static methods...
//

// ****************************************************************************
// Method: QvisNoDefaultColorTableButton::clearAllColorTables
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
//   Justin Privitera, Wed Jul 13 15:19:47 PDT 2022
//   Clear out both lists of color table names and set both popup bools.
//   
// ****************************************************************************

void
QvisNoDefaultColorTableButton::clearAllColorTables()
{
    colorTableNames[DISC].clear();
    colorTableNames[CONT].clear();

    // Clear out the popup menu.
    popupHasEntries[DISC] = false;
    popupHasEntries[CONT] = false;
}

// ****************************************************************************
// Method: QvisNoDefaultColorTableButton::addColorTable
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
//   Removed category arg and removed mappedColorTableNames.
// 
//   Justin Privitera, Wed Jul 13 15:19:47 PDT 2022
//   Figure out the right destination for incoming ctNames.
//
// ****************************************************************************

void
QvisNoDefaultColorTableButton::addColorTable(const QString &ctName)
{
    std::string name_str = ctName.toStdString();
    int index = colorTableAtts->GetColorTableIndex(name_str);
    int ct_discrete = colorTableAtts->GetColorTables(index).GetDiscreteFlag();
    colorTableNames[ct_discrete].append(ctName);
    colorTableNames[ct_discrete].sort();
}

// ****************************************************************************
// Method: QvisNoDefaultColorTableButton::updateColorTableButtons
//
// Purpose: 
//   This is a static method that iterates through all instances of
//   QvisNoDefaultColorTableButton to make sure that the color table that they use is
//   a valid color table. This will also be used to update their color table
//   pixmaps -- someday.
//
// Programmer: Brad Whitlock
// Creation:   Sat Jun 16 20:13:46 PST 2001
//
// Modifications:
//   Justin Privitera, Thu Jun 16 18:01:49 PDT 2022
//   Handle the case where the color tables are gone by doing nothing.
// 
//   Justin Privitera, Wed Jul 13 15:19:47 PDT 2022
//   Added call to getbuttontype() to specify which button.
// 
//   Justin Privitera, Fri Sep  2 16:46:21 PDT 2022
//   Guards are in place now to protect the buttons from falling out of sync
//   with the color table attributes and to ensure that they always represent
//   valid color table choices.
// 
//   Justin Privitera, Mon Aug 21 15:54:50 PDT 2023
//   Changed ColorTableAttributes `names` to `colorTableNames`.
// ****************************************************************************

void
QvisNoDefaultColorTableButton::updateColorTableButtons()
{
    for(size_t i = 0; i < buttons.size(); ++i)
    {
        int myButtonType = buttons[i]->getButtonType();
        auto ctName = buttons[i]->getColorTable();
        // If the color table is in our local (possibly filtered) list of CTs
        if (getColorTableIndex(ctName, myButtonType) != -1)
            buttons[i]->setIcon(getIcon(buttons[i]->text()));
        // Else the color table is not in our local list of CTs
        else
        {
            // If the color table was deleted
            if (colorTableAtts->GetColorTableIndex(ctName.toStdString()) == -1)
            {
                // If there are no CTs of the correct type in the current filtering selection
                if (colorTableNames[myButtonType].isEmpty())
                {
                    // Then we must find any CT that is continuous/discrete as a fall-back
                    for (int i = 0; i < colorTableAtts->GetNumColorTables(); i ++)
                    {
                        // Does this color table match the type of this button?
                        if (colorTableAtts->GetColorTables(i).GetDiscreteFlag() == myButtonType)
                        {
                            std::string myColorTable{colorTableAtts->GetColorTableNames()[i]};
                            if (myButtonType == CONT)
                                colorTableAtts->SetDefaultContinuous(myColorTable);
                            else
                                colorTableAtts->SetDefaultDiscrete(myColorTable);
                            buttons[i]->setColorTable(QString(myColorTable.c_str()));
                            break;
                        }
                    }
                    // It is impossible to delete the last continuous/discrete color table
                    // thanks to code in the CT window so we don't have to worry about not
                    // finding one.
                }
                // Else there are CTs here of the correct type
                else
                {
                    // TODO how do you get to this block?
                    // TODO I hate how this uses a qstring but above we use a string for the same thing
                    // This code might *seem* redundant, but it ensures `setColorTable`
                    // hits the first case, instead of it having to go through
                    // 3 conditions to get to the right behavior.
                    QString myColorTable{colorTableNames[myButtonType][0]};
                    if (myButtonType == CONT)
                        colorTableAtts->SetDefaultContinuous(myColorTable.toStdString());
                    else
                        colorTableAtts->SetDefaultDiscrete(myColorTable.toStdString());
                    buttons[i]->setColorTable(myColorTable);
                }
            }
            // Otherwise, we don't want anything to change.
            // When filtering occurs, it shouldn't change the buttons.
        }
    }
}

// ****************************************************************************
// Method: QvisNoDefaultColorTableButton::getColorTableIndex
//
// Purpose: 
//   Returns the index of the specified color table in the internal color
//   table list. If the color table is not found, -1 is returned.
//
// Arguments:
//   ctName : The name of the color table to look for.
//   whichButton : The index of the ct button.
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
//   Justin Privitera, Wed Jul 13 14:48:42 PDT 2022
//   Added the whichButton argument to take into account the fact that there
//   are 2 buttons with different colorTableNames.
//   
// ****************************************************************************

int
QvisNoDefaultColorTableButton::getColorTableIndex(const QString &ctName,
                                                  const int whichButton)
{
    return colorTableNames[whichButton].indexOf(ctName);
}

// ****************************************************************************
// Method: QvisNoDefaultColorTableButton::regeneratePopupMenu
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
//   Justin Privitera, Wed Jul 13 14:48:42 PDT 2022
//   Most member vars this function deals with are now arrays, so indexing
//   was added.
//
// ****************************************************************************

void
QvisNoDefaultColorTableButton::regeneratePopupMenu()
{
    // Remove all items and add the default.
    QList<QAction*> actions = colorTableMenuActionGroup[buttonType]->actions();
    for (int i = 0; i < actions.count(); ++i)
        colorTableMenuActionGroup[buttonType]->removeAction(actions[i]);
    colorTableMenu[buttonType]->clear();

    // Add an item for each color table.
    for (int i = 0; i < colorTableNames[buttonType].size(); ++i)
    {
        QAction *action = colorTableMenu[buttonType]->addAction(
            makeIcon(colorTableNames[buttonType].at(i)), 
            colorTableNames[buttonType].at(i));
        colorTableMenuActionGroup[buttonType]->addAction(action);
    }

    // Indicate that we've added choices to the menu.
    popupHasEntries[buttonType] = true;
}

// ****************************************************************************
// Method: QvisNoDefaultColorTableButton::getIcon
//
// Purpose: 
//   This method gets the existing icon or makes one if necessary.
//
// Programmer: Brad Whitlock
// Creation:   Wed Apr 25 16:04:54 PDT 2012
//
// Modifications:
//   Justin Privitera, Wed Jul 13 14:48:42 PDT 2022
//   Modified to handle the fact that there are now two lists of actions.
//
// ****************************************************************************

QIcon
QvisNoDefaultColorTableButton::getIcon(const QString &ctName)
{
    QList<QAction*> a1 = colorTableMenuActionGroup[DISC]->actions();
    QList<QAction*> a2 = colorTableMenuActionGroup[CONT]->actions();
    for (int i = 0; i < a1.size(); i ++)
        if (a1[i]->text() == ctName)
            return a1[i]->icon();
    for (int i = 0; i < a2.size(); i ++)
        if (a2[i]->text() == ctName)
            return a2[i]->icon();

    return makeIcon(ctName);
}

// ****************************************************************************
// Method: QvisNoDefaultColorTableButton::makeIcon
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
QvisNoDefaultColorTableButton::makeIcon(const QString &ctName)
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
// Method: QvisNoDefaultColorTableButton::setColorTableAttributes
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
QvisNoDefaultColorTableButton::setColorTableAttributes(ColorTableAttributes *cAtts)
{
    colorTableAtts = cAtts;
}
