// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <LockActionsUI.h>
#include <ViewerActionLogic.h>
#include <ViewerPlotList.h>
#include <ViewerPopupMenu.h>
#include <ViewerProperties.h>
#include <ViewerWindow.h>
#include <ViewerWindowManager.h>

// Include icons
#include <timelockon.xpm>
#include <timelockoff.xpm>
#include <viewlockon.xpm>
#include <viewlockoff.xpm>
#include <toollockon.xpm>
#include <toollockoff.xpm>

// ****************************************************************************
// Method: ToggleLockViewActionUI::ToggleAllowPopupActionUI
//
// Purpose:
//   Constructor for the ToggleAllowPopupActionUI class.
//
// Arguments:
//   win : The viewer window that owns this ActionUI.
//
// Programmer: Marc Durant
// Creation:   Tue Jan 10 09:18:00 MST 2012
//
// ****************************************************************************
ToggleAllowPopupActionUI::ToggleAllowPopupActionUI(ViewerActionLogic *L) :
  ViewerActionUIToggle(L)
{
    SetAllText(tr("Allow Popup"));
    if (!GetViewerProperties()->GetNowin())
        SetIcons(QPixmap(toollockon_xpm), QPixmap(toollockoff_xpm));
}

bool
ToggleAllowPopupActionUI::Checked() const
{
#if 1
    cerr << "NEED TO IMPLEMENT: ToggleAllowPopupActionUI::Checked()" << endl;
    return true;
#else
    return GetLogic()->GetWindow()->GetPopupMenu()->IsEnabled();
#endif
}

bool
ToggleAllowPopupActionUI::Enabled() const
{
    return false;
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ToggleLockViewActionUI::ToggleLockViewActionUI
//
// Purpose: 
//   Constructor for the ToggleLockViewActionUI class.
//
// Arguments:
//   win : The viewer window that owns this ActionUI.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 18 17:15:36 PST 2005
//
// Modifications:
//   
// ****************************************************************************

ToggleLockViewActionUI::ToggleLockViewActionUI(ViewerActionLogic *L) :
    ViewerActionUIToggle(L)
{
    SetAllText(tr("Lock view"));
    if (!GetViewerProperties()->GetNowin())
        SetIcons(QPixmap(viewlockon_xpm), QPixmap(viewlockoff_xpm));
}

bool
ToggleLockViewActionUI::Enabled() const
{
    // This ActionUI should only be enabled if the window to which the ActionUI belongs
    // has plots in it.
    return (GetLogic()->GetWindow()->GetPlotList()->GetNumPlots() > 0);
}

bool
ToggleLockViewActionUI::Checked() const
{
    return GetLogic()->GetWindow()->GetViewIsLocked();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ToggleLockTimeActionUI::ToggleLockTimeActionUI
//
// Purpose: 
//   Constructor for the ToggleLockTimeActionUI class.
//
// Arguments:
//   win : The viewer window that owns this ActionUI.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 18 17:15:36 PST 2005
//
// Modifications:
//   
// ****************************************************************************

ToggleLockTimeActionUI::ToggleLockTimeActionUI(ViewerActionLogic *L) :
    ViewerActionUIToggle(L)
{
    SetAllText(tr("Lock time"));
    if (!GetViewerProperties()->GetNowin())
        SetIcons(QPixmap(timelockon_xpm), QPixmap(timelockoff_xpm));
}

bool
ToggleLockTimeActionUI::Checked() const
{
    return GetLogic()->GetWindow()->GetTimeLock();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ToggleLockToolActionUI::ToggleLockToolActionUI
//
// Purpose: 
//   Constructor for the ToggleLockToolActionUI class.
//
// Arguments:
//   win : The viewer window that owns this ActionUI.
//
// Programmer: Jeremy Meredith
// Creation:   February 15, 2008
//
// Modifications:
//   
// ****************************************************************************

ToggleLockToolActionUI::ToggleLockToolActionUI(ViewerActionLogic *L) :
    ViewerActionUIToggle(L)
{
    SetAllText(tr("Lock tools"));
    if (!GetViewerProperties()->GetNowin())
        SetIcons(QPixmap(toollockon_xpm), QPixmap(toollockoff_xpm));
}

bool
ToggleLockToolActionUI::Checked() const
{
    return GetLogic()->GetWindow()->GetToolLock();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: TurnOffAllLocksActionUI::TurnOffAllLocksActionUI
//
// Purpose: 
//   Constructor for the TurnOffAllLocksActionUI class.
//
// Arguments:
//   win : The viewer window that owns this ActionUI.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 23 10:35:14 PST 2008
//
// Modifications:
//   
// ****************************************************************************

TurnOffAllLocksActionUI::TurnOffAllLocksActionUI(ViewerActionLogic *L) :
    ViewerActionUISingle(L)
{
    SetAllText(tr("Unlock everything"));
}
