/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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

