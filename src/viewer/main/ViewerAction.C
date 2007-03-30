#include <ViewerAction.h>
#include <qaction.h>
#include <qiconset.h>
#include <qpopupmenu.h>
#include <qtoolbar.h>

// ****************************************************************************
// Method: ViewerAction::ViewerAction
//
// Purpose: 
//   Constructor for the ViewerAction class.
//
// Arguments:
//   win  : The window that owns the action.
//   name : The name of the action.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:02:55 PST 2003
//
// Modifications:
//   
// ****************************************************************************

ViewerAction::ViewerAction(ViewerWindow *win, const char *name) : 
    ViewerActionBase(win, name)
{
    // Create a new QAction and make it call our Activate method when
    // it is activated.
    QString n; n.sprintf("%s_action", name);
    action = new QAction(0, name);
    connect(action, SIGNAL(activated()),
            this, SLOT(Activate()));
    connect(action, SIGNAL(toggled(bool)),
            this, SLOT(HandleToggle(bool)));
}

// ****************************************************************************
// Method: ViewerAction::~ViewerAction
//
// Purpose: 
//   Destructor for the ViewerAction class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:03:29 PST 2003
//
// Modifications:
//   
// ****************************************************************************

ViewerAction::~ViewerAction()
{
    if(action)
        delete action;
}

// ****************************************************************************
// Method: ViewerAction::Setup
//
// Purpose: 
//   This method is invoked when the action is activated through a menu or a
//   toolbar. Actions that need to set the args ViewerRPC do it here.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:03:29 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerAction::Setup()
{
}

// ****************************************************************************
// Method: ViewerAction::Update
//
// Purpose: 
//   Updates the action's state which affects how it looks in the toolbar and
//   the menu.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:03:29 PST 2003
//
// Modifications:
//   
// ****************************************************************************
  
void
ViewerAction::Update()
{
    // Update the action's enabled state.
    bool actionShouldBeEnabled = Enabled();
    if(action->isEnabled() != actionShouldBeEnabled)
        action->setEnabled(actionShouldBeEnabled);

    // Update the action's toggled state if it is a toggle action.
    if(action->isToggleAction())
    {
        bool actionShouldBeToggled = Toggled();
        if(action->isOn() != actionShouldBeToggled)
        {
            action->blockSignals(true);
            action->setOn(actionShouldBeToggled);
            action->blockSignals(false);
        }
    }
}

void
ViewerAction::SetAllText(const char *text)
{
    SetText(text);
    SetMenuText(text);
    SetToolTip(text);
}

void
ViewerAction::SetText(const char *text)
{
    action->setText(text);
}

void
ViewerAction::SetMenuText(const char *text)
{
    action->setMenuText(text);
}

void
ViewerAction::SetToolTip(const char *text)
{
    action->setToolTip(text);
}

void
ViewerAction::SetIconSet(const QIconSet &icons)
{
    action->setIconSet(icons);
}

void
ViewerAction::SetToggleAction(bool val)
{
    action->setToggleAction(val);
}

// ****************************************************************************
// Method: ViewerAction::ConstructMenu
//
// Purpose: 
//   Adds the action to a menu.
//
// Arguments:
//   menu : The menu to which the action will be added.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:03:29 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerAction::ConstructMenu(QPopupMenu *menu)
{
    // simplest case
    action->addTo(menu);
}

// ****************************************************************************
// Method: ViewerAction::RemoveFromMenu
//
// Purpose: 
//   Removes the action from a menu.
//
// Arguments:
//   menu : The menu from which to remove the action.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 25 10:04:41 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerAction::RemoveFromMenu(QPopupMenu *menu)
{
    // simplest case
    action->removeFrom(menu);
}

// ****************************************************************************
// Method: ViewerAction::ConstructToolbar
//
// Purpose: 
//   Adds the action to a toolbar.
//
// Arguments:
//   toolbar : The toolbar to which the action will be added.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:03:29 PST 2003
//
// Modifications:
//   Brad Whitlock, Thu Mar 20 14:02:33 PST 2003
//   I added code to prevent actions that have no icons from being added
//   to the toolbar.
//
// ****************************************************************************

void
ViewerAction::ConstructToolbar(QToolBar *toolbar)
{
    // simplest case
    if(!action->iconSet().pixmap().isNull())
        action->addTo(toolbar);
}

// ****************************************************************************
// Method: ViewerAction::RemoveFromToolbar
//
// Purpose: 
//   Removes the action from a toolbar.
//
// Arguments:
//   toolbar : The toolbar from which the action will be removed.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:03:29 PST 2003
//
// Modifications:
//   Brad Whitlock, Thu Mar 20 14:02:33 PST 2003
//   I added code to prevent actions that have no icons from being added
//   to the toolbar.
//
// ****************************************************************************

void
ViewerAction::RemoveFromToolbar(QToolBar *toolbar)
{
    // simplest case
    if(!action->iconSet().pixmap().isNull())
        action->removeFrom(toolbar);
}

// ****************************************************************************
// Method: ViewerAction::HandleToggle
//
// Purpose: 
//   Called when the button is toggled.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:03:29 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerAction::HandleToggle(bool)
{
}
