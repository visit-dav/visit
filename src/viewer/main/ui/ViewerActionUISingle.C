// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <ViewerActionUISingle.h>
#include <QAction>
#include <QIcon>
#include <QMenu>
#include <QToolBar>

// ****************************************************************************
// Method: ViewerActionUISingle::ViewerActionUISingle
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
//   Brad Whitlock, Thu May 22 14:57:42 PDT 2008
//   Qt 4.
//
// ****************************************************************************

ViewerActionUISingle::ViewerActionUISingle(ViewerActionLogic *L) : ViewerActionUI(L)
{
    // Create a new QAction and make it call our Activate method when
    // it is activated.
    action = new QAction(0);
    connect(action, SIGNAL(triggered()),
            this, SLOT(Activate()));
}

// ****************************************************************************
// Method: ViewerActionUISingle::~ViewerActionUISingle
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

ViewerActionUISingle::~ViewerActionUISingle()
{
    if(action)
        delete action;
}

// ****************************************************************************
// Method: ViewerActionUISingle::Update
//
// Purpose: 
//   Updates the action's state which affects how it looks in the toolbar and
//   the menu.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:03:29 PST 2003
//
// Modifications:
//   Brad Whitlock, Thu May 22 15:02:25 PDT 2008
//   Qt 4.
//
// ****************************************************************************
  
void
ViewerActionUISingle::Update()
{
    // Update the action's enabled state.
    bool actionShouldBeEnabled = Enabled();
    if(action->isEnabled() != actionShouldBeEnabled)
        action->setEnabled(actionShouldBeEnabled);

    // Update the action's toggled state if it is a toggle action.
    if(action->isCheckable())
    {
        bool actionShouldBeChecked = Checked();
        if(action->isChecked() != actionShouldBeChecked)
        {
            action->blockSignals(true);
            action->setChecked(actionShouldBeChecked);
            action->blockSignals(false);
        }
    }
}

void
ViewerActionUISingle::SetText(const QString &text)
{
    action->setText(text);
}

void
ViewerActionUISingle::SetMenuText(const QString &text)
{
    action->setText(text);
}

void
ViewerActionUISingle::SetToolTip(const QString &text)
{
    action->setToolTip(text);
}

void
ViewerActionUISingle::SetIcon(const QIcon &icon)
{
    action->setIcon(icon);
}

void
ViewerActionUISingle::SetToggleAction(bool val)
{
    action->setCheckable(val);
}

// ****************************************************************************
// Method: ViewerActionUISingle::ConstructMenu
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
//   Brad Whitlock, Fri May 23 10:24:05 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
ViewerActionUISingle::ConstructMenu(QMenu *menu)
{
    // simplest case
    menu->addAction(action);
}

// ****************************************************************************
// Method: ViewerActionUISingle::RemoveFromMenu
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
//   Brad Whitlock, Fri May 23 10:24:05 PDT 2008
//   Qt 4.
//   
// ****************************************************************************

void
ViewerActionUISingle::RemoveFromMenu(QMenu *menu)
{
    // simplest case
    menu->removeAction(action);
}

// ****************************************************************************
// Method: ViewerActionUISingle::ConstructToolbar
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
//   Brad Whitlock, Thu May 22 15:00:39 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
ViewerActionUISingle::ConstructToolbar(QToolBar *toolbar)
{
    // simplest case
    if(!action->icon().isNull())
        toolbar->addAction(action);
}

// ****************************************************************************
// Method: ViewerActionUISingle::RemoveFromToolbar
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
//   Brad Whitlock, Thu May 22 15:00:27 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
ViewerActionUISingle::RemoveFromToolbar(QToolBar *toolbar)
{
    // simplest case
    if(!action->icon().isNull())
        toolbar->removeAction(action);
}

void
ViewerActionUISingle::UpdateConstruction()
{
}
