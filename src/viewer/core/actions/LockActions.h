// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEWER_LOCK_ACTIONS_H
#define VIEWER_LOCK_ACTIONS_H
#include <ViewerActionLogic.h>

// ****************************************************************************
// Class: ToggleLockViewAction
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

class VIEWERCORE_API ToggleLockViewAction : public ViewerActionLogic
{
public:
    ToggleLockViewAction(ViewerWindow *win);
    virtual ~ToggleLockViewAction(){}

    virtual void Execute();
};

// ****************************************************************************
// Class: ToggleLockTimeAction
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

class VIEWERCORE_API ToggleLockTimeAction : public ViewerActionLogic
{
public:
    ToggleLockTimeAction(ViewerWindow *win);
    virtual ~ToggleLockTimeAction(){}

    virtual void Execute();
};

// ****************************************************************************
// Class: ToggleLockToolAction
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

class VIEWERCORE_API ToggleLockToolAction : public ViewerActionLogic
{
public:
    ToggleLockToolAction(ViewerWindow *win);
    virtual ~ToggleLockToolAction(){}

    virtual void Execute();
};

// ****************************************************************************
// Class: TurnOffAllLocksAction
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

class VIEWERCORE_API TurnOffAllLocksAction : public ViewerActionLogic
{
public:
    TurnOffAllLocksAction(ViewerWindow *win);
    virtual ~TurnOffAllLocksAction(){}

    virtual void Execute();
};

#endif
