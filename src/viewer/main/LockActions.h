#ifndef VIEWER_LOCK_ACTIONS_H
#define VIEWER_LOCK_ACTIONS_H
#include <viewer_exports.h>
#include <ViewerToggleAction.h>

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

class VIEWER_API ToggleLockViewAction : public ViewerToggleAction
{
public:
    ToggleLockViewAction(ViewerWindow *win);
    virtual ~ToggleLockViewAction(){}

    virtual void Execute();
    virtual bool Enabled() const;
    virtual bool Toggled() const;
};

// ****************************************************************************
// Class: ToggleLockTimeAction
//
// Purpose:
//   Handles the lock view action.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 18 17:12:50 PST 2005
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API ToggleLockTimeAction : public ViewerToggleAction
{
public:
    ToggleLockTimeAction(ViewerWindow *win);
    virtual ~ToggleLockTimeAction(){}

    virtual void Execute();
    virtual bool Toggled() const;
};

#endif
