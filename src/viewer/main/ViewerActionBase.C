#include <ViewerActionBase.h>
#include <ViewerActionManager.h>
#include <ViewerSubject.h>
#include <ViewerWindow.h>
#include <ViewerWindowManager.h>

// An external pointer to the ViewerSubject.
extern ViewerSubject *viewerSubject;

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
//   win  : The viewer window that owns the action.
//   name : The name of the object instance.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:16:35 PST 2003
//
// Modifications:
//   Brad Whitlock, Fri Apr 15 09:06:46 PDT 2005
//   Added rpcType initialization.
//
// ****************************************************************************

ViewerActionBase::ViewerActionBase(ViewerWindow *win, const char *name) :
    QObject(0, name)
{
    window = win;
    windowId = window->GetWindowId();
    allowVisualRepresentation = true;
    rpcType = ViewerRPC::CloseRPC;
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
// Method: ViewerActionBase::GetArgs
//
// Purpose: 
//   Returns a reference to the args object.
//
// Returns:    A reference to the args object.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 15 10:46:22 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

const ViewerRPC &
ViewerActionBase::GetArgs()
{
    return args;
}

// ****************************************************************************
// Method: ViewerActionBase::SetRPCType
//
// Purpose: 
//   Lets the action know which RPC it is associated with.
//
// Arguments:
//   t : The RPC type that the action handles.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 15 09:09:21 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
ViewerActionBase::SetRPCType(ViewerRPC::ViewerRPCType t)
{
    rpcType = t;
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
//   Brad Whitlock, Thu Apr 14 16:16:11 PST 2005
//   I added code to postpone the action's viewer rpc.
//
// ****************************************************************************

void
ViewerActionBase::Activate(bool interactive)
{
    if(interactive)
    {
        // Allow the action to store values in the args object.      
        Setup();

        // Postpone the action until it is safe to execute it by scheduling it
        // with the ViewerSubject. By always scheduling interactive actions
        // in this way, we make it safe to handle them with other input that
        // came in from the client.
        args.SetRPCType(rpcType);
        viewerSubject->PostponeAction(this);
    }
    else
    {
        // Before handling the action, do this.
        PreExecute();

        // Handle the action
        Execute();

        // Tell the action manager to update all of the actions.
        window->GetActionManager()->Update();
    }

    // Hide the menu since we're done with the action.
    window->HideMenu();
}
