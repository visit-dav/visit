// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <ViewerActionLogic.h>
#include <ViewerActionManager.h>
#include <ViewerActionUIMultiple.h>
#include <ViewerPlotList.h>
#include <ViewerWindow.h>
#include <QAction>
#include <QActionGroup>
#include <QIcon>
#include <QMenu>
#include <QToolBar>

#include <VisItException.h>
//#include <DebugStream.h>

// ****************************************************************************
// Method: ViewerActionUIMultiple::ViewerActionUIMultiple
//
// Purpose: 
//   Constructor for the ViewerActionUIMultiple class.
//
// Arguments:
//   win  : The window that owns the action.
//   name : The name of the action.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:08:54 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Aug 26 17:12:41 PST 2003
//   I added the isExclusive flag and removed the Selected slot function.
//
// ****************************************************************************

ViewerActionUIMultiple::ViewerActionUIMultiple(ViewerActionLogic *L) : 
    ViewerActionUI(L), text(), menuText(),
    toolTip(), icon(), children()
{
    iconSpecified = false;
    toggled = false;
    actionMenu = 0;
    isExclusive = true;

    // Create a new QActionGroup and make it call our Activate method when
    // it is activated.
    action = new QActionGroup(0);
    action->setExclusive(true);
    connect(action, SIGNAL(triggered(QAction*)),
            this, SLOT(ActivateHelper(QAction*)));
}

// ****************************************************************************
// Method: ViewerActionUIMultiple::~ViewerActionUIMultiple
//
// Purpose: 
//   Destructor for the ViewerActionUIMultiple class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:08:54 PST 2003
//
// Modifications:
//   
// ****************************************************************************

ViewerActionUIMultiple::~ViewerActionUIMultiple()
{
    if(action)
        delete action;
}

// ****************************************************************************
// Method: ViewerActionUIMultiple::Enabled
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
ViewerActionUIMultiple::Enabled() const
{
    bool someEnabled = false;
    for(int i = 0; i < (int)children.size() && !someEnabled; ++i)
        someEnabled |= ChoiceEnabled(i);

    return someEnabled;
}

// ****************************************************************************
// Method: ViewerActionUIMultiple::ChoiceEnabled
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
ViewerActionUIMultiple::ChoiceEnabled(int) const
{
    return true;
}

// ****************************************************************************
// Method: ViewerActionUIMultiple::ChoiceChecked
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
ViewerActionUIMultiple::ChoiceChecked(int i) const
{
    bool retval = false;

    if(action->isExclusive())
        retval = (i == 0);

    return retval;
}

// ****************************************************************************
// Method: ViewerActionUIMultiple::HideChoice
//
// Purpose: 
//   Hides a choice from the action.
//
// Arguments:
//   i : The index of the thing to hide.
//
// Returns:    
//
// Note:       
//
// Programmer: Dirk Schubert (Allinea Software)
// Creation:   Fri Oct 12, 2012
//
// Modifications:
//
// ****************************************************************************

void
ViewerActionUIMultiple::HideChoice(int i)
{
    children[i]->setVisible(false);
}

// ****************************************************************************
// Method: ViewerActionUIMultiple::Update
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
//   Brad Whitlock, Tue Aug 26 17:13:29 PST 2003
//   I changed the code to fix some problems with the toolbar buttons not
//   updating correctly.
//
//   Brad Whitlock, Tue May 27 13:11:23 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
ViewerActionUIMultiple::Update()
{
    if(action == 0 || actionMenu == 0)
        return;

    action->blockSignals(true);

    // See if the action as whole should be enabled.
    bool actionShouldBeEnabled = Enabled();
    int enabledChildCount = 0;

    // Update the action's enabled state.
    if(action->isEnabled() != actionShouldBeEnabled)
        action->setEnabled(actionShouldBeEnabled);

    // Update the child actions
    bool foundTrue = false;
    for(int i = 0; i < (int)children.size(); ++i)
    {
        bool choiceShouldBeEnabled = actionShouldBeEnabled ? ChoiceEnabled(i) : false;
        if(choiceShouldBeEnabled)
            ++enabledChildCount;
  
        bool choiceShouldBeToggled = ChoiceChecked(i);

        if(children[i]->isChecked() != choiceShouldBeToggled)
        {
            bool toggled = false;
            if(isExclusive)
            {
                if(!foundTrue)
                {
                    toggled = choiceShouldBeToggled;
                    if(choiceShouldBeToggled)
                        foundTrue = true;
                }
                else
                    toggled = false;
            }
            else
                toggled = choiceShouldBeToggled;

            children[i]->blockSignals(true);
            children[i]->setChecked(toggled);
            children[i]->blockSignals(false);
        }

        if(children[i]->isEnabled() != choiceShouldBeEnabled)
            children[i]->setEnabled(choiceShouldBeEnabled);
    }   

    // Enable or disable the menu based on how many child actions were enabled.
    if(actionMenu->parentWidget() && actionMenu->parentWidget()->inherits("QMenu"))
    {
        actionMenu->setEnabled(enabledChildCount>0);
    }

    // Summarily enable the Add plot menu whenever database is open
    bool dbIsOpen = (GetLogic()->GetWindow()->GetPlotList()->GetHostDatabaseName().length() > 0);
    if (dbIsOpen && menuText == "Add plot")
        actionMenu->setEnabled(true);

    action->blockSignals(false);
}

void
ViewerActionUIMultiple::SetText(const QString &txt)
{
    text = txt;
}

void
ViewerActionUIMultiple::SetMenuText(const QString &txt)
{
    menuText = txt;
}

void
ViewerActionUIMultiple::SetToolTip(const QString &txt)
{
    toolTip = txt;
}

void
ViewerActionUIMultiple::SetIcon(const QIcon &i)
{
    iconSpecified = true;
    icon = i;
}

// ****************************************************************************
// Method: ViewerActionUIMultiple::AddChoice
//
// Purpose: 
//   Adds a choice to the action.
//
// Arguments:
//   menuText : The text that will appear in the menu.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:08:54 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Aug 26 17:34:15 PST 2003
//   Made the new action be a child of the action group.
//
//   Brad Whitlock, Tue Apr 29 11:18:06 PDT 2008
//   Use QString.
//
// ****************************************************************************

void
ViewerActionUIMultiple::AddChoice(const QString &menuText)
{
    // Create an action that is a child to this action group.
    QAction *newAction = new QAction(action);
    newAction->setText(menuText);
    newAction->setToolTip(menuText);
    newAction->setCheckable(true);

    // Save the child pointer for later
    children.push_back(newAction);
}

// ****************************************************************************
// Method: ViewerActionUIMultiple::AddChoice
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
//   Brad Whitlock, Tue Aug 26 17:34:15 PST 2003
//   Made the new action be a child of the action group.
//
//   Brad Whitlock, Tue Apr 29 11:18:58 PDT 2008
//   Use QString.
//
//   Brad Whitlock, Tue May 27 13:23:41 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
ViewerActionUIMultiple::AddChoice(const QString &menuText, const QString &toolTip,
                                const QPixmap &small_icon,
                                const QPixmap &large_icon)
{
    // Create an action that is a child to this action group.
    QAction *newAction = new QAction(action);
    newAction->setText(menuText);
    newAction->setToolTip(toolTip);
    newAction->setIcon(QIcon(large_icon));
    newAction->setCheckable(true);

    // Save the child pointer for later
    children.push_back(newAction);
}

// ****************************************************************************
// Method: ViewerActionUIMultiple::AddChoice
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
//   Brad Whitlock, Tue Aug 26 17:34:15 PST 2003
//   Made the new action be a child of the action group.
//
//   Brad Whitlock, Tue Apr 29 11:19:24 PDT 2008
//   Use QString.
//
//   Brad Whitlock, Tue May 27 13:24:16 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
ViewerActionUIMultiple::AddChoice(const QString &menuText, const QString &toolTip,
                                const QPixmap &icon)
{
    // Create an action that is a child to this action group.
    QAction *newAction = new QAction(action);
    newAction->setText(menuText);
    newAction->setToolTip(toolTip);
    newAction->setIcon(QIcon(icon));
    newAction->setCheckable(true);

    // Save the child pointer for later
    children.push_back(newAction);
}

// ****************************************************************************
// Method: ViewerActionUIMultiple::SetExclusive
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
//   Brad Whitlock, Tue Aug 26 17:16:43 PST 2003
//   Saved the flag in isExclusive instead of setting the value in the action.
//
// ****************************************************************************

void
ViewerActionUIMultiple::SetExclusive(bool val)
{
    isExclusive = val;
    action->setExclusive(val);
}

// ****************************************************************************
// Method: ViewerActionUIMultiple::ConstructMenu
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
//   Brad Whitlock, Fri May 23 12:05:59 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
ViewerActionUIMultiple::ConstructMenu(QMenu *menu)
{
    // Create a new menu and add all of the actions to it.
    if(iconSpecified)
        actionMenu = menu->addMenu(icon, menuText);
    else
        actionMenu = menu->addMenu(menuText);

    for(size_t i = 0; i < children.size(); ++i)
        actionMenu->addAction(children[i]);
}

// ****************************************************************************
// Method: ViewerActionUIMultiple::RemoveFromMenu
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
ViewerActionUIMultiple::RemoveFromMenu(QMenu *menu)
{
    if(actionMenu != 0)
    {
        for(size_t i = 0; i < children.size(); ++i)
            actionMenu->removeAction(children[i]);
        delete actionMenu;
    }
    actionMenu = 0;
}

// ****************************************************************************
// Method: ViewerActionUIMultiple::ConstructToolbar
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
//   Brad Whitlock, Tue May 27 13:36:38 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
ViewerActionUIMultiple::ConstructToolbar(QToolBar *toolbar)
{
    // simplest case
    if(toolbar)
    {
        for(int i = 0; i < action->actions().size(); ++i)
            toolbar->addAction(action->actions().at(i));
    }
}

// ****************************************************************************
// Method: ViewerActionUIMultiple::RemoveFromToolbar
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
//   Brad Whitlock, Tue May 27 13:39:16 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
ViewerActionUIMultiple::RemoveFromToolbar(QToolBar *toolbar)
{
    if(toolbar)
    {
        for(int i = 0; i < action->actions().size(); ++i)
            toolbar->removeAction(action->actions().at(i));
    }
}

// ****************************************************************************
// Method: ViewerActionUIMultiple::UpdateConstruction
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
ViewerActionUIMultiple::UpdateConstruction()
{
    GetLogic()->GetWindow()->GetActionManager()->UpdateActionConstruction(
        GetLogic()->GetRPCType());
}

// ****************************************************************************
// Method: ViewerActionUIMultiple::ActivateHelper
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
//   Brad Whitlock, Tue Aug 5 23:05:34 PST 2008
//   Changed the implementation.
//
//   Brad Whitlock, Fri Jan 9 15:07:35 PST 2009
//   Added exception handling to prevent exceptions from being propagated into
//   the Qt event loop.
//
//   Mark C. Miller, Wed Jun 17 17:46:18 PDT 2009
//   Replaced CATCHALL(...) with CATCHALL
// ****************************************************************************

void
ViewerActionUIMultiple::ActivateHelper(QAction *a)
{
    TRY
    {
        SetActiveAction(action->actions().indexOf(a));
        toggled = a->isChecked();
        Activate();
    }
    CATCHALL
    {
        ; // nothing
    }
    ENDTRY
}

// ****************************************************************************
// Method: ViewerActionUIMultiple::PostponeExecute
//
// Purpose:
//   This method is called when we want to postpone the execution of the
//   action until later when the event loop is resumed.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 21 23:11:30 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void 
ViewerActionUIMultiple::PostponeExecute()
{
    GetLogic()->PostponeExecute(GetActiveAction(), toggled);
}
