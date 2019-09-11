// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <ViewerActionLogic.h>
#include <ViewerActionManager.h>
#include <ViewerWindow.h>
#include <ViewerWindowManager.h>

// Static members
ViewerRPC ViewerActionLogic::args;
ViewerWindowManager *ViewerActionLogic::windowMgr = 0;
void (*ViewerActionLogic::PostponeActionCB)(int, const ViewerRPC &, void *) = NULL;
void *ViewerActionLogic::PostponeActionCBData = NULL;

// ****************************************************************************
// Method: ViewerActionLogic::ViewerActionLogic
//
// Purpose: 
//   Constructor
//
// Arguments:
//   win  : The viewer window that owns the action.
//   n    : The name of the object instance.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:16:35 PST 2003
//
// Modifications:
//
// ****************************************************************************

ViewerActionLogic::ViewerActionLogic(ViewerWindow *win) : ViewerBase()
{
    window = win;
    rpcType = ViewerRPC::CloseRPC;
    windowMgr = ViewerWindowManager::Instance();
}

// ****************************************************************************
// Method: ViewerActionLogic::~ViewerActionLogic
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

ViewerActionLogic::~ViewerActionLogic()
{
    // nothing
}

// ****************************************************************************
// Method: ViewerActionLogic::GetWindow
//
// Purpose:
//   Get the window for the logic.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 18 15:20:58 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerWindow *
ViewerActionLogic::GetWindow() const
{
    return window;
}

// ****************************************************************************
// Method: ViewerActionLogic::SetArgs
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
ViewerActionLogic::SetArgs(const ViewerRPC &a)
{
    args = a;
}

// ****************************************************************************
// Method: ViewerActionLogic::GetArgs
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
ViewerActionLogic::GetArgs()
{
    return args;
}

// ****************************************************************************
// Method: ViewerActionLogic::SetRPCType
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
ViewerActionLogic::SetRPCType(ViewerRPC::ViewerRPCType t)
{
    rpcType = t;
}

ViewerRPC::ViewerRPCType
ViewerActionLogic::GetRPCType() const
{
    return rpcType;
}

// ****************************************************************************
// Method: ViewerActionLogic::GetName
//
// Purpose: 
//   Returns the action name based on its RPC type.
//
// Returns:    The action name.
//
// Programmer: Brad Whitlock
// Creation:   Wed May 28 10:17:23 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

std::string
ViewerActionLogic::GetName() const
{
    return ViewerRPC::ViewerRPCType_ToString(rpcType);
}

// ****************************************************************************
// Method: ViewerActionLogic::Setup
//
// Purpose:
//   Setup method for action logic.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 18 15:09:50 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void 
ViewerActionLogic::Setup(int activeAction, bool toggled)
{
}

// ****************************************************************************
// Method: ViewerActionLogic::PostponeExecute
//
// Purpose:
//   Postpones the action so it gets executed later.
//
// Arguments:
//   activeAction : An index that can be useful for multi-actions.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 18 16:17:15 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
ViewerActionLogic::PostponeExecute(int activeAction, bool toggled)
{
    Setup(activeAction, toggled);

    // Postpone the action until it is safe to execute. By always scheduling 
    // interactive actions in this way, we make it safe to handle them with other
    // input that came in from the client.
    args.SetRPCType(rpcType);

    if(PostponeActionCB != NULL)
        (*PostponeActionCB)(window->GetWindowId(), args, PostponeActionCBData);
    else
    {
         // There is no callback to postpone execution, so execute now.
         Execute();
    }
}

// ****************************************************************************
// Method: ViewerActionLogic::SetPostponeActionCallback
//
// Purpose:
//   Set the callback function that will be used to handle postponed actions.
//
// Arguments:
//   cb : The callback function.
//   cbdata : The callback data.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  3 14:24:08 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
ViewerActionLogic::SetPostponeActionCallback(
    void (*cb)(int, const ViewerRPC &, void *), void *cbdata)
{
    PostponeActionCB = cb;
    PostponeActionCBData = cbdata;
}

bool
ViewerActionLogic::CopyFrom(const ViewerActionLogic *)
{
    return false;
}

bool
ViewerActionLogic::CreateNode(DataNode *)
{
    return false;
}

void
ViewerActionLogic::SetFromNode(DataNode *,const std::string &)
{
}

