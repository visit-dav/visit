#include <LockActions.h>
#include <ViewerPlotList.h>
#include <ViewerWindow.h>
#include <ViewerWindowManager.h>

// Include icons
#include <timelockon.xpm>
#include <timelockoff.xpm>
#include <viewlockon.xpm>
#include <viewlockoff.xpm>

// ****************************************************************************
// Method: ToggleLockViewAction::ToggleLockViewAction
//
// Purpose: 
//   Constructor for the ToggleLockViewAction class.
//
// Arguments:
//   win : The viewer window that owns this action.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 18 17:15:36 PST 2005
//
// Modifications:
//   
// ****************************************************************************

ToggleLockViewAction::ToggleLockViewAction(ViewerWindow *win) :
    ViewerToggleAction(win, "ToggleLockViewAction")
{
    SetAllText("Lock view");
    if (!win->GetNoWinMode())
        SetIcons(QPixmap(viewlockon_xpm), QPixmap(viewlockoff_xpm));
}

void
ToggleLockViewAction::Execute()
{
    windowMgr->ToggleLockViewMode(windowId);
}

bool
ToggleLockViewAction::Enabled() const
{
    // This action should only be enabled if the window to which the action belongs
    // has plots in it.
    return (window->GetPlotList()->GetNumPlots() > 0);
}

bool
ToggleLockViewAction::Toggled() const
{
    return window->GetViewIsLocked();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ToggleLockTimeAction::ToggleLockTimeAction
//
// Purpose: 
//   Constructor for the ToggleLockTimeAction class.
//
// Arguments:
//   win : The viewer window that owns this action.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 18 17:15:36 PST 2005
//
// Modifications:
//   
// ****************************************************************************

ToggleLockTimeAction::ToggleLockTimeAction(ViewerWindow *win) :
    ViewerToggleAction(win, "ToggleLockTimeAction")
{
    SetAllText("Lock time");
    if (!win->GetNoWinMode())
        SetIcons(QPixmap(timelockon_xpm), QPixmap(timelockoff_xpm));
}

void
ToggleLockTimeAction::Execute()
{
    windowMgr->ToggleLockTime(windowId);
}

bool
ToggleLockTimeAction::Toggled() const
{
    return window->GetTimeLock();
}
