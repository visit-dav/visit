/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
//   Brad Whitlock, Mon Feb 12 17:21:16 PST 2007
//   ViewerBase initialization.
//
// ****************************************************************************

ViewerActionBase::ViewerActionBase(ViewerWindow *win, const char *name) :
    ViewerBase(0, name)
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
