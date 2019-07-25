// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <ToolbarActionsUI.h>

#include <ViewerWindowManager.h>
#include <ViewerWindowManagerAttributes.h>
#include <ViewerActionManager.h>
#include <ViewerActionLogic.h>
#include <ViewerProperties.h>

#include <ViewerPopupMenu.h>
#include <ViewerToolbar.h>
#include <ViewerWindowUI.h>

#include <QAction>

///////////////////////////////////////////////////////////////////////////////

void
EnableToolbarAction::Setup(int activeAction, bool toggled)
{
    args.SetIntArg1(activeAction);
    args.SetBoolFlag(toggled);
}

void
EnableToolbarAction::Execute()
{
    // Set the enabled state of the action group.
    ViewerActionManager *actionMgr = window->GetActionManager();
    actionMgr->SetActionGroupEnabled(args.GetIntArg1(),
                                     args.GetBoolFlag());
}

EnableToolbarActionUI::EnableToolbarActionUI(ViewerActionLogic *L) : ViewerActionUIMultiple(L)
{
    SetAllText(tr("Toolbars"));
    SetToolTip(tr("Enable toolbars"));
    SetExclusive(false);

    // Add all of the action groups as choices.
    ViewerActionManager *actionMgr = GetLogic()->GetWindow()->GetActionManager();
    stringVector names(actionMgr->GetActionGroupNames(false));
    for(size_t i = 0; i < names.size(); ++i)
        AddChoice(names[i].c_str());
}

bool
EnableToolbarActionUI::ChoiceChecked(int i) const
{
    ViewerActionManager *actionMgr = GetLogic()->GetWindow()->GetActionManager();
    return actionMgr->GetActionGroupEnabled(i);
}

///////////////////////////////////////////////////////////////////////////////

void 
HideToolbarsAction::Execute()
{
    ViewerWindowUI *win = ViewerWindowUI::SafeDownCast(window);
    if(win != NULL && win->GetToolbar() != NULL)
    {
        win->GetToolbar()->HideAll();

        // Record that the toolbars should be off.
        ViewerWindowManager::Instance()->GetWindowAtts()->SetToolbarsVisible(false);
    }
}

HideToolbarsActionUI::HideToolbarsActionUI(ViewerActionLogic *L) : ViewerActionUISingle(L)
{
    SetAllText(tr("Hide toolbars"));
}

///////////////////////////////////////////////////////////////////////////////

void
HideToolbarsForAllWindowsAction::Execute()
{
    std::vector<ViewerWindow *> windows(ViewerWindowManager::Instance()->GetWindows());
    for(size_t i = 0; i < windows.size(); ++i)
    {
        ViewerWindowUI *win = ViewerWindowUI::SafeDownCast(windows[i]);
        if(win != NULL)
            win->GetToolbar()->HideAll();
    }

    // Record that the toolbars should be off.
    ViewerWindowManager::Instance()->GetWindowAtts()->SetToolbarsVisible(false);
}

HideToolbarsForAllWindowsActionUI::HideToolbarsForAllWindowsActionUI(ViewerActionLogic *L) : ViewerActionUISingle(L)
{
    SetAllText(tr("Hide toolbars (all windows)"));
}

///////////////////////////////////////////////////////////////////////////////

void
ShowToolbarsAction::Execute()
{
    ViewerWindowUI *win = ViewerWindowUI::SafeDownCast(window);
    if(win != NULL && win->GetToolbar() != NULL)
    {
        // Record that the toolbars should be on.
        ViewerWindowManager::Instance()->GetWindowAtts()->SetToolbarsVisible(true);

        win->GetToolbar()->ShowAll();
    }
}

ShowToolbarsActionUI::ShowToolbarsActionUI(ViewerActionLogic *L) : ViewerActionUISingle(L)
{
    SetAllText(tr("Show toolbars"));
}

////////////////////////////////////////////////////////////////////////////////

void
ShowToolbarsForAllWindowsAction::Execute()
{
    // Record that the toolbars should be on.
    ViewerWindowManager::Instance()->GetWindowAtts()->SetToolbarsVisible(true);

    std::vector<ViewerWindow *> windows(ViewerWindowManager::Instance()->GetWindows());
    for(size_t i = 0; i < windows.size(); ++i)
    {
        ViewerWindowUI *win = ViewerWindowUI::SafeDownCast(windows[i]);
        if(win != NULL)
            win->GetToolbar()->ShowAll();
    }
}

ShowToolbarsForAllWindowsActionUI::ShowToolbarsForAllWindowsActionUI(ViewerActionLogic *L) : ViewerActionUISingle(L)
{
    SetAllText(tr("Show toolbars (all windows)"));
}

////////////////////////////////////////////////////////////////////////////////

void 
SetToolbarIconSizeAction::Setup(int activeAction, bool toggled)
{
    args.SetBoolFlag(toggled);
}

void
SetToolbarIconSizeAction::Execute()
{
    bool usesLargeIcons = args.GetBoolFlag(); //ViewerWindowManager::Instance()->GetWindowAtts()->GetLargeIcons();

    // Set the icon size.
    std::vector<ViewerWindow *> windows(ViewerWindowManager::Instance()->GetWindows());
    for(size_t i = 0; i < windows.size(); ++i)
    {
        ViewerWindowUI *win = ViewerWindowUI::SafeDownCast(windows[i]);
        if(win != NULL)
            win->SetLargeIcons(!usesLargeIcons);
    }
}

SetToolbarIconSizeActionUI::SetToolbarIconSizeActionUI(ViewerActionLogic *L) : ViewerActionUIToggle(L)
{
    SetAllText(tr("Use large icons (all windows)"));
    bool usesLargeIcons = ViewerWindowManager::Instance()->GetWindowAtts()->GetLargeIcons();
    toggled = !usesLargeIcons;
}

//
// Note: This method is from ViewerToggleAction but was transplanted
//       here to ensure that the action was updated. When using the
//       immediate base class's Update method the popup menu was not
//       updating on some platforms.
//
void
SetToolbarIconSizeActionUI::Update()
{
    // Update the action's enabled state.
    bool actionShouldBeEnabled = Enabled();
    if(action->isEnabled() != actionShouldBeEnabled)
        action->setEnabled(actionShouldBeEnabled);

    // Update the action's toggled state if it is a toggle action.
    if(action->isCheckable())
    {
        bool actionShouldBeToggled = Checked();
        if(toggled != actionShouldBeToggled)
        {
            // Set the appropriate icon into the action.
            if (!GetViewerProperties()->GetNowin() &&
                !action->icon().isNull())
            {
                if(actionShouldBeToggled)
                    SetIcon(QIcon(*toggledIcon));
                else
                    SetIcon(QIcon(*regularIcon));
            }
            action->blockSignals(true);
            action->setChecked(actionShouldBeToggled);
            action->blockSignals(false);
        }
        toggled = actionShouldBeToggled;
    }
}

bool
SetToolbarIconSizeActionUI::Checked() const
{
    return ViewerWindowManager::Instance()->GetWindowAtts()->GetLargeIcons();
}

////////////////////////////////////////////////////////////////////////////////

void
ToggleAllowPopupAction::Setup(int activeAction, bool toggled)
{
    args.SetWindowId(window->GetWindowId());
}

// ****************************************************************************
// Method: ViewerWindowManager::ToggleAllowPopup
//
// Purpose:
//   This method toggles the popup menu mode for the specified window.
//
// Arguments:
//    windowIndex : This is a zero-origin integer that specifies the index
//                  of the window we want to change. If the value is -1, use
//                  use the active window.
//
// Programmer: Marc Durant
// Creation:   Tue Jan 10 09:18:00 MST 2012
// ****************************************************************************

void
ToggleAllowPopupAction::Execute()
{
    int index = args.GetWindowId();
    ViewerWindow *win = NULL;
    if(index == -1)
        win = ViewerWindowManager::Instance()->GetActiveWindow();
    else
        win = ViewerWindowManager::Instance()->GetWindow(index);

    ViewerWindowUI *winUI = ViewerWindowUI::SafeDownCast(win);
    if(winUI != NULL)
    {
        ViewerPopupMenu *menu = winUI->GetPopupMenu();
        bool popupAllowed = menu->IsEnabled();
        menu->SetEnabled(!popupAllowed);
        ViewerWindowManager::Instance()->UpdateWindowInformation(WINDOWINFO_WINDOWFLAGS, index);
    }
}

