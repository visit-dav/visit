#include <ViewerActionManager.h>
#include <ViewerAction.h>
#include <ViewerMultipleAction.h>
#include <ViewerPopupMenu.h>
#include <ViewerToggleAction.h>
#include <ViewerToolbar.h>
#include <ViewerWindow.h>

#include <AnimationActions.h>
#include <LockActions.h>
#include <PlotAndOperatorActions.h>
#include <WindowActions.h>
#include <ViewActions.h>

#include <DebugStream.h>
#include <ViewerWindowManager.h>

#include <ActionGroupDescription.h>
#include <ViewerWindowManagerAttributes.h>
#include <DataNode.h>

#include <qaction.h>

// ****************************************************************************
// Class: EnableToolbarAction
//
// Purpose:
//   This action enables or disables toolbars.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 15:51:32 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class EnableToolbarAction : public ViewerMultipleAction
{
public:
    EnableToolbarAction(ViewerWindow *win) :
        ViewerMultipleAction(win, "Enable toolbars")
    {
        SetAllText("Toolbars");
        SetToolTip("Enable toolbars");
        SetExclusive(false);

        // Add all of the action groups as choices.
        ViewerActionManager *actionMgr = window->GetActionManager();
        stringVector names(actionMgr->GetActionGroupNames(false));
        for(int i = 0; i < names.size(); ++i)
            AddChoice(names[i].c_str());
    }

    virtual ~EnableToolbarAction() { }

    virtual void Setup()
    {
        args.SetIntArg1(activeAction);
        args.SetBoolFlag(toggled);
    }

    virtual void Execute(int val)
    {
        // Set the enabled state of the action group.
        ViewerActionManager *actionMgr = window->GetActionManager();
        actionMgr->SetActionGroupEnabled(args.GetIntArg1(),
                                         args.GetBoolFlag());
    }

    virtual bool ChoiceToggled(int i) const
    {
        ViewerActionManager *actionMgr = window->GetActionManager();
        return actionMgr->GetActionGroupEnabled(i);
    }

    virtual bool AllowInToolbar() const { return false; }
};

// ****************************************************************************
// Class: HideToolbarsAction
//
// Purpose:
//   This action temporarily hides toolbars.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 15:52:14 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Feb 24 13:30:24 PST 2004
//   I added code to record that toolbars should be off.
//
// ****************************************************************************

class HideToolbarsAction : public ViewerAction
{
public:
    HideToolbarsAction(ViewerWindow *win) : ViewerAction(win, "HideToolbarsAction")
    {
        SetAllText("Hide toolbars");
    }

    virtual ~HideToolbarsAction() { }

    virtual void Execute()
    {
        ViewerToolbar *tb = window->GetToolbar();
        if(tb)
        {
            tb->HideAll();

            // Record that the toolbars should be off.
            windowMgr->GetWindowAtts()->SetToolbarsVisible(false);
        }
    }

    virtual bool AllowInToolbar() const { return false; }
};

// ****************************************************************************
// Class: HideToolbarsForAllWindowsAction
//
// Purpose:
//   This action temporarily hides toolbars in all viewer windows.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 1 10:34:13 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Feb 24 13:30:24 PST 2004
//   I added code to record that toolbars should be off.
//   
// ****************************************************************************

class HideToolbarsForAllWindowsAction : public ViewerAction
{
public:
    HideToolbarsForAllWindowsAction(ViewerWindow *win) : ViewerAction(win,
        "HideToolbarsForAllWindowsAction")
    {
        SetAllText("Hide toolbars (all windows)");
    }

    virtual ~HideToolbarsForAllWindowsAction() { }

    virtual void Execute()
    {
        windowMgr->HideToolbarsForAllWindows();

        // Record that the toolbars should be off.
        windowMgr->GetWindowAtts()->SetToolbarsVisible(false);
    }

    virtual bool AllowInToolbar() const { return false; }
};

// ****************************************************************************
// Class: ShowToolbarsAction
//
// Purpose:
//   This shows the toolbars that are marked as being visible.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 15:52:35 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Feb 24 13:30:24 PST 2004
//   I added code to record that toolbars should be on.
//   
// ****************************************************************************

class ShowToolbarsAction : public ViewerAction
{
public:
    ShowToolbarsAction(ViewerWindow *win) : ViewerAction(win, "ShowToolbarsAction")
    {
        SetAllText("Show toolbars");
    }

    virtual ~ShowToolbarsAction() { }

    virtual void Execute()
    {
        ViewerToolbar *tb = window->GetToolbar();
        if(tb)
        {
            // Record that the toolbars should be on.
            windowMgr->GetWindowAtts()->SetToolbarsVisible(true);

            tb->ShowAll();
        }
    }

    virtual bool AllowInToolbar() const { return false; }
};

// ****************************************************************************
// Class: ShowToolbarsForAllWindowsAction
//
// Purpose:
//   This action shows toolbars in all viewer windows.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 1 10:34:13 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Feb 24 13:30:24 PST 2004
//   I added code to record that toolbars should be on.
//   
// ****************************************************************************

class ShowToolbarsForAllWindowsAction : public ViewerAction
{
public:
    ShowToolbarsForAllWindowsAction(ViewerWindow *win) : ViewerAction(win,
        "ShowToolbarsForAllWindowsAction")
    {
        SetAllText("Show toolbars (all windows)");
    }

    virtual ~ShowToolbarsForAllWindowsAction() { }

    virtual void Execute()
    {
        // Record that the toolbars should be on.
        windowMgr->GetWindowAtts()->SetToolbarsVisible(true);

        windowMgr->ShowToolbarsForAllWindows();
    }

    virtual bool AllowInToolbar() const { return false; }
};

// ****************************************************************************
// Class: SetToolbarIconSizeAction
//
// Purpose:
//   This action tells the window manager to make all vis windows use either
//   large or small icons in the their toolbars.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 16 09:52:26 PDT 2004
//
// Modifications:
//   Brad Whitlock, Mon Aug 2 10:17:06 PDT 2004
//   Fixed to account for changes in ViewerToggleAction.
//
// ****************************************************************************

class SetToolbarIconSizeAction : public ViewerToggleAction
{
public:
    SetToolbarIconSizeAction(ViewerWindow *win) : ViewerToggleAction(win,
        "SetToolbarIconSizeAction")
    {
        SetAllText("Use large icons (all windows)");
        toggled = !windowMgr->UsesLargeIcons();
    }

    virtual ~SetToolbarIconSizeAction() { }

    virtual void Execute()
    {
        // Set the icon size.
        windowMgr->SetLargeIcons(!windowMgr->UsesLargeIcons());
    }

    //
    // Note: This method is from ViewerToggleAction but was transplanted
    //       here to ensure that the action was updated. When using the
    //       immediate base class's Update method the popup menu was not
    //       updating on some platforms.
    //
    virtual void Update()
    {
        // Update the action's enabled state.
        bool actionShouldBeEnabled = Enabled();
        if(action->isEnabled() != actionShouldBeEnabled)
            action->setEnabled(actionShouldBeEnabled);

        // Update the action's toggled state if it is a toggle action.
        if(action->isToggleAction())
        {
            bool actionShouldBeToggled = Toggled();
            if(toggled != actionShouldBeToggled)
            {
                // Set the appropriate icon into the action.
                if (!window->GetNoWinMode() &&
                    !action->iconSet().pixmap().isNull())
                {
                    if(actionShouldBeToggled)
                        SetIconSet(QIconSet(*toggledIcon));
                    else
                        SetIconSet(QIconSet(*regularIcon));
                }
                action->blockSignals(true);
                action->setOn(actionShouldBeToggled);
                action->blockSignals(false);
            }
            toggled = actionShouldBeToggled;
        }
    }

    virtual bool Toggled() { return windowMgr->UsesLargeIcons(); }

    virtual bool AllowInToolbar() const { return false; }
};

// ****************************************************************************
// Method: ViewerActionManager::ViewerActionManager
//
// Purpose: 
//   This is the constructor for the ViewerActionManager class.
//
// Arguments:
//   win : A pointer to the ViewerWindow that owns this object.
//
// Note:       Here we instantiate all of the actions.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 30 11:01:20 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Feb 25 08:28:50 PDT 2003
//   I added an action for the new SaveViewRPC.
//
//   Brad Whitlock, Thu Feb 27 14:24:23 PST 2003
//   I added ClearWindowAction.
//
//   Brad Whitlock, Wed Mar 12 15:07:40 PST 2003
//   I added AddOperatorAction, RemoveLastOperatorAction,
//   RemoveAllOperatorsAction, AddPlotAction, DrawPlotsAction,
//   HideActivePlotsAction, and DeleteActivePlotsAction.
//
//   Brad Whitlock, Thu Apr 10 09:51:41 PDT 2003
//   I added PromoteOperatorAction, DemoteOperatorAction, and
//   RemoveOperatorAction.
//
//   Brad Whitlock, Fri Apr 11 08:48:02 PDT 2003
//   I converted a lot of plot-related behaviors to actions.
//
//   Kathleen Bonnell, Thu May 15 11:52:56 PDT 2003 
//   Added ToggleFullFrameAction. 
//
//   Brad Whitlock, Mon Jun 23 16:33:39 PST 2003
//   I added ClearPickPointsAction, ClearRefLinesAction.
//
//   Brad Whitlock, Mon Dec 29 09:59:34 PDT 2003
//   I added SetCenterOfRotationAction and ChooseCenterOfRotationAction.
//
//   Brad Whitlock, Sun Jan 25 02:19:07 PDT 2004
//   I added a new animation action and renamed some others.
//
//   Brad Whitlock, Fri Mar 18 17:27:44 PST 2005
//   I added ToggleLockTimeAction.
//
//   Brad Whitlock, Tue Mar 7 17:58:19 PST 2006
//   I added RedoViewAction.
//
// ****************************************************************************


ViewerActionManager::ViewerActionManager(ViewerWindow *win) : actionGroups()
{
    // Create an action array large enough to contain all of the actions.
    actions = new ViewerActionBase *[(int)ViewerRPC::MaxRPC];
    for(int i = 0; i < (int)ViewerRPC::MaxRPC; ++i)
        actions[i] = 0;

    // Add the actions that we want to be available in menus and toolbars.
    AddAction(new ChooseCenterOfRotationAction(win), ViewerRPC::ChooseCenterOfRotationRPC);
    AddAction(new ResetViewAction(win), ViewerRPC::ResetViewRPC);    
    AddAction(new RecenterViewAction(win), ViewerRPC::RecenterViewRPC);
    AddAction(new SaveViewAction(win), ViewerRPC::SaveViewRPC);
    AddAction(new SetCenterOfRotationAction(win), ViewerRPC::SetCenterOfRotationRPC);
    AddAction(new ToggleFullFrameAction(win), ViewerRPC::ToggleFullFrameRPC);
    AddAction(new UndoViewAction(win), ViewerRPC::UndoViewRPC);
    AddAction(new RedoViewAction(win), ViewerRPC::RedoViewRPC);

    AddAction(new ToggleLockViewAction(win), ViewerRPC::ToggleLockViewModeRPC);
    AddAction(new ToggleLockTimeAction(win), ViewerRPC::ToggleLockTimeRPC);

    AddAction(new AddWindowAction(win), ViewerRPC::AddWindowRPC);
    AddAction(new CloneWindowAction(win), ViewerRPC::CloneWindowRPC);
    AddAction(new DeleteWindowAction(win), ViewerRPC::DeleteWindowRPC);
    AddAction(new SetActiveWindowAction(win), ViewerRPC::SetActiveWindowRPC);

    AddAction(new ClearWindowAction(win), ViewerRPC::ClearWindowRPC);
    AddAction(new ClearAllWindowsAction(win), ViewerRPC::ClearAllWindowsRPC);
    AddAction(new ClearPickPointsAction(win), ViewerRPC::ClearPickPointsRPC);
    AddAction(new ClearReferenceLinesAction(win), ViewerRPC::ClearRefLinesRPC);

    AddAction(new InvertBackgroundAction(win), ViewerRPC::InvertBackgroundRPC);
    AddAction(new SetWindowLayoutAction(win), ViewerRPC::SetWindowLayoutRPC);

    AddAction(new SetWindowModeAction(win), ViewerRPC::SetWindowModeRPC);
    AddAction(new EnableToolAction(win), ViewerRPC::EnableToolRPC);

    AddAction(new ToggleSpinModeAction(win), ViewerRPC::ToggleSpinModeRPC);
    AddAction(new TogglePerspectiveViewAction(win), ViewerRPC::TogglePerspectiveViewRPC);
    AddAction(new ToggleBoundingBoxModeAction(win), ViewerRPC::ToggleBoundingBoxModeRPC);

    AddAction(new AnimationStopAction(win), ViewerRPC::AnimationStopRPC);
    AddAction(new TimeSliderForwardStepAction(win), ViewerRPC::TimeSliderNextStateRPC);
    AddAction(new TimeSliderReverseStepAction(win), ViewerRPC::TimeSliderPreviousStateRPC);
    AddAction(new AnimationPlayAction(win), ViewerRPC::AnimationPlayRPC);
    AddAction(new AnimationReversePlayAction(win), ViewerRPC::AnimationReversePlayRPC);
    AddAction(new AnimationSetNFramesAction(win), ViewerRPC::AnimationSetNFramesRPC);
    AddAction(new SetTimeSliderStateAction(win), ViewerRPC::SetTimeSliderStateRPC);
    AddAction(new SetActiveTimeSliderAction(win), ViewerRPC::SetActiveTimeSliderRPC);

    AddAction(new AddOperatorAction(win), ViewerRPC::AddOperatorRPC);
    AddAction(new PromoteOperatorAction(win), ViewerRPC::PromoteOperatorRPC);
    AddAction(new DemoteOperatorAction(win), ViewerRPC::DemoteOperatorRPC);
    AddAction(new RemoveOperatorAction(win), ViewerRPC::RemoveOperatorRPC);
    AddAction(new RemoveLastOperatorAction(win), ViewerRPC::RemoveLastOperatorRPC);
    AddAction(new RemoveAllOperatorsAction(win), ViewerRPC::RemoveAllOperatorsRPC);
    AddAction(new SetOperatorOptionsAction(win), ViewerRPC::SetOperatorOptionsRPC);

    AddAction(new AddPlotAction(win), ViewerRPC::AddPlotRPC);
    AddAction(new DrawPlotsAction(win), ViewerRPC::DrawPlotsRPC);
    AddAction(new HideActivePlotsAction(win), ViewerRPC::HideActivePlotsRPC);
    AddAction(new DeleteActivePlotsAction(win), ViewerRPC::DeleteActivePlotsRPC);
    AddAction(new SetActivePlotsAction(win), ViewerRPC::SetActivePlotsRPC);
    AddAction(new ChangeActivePlotsVarAction(win), ViewerRPC::ChangeActivePlotsVarRPC);
    AddAction(new SetPlotSILRestrictionAction(win), ViewerRPC::SetPlotSILRestrictionRPC);
    AddAction(new SetPlotOptionsAction(win), ViewerRPC::SetPlotOptionsRPC);
    AddAction(new SetPlotFrameRangeAction(win), ViewerRPC::SetPlotFrameRangeRPC);
    AddAction(new DeletePlotKeyframeAction(win), ViewerRPC::DeletePlotKeyframeRPC);
    AddAction(new MovePlotKeyframeAction(win), ViewerRPC::MovePlotKeyframeRPC);
    AddAction(new SetPlotDatabaseStateAction(win), ViewerRPC::SetPlotDatabaseStateRPC);
    AddAction(new DeletePlotDatabaseKeyframeAction(win), ViewerRPC::DeletePlotDatabaseKeyframeRPC);
    AddAction(new MovePlotDatabaseKeyframeAction(win), ViewerRPC::MovePlotDatabaseKeyframeRPC);
}

// ****************************************************************************
// Method: ViewerActionManager::~ViewerActionManager
//
// Purpose: 
//   This is the destructor for the ViewerActionManager class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 30 11:05:19 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

ViewerActionManager::~ViewerActionManager()
{
    for(int i = 0; i < (int)ViewerRPC::MaxRPC; ++i)
        delete actions[i];
    delete [] actions;
}

// ****************************************************************************
// Method: ViewerActionManager::CopyFrom
//
// Purpose: 
//   Copies action attributes from 1 action manager to this action manager.
//
// Arguments:
//   mgr : The source action manager.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 15 17:27:05 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
ViewerActionManager::CopyFrom(const ViewerActionManager *mgr)
{
    for(int i = 0; i < (int)ViewerRPC::MaxRPC; ++i)
    {
        if(actions[i] != 0 && mgr->actions[i] != 0)
            actions[i]->CopyFrom(mgr->actions[i]);
    }
}

// ****************************************************************************
// Method: ViewerActionManager::EnableActions
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
// ****************************************************************************

void
ViewerActionManager::EnableActions(ViewerWindowManagerAttributes *wma)
{
    //
    // Now that we're sure that wma has some action groups in it, use it
    // to create the action groups.
    //    
    for(int i = 0; i < wma->GetNumActionGroupDescriptions(); ++i)
    {
        const ActionGroupDescription &ag = wma->GetActionGroupDescription(i);

        // Create an ActionGroup object based on the ActionGroupDescription
        // object and add the new action group to the manager.
        ActionGroup newAction(ag.GetName());
        newAction.enabled = ag.GetVisible();
        for(int j = 0; j < ag.GetActions().size(); ++j)
        {
            ActionIndex index = ag.GetAction(j);
            if(index != ViewerRPC::MaxRPC)
                newAction.AddAction(index);
            else
            {
                debug1 << "ViewerActionManager::EnableActions: "
                       << ag.GetActions()[j].c_str() << " is not a valid action name" << endl;
            }
        }

        AddActionGroup(newAction);
    }

    //
    // Now that the action groups are defined, add the actions to the menus.
    //
    RealizeActionGroups(wma->GetToolbarsVisible(), wma->GetLargeIcons());
}

// ****************************************************************************
// Method: ViewerActionManager::RealizeActionGroups
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

void
ViewerActionManager::RealizeActionGroups(bool toolbarsVisible, bool largeIcons)
{
    int i;

    // Get a pointer to the viewer window.
    ViewerWindow *win = 0;
    for(i = 0; i < (int)ViewerRPC::MaxRPC; ++i)
    {
        ViewerActionBase *action = GetAction(ActionIndex(i));
        if(action)
        {
            win = action->GetWindow();
            break;
        }
    }

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
        for(int j = 0; j < a.size(); ++j)
        {
            ViewerActionBase *action = GetAction(a[j]);
            if(action && action->VisualEnabled())
            {
                // Add the action to the popup menu.
                win->GetPopupMenu()->AddAction(pos->name, action);

                // Add the action to the toolbar.
                win->GetToolbar()->AddAction(pos->name, action);
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
        ViewerActionBase *action = GetAction(ActionIndex(i));
        if(action && action->VisualEnabled())
        {
            // Add the action to the popup menu.
            if(actionInNGroups[i] == 0 || action->MenuTopLevel())
                win->GetPopupMenu()->AddAction(action);
 
            if(action->CanHaveOwnToolbar())
            {
                ActionGroup newGroup(action->GetName());
                newGroup.AddAction(ActionIndex(i));
                AddActionGroup(newGroup);
                win->GetToolbar()->AddAction(action->GetName(), action);
            }
            else if(actionInNGroups[i] == 0 && action->AllowInToolbar())
            {
                debug1 << "Adding " << ViewerRPC::ViewerRPCType_ToString(ActionIndex(i)).c_str() << " to the general toolbar" << endl;
                win->GetToolbar()->AddAction("general", action);
            }
        }
    }

    delete [] actionInNGroups;

    //
    // These are special actions that modify the toolbar. They have to go
    // here because the EnableToolbarAction action inquires about the action
    // groups in its constructor.
    //
    ViewerActionBase *a1 = new EnableToolbarAction(win);
    ViewerActionBase *a2 = new HideToolbarsAction(win);
    ViewerActionBase *a3 = new HideToolbarsForAllWindowsAction(win);
    ViewerActionBase *a4 = new ShowToolbarsAction(win);
    ViewerActionBase *a5 = new ShowToolbarsForAllWindowsAction(win);
    ViewerActionBase *a6 = new SetToolbarIconSizeAction(win);
    AddAction(a1, ViewerRPC::EnableToolbarRPC);
    AddAction(a2, ViewerRPC::HideToolbarsRPC);
    AddAction(a3, ViewerRPC::HideToolbarsForAllWindowsRPC);
    AddAction(a4, ViewerRPC::ShowToolbarsRPC);
    AddAction(a5, ViewerRPC::ShowToolbarsForAllWindowsRPC);
    AddAction(a6, ViewerRPC::SetToolbarIconSizeRPC);
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
    win->GetPopupMenu()->AddAction("Customize", a1);
    win->GetPopupMenu()->AddAction("Customize", a2);
    win->GetPopupMenu()->AddAction("Customize", a3);
    win->GetPopupMenu()->AddAction("Customize", a4);
    win->GetPopupMenu()->AddAction("Customize", a5);
    win->GetPopupMenu()->AddAction("Customize", a6);

    //
    // Enable or disable the action groups based on user-settings.
    //
    for(i = 0; i < actionGroups.size(); ++i)
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
// Method: ViewerActionManager::UpdateActionConstruction
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
// ****************************************************************************

void
ViewerActionManager::UpdateActionConstruction(ViewerActionBase *action)
{
    //
    // Remove the action from the popup menu and the toolbar.
    //
    ViewerWindow *win = action->GetWindow();
    win->GetPopupMenu()->RemoveAction(action);
    win->GetToolbar()->RemoveAction(action);

    //
    // Add the action back into the menu and toolbar.
    //
    int numMemberships = 0;
    ActionGroupVector::const_iterator pos = actionGroups.begin();
    for(;pos != actionGroups.end(); ++pos)
    {
        const ActionIndexVector &a = pos->actions;
        for(int j = 0; j < a.size(); ++j)
        {
            if(GetAction(a[j]) == action && action->VisualEnabled())
            {
                // Add the action to the popup menu.
                win->GetPopupMenu()->AddAction(pos->name, action);

                // Add the action to the toolbar.
                win->GetToolbar()->AddAction(pos->name, action);

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
    if(action->VisualEnabled())
    {
        
        // Add the action to the popup menu.
        if(numMemberships == 0 || action->MenuTopLevel())
            win->GetPopupMenu()->AddAction(action);
 
        if(action->CanHaveOwnToolbar())
        {
            int i;
            for(i = 0; i < (int)ViewerRPC::MaxRPC; ++i)
                if(actions[i] == action)
                    break;

            ActionGroup newGroup(action->GetName());
            newGroup.AddAction(ActionIndex(i));
            AddActionGroup(newGroup);
            win->GetToolbar()->AddAction(action->GetName(), action);
        }
        else if(numMemberships == 0)
            win->GetToolbar()->AddAction("general", action);
    }
}

// ****************************************************************************
// Method: ViewerActionManager::UpdateActionInformation
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
// ****************************************************************************

void
ViewerActionManager::UpdateActionInformation(ViewerWindowManagerAttributes *wma)
{
    wma->ClearActionGroupDescriptions();

    for(int i = 0; i < actionGroups.size(); ++i)
    {
        const ActionGroup &ag = actionGroups[i];
        if(ag.name != "Customize")
        {
            ActionGroupDescription description(ag.name);
            description.SetVisible(ag.enabled);
            for(int j = 0; j < ag.actions.size(); ++j)
            {
                ViewerActionBase *action = GetAction(ag.actions[j]);
                if(action && !action->CanHaveOwnToolbar())
                {
                    description.AddAction(ag.actions[j]);
                }
            }

            wma->AddActionGroupDescription(description);
        }
    }
}

// ****************************************************************************
// Method: ViewerActionManager::HandleAction
//
// Purpose: 
//   This method calls the action prescribed by the rpc.
//
// Arguments:
//   rpc : The rpc to execute.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 30 11:05:47 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerActionManager::HandleAction(const ViewerRPC &rpc)
{
    ViewerActionBase *action = GetAction(rpc.GetRPCType());
    if(action)
    {
        // Store the action's arguments.
        ViewerActionBase::SetArgs(rpc);

        // Process the action.
        action->Activate(false);
    }
}

// ****************************************************************************
// Method: ViewerActionManager::Update
//
// Purpose: 
//   Updates all of the actions.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 31 12:22:22 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerActionManager::Update()
{
    if(!ViewerWindow::GetNoWinMode())
        ViewerWindowManager::Instance()->UpdateActions();
}

// ****************************************************************************
// Method: ViewerActionManager::UpdateSingleWindow
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
ViewerActionManager::UpdateSingleWindow()
{
    if(!ViewerWindow::GetNoWinMode())
    {
        // Update the actions.
        for(int i = 0; i < (int)ViewerRPC::MaxRPC; ++i)
        {
            ViewerActionBase *action = GetAction(ActionIndex(i));
            if(action)
                action->Update();
        }

        // Update the action groups in the popup menu.
        UpdatePopup();
    }
}

// ****************************************************************************
// Method: ViewerActionManager::UpdatePopup
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
ViewerActionManager::UpdatePopup()
{
    // Enable the action groups that have actions in them.
    for(ActionGroupVector::const_iterator pos = actionGroups.begin();
        pos != actionGroups.end(); ++pos)
    {
        const ActionIndexVector &a = pos->actions;
        bool hasEnabledActions = false;
        ViewerWindow *win = 0;
        for(int i = 0; i < a.size() && !hasEnabledActions; ++i)
        {
            int actionIndex = a[i];
            ViewerActionBase *action = actions[actionIndex];
            if(action)
            {
                hasEnabledActions |= action->Enabled();
                win = action->GetWindow();
            }
        }

        if(win)
        {
            if(hasEnabledActions)
                win->GetPopupMenu()->EnableMenu(pos->name);
            else
                win->GetPopupMenu()->DisableMenu(pos->name);
        }
    }
}

// ****************************************************************************
// Method: ViewerActionManager::GetAction
//
// Purpose: 
//   Returns the action for the specified index.
//
// Arguments:
//   index : The index of the action to return.
//
// Returns:    A pointer to the action.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 30 11:14:24 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

ViewerActionBase *
ViewerActionManager::GetAction(ActionIndex index) const
{
    int i(index);
    return actions[i];
}

// ****************************************************************************
// Method: ViewerActionManager::AddAction
//
// Purpose: 
//   Adds an action to the manager so that when the specified action must be
//   handled, we have an object to handle it.
//
// Arguments:
//   action : The action object to handle the action.
//   index  : The action with which to associate the action object.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 30 11:15:11 PDT 2003
//
// Modifications:
//   Brad Whitlock, Fri Apr 15 09:12:03 PDT 2005
//   Added code to tell the action its RPC type.
//
// ****************************************************************************

void
ViewerActionManager::AddAction(ViewerActionBase *action, ActionIndex index)
{
    int i(index);
    actions[i] = action;
    action->SetRPCType((ViewerRPC::ViewerRPCType)index);
}

// ****************************************************************************
// Method: ViewerActionManager::CreateActionGroup
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
ViewerActionManager::AddActionGroup(const ViewerActionManager::ActionGroup &group)
{
    for(int i = 0; i < actionGroups.size(); ++i)
    {
        if(group.name == actionGroups[i].name)
        {
            for(int j = 0; j < group.actions.size(); ++j)
                actionGroups[i].AddAction(group.actions[j]);
            return;
        }
    }

    // Add the action group to the list.
    actionGroups.push_back(group);
}

// ****************************************************************************
// Method: ViewerActionManager::GetNumberOfActionGroupMemberships
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
ViewerActionManager::GetNumberOfActionGroupMemberships(ActionIndex index) const
{
    int count = 0;
    ViewerActionBase *action = GetAction(index);
    if(action)
    {
        ActionGroupVector::const_iterator pos;
        for(pos = actionGroups.begin(); pos != actionGroups.end(); ++pos)
        {
            const ActionIndexVector &a = pos->actions;
            for(int i = 0; i < a.size(); ++i)
                count += ((a[i] == index) ? 1 : 0);
        }
    }

    return count;
}

// ****************************************************************************
// Method: ViewerActionManager::SetActionGroupEnabled
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
// ****************************************************************************

void
ViewerActionManager::SetActionGroupEnabled(int index, bool val, bool update)
{
    // Look for the named action group.
    if(index >= 0 && index < actionGroups.size())
    {
        // Set the action group's enabled value.
        actionGroups[index].enabled = val;

        // Update the toolbar by hiding or showing it.
        if(update)
        {
            // Look for the first non Null action and get its window.
            for(int i = 0; i < (int)ViewerRPC::MaxRPC; ++i)
            {
                ViewerActionBase *action = GetAction(ActionIndex(i));
                if(action)
                {
                    ViewerToolbar *tb = action->GetWindow()->GetToolbar();
                    if(tb)
                    {
                        if(val)
                            tb->Show(actionGroups[index].name);
                        else
                            tb->Hide(actionGroups[index].name);
                    }
                    break;
                }
            }
        }

        // Update the visible flag in the window manager attributes so
        // that we can pass settings on to newly created windows.
        ViewerWindowManagerAttributes *wma;
        wma = ViewerWindowManager::Instance()->GetWindowAtts();
        if(index < wma->GetNumActionGroupDescriptions())
        {
            ActionGroupDescription &ag = wma->operator[](index);
            ag.SetVisible(val);
        }
    }
}

// ****************************************************************************
// Method: ViewerActionManager::GetActionGroupEnabled
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
ViewerActionManager::GetActionGroupEnabled(int index) const
{
    return (index >= 0 && index < actionGroups.size()) ? actionGroups[index].enabled : false;    
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
ViewerActionManager::GetActionGroupNames(bool evenOnesWithNoToolbars) const
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
// Method: ViewerActionManager::CreateNode
//
// Purpose: 
//   Lets each action add its information to the config file node.
//
// Arguments:
//   parentNode : The node to which data will be added.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 1 10:14:39 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerActionManager::CreateNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *mgrNode = new DataNode("ViewerActionManager");

    // Let each action have the chance to add itself to the node.
    bool addToNode = false;
    for(int i = 0; i < (int)ViewerRPC::MaxRPC; ++i)
    {
        ViewerActionBase *action = GetAction(ActionIndex(i));
        if(action != 0 && action->CreateNode(mgrNode))
            addToNode = true;
    }

    if(addToNode)
        parentNode->AddNode(mgrNode);
    else
        delete mgrNode;
}

// ****************************************************************************
// Method: ViewerActionManager::SetFromNode
//
// Purpose: 
//   Lets each action initialize itself from data in the node.
//
// Arguments:
//   parentNode : The node from which config information is read.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 1 10:15:23 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerActionManager::SetFromNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *mgrNode = parentNode->GetNode("ViewerActionManager");
    if(mgrNode == 0)
        return;

    // Let each action have the chance to initialize itself using the node.
    for(int i = 0; i < (int)ViewerRPC::MaxRPC; ++i)
    {
        ViewerActionBase *action = GetAction(ActionIndex(i));
        if(action != 0)
            action->SetFromNode(mgrNode);
    }
}


//
// ActionGroup
//

ViewerActionManager::ActionGroup::ActionGroup() : name(), actions()
{
    enabled = true;
    canHaveToolbar = true;
}

ViewerActionManager::ActionGroup::ActionGroup(const std::string &n) : name(n), actions()
{
    enabled = true;
    canHaveToolbar = true;
}

ViewerActionManager::ActionGroup::ActionGroup(const ActionGroup &obj)
{
    name = obj.name;
    actions = obj.actions;
    enabled = obj.enabled;
    canHaveToolbar = obj.canHaveToolbar;
}

ViewerActionManager::ActionGroup::~ActionGroup()
{
}

void
ViewerActionManager::ActionGroup::operator =(const ActionGroup &obj)
{
    name = obj.name;
    actions = obj.actions;
    enabled = obj.enabled;
    canHaveToolbar = obj.canHaveToolbar;
}

void
ViewerActionManager::ActionGroup::AddAction(ActionIndex index)
{
    // Return early if the action is already in the list.
    for(int i = 0; i < actions.size(); ++i)
    {
        if(actions[i] == index)
             return;
    }

    // Add the action to the action group.
    actions.push_back(index);
}
