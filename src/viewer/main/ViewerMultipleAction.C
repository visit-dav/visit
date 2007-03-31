#include <ViewerMultipleAction.h>
#include <ViewerActionManager.h>
#include <ViewerWindow.h>
#include <qaction.h>
#include <qiconset.h>
#include <qpopupmenu.h>
#include <qtoolbar.h>

#include <DebugStream.h>

// ****************************************************************************
// Method: ViewerMultipleAction::ViewerMultipleAction
//
// Purpose: 
//   Constructor for the ViewerMultipleAction class.
//
// Arguments:
//   win  : The window that owns the action.
//   name : The name of the action.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:08:54 PST 2003
//
// Modifications:
//   
// ****************************************************************************

ViewerMultipleAction::ViewerMultipleAction(ViewerWindow *win, const char *name) : 
    ViewerActionBase(win, name), children()
{
    iconSpecified = false;
    activeAction = 0;
    toggled = false;
    actionMenu = 0;
    actionMenuId = -1;

    // Create a new QActionGroup and make it call our Activate method when
    // it is activated.
    QString n; n.sprintf("%s_action", name);
    action = new QActionGroup(0, name);
    connect(action, SIGNAL(selected(QAction *)),
            this, SLOT(Selected(QAction *)));
}

// ****************************************************************************
// Method: ViewerMultipleAction::~ViewerMultipleAction
//
// Purpose: 
//   Destructor for the ViewerMultipleAction class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:08:54 PST 2003
//
// Modifications:
//   
// ****************************************************************************

ViewerMultipleAction::~ViewerMultipleAction()
{
    if(action)
        delete action;
}

// ****************************************************************************
// Method: ViewerMultipleAction::Setup
//
// Purpose: 
//   This method is called when performing the action needs to set some values
//   into the args ViewerRPC.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:08:54 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMultipleAction::Setup()
{
}

// ****************************************************************************
// Method: ViewerMultipleAction::Execute
//
// Purpose: 
//   Executes the action.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:08:54 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMultipleAction::Execute()
{
    Execute(activeAction);
}

// ****************************************************************************
// Method: ViewerMultipleAction::Enabled
//
// Purpose: 
//   Returns whether or not the action should be enabled.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 31 11:00:57 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerMultipleAction::Enabled() const
{
    bool someEnabled = false;
    for(int i = 0; i < children.size() && !someEnabled; ++i)
        someEnabled |= ChoiceEnabled(i);

    return someEnabled;
}

// ****************************************************************************
// Method: ViewerMultipleAction::ChoiceEnabled
//
// Purpose: 
//   Returns whether or not the choice should be enabled.
//
// Arguments:
//   i : The index of the choice to consider.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 31 11:00:57 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerMultipleAction::ChoiceEnabled(int) const
{
    return true;
}

// ****************************************************************************
// Method: ViewerMultipleAction::ChoiceToggled
//
// Purpose: 
//   Returns whether or not the action should be enabled.
//
// Arguments:
//   i : The index of the choice to consider.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 31 11:00:57 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerMultipleAction::ChoiceToggled(int i) const
{
    bool retval = false;

    if(action->isExclusive())
        retval = (i == 0);

    return retval;
}

// ****************************************************************************
// Method: ViewerMultipleAction::Update
//
// Purpose: 
//   Updates the actions so they reflect the correct toggle and enabled states.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 31 11:47:21 PDT 2003
//
// Modifications:
//   Brad Whitlock, Wed Jul 2 09:36:19 PDT 2003
//   Added code to return early if the action or the action menu has not
//   been created yet.
//
// ****************************************************************************

void
ViewerMultipleAction::Update()
{
    if(action == 0 || actionMenu == 0)
        return;

    action->blockSignals(true);

    // See if the action as whole should be enabled.
    bool actionShouldBeEnabled = Enabled();

    // Update the child actions
    int enabledChildCount = 0;
    for(int i = 0; i < children.size(); ++i)
    {
        bool choiceShouldBeEnabled = actionShouldBeEnabled ? ChoiceEnabled(i) : false;
        if(choiceShouldBeEnabled)
            ++enabledChildCount;
        if(children[i]->isEnabled() != choiceShouldBeEnabled)
            children[i]->setEnabled(choiceShouldBeEnabled);

        bool choiceShouldBeToggled = ChoiceToggled(i);
        if(children[i]->isOn() != choiceShouldBeToggled)
        {
            children[i]->blockSignals(true);
            children[i]->setOn(choiceShouldBeToggled);
            children[i]->blockSignals(false);
        }
    }

    // Update the action's enabled state.
    if(action->isEnabled() != actionShouldBeEnabled)
        action->setEnabled(actionShouldBeEnabled);

    // Enable or disable the menu based on how many child actions were enabled.
    if(VisualEnabled())
    {
        if(actionMenu->parentWidget() && actionMenu->parentWidget()->inherits("QPopupMenu"))
        {
            QPopupMenu *popup = (QPopupMenu *)actionMenu->parentWidget();
            popup->setItemEnabled(actionMenuId, enabledChildCount>0);
        }
    }

    action->blockSignals(false);
}

void
ViewerMultipleAction::SetAllText(const char *text)
{
    SetText(text);
    SetMenuText(text);
    SetToolTip(text);
}

void
ViewerMultipleAction::SetText(const char *text)
{
    action->setText(text);
}

void
ViewerMultipleAction::SetMenuText(const char *text)
{
    action->setMenuText(text);
}

void
ViewerMultipleAction::SetToolTip(const char *text)
{
    action->setToolTip(text);
}

void
ViewerMultipleAction::SetIconSet(const QIconSet &icons)
{
    iconSpecified = true;
    action->setIconSet(icons);
}

// ****************************************************************************
// Method: ViewerMultipleAction::ConnectChildAction
//
// Purpose: 
//   Connects a child action to one of the internal helper slots. This is done
//   to make non-exclusive actions work.
//
// Arguments:
//   newAction : the action to connect.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:08:54 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMultipleAction::ConnectChildAction(QAction *newAction)
{
    // If the action is not exclusive then connect the new child action's
    // activated signal to one of our helper slots.
    if(!action->isExclusive())
    {
#define CONNECT_HELPER(I) connect(newAction, SIGNAL(activated()),\
                                  this, SLOT(activate##I()))

        if(children.size() == 0)
            CONNECT_HELPER(0);
        else if(children.size() == 1)
            CONNECT_HELPER(1);
        else if(children.size() == 2)
            CONNECT_HELPER(2);
        else if(children.size() == 3)
            CONNECT_HELPER(3);
        else if(children.size() == 4)
            CONNECT_HELPER(4);
        else if(children.size() == 5)
            CONNECT_HELPER(5);
        else if(children.size() == 6)
            CONNECT_HELPER(6);
        else if(children.size() == 7)
            CONNECT_HELPER(7);
        else if(children.size() == 8)
            CONNECT_HELPER(8);
        else if(children.size() == 9)
            CONNECT_HELPER(9);
        else if(children.size() == 10)
            CONNECT_HELPER(10);
        else if(children.size() == 11)
            CONNECT_HELPER(11);
        else if(children.size() == 12)
            CONNECT_HELPER(12);
        else if(children.size() == 13)
            CONNECT_HELPER(13);
        else if(children.size() == 14)
            CONNECT_HELPER(14);
        else if(children.size() == 15)
            CONNECT_HELPER(15);
        else if(children.size() == 16)
            CONNECT_HELPER(16);
        else if(children.size() == 17)
            CONNECT_HELPER(17);
        else if(children.size() == 18)
            CONNECT_HELPER(18);
        else if(children.size() == 19)
            CONNECT_HELPER(19);
        else
        {
            debug1 << "ViewerMultipleAction::ConnectChildAction: "
                   << "Can't connect another action." << endl;
        }
    }
}

// ****************************************************************************
// Method: ViewerMultipleAction::AddChoice
//
// Purpose: 
//   Adds a choice to the action.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:08:54 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMultipleAction::AddChoice(const char *menuText)
{
    // Create an action that is a child to this action group.
    QAction *newAction = new QAction(0, menuText);
    newAction->setText(menuText);
    newAction->setMenuText(menuText);
    newAction->setToolTip(menuText);
    newAction->setToggleAction(true);
    action->insert(newAction);
    ConnectChildAction(newAction);

    // Save the child pointer for later
    children.push_back(newAction);
}

// ****************************************************************************
// Method: ViewerMultipleAction::AddChoice
//
// Purpose: 
//   Adds a choice to the action.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Sean Ahern
// Creation:   Thu Feb 20 18:29:17 America/Los_Angeles 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMultipleAction::AddChoice(const char *menuText, const char *toolTip,
                                const QPixmap &small_icon,
                                const QPixmap &large_icon)
{
    // Create an action that is a child to this action group.
    QAction *newAction = new QAction(0, menuText);
    newAction->setText(menuText);
    newAction->setMenuText(menuText);
    newAction->setToolTip(toolTip);
    newAction->setIconSet(QIconSet(small_icon, large_icon));
    newAction->setToggleAction(true);
    action->insert(newAction);
    ConnectChildAction(newAction);

    // Save the child pointer for later
    children.push_back(newAction);
}

// ****************************************************************************
// Method: ViewerMultipleAction::AddChoice
//
// Purpose: 
//   Adds a choice to the action.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:08:54 PST 2003
//
// Modifications:
//   
// ****************************************************************************
void
ViewerMultipleAction::AddChoice(const char *menuText, const char *toolTip,
                                const QPixmap &icon)
{
    // Create an action that is a child to this action group.
    QAction *newAction = new QAction(0, menuText);
    newAction->setText(menuText);
    newAction->setMenuText(menuText);
    newAction->setToolTip(toolTip);
    newAction->setIconSet(QIconSet(icon));
    newAction->setToggleAction(true);
    action->insert(newAction);
    ConnectChildAction(newAction);

    // Save the child pointer for later
    children.push_back(newAction);
}

// ****************************************************************************
// Method: ViewerMultipleAction::SetExclusive
//
// Purpose: 
//   Makes the action have radio behavior among its choices.
//
// Arguments:
//   val : Whether or not to have radio behavior.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:08:54 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMultipleAction::SetExclusive(bool val)
{
    // Set the action's exclusive flag.
    action->setExclusive(val);
}

// ****************************************************************************
// Method: ViewerMultipleAction::ConstructMenu
//
// Purpose: 
//   Adds the action to a menu.
//
// Arguments:
//   menu : The menu to which the action is added.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:08:54 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMultipleAction::ConstructMenu(QPopupMenu *menu)
{
    // Create a new menu and add all of the actions to it.
    actionMenu = new QPopupMenu(menu, "ViewerMultipleAction");
    for(int i = 0; i < children.size(); ++i)
        children[i]->addTo(actionMenu);
    // Insert the new menu into the old menu.
    if(iconSpecified)
        actionMenuId = menu->insertItem(action->iconSet(), action->menuText(), actionMenu);
    else
        actionMenuId = menu->insertItem(action->menuText(), actionMenu);
}

// ****************************************************************************
// Method: ViewerMultipleAction::RemoveFromMenu
//
// Purpose: 
//   Removes the action from the menu if it exists in the menu.
//
// Arguments:
//   menu : The menu from which the action will be removed.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 25 10:21:43 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMultipleAction::RemoveFromMenu(QPopupMenu *menu)
{
    // Look for the id in the menu.
    bool hasId = false;
    for(int i = 0; i < menu->count() && !hasId; ++i)
        hasId = (menu->idAt(i) == actionMenuId);
    
    // If the action is in this menu, then remove it.
    if(hasId && action->menuText() == menu->text(actionMenuId))
    {
        for(int i = 0; i < children.size(); ++i)
            children[i]->removeFrom(actionMenu);

        menu->removeItem(actionMenuId);
        actionMenu = 0;
    }
}

// ****************************************************************************
// Method: ViewerMultipleAction::ConstructToolbar
//
// Purpose: 
//   Adds the action to a toolbar.
//
// Arguments:
//   toolbar : The toolbar to which the action is added.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:08:54 PST 2003
//
// Modifications:
//   Brad Whitlock, Mon Jul 28 18:04:57 PST 2003
//   Made sure the pointer to the toolbar was not NULL.
//
// ****************************************************************************

void
ViewerMultipleAction::ConstructToolbar(QToolBar *toolbar)
{
    // simplest case
    if(toolbar)
        action->addTo(toolbar);
}

// ****************************************************************************
// Method: ViewerMultipleAction::RemoveFromToolbar
//
// Purpose: 
//   Removes the action from a toolbar.
//
// Arguments:
//   toolbar : The toolbar from which the action is removed.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 26 08:58:47 PDT 2003
//
// Modifications:
//   Brad Whitlock, Mon Jul 28 18:04:57 PST 2003
//   Made sure the pointer to the toolbar was not NULL.
//   
// ****************************************************************************

void
ViewerMultipleAction::RemoveFromToolbar(QToolBar *toolbar)
{
    if(toolbar)
        action->removeFrom(toolbar);
}

// ****************************************************************************
// Method: ViewerMultipleAction::UpdateConstruction
//
// Purpose: 
//   Tells the action manager to update this action in the toolbar and popup
//   menu. This is used when the action changes its available choices and must
//   be recreated.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 26 08:57:44 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMultipleAction::UpdateConstruction()
{
    window->GetActionManager()->UpdateActionConstruction(this);
}

// ****************************************************************************
// Method: ViewerMultipleAction::ActivateHelper
//
// Purpose: 
//   Helper function for calling Activate.
//
// Arguments:
//   i : The index of the thing to activate.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:08:54 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMultipleAction::ActivateHelper(int i)
{
    activeAction = i;
    toggled = children[i]->isOn();
    Activate();
}

//
// Qt slot functions
//

// ****************************************************************************
// Method: ViewerMultipleAction::Selected
//
// Purpose: 
//   This is a Qt slot function that is called when the action is exclusive and
//   you change the action selection.
//
// Arguments:
//   a : The action that caused the call.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:08:54 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMultipleAction::Selected(QAction *a)
{
    // Set the activeAction member
    for(int i = 0; i < children.size(); ++i)
    {
       if(children[i] == a)
       {
           activeAction = i;
           break;
       }
    }

    Activate();
}

void ViewerMultipleAction::activate0(){ ActivateHelper(0); }
void ViewerMultipleAction::activate1(){ ActivateHelper(1); }
void ViewerMultipleAction::activate2(){ ActivateHelper(2); }
void ViewerMultipleAction::activate3(){ ActivateHelper(3); }
void ViewerMultipleAction::activate4(){ ActivateHelper(4); }
void ViewerMultipleAction::activate5(){ ActivateHelper(5); }
void ViewerMultipleAction::activate6(){ ActivateHelper(6); }
void ViewerMultipleAction::activate7(){ ActivateHelper(7); }
void ViewerMultipleAction::activate8(){ ActivateHelper(8); }
void ViewerMultipleAction::activate9(){ ActivateHelper(9); }
void ViewerMultipleAction::activate10(){ ActivateHelper(10); }
void ViewerMultipleAction::activate11(){ ActivateHelper(11); }
void ViewerMultipleAction::activate12(){ ActivateHelper(12); }
void ViewerMultipleAction::activate13(){ ActivateHelper(13); }
void ViewerMultipleAction::activate14(){ ActivateHelper(14); }
void ViewerMultipleAction::activate15(){ ActivateHelper(15); }
void ViewerMultipleAction::activate16(){ ActivateHelper(16); }
void ViewerMultipleAction::activate17(){ ActivateHelper(17); }
void ViewerMultipleAction::activate18(){ ActivateHelper(18); }
void ViewerMultipleAction::activate19(){ ActivateHelper(19); }




