// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <LockActions.h>
#include <ViewerWindow.h>
#include <ViewerWindowManager.h>

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
    ViewerActionLogic(win)
{
}

void
ToggleLockViewAction::Execute()
{
    windowMgr->ToggleLockViewMode(GetWindow()->GetWindowId());
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
    ViewerActionLogic(win)
{
}

void
ToggleLockTimeAction::Execute()
{
    windowMgr->ToggleLockTime(GetWindow()->GetWindowId());
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ToggleLockToolAction::ToggleLockToolAction
//
// Purpose: 
//   Constructor for the ToggleLockToolAction class.
//
// Arguments:
//   win : The viewer window that owns this action.
//
// Programmer: Jeremy Meredith
// Creation:   February 15, 2008
//
// Modifications:
//   
// ****************************************************************************

ToggleLockToolAction::ToggleLockToolAction(ViewerWindow *win) :
    ViewerActionLogic(win)
{
}

void
ToggleLockToolAction::Execute()
{
    windowMgr->ToggleLockTools(GetWindow()->GetWindowId());
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: TurnOffAllLocksAction::TurnOffAllLocksAction
//
// Purpose: 
//   Constructor for the TurnOffAllLocksAction class.
//
// Arguments:
//   win : The viewer window that owns this action.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 23 10:35:14 PST 2008
//
// Modifications:
//   
// ****************************************************************************

TurnOffAllLocksAction::TurnOffAllLocksAction(ViewerWindow *win) :
    ViewerActionLogic(win)
{
}

// ****************************************************************************
// Method: TurnOffAllLocksAction::Execute
//
// Purpose: 
//   Turns off all of the locks for all windows.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 23 10:36:28 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
TurnOffAllLocksAction::Execute()
{
    windowMgr->TurnOffAllLocks();
}
