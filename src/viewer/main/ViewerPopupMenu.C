#include <ViewerPopupMenu.h>
#include <ViewerActionBase.h>
#include <ViewerWindow.h>

#include <qpopupmenu.h>
#include <qbuttongroup.h>
#include <qcursor.h>
#include <qradiobutton.h>
#include <iostream.h>

// ****************************************************************************
//  Method: ViewerPopupMenu::ViewerPopupMenu
//
//  Purpose: 
//    Constructor for the ViewerPopupMenu class.
//
//  Arguments:
//    win : A pointer to the window that owns this menu.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Nov 7 12:02:49 PDT 2000
//
//  Modifications:
//
// ****************************************************************************

ViewerPopupMenu::ViewerPopupMenu(ViewerWindow *win) : menus()
{
    //
    // Keep a pointer to the window that created this menu.
    //
    window = win;

    //
    // Create the popup menu widget.
    //
    if(win->GetNoWinMode())
        popup = 0;
    else
    {
        QString name;
        name.sprintf("ViewerPopupMenu%d", win->GetWindowId());
        popup = new QPopupMenu(0, name);
    }
}

// ****************************************************************************
//  Method: ViewerPopupMenu::~ViewerPopupMenu
//
//  Purpose: 
//    Destructor for the ViewerPopupMenu class.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Nov 7 12:04:02 PDT 2000
//
//  Modifications:
//   
// ****************************************************************************

ViewerPopupMenu::~ViewerPopupMenu()
{
}

// ****************************************************************************
//  Method: ViewerPopupMenu::ShowMenu
//
//  Purpose: 
//    Shows the popup menu at the current cursor location.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Nov 7 12:04:21 PDT 2000
//
//  Modifications:
//   Brad Whitlock, Mon Nov 4 13:12:51 PST 2002
//   I made the menu show manually when it is disabled because otherwise it
//   does not show up.
//
// ****************************************************************************

void
ViewerPopupMenu::ShowMenu()
{
    if(popup)
    {
        if(popup->isEnabled())
            popup->popup(QCursor::pos());
        else
        {
            popup->move(QCursor::pos());
            popup->show();
        }
    }
}

// ****************************************************************************
//  Method: ViewerPopupMenu::HideMenu
//
//  Purpose: 
//    Hides the popup menu.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Nov 7 12:04:49 PDT 2000
//
//  Modifications:
//   
// ****************************************************************************

void
ViewerPopupMenu::HideMenu()
{
    if(popup)
        popup->hide();
}

// ****************************************************************************
// Method: ViewerPopupMenu::SetEnabled
//
// Purpose: 
//   Sets the enabled state of the viewer popup menu.
//
// Arguments:
//   val : The new enabled state.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 30 15:25:03 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerPopupMenu::SetEnabled(bool val)
{
    if(popup)
        popup->setEnabled(val);
}

// ****************************************************************************
// Method: ViewerPopupMenu::AddAction
//
// Purpose: 
//   Lets the action add its own entry into the menu.
//
// Arguments:
//   action : The action that wants to be added to the menu.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 29 14:44:18 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerPopupMenu::AddAction(ViewerActionBase *action)
{
    if(popup)
        action->ConstructMenu(popup);
}

// ****************************************************************************
// Method: ViewerPopupMenu::AddAction
//
// Purpose: 
//   Lets the action add its own entry into the menu.
//
// Arguments:
//   menuName : The name of the menu to which the action will be added.
//   action   : The action that wants to be added to the menu.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 29 14:44:18 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerPopupMenu::AddAction(const std::string &menuName, ViewerActionBase *action)
{
    if(popup)
    {
        QPopupMenu *menu = CreateMenu(menuName);
        if(menu)
            action->ConstructMenu(menu);
    }
}

// ****************************************************************************
// Method: ViewerPopupMenu::RemoveAction
//
// Purpose: 
//   Lets the action remove itself from all of the menus.
//
// Arguments:
//   action : The action to remove from the menus.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 25 10:14:25 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerPopupMenu::RemoveAction(ViewerActionBase *action)
{
    if(popup)
    {
        action->RemoveFromMenu(popup);
        MenuMap::iterator pos;
        for(pos = menus.begin(); pos != menus.end(); ++pos)
            action->RemoveFromMenu(pos->second.menu);
    }
}

// ****************************************************************************
// Method: ViewerPopupMenu::EnableMenu
//
// Purpose: 
//   Enables a submenu.
//
// Arguments:
//   menuName : The name of the menu to Enable.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 31 14:13:25 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerPopupMenu::EnableMenu(const std::string &menuName)
{
    if(popup)
    {
        MenuMap::iterator pos;
        if((pos = menus.find(menuName)) != menus.end())
            popup->setItemEnabled(pos->second.menuId, true);
    }
}

// ****************************************************************************
// Method: ViewerPopupMenu::DisableMenu
//
// Purpose: 
//   Disables a submenu.
//
// Arguments:
//   menuName : The name of the menu to disable.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 31 14:13:25 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerPopupMenu::DisableMenu(const std::string &menuName)
{
    if(popup)
    {
        MenuMap::iterator pos;
        if((pos = menus.find(menuName)) != menus.end())
            popup->setItemEnabled(pos->second.menuId, false);
    }
}

// ****************************************************************************
// Method: ViewerPopupMenu::CreateMenu
//
// Purpose: 
//   Creates a menu with the given name.
//
// Arguments:
//   menuName : The name of the menu to create.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 30 10:56:12 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

QPopupMenu *
ViewerPopupMenu::CreateMenu(const std::string &menuName)
{
    if(popup)
    {
        MenuMap::iterator pos;
        if((pos = menus.find(menuName)) != menus.end())
            return pos->second.menu;
        else
        {
            // Create a new popup menu and store it in the map.
            QString name;
            name.sprintf("Menu_%s_%d", menuName.c_str(), window->GetWindowId());
            QPopupMenu *menu = new QPopupMenu(popup, name);

            // Insert the new popup menu into the parent popup menu.
            int id = popup->insertItem(menuName.c_str(), menu);

            // Store the menu pointer and id for later.
            menus[menuName] = SubMenuInfo(menu, id);

            return menu;
        }
    }

    return 0;
}

//
// ViewerPopupMenu::SubMenuInfo class
//

ViewerPopupMenu::SubMenuInfo::SubMenuInfo()
{
    menu = 0;
    menuId = 0;
}

ViewerPopupMenu::SubMenuInfo::SubMenuInfo(const ViewerPopupMenu::SubMenuInfo &obj)
{
    menu = obj.menu;
    menuId = obj.menuId;
}

ViewerPopupMenu::SubMenuInfo::SubMenuInfo(QPopupMenu *m, int id)
{
    menu = m;
    menuId = id;
}

ViewerPopupMenu::SubMenuInfo::~SubMenuInfo()
{
}

void
ViewerPopupMenu::SubMenuInfo::operator =(const ViewerPopupMenu::SubMenuInfo &obj)
{
    menu = obj.menu;
    menuId = obj.menuId;
}
