#include <ViewerActionBase.h>
#include <ViewerActionManager.h>
#include <ViewerWindow.h>
#include <ViewerWindowManager.h>

// Static members
ViewerRPC ViewerActionBase::args;
ViewerWindowManager *ViewerActionBase::windowMgr = 0;

// ****************************************************************************
// Method: ViewerActionBase::ViewerActionBase
//
// Purpose: 
//   Constructor
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:16:35 PST 2003
//
// Modifications:
//   
// ****************************************************************************

ViewerActionBase::ViewerActionBase(ViewerWindow *win, const char *name) :
    QObject(0, name)
{
    window = win;
    windowId = window->GetWindowId();
    allowVisualRepresentation = true;
    windowMgr = ViewerWindowManager::Instance();
}

// ****************************************************************************
// Method: ViewerActionBase::~ViewerActionBase
//
// Purpose: 
//   Destructor
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:16:35 PST 2003
//
// Modifications:
//   
// ****************************************************************************

ViewerActionBase::~ViewerActionBase()
{
    // nothing
}

// ****************************************************************************
// Method: ViewerActionBase::SetArgs
//
// Purpose: 
//   Sets the specified ViewerRPC values into the args object which is used
//   by actions if they require arguments.
//
// Arguments:
//   a : The arguments to use in the Execute method.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:16:35 PST 2003
//
// Modifications:
//   
// ****************************************************************************
     
void
ViewerActionBase::SetArgs(const ViewerRPC &a)
{
    args = a;
}

// ****************************************************************************
// Method: ViewerActionBase::Activate
//
// Purpose: 
//   Slot function that calls the other Activate method.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:16:35 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerActionBase::Activate()
{
    Activate(true);
}

// ****************************************************************************
// Method: ViewerActionBase::Activate
//
// Purpose: 
//   This method is called when all actions are activated.
//
// Arguments:
//   setup : Whether or not to execute the setup method.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:16:35 PST 2003
//
// Modifications:
//   Brad Whitlock, Thu Sep 11 08:53:10 PDT 2003
//   The window's HideMenu method does not seem to always be called so let's
//   hide it after executing each action if it needs to be hidden.
//
// ****************************************************************************

void
ViewerActionBase::Activate(bool setup)
{
    // Allow the action to store values in the args object.
    if(setup)
        Setup();

    // Before handling the action, do this.
    PreExecute();

    // Handle the action
    Execute();

    // Tell the action manager to update all of the actions.
    window->GetActionManager()->Update();

    // Hide the menu since we're done with the action.
    window->HideMenu();
}
