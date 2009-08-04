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

#include <ViewerMultipleAction.h>
#include <ViewerActionManager.h>
#include <ViewerWindow.h>
#include <QAction>
#include <QIcon>
#include <QMenu>
#include <QToolBar>

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
//   Brad Whitlock, Tue Aug 26 17:12:41 PST 2003
//   I added the isExclusive flag and removed the Selected slot function.
//
// ****************************************************************************

ViewerMultipleAction::ViewerMultipleAction(ViewerWindow *win) : 
    ViewerActionBase(win), children(), text(), menuText(),
    toolTip(), icon()
{
    iconSpecified = false;
    activeAction = 0;
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
// Method: ViewerMultipleAction::ChoiceChecked
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
ViewerMultipleAction::ChoiceChecked(int i) const
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
//   Brad Whitlock, Tue Aug 26 17:13:29 PST 2003
//   I changed the code to fix some problems with the toolbar buttons not
//   updating correctly.
//
//   Brad Whitlock, Tue May 27 13:11:23 PDT 2008
//   Qt 4.
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
    int enabledChildCount = 0;

    // Update the action's enabled state.
    if(action->isEnabled() != actionShouldBeEnabled)
        action->setEnabled(actionShouldBeEnabled);

    // Update the child actions
    bool foundTrue = false;
    for(int i = 0; i < children.size(); ++i)
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
    if(VisualEnabled())
    {
        if(actionMenu->parentWidget() && actionMenu->parentWidget()->inherits("QMenu"))
        {
            actionMenu->setEnabled(enabledChildCount>0);
        }
    }

    action->blockSignals(false);
}

void
ViewerMultipleAction::SetAllText(const QString &text)
{
    SetText(text);
    SetMenuText(text);
    SetToolTip(text);
}

void
ViewerMultipleAction::SetText(const QString &txt)
{
    text = txt;
}

void
ViewerMultipleAction::SetMenuText(const QString &txt)
{
    menuText = txt;
}

void
ViewerMultipleAction::SetToolTip(const QString &txt)
{
    toolTip = txt;
}

void
ViewerMultipleAction::SetIcon(const QIcon &i)
{
    iconSpecified = true;
    icon = i;
}

// ****************************************************************************
// Method: ViewerMultipleAction::AddChoice
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
ViewerMultipleAction::AddChoice(const QString &menuText)
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
ViewerMultipleAction::AddChoice(const QString &menuText, const QString &toolTip,
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
ViewerMultipleAction::AddChoice(const QString &menuText, const QString &toolTip,
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
//   Brad Whitlock, Tue Aug 26 17:16:43 PST 2003
//   Saved the flag in isExclusive instead of setting the value in the action.
//
// ****************************************************************************

void
ViewerMultipleAction::SetExclusive(bool val)
{
    isExclusive = val;
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
//   Brad Whitlock, Fri May 23 12:05:59 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
ViewerMultipleAction::ConstructMenu(QMenu *menu)
{
    // Create a new menu and add all of the actions to it.
    if(iconSpecified)
        actionMenu = menu->addMenu(icon, menuText);
    else
        actionMenu = menu->addMenu(menuText);

    for(int i = 0; i < children.size(); ++i)
        actionMenu->addAction(children[i]);
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
ViewerMultipleAction::RemoveFromMenu(QMenu *menu)
{
    if(actionMenu != 0)
    {
        for(int i = 0; i < children.size(); ++i)
            actionMenu->removeAction(children[i]);
        delete actionMenu;
    }
    actionMenu = 0;
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
//   Brad Whitlock, Tue May 27 13:36:38 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
ViewerMultipleAction::ConstructToolbar(QToolBar *toolbar)
{
    // simplest case
    if(toolbar)
    {
        for(int i = 0; i < action->actions().size(); ++i)
            toolbar->addAction(action->actions().at(i));
    }
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
//   Brad Whitlock, Tue May 27 13:39:16 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
ViewerMultipleAction::RemoveFromToolbar(QToolBar *toolbar)
{
    if(toolbar)
    {
        for(int i = 0; i < action->actions().size(); ++i)
            toolbar->removeAction(action->actions().at(i));
    }
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
ViewerMultipleAction::ActivateHelper(QAction *a)
{
    TRY
    {
        activeAction = action->actions().indexOf(a);
        toggled = a->isChecked();
        Activate();
    }
    CATCHALL
    {
        ; // nothing
    }
    ENDTRY
}

