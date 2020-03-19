// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEWER_LOCK_ACTIONS_UI_H
#define VIEWER_LOCK_ACTIONS_UI_H
#include <viewer_exports.h>
#include <ViewerActionUIToggle.h>

// ****************************************************************************
// Class: ToggleAllowPopupActionUI
//
// Purpose:
//   Handles the toggle allow popup action.
//
// Notes:
//
// Programmer: Marc Durant
// Creation:   Tue Jan 10 09:18:00 MST 2012
//
// ****************************************************************************

class VIEWER_API ToggleAllowPopupActionUI : public ViewerActionUIToggle
{
public:
    ToggleAllowPopupActionUI(ViewerActionLogic *L);
    virtual ~ToggleAllowPopupActionUI(){}

    virtual bool Enabled() const;
    virtual bool Checked() const;
};

// ****************************************************************************
// Class: ToggleLockViewActionUI
//
// Purpose:
//   Handles the lock view action.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 16:44:34 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API ToggleLockViewActionUI : public ViewerActionUIToggle
{
public:
    ToggleLockViewActionUI(ViewerActionLogic *L);
    virtual ~ToggleLockViewActionUI(){}

    virtual bool Enabled() const;
    virtual bool Checked() const;
};

// ****************************************************************************
// Class: ToggleLockTimeActionUI
//
// Purpose:
//   Handles the lock time action.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 18 17:12:50 PST 2005
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API ToggleLockTimeActionUI : public ViewerActionUIToggle
{
public:
    ToggleLockTimeActionUI(ViewerActionLogic *L);
    virtual ~ToggleLockTimeActionUI(){}

    virtual bool Checked() const;
};

// ****************************************************************************
// Class: ToggleLockToolActionUI
//
// Purpose:
//   Handles the lock tool action.
//
// Notes:      
//
// Programmer: Jeremy Meredith
// Creation:   February 15, 2008
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API ToggleLockToolActionUI : public ViewerActionUIToggle
{
public:
    ToggleLockToolActionUI(ViewerActionLogic *L);
    virtual ~ToggleLockToolActionUI(){}

    virtual bool Checked() const;
};

// ****************************************************************************
// Class: TurnOffAllLocksActionUI
//
// Purpose:
//   Handles the turn off all locks action.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 23 10:33:47 PST 2008
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API TurnOffAllLocksActionUI : public ViewerActionUISingle
{
public:
    TurnOffAllLocksActionUI(ViewerActionLogic *L);
    virtual ~TurnOffAllLocksActionUI(){}

    virtual bool AllowInToolbar() const { return false; }
};

#endif
