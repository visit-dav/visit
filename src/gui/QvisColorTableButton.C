#include <QvisColorTableButton.h>
#include <qpopupmenu.h>
#include <qapplication.h>
#include <qtooltip.h>

//
// Static members.
//

int         QvisColorTableButton::numInstances = 0;
QPopupMenu *QvisColorTableButton::colorTablePopup = 0;
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
// ****************************************************************************

QvisColorTableButton::QvisColorTableButton(QWidget *parent, const char *name) :
    QPushButton(parent,name), colorTable("Default")
{
    // Increase the instance count.
    ++numInstances;

    // Create the button's color table popup menu.
    if(colorTablePopup == 0)
    {
        colorTablePopup = new QPopupMenu;
        colorTablePopup->insertItem("Default", 0);
        colorTablePopup->insertSeparator();
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

    if(numInstances == 0)
    {
        // Delete the popup menu if it exists because it will not be deleted
        // unless we do it since it is a parentless widget.
        if(colorTablePopup != 0)
        {
            delete colorTablePopup;
            colorTablePopup = 0;
        }

        // Delete the color table names.
        delete [] colorTableNames;
        numColorTableNames = 0;
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
     return QSize(125, 25);
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
    QToolTip::add(this, colorTable);
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
        QToolTip::add(this, colorTable);
    }
    else
    {
        QString def("Default");
        setText(def);
        QToolTip::add(this, def);
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
    if(isDown() && colorTablePopup)
    {
        // If the popup menu does not have anything in it, fill it up.
        if(!popupHasEntries)
            regeneratePopupMenu();

        QPoint p1(mapToGlobal(rect().bottomLeft()));
        QPoint p2(mapToGlobal(rect().topRight()));
        QPoint buttonMiddle(p1.x() + ((p2.x() - p1.x()) >> 1),
                            p1.y() + ((p2.y() - p1.y()) >> 1));

        // Disconnect all other color table buttons.
        for(int i = 0; i < buttons.size(); ++i)
        {
            disconnect(colorTablePopup, SIGNAL(activated(int)),
                       buttons[i], SLOT(colorTableSelected(int)));
        }

        // Connect this colorbutton to the popup menu.
        connect(colorTablePopup, SIGNAL(activated(int)),
                this, SLOT(colorTableSelected(int)));

        // Figure out a good place to popup the menu.
        int menuW = colorTablePopup->sizeHint().width();
        int menuH = colorTablePopup->sizeHint().height();
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
        colorTablePopup->exec(QPoint(menuX, menuY));
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
// ****************************************************************************

void
QvisColorTableButton::colorTableSelected(int index)
{
    if(index == 0)
    {
        QString def("Default");
        emit selectedColorTable(true, def);
        setText(def);
        QToolTip::add(this, def);
    }
    else if(index < numColorTableNames + 2)
    {
        emit selectedColorTable(false, colorTableNames[index-2]);
        setText(colorTableNames[index-2]);
        QToolTip::add(this, colorTableNames[index-2]);
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
    for(int i = 0; i < buttons.size(); ++i)
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
//   
// ****************************************************************************

void
QvisColorTableButton::regeneratePopupMenu()
{
    // Remove all items and add the default.
    colorTablePopup->clear();
    colorTablePopup->insertItem("Default", 0);
    colorTablePopup->insertSeparator();
    
    // Add an item for each color table.
    for(int i = 0; i < numColorTableNames; ++i)
        colorTablePopup->insertItem(colorTableNames[i], i + 2);

    // Indicate that we've added choices to the menu.
    popupHasEntries = true;
}
