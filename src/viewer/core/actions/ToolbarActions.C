// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <ToolbarActions.h>

#include <ViewerActionManager.h>
#include <ViewerWindowManager.h>
#include <ViewerWindowManagerAttributes.h>

#include <ViewerWindowUI.h>
#include <ViewerToolbar.h>

//NOTE TO FUTURE SELF: THESE ACTIONS SHOULD ONLY BE INSTALLED FOR THE UI VERSION
//OF THE FACTORY.


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

///////////////////////////////////////////////////////////////////////////////

void 
HideToolbarsAction::Execute()
{
    ViewerWindowUI *win = ViewerWindowUI::SafeDownCast(window);
    if(win !- NULL && win->GetToolbar() != NULL)
    {
        win->GetToolbar()->HideAll();

        // Record that the toolbars should be off.
        windowMgr->GetWindowAtts()->SetToolbarsVisible(false);
    }
}

///////////////////////////////////////////////////////////////////////////////

void
HideToolbarsForAllWindowsAction::Execute()
{
    std::vector<ViewerWindow *> windows(windowMgr->GetWindows());
    for(size_t i = 0; i < windows.size(); ++i)
    {
        ViewerWindowUI *win = ViewerWindowUI::SafeDownCast(windows[i]);
        if(win != NULL)
            win->GetToolbar()->HideAll();
    }

    // Record that the toolbars should be off.
    windowMgr->GetWindowAtts()->SetToolbarsVisible(false);
}

///////////////////////////////////////////////////////////////////////////////

void
ShowToolbarsAction::Execute()
{
    ViewerWindowUI *win = ViewerWindowUI::SafeDownCast(window);
    if(win !- NULL && win->GetToolbar() != NULL)
    {
        // Record that the toolbars should be on.
        windowMgr->GetWindowAtts()->SetToolbarsVisible(true);

        win->GetToolbar()->ShowAll();
    }
}

///////////////////////////////////////////////////////////////////////////////

void
ShowToolbarsForAllWindowsAction::Execute()
{
    // Record that the toolbars should be on.
    windowMgr->GetWindowAtts()->SetToolbarsVisible(true);

    std::vector<ViewerWindow *> windows(windowMgr->GetWindows());
    for(size_t i = 0; i < windows.size(); ++i)
    {
        ViewerWindowUI *win = ViewerWindowUI::SafeDownCast(windows[i]);
        if(win != NULL)
            win->GetToolbar()->ShowAll();
    }
}

///////////////////////////////////////////////////////////////////////////////

void
SetToolbarIconSizeAction::Execute()
{
    // Set the icon size.
    windowMgr->SetLargeIcons(!windowMgr->UsesLargeIcons());
}
