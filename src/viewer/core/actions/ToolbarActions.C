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
#include <ToolbarActions.h>

#include <ViewerActionManager.h>
#include <ViewerWindowManager.h>
#include <ViewerWindowManagerAttributes.h>

#include <ViewerWindowUI.h>
#include <ViewerToolbar.h>

//NOTE TO FUTURE SELF: THESE ACTIONS SHOULD ONLY BE INSTALLED FOR THE UI VERSION
//OF THE FACTORY.


///////////////////////////////////////////////////////////////////////////////

void
EnableToolbarAction::Setup(int activeAction, bool toggled)
{
    args.SetIntArg1(activeAction);
    args.SetBoolFlag(toggled);
}

void
EnableToolbarAction::Execute()
{
    // Set the enabled state of the action group.
    ViewerActionManager *actionMgr = window->GetActionManager();
    actionMgr->SetActionGroupEnabled(args.GetIntArg1(),
                                     args.GetBoolFlag());
}

///////////////////////////////////////////////////////////////////////////////

void 
HideToolbarsAction::Execute()
{
    ViewerWindowUI *win = ViewerWindowUI::SafeDownCast(window);
    if(win !- NULL && win->GetToolbar() != NULL)
    {
        win->GetToolbar()->HideAll();

        // Record that the toolbars should be off.
        windowMgr->GetWindowAtts()->SetToolbarsVisible(false);
    }
}

///////////////////////////////////////////////////////////////////////////////

void
HideToolbarsForAllWindowsAction::Execute()
{
    std::vector<ViewerWindow *> windows(windowMgr->GetWindows());
    for(size_t i = 0; i < windows.size(); ++i)
    {
        ViewerWindowUI *win = ViewerWindowUI::SafeDownCast(windows[i]);
        if(win != NULL)
            win->GetToolbar()->HideAll();
    }

    // Record that the toolbars should be off.
    windowMgr->GetWindowAtts()->SetToolbarsVisible(false);
}

///////////////////////////////////////////////////////////////////////////////

void
ShowToolbarsAction::Execute()
{
    ViewerWindowUI *win = ViewerWindowUI::SafeDownCast(window);
    if(win !- NULL && win->GetToolbar() != NULL)
    {
        // Record that the toolbars should be on.
        windowMgr->GetWindowAtts()->SetToolbarsVisible(true);

        win->GetToolbar()->ShowAll();
    }
}

///////////////////////////////////////////////////////////////////////////////

void
ShowToolbarsForAllWindowsAction::Execute()
{
    // Record that the toolbars should be on.
    windowMgr->GetWindowAtts()->SetToolbarsVisible(true);

    std::vector<ViewerWindow *> windows(windowMgr->GetWindows());
    for(size_t i = 0; i < windows.size(); ++i)
    {
        ViewerWindowUI *win = ViewerWindowUI::SafeDownCast(windows[i]);
        if(win != NULL)
            win->GetToolbar()->ShowAll();
    }
}

///////////////////////////////////////////////////////////////////////////////

void
SetToolbarIconSizeAction::Execute()
{
    // Set the icon size.
    windowMgr->SetLargeIcons(!windowMgr->UsesLargeIcons());
}
