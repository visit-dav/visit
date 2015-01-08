/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

#include <ViewerActionManagerUI.h>
//#include <ViewerMethods.h>
#include <ViewerProperties.h>
#include <ViewerWindowManager.h>

/*
#include <AnimationActions.h>
#include <LockActions.h>
#include <PlotAndOperatorActions.h>
#include <ToolbarActionsUI.h>
#include <WindowActions.h>
#include <ViewActions.h>*/

#include <ViewerActionUI.h>
#include <ViewerPopupMenu.h>
#include <ViewerToolbar.h>
#include <ViewerWindowUI.h>
#include <ToolbarActionsUI.h>

#include <DebugStream.h>
#include <InstallationFunctions.h>

#include <ActionGroupDescription.h>
#include <ViewerWindowManagerAttributes.h>
#include <DataNode.h>

#include <QAction>

// ****************************************************************************
// Method: ViewerActionManagerUI::ViewerActionManagerUI
//
// Purpose: 
//   This is the constructor for the ViewerActionManagerUI class.
//
// Arguments:
//   win : A pointer to the ViewerWindow that owns this object.
//
// Note:       Here we instantiate all of the actions.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 21 13:24:03 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerActionManagerUI::ViewerActionManagerUI() : ViewerActionManager(),
    actionGroups(), ui(), actionsCreated(false)
{
    // Create an action array large enough to contain all of the actions.
    int n = (int)ViewerRPC::MaxRPC;
    ui.resize(n);
    for(int i = 0; i < n; ++i)
        ui[i] = NULL;
}

// ****************************************************************************
// Method: ViewerActionManagerUI::~ViewerActionManagerUI
//
// Purpose: 
//   This is the destructor for the ViewerActionManagerUI class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 30 11:05:19 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

ViewerActionManagerUI::~ViewerActionManagerUI()
{
}

// ****************************************************************************
// Method: ViewerActionManagerUI::SetUI
//
// Purpose: 
//   Adds an action UI to the manager.
//
// Arguments:
//   index  : The action with which to associate the action object.
//   u      : The UI handler for the object.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 21 13:22:07 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
ViewerActionManagerUI::SetUI(ActionIndex index, ViewerActionUI *u)
{
    int i(index);
    ui[i] = u;
}

// ****************************************************************************
// Method: ViewerActionManagerUI::SetUI
//
// Purpose: 
//   Adds an action UI to the manager.
//
// Arguments:
//   index  : The action with which to associate the action object.
//   u      : The UI handler for the object.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 21 13:22:07 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerActionUI *
ViewerActionManagerUI::GetUI(ActionIndex index)
{
    int i(index);
    return ui[i];
}

// ****************************************************************************
// Method: ViewerActionManagerUI::UpdateSingleWindow
//
// Purpose: 
//   Updates the actions in the window that owns the action manager.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 31 13:15:16 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerActionManagerUI::UpdateSingleWindow()
{
    if(!GetViewerProperties()->GetNowin())
    {
        // Update the actions.
        for(int i = 0; i < (int)ViewerRPC::MaxRPC; ++i)
        {
            if(ui[i] != NULL)
                ui[i]->Update();
        }

        // Update the action groups in the popup menu.
        UpdatePopup();
    }
}

// ****************************************************************************
// Method: ViewerActionManagerUI::SetActionGroupEnabled
//
// Purpose: 
//   Sets whether or not an actiongroup is enabled or disabled.
//
// Arguments:
//   index : The index of the action group.
//   bool  : Whether or not the action group is enabled.
//
// Programmer: Brad Whitlock
// Creation:   Mon Feb 3 10:53:26 PDT 2003
//
// Modifications:
//   Brad Whitlock, Fri Apr 4 16:07:58 PST 2003
//   I added code to update the window manager attributes so new windows
//   get the same toolbars as the last updated window.
//
//   Brad Whitlock, Wed Jan 23 11:12:47 PST 2008
//   Changed because ViewerWindowManagerAttributes was regenerated.
//
// ****************************************************************************

void
ViewerActionManagerUI::SetActionGroupEnabled(int index, bool val, bool update)
{
    // Look for the named action group.
    if(index >= 0 && (size_t)index < actionGroups.size())
    {
        // Set the action group's enabled value.
        actionGroups[index].enabled = val;

        // Update the toolbar by hiding or showing it.
        if(update)
        {
            // Look for the first non Null action and get its window.
            for(int i = 0; i < (int)ViewerRPC::MaxRPC; ++i)
            {
                if(logic[i] != NULL)
                {
                    ViewerWindowUI *win = ViewerWindowUI::SafeDownCast(logic[i]->GetWindow());
                    if(win != NULL && win->GetToolbar() != NULL)
                    {
                        if(val)
                            win->GetToolbar()->Show(actionGroups[index].name);
                        else
                            win->GetToolbar()->Hide(actionGroups[index].name);
                    }
                    break;
                }
            }
        }

        // Update the visible flag in the window manager attributes so
        // that we can pass settings on to newly created windows.
        ViewerWindowManagerAttributes *wma;
        wma = ViewerWindowManager::Instance()->GetWindowAtts();
        if(index < wma->GetNumActionConfigurations())
        {
            ActionGroupDescription &ag = wma->operator[](index);
            ag.SetVisible(val);
        }
    }
}

// ****************************************************************************
// Method: ViewerActionManagerUI::GetActionGroupEnabled
//
// Purpose: 
//   Returns whether or not an action group is enabled.
//
// Arguments:
//   index : The index of the action group.
//
// Programmer: Brad Whitlock
// Creation:   Mon Feb 3 10:54:48 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerActionManagerUI::GetActionGroupEnabled(int index) const
{
    return (index >= 0 && (size_t)index < actionGroups.size()) ? actionGroups[index].enabled : false;    
}

// ****************************************************************************
// Method: GetActionGroupNames
//
// Purpose: 
//   Returns the names of the action groups.
//
// Arguments:
//   evenOnesWithNoToolbars : Makes the action groups with no toolbars be
//                            returned in the list too.
//
// Returns:    A vector of strings containing the names of the action groups.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 15:53:59 PST 2003
//
// Modifications:
//   
// ****************************************************************************

stringVector
ViewerActionManagerUI::GetActionGroupNames(bool evenOnesWithNoToolbars) const
{
    stringVector retval;
    ActionGroupVector::const_iterator pos;
    for(pos = actionGroups.begin(); pos != actionGroups.end(); ++pos)
    {
        if(evenOnesWithNoToolbars || pos->canHaveToolbar)
            retval.push_back(pos->name);
    }

    return retval;
}

// ****************************************************************************
// Method: ViewerActionManagerUI::EnableActions
//
// Purpose: 
//   This method is called when a new window is created. Its job is to
//   process the settings to customize the actions and actiongroups so that
//   the toolbar and popup menu will be customized to user-settings.
//
// Arguments:
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 30 11:07:03 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Feb 25 08:29:40 PDT 2003
//   I added a the new SaveViewRPC to the view action group.
//
//   Brad Whitlock, Thu Feb 27 14:25:13 PST 2003
//   I added ClearWindowRPC to the window action group.
//
//   Brad Whitlock, Wed Mar 12 15:07:19 PST 2003
//   I moved initialization of the default action groups to the
//   ViewerWindowManagerAttributes.
//
//   Brad Whitlock, Tue Feb 24 13:27:48 PST 2004
//   I passed the toolbar's visibility flag to RealizeActionGroups.
//
//   Brad Whitlock, Tue Mar 16 14:24:36 PST 2004
//   I added another flag to RealizeActionGroups.
//
//   Brad Whitlock, Wed Jan 23 11:11:50 PST 2008
//   Changed because ViewerWindowManagerAttributes was regenerated.
//
// ****************************************************************************

void
ViewerActionManagerUI::EnableActions(ViewerWindowManagerAttributes *wma)
{
    if(actionsCreated)
         return;

    //
    // Now that we're sure that wma has some action groups in it, use it
    // to create the action groups.
    //    
    for(int i = 0; i < wma->GetNumActionConfigurations(); ++i)
    {
        const ActionGroupDescription &ag = wma->GetActionConfigurations(i);

        // Create an ActionGroup object based on the ActionGroupDescription
        // object and add the new action group to the manager.
        ActionGroup newAction(ag.GetName());
        newAction.enabled = ag.GetVisible();
        for(int j = 0; j < (int)ag.GetActions().size(); ++j)
        {
            ActionIndex index = ag.GetAction(j);
            if(index != ViewerRPC::MaxRPC)
                newAction.AddAction(index);
            else
            {
                debug1 << "ViewerActionManagerUI::EnableActions: "
                       << ag.GetActions()[j].c_str() << " is not a valid action name" << endl;
            }
        }

        AddActionGroup(newAction);
    }

    //
    // Now that the action groups are defined, add the actions to the menus.
    //
    RealizeActionGroups(wma->GetToolbarsVisible(), wma->GetLargeIcons());
    actionsCreated = true;
}

// ****************************************************************************
// Method: ViewerActionManagerUI::SaveActionGroups
//
// Purpose: 
//   Adds information about the action groups to the window manager attributes.
//
// Arguments:
//   wma : The window manager attributes.
//
// Note:       This is how we update the window manager attributes before we
//             save them. This method populates the part of the window manager
//             attributes that lets us save toolbar settings.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 14:15:57 PST 2003
//
// Modifications:
//   Brad Whitlock, Thu Feb 6 11:14:44 PDT 2003
//   I added code to clear out the action group descriptions stored in the
//   window manager attributes.
//
//   Brad Whitlock, Wed Jan 23 11:12:47 PST 2008
//   Changed because ViewerWindowManagerAttributes was regenerated.
//
// ****************************************************************************

void
ViewerActionManagerUI::SaveActionGroups(ViewerWindowManagerAttributes *wma)
{
    wma->ClearActionConfigurations();

    for(size_t i = 0; i < actionGroups.size(); ++i)
    {
        const ActionGroup &ag = actionGroups[i];
        if(ag.name != "Customize")
        {
            ActionGroupDescription description(ag.name);
            description.SetVisible(ag.enabled);
            for(size_t j = 0; j < ag.actions.size(); ++j)
            {
                ViewerActionUI *action = ui[ag.actions[j]];
                if(action != NULL && !action->CanHaveOwnToolbar())
                {
                    description.AddAction(ag.actions[j]);
                }
            }

            wma->AddActionConfigurations(description);
        }
    }
}

// ****************************************************************************
// Method: ViewerActionManagerUI::UpdateActionConstruction
//
// Purpose: 
//   Removes an action from all of the menus and toolbars to which it belongs
//   and then adds it back so it has the right number of sub actions.
//
// Arguments:
//   action : The action that we're updating in the menu and the toolbar.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 25 09:54:06 PDT 2003
//
// Modifications:
//   Brad Whitlock, Mon Mar 3 08:24:01 PDT 2003
//   Fixed multiple i declaration.
//
//   Brad Whitlock, Fri May 23 10:35:27 PDT 2008
//   Qt 4.
//
//   Brad Whitlock, Mon Aug 18 16:38:16 PDT 2014
//   Updated to ViewerActionUI API.
//
// ****************************************************************************

void
ViewerActionManagerUI::UpdateActionConstruction(ActionIndex idx)
{
    ViewerActionUI *actionUI = ui[idx];
    if(actionUI == NULL)
        return;

    //
    // Remove the action from the popup menu and the toolbar.
    //
    ViewerWindowUI *win = ViewerWindowUI::SafeDownCast(actionUI->GetLogic()->GetWindow());
    if(win == NULL)
        return;
    win->GetPopupMenu()->RemoveAction(actionUI);
    win->GetToolbar()->RemoveAction(actionUI);

    //
    // Add the action back into the menu and toolbar.
    //
    int numMemberships = 0;
    ActionGroupVector::const_iterator pos = actionGroups.begin();
    for(;pos != actionGroups.end(); ++pos)
    {
        const ActionIndexVector &a = pos->actions;
        for(size_t j = 0; j < a.size(); ++j)
        {
            if(ui[a[j]] == actionUI)
            {
                // Add the action to the popup menu.
                win->GetPopupMenu()->AddAction(pos->name, actionUI);

                // Add the action to the toolbar.
                win->GetToolbar()->AddAction(pos->name, actionUI);

                ++numMemberships;
            }
        }
    }

    //
    // Now that we've created the actions in the action groups, iterate
    // through the actions one last time and any that have a non-zero 
    // count in the actionInNGroups array should be added to the
    // general toolbar so that no actions are left out. This also gives
    // us an opportunity to add actions to the top level of the popup menu
    // if they indicate that they prefer to do that.
    //
//    if(action->VisualEnabled())
    {
        
        // Add the action to the popup menu.
        if(numMemberships == 0 || actionUI->MenuTopLevel())
            win->GetPopupMenu()->AddAction(actionUI);
 
        if(actionUI->CanHaveOwnToolbar())
        {
            int i;
            for(i = 0; i < (int)ViewerRPC::MaxRPC; ++i)
                if(ui[i] == actionUI)
                    break;

            std::string actionName(logic[i]->GetName());
            ActionGroup newGroup(actionName);
            newGroup.AddAction(ActionIndex(i));
            AddActionGroup(newGroup);
            win->GetToolbar()->AddAction(actionName, actionUI);
        }
        else if(numMemberships == 0)
            win->GetToolbar()->AddAction("general", actionUI);
    }
}

///////////////////////////////////////////////////////////////////////////////
//// PRIVATE METHODS
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ViewerActionManagerUI::AddActionGroup
//
// Purpose: 
//   Creates an action group with the specified name.
//
// Arguments:
//   name : The name of the action group to create.
//
// Returns:    The id of the new action group.
//
// Note:       An action group is a collection of actions that can be added
//             as a unit into the toolbar or the menu.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 30 11:16:42 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerActionManagerUI::AddActionGroup(const ViewerActionManagerUI::ActionGroup &group)
{
    for(size_t i = 0; i < actionGroups.size(); ++i)
    {
        if(group.name == actionGroups[i].name)
        {
            for(size_t j = 0; j < group.actions.size(); ++j)
                actionGroups[i].AddAction(group.actions[j]);
            return;
        }
    }

    // Add the action group to the list.
    actionGroups.push_back(group);
}

// ****************************************************************************
// Method: ViewerActionManagerUI::RealizeActionGroups
//
// Purpose: 
//   Creates the action groups in the form of menus and toolbars.
//
// Arguments:
//   toolbarsVisible : Whether the toolbar should be visible.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 14:14:03 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Apr 1 10:39:12 PDT 2003
//   I added HideToolbarForAllWindows to the Customize menu.
//
//   Brad Whitlock, Tue Jun 24 13:37:26 PST 2003
//   I changed the code that adds actions to the general toolbar so that
//   actions that are not supposed to be added to the toolbar are not added.
//
//   Brad Whitlock, Tue Feb 24 14:08:48 PST 2004
//   I added an argument to set the visibilty of the toolbar.
//
//   Brad Whitlock, Mon Mar 15 14:55:06 PST 2004
//   I added a new action to set the toolbar's icon size.
//
// ****************************************************************************

#define INSTALL_ACTION_UI(RPC, UI) \
{\
    ViewerActionLogic *logic = GetLogic(RPC);\
    if(logic != NULL)\
        SetUI(RPC, new UI(logic));\
}

void
ViewerActionManagerUI::RealizeActionGroups(bool toolbarsVisible, bool largeIcons)
{
    int i;

    // Get a pointer to the viewer window.
    ViewerWindowUI *win = NULL;
    for(i = 0; i < (int)ViewerRPC::MaxRPC; ++i)
    {
        if(logic[i] != NULL)
        {
            win = ViewerWindowUI::SafeDownCast(logic[i]->GetWindow());
            break;
        }
    }
    if(win == NULL)
        return;

    //
    // Set the window's icon size.
    //
    win->SetLargeIcons(largeIcons);

    //
    // Set whether the toolbar is hidden or showing before it has any
    // toolbars. This way when they do get created, they never get shown
    // if the toolbar is hidden.
    //
    if(toolbarsVisible)
        win->GetToolbar()->ShowAll();
    else
        win->GetToolbar()->HideAll();

    // Determine the number of action groups that each action belongs to.
    int *actionInNGroups = new int[(int)ViewerRPC::MaxRPC];
    for(i = 0; i < (int)ViewerRPC::MaxRPC; ++i)
        actionInNGroups[i] = GetNumberOfActionGroupMemberships(ActionIndex(i));

    //
    // Add the actions in each actiongroup to the menu and the toolbar.
    //
    ActionGroupVector::const_iterator pos = actionGroups.begin();
    for(;pos != actionGroups.end(); ++pos)
    {
        const ActionIndexVector &a = pos->actions;
        for(size_t j = 0; j < a.size(); ++j)
        {
            if(ui[a[j]] != NULL)
            {
                // Add the action to the popup menu.
                win->GetPopupMenu()->AddAction(pos->name, ui[a[j]]);

                // Add the action to the toolbar.
                win->GetToolbar()->AddAction(pos->name, ui[a[j]]);
            }
        }
    }

    //
    // Now that we've created the actions in the action groups, iterate
    // through the actions one last time and any that have a non-zero 
    // count in the actionInNGroups array should be added to the
    // general toolbar so that no actions are left out. This also gives
    // us an opportunity to add actions to the top level of the popup menu
    // if they indicate that they prefer to do that.
    //
    for(i = 0; i < (int)ViewerRPC::MaxRPC; ++i)
    {
        if(ui[i] != NULL)
        {
            // Add the action to the popup menu.
            if(actionInNGroups[i] == 0 || ui[i]->MenuTopLevel())
                win->GetPopupMenu()->AddAction(ui[i]);
 
            if(ui[i]->CanHaveOwnToolbar())
            {
                std::string actionName(ui[i]->GetLogic()->GetName()); // menu text instead?
                ActionGroup newGroup(actionName);
                newGroup.AddAction(ActionIndex(i));
                AddActionGroup(newGroup);
                win->GetToolbar()->AddAction(actionName, ui[i]);
            }
            else if(actionInNGroups[i] == 0 && ui[i]->AllowInToolbar())
            {
                debug1 << "Adding " << ViewerRPC::ViewerRPCType_ToString(ActionIndex(i)).c_str() << " to the general toolbar" << endl;
                win->GetToolbar()->AddAction("general", ui[i]);
            }
        }
    }

    delete [] actionInNGroups;

    //
    // These are special actions that modify the toolbar. They have to go
    // here because the EnableToolbarAction action inquires about the action
    // groups in its constructor.
    //
    // These UI's correspond to actions that only get installed when we have a UI.
    INSTALL_ACTION_UI(ViewerRPC::EnableToolbarRPC,              EnableToolbarActionUI);
    INSTALL_ACTION_UI(ViewerRPC::HideToolbarsRPC,               HideToolbarsActionUI);
    INSTALL_ACTION_UI(ViewerRPC::HideToolbarsForAllWindowsRPC,  HideToolbarsForAllWindowsActionUI);
    INSTALL_ACTION_UI(ViewerRPC::ShowToolbarsRPC,               ShowToolbarsActionUI);
    INSTALL_ACTION_UI(ViewerRPC::ShowToolbarsForAllWindowsRPC,  ShowToolbarsForAllWindowsActionUI);
    INSTALL_ACTION_UI(ViewerRPC::SetToolbarIconSizeRPC,         SetToolbarIconSizeActionUI);

    // Create an action group that contains the toolbar manipulation actions.
    ActionGroup customizeGroup("Customize");
    customizeGroup.canHaveToolbar = false;
    customizeGroup.AddAction(ViewerRPC::EnableToolbarRPC);
    customizeGroup.AddAction(ViewerRPC::HideToolbarsRPC);
    customizeGroup.AddAction(ViewerRPC::HideToolbarsForAllWindowsRPC);
    customizeGroup.AddAction(ViewerRPC::ShowToolbarsRPC);
    customizeGroup.AddAction(ViewerRPC::ShowToolbarsForAllWindowsRPC);
    customizeGroup.AddAction(ViewerRPC::SetToolbarIconSizeRPC);
    AddActionGroup(customizeGroup);
    // Add the actions to the customize menu.
    win->GetPopupMenu()->AddAction("Customize", GetUI(ViewerRPC::EnableToolbarRPC));
    win->GetPopupMenu()->AddAction("Customize", GetUI(ViewerRPC::HideToolbarsRPC));
    win->GetPopupMenu()->AddAction("Customize", GetUI(ViewerRPC::HideToolbarsForAllWindowsRPC));
    win->GetPopupMenu()->AddAction("Customize", GetUI(ViewerRPC::ShowToolbarsRPC));
    win->GetPopupMenu()->AddAction("Customize", GetUI(ViewerRPC::ShowToolbarsForAllWindowsRPC));
    win->GetPopupMenu()->AddAction("Customize", GetUI(ViewerRPC::SetToolbarIconSizeRPC));

    //
    // Enable or disable the action groups based on user-settings.
    //
    for(i = 0; i < (int)actionGroups.size(); ++i)
    {
        SetActionGroupEnabled(i, actionGroups[i].enabled);
    }

    // Show the general toolbar if it exists.
    win->GetToolbar()->Show("general");

    // Update the windows so that the new window and all of the other windows
    // reflect the right state.
    Update();
}

// ****************************************************************************
// Method: ViewerActionManagerUI::GetNumberOfActionGroupMemberships
//
// Purpose: 
//   Returns the number of action groups to which an action belongs.
//
// Arguments:
//   index : The action in which we're interested.
//
// Returns:    The number of action groups to which the action belongs.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 11:28:32 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

int
ViewerActionManagerUI::GetNumberOfActionGroupMemberships(ActionIndex index) const
{
    int count = 0;
    if(ui[index] != NULL)
    {
        ActionGroupVector::const_iterator pos;
        for(pos = actionGroups.begin(); pos != actionGroups.end(); ++pos)
        {
            const ActionIndexVector &a = pos->actions;
            for(size_t i = 0; i < a.size(); ++i)
                count += ((a[i] == index) ? 1 : 0);
        }
    }

    return count;
}

// ****************************************************************************
// Method: ViewerActionManagerUI::UpdatePopup
//
// Purpose: 
//   This method is called when we need to update the popup menu. It enables
//   the various items in the menu based on the actiongroups that have enabled
//   actions.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 31 14:15:18 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerActionManagerUI::UpdatePopup()
{
    // Enable the action groups that have actions in them.
    for(ActionGroupVector::const_iterator pos = actionGroups.begin();
        pos != actionGroups.end(); ++pos)
    {
        const ActionIndexVector &a = pos->actions;
        bool hasEnabledActions = false;
        ViewerWindowUI *win = NULL;
        for(size_t i = 0; i < a.size() && !hasEnabledActions; ++i)
        {
            int actionIndex = a[i];
            if(logic[actionIndex] != NULL && ui[actionIndex] != NULL)
            {
                hasEnabledActions |= ui[actionIndex]->Enabled();
                win = ViewerWindowUI::SafeDownCast(logic[actionIndex]->GetWindow());
            }
        }

        if(win != NULL)
        {
            if(hasEnabledActions)
                win->GetPopupMenu()->EnableMenu(pos->name);
            else
                win->GetPopupMenu()->DisableMenu(pos->name);
        }
    }
}

//
// ActionGroup
//

ViewerActionManagerUI::ActionGroup::ActionGroup() : name(), actions()
{
    enabled = true;
    canHaveToolbar = true;
}

ViewerActionManagerUI::ActionGroup::ActionGroup(const std::string &n) : name(n), actions()
{
    enabled = true;
    canHaveToolbar = true;
}

ViewerActionManagerUI::ActionGroup::ActionGroup(const ActionGroup &obj)
{
    name = obj.name;
    actions = obj.actions;
    enabled = obj.enabled;
    canHaveToolbar = obj.canHaveToolbar;
}

ViewerActionManagerUI::ActionGroup::~ActionGroup()
{
}

void
ViewerActionManagerUI::ActionGroup::operator =(const ActionGroup &obj)
{
    name = obj.name;
    actions = obj.actions;
    enabled = obj.enabled;
    canHaveToolbar = obj.canHaveToolbar;
}

void
ViewerActionManagerUI::ActionGroup::AddAction(ActionIndex index)
{
    // Return early if the action is already in the list.
    for(size_t i = 0; i < actions.size(); ++i)
    {
        if(actions[i] == index)
             return;
    }

    // Add the action to the action group.
    actions.push_back(index);
}
