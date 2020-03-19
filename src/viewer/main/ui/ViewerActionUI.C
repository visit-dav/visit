// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
