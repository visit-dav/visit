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
#include <ViewerActionUI.h>
#include <ViewerActionLogic.h>
#include <ViewerActionManager.h>

#include <ViewerWindowUI.h>

// ****************************************************************************
// Method: ViewerActionUI::ViewerActionUI
//
// Purpose: 
//   Constructor
//
// Arguments:
//   L  : The action logic.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 17:16:35 PST 2003
//
// Modifications:
//
// ****************************************************************************

ViewerActionUI::ViewerActionUI(ViewerActionLogic *L) : ViewerBaseUI(0)
{
    logic = L;
    activeAction = 0;
}

// ****************************************************************************
// Method: ViewerActionUI::~ViewerActionUI
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

ViewerActionUI::~ViewerActionUI()
{
    // nothing
}

// ****************************************************************************
// Method: ViewerActionUI::Activate
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
ViewerActionUI::Activate()
{
    ActivateEx(true);
}

// ****************************************************************************
// Method: ViewerActionUI::Activate
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
//   Brad Whitlock, Fri Jan 9 15:07:35 PST 2009
//   Added exception handling to prevent exceptions from being propagated into
//   the Qt event loop.
//
// ****************************************************************************

void
ViewerActionUI::ActivateEx(bool interactive)
{
    TRY
    {
        if(interactive)
        {
            // Postpone the action until safe to execute.
            PostponeExecute();
        }
        else
        {
// DO WE STILL NEED THIS WITH THE UI SEPARATION?
            // Before handling the action, do this.
            PreExecute();

            // Handle the action
            GetLogic()->Execute();

            // Tell the action manager to update all of the actions.
            GetLogic()->GetWindow()->GetActionManager()->Update();
        }

        // Hide the menu since we're done with the action.
        ViewerWindowUI *win = ViewerWindowUI::SafeDownCast(GetLogic()->GetWindow());
        if(win != NULL)
            win->HideMenu();
    }
    CATCHALL
    {
        ; // nothing
    }
    ENDTRY
}

void
ViewerActionUI::PostponeExecute()
{
    // Postpone the action until safe to execute.
    GetLogic()->PostponeExecute(activeAction, true);
}

void 
ViewerActionUI::PreExecute()
{
}

void
ViewerActionUI::SetAllText(const QString &text)
{
    SetText(text);
    SetMenuText(text);
    SetToolTip(text);
}

void
ViewerActionUI::SetToggleAction(bool val)
{
}

ViewerActionLogic *
ViewerActionUI::GetLogic() const
{
    return logic;
}

void
ViewerActionUI::SetActiveAction(int idx)
{
    activeAction = idx;
}

int
ViewerActionUI::GetActiveAction() const
{
    return activeAction;
}
