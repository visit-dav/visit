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

#include <WindowActions.h>

#include <ViewerPlotList.h>
#include <ViewerProperties.h>
#include <ViewerQueryManager.h>
#include <ViewerWindow.h>
#include <ViewerWindowManager.h>


///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: AddWindowAction::Execute
//
// Purpose: 
//   Executes the work for AddWindowAction.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:31:37 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
AddWindowAction::Execute()
{
    windowMgr->AddWindow();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: CloneWindowAction::AddWindowAction
//
// Purpose: 
//   Does the work for CloneWindowAction.
//
// Arguments:
//   win : The window to which the action belongs.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:31:37 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
CloneWindowAction::Execute()
{
    windowMgr->SetActiveWindow(window->GetWindowId() + 1);
    windowMgr->CloneWindow();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: DeleteWindowAction::Execute
//
// Purpose: 
//   Does the work for DeleteWindowAction.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:33:41 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
DeleteWindowAction::Execute()
{
    window->SendDeleteMessage();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ClearWindowAction::Execute
//
// Purpose: 
//   Does the work for ClearWindowAction.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:34:54 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ClearWindowAction::Execute()
{
    window->ClearWindow(args.GetBoolFlag());
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ClearAllWindowsAction::Execute
//
// Purpose: 
//   Does the work for ClearAllWindowsAction.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:36:07 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ClearAllWindowsAction::Execute()
{
    windowMgr->ClearAllWindows();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ClearPickPointsAction::Execute
//
// Purpose: 
//   Does the work for ClearPickPointsAction.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:34:54 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ClearPickPointsAction::Execute()
{
    window->ClearPickPoints();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ClearReferenceLinesAction::Execute
//
// Purpose: 
//   Does the work for ClearReferenceLinesAction.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:34:54 PST 2003
//
// Modifications:
//   Kathleen Bonnell, Wed Mar 23 15:09:16 PST 2005
//   Tell ViewerQueryManager that reflines have been cleared. 
//   
// ****************************************************************************

void
ClearReferenceLinesAction::Execute()
{
    window->ClearRefLines();
    ViewerQueryManager::Instance()->ClearRefLines(window);
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetActiveWindowAction::Setup
//
// Purpose: 
//   This method is called when the toolbar or popup menu is selected. It
//   stores relevant information into the args object so it is ready to go
//   when the Execute method needs it.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:36:42 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
SetActiveWindowAction::Setup(int activeAction, bool toggled)
{
    args.SetWindowId(window->GetWindowId() + 1);
    args.SetBoolFlag(true);
}

// ****************************************************************************
// Method: SetActiveWindowAction::Execute
//
// Purpose: 
//   Does the work for SetActiveWindowAction.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:37:40 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void 
SetActiveWindowAction::Execute()
{
    int id = args.GetWindowId();
    bool raiseWindow = args.GetBoolFlag();
    windowMgr->SetActiveWindow(id, raiseWindow);
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ToggleSpinModeAction::Execute
//
// Purpose: 
//   Does the work for ToggleSpinModeAction.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:39:13 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ToggleSpinModeAction::Execute()
{
    windowMgr->ToggleSpinMode(window->GetWindowId());
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetWindowLayoutAction::Setup
//
// Purpose: 
//   This method is called when the toolbar or popup menu are selected. It stores
//   values into the args object so that it is ready for the Execute method.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:43:15 PST 2003
//
// Modifications:
//   Eric Brugger, Thu Jun 30 09:45:55 PDT 2005
//   Added a 2x3 window layout and removed the 4x4 window layout.
//   
// ****************************************************************************

void
SetWindowLayoutAction::Setup(int activeAction, bool toggled)
{
    if(activeAction == 1)
        args.SetWindowLayout(2);
    else if(activeAction == 2)
        args.SetWindowLayout(4);
    else if(activeAction == 3)
        args.SetWindowLayout(6);
    else if(activeAction == 4)
        args.SetWindowLayout(8);
    else if(activeAction == 5)
        args.SetWindowLayout(9);
    else
        args.SetWindowLayout(1);
}

// ****************************************************************************
// Method: SetWindowLayoutAction::Execute
//
// Purpose: 
//   Does the work for SetWindowLayoutAction.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:43:56 PST 2003
//
// Modifications:
//   Brad Whitlock, Wed Apr 27 15:56:56 PST 2005
//   Added code to set the active window before changing layouts.
//
// ****************************************************************************

void
SetWindowLayoutAction::Execute()
{
    // Set the active window, if necessary, before changing the layout.
    if(window != windowMgr->GetActiveWindow())
        windowMgr->SetActiveWindow(window->GetWindowId() + 1);

    windowMgr->SetWindowLayout(args.GetWindowLayout());
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: InvertBackgroundAction::Execute
//
// Purpose: 
//   Does the work for InvertBackgroundAction.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:45:58 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
InvertBackgroundAction::Execute()
{
    windowMgr->InvertBackgroundColor(window->GetWindowId());
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetWindowModeAction::Setup
//
// Purpose: 
//   This method is called when the toolbar or popup menu are used. It stores
//   values into the args object so it is ready for the Execute method.
//
// Programmer: Kathleen Bonnell
// Creation:   July 22, 2003
//
// Modifications:
//   
// ****************************************************************************

void
SetWindowModeAction::Setup(int activeAction, bool toggled)
{
    args.SetWindowMode(activeAction);
}

// ****************************************************************************
// Method: SetWindowModeAction::Execute
//
// Purpose: 
//   Does the work for SetWindowModeAction.
//
// Arguments:
//   val : The choice that the user made.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:48:08 PST 2003
//
// Modifications:
//   Kathleen Bonnell, Tue Jul 22 16:24:52 PDT 2003
//   Made it use args.GetWindowMode so that can be set from CLI.
//   
// ****************************************************************************

void
SetWindowModeAction::Execute()
{
    INTERACTION_MODE mode = (INTERACTION_MODE)args.GetWindowMode();
    windowMgr->SetInteractionMode(mode, window->GetWindowId());
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: EnableToolAction::Setup
//
// Purpose: 
//   This method is called when the toolbar or popup menu are used. It stores
//   values into the args object so it is ready for the Execute method.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:52:32 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
EnableToolAction::Setup(int activeAction, bool toggled)
{
    args.SetToolId(activeAction);
    args.SetBoolFlag(toggled);
}

// ****************************************************************************
// Method: EnableToolAction::Execute
//
// Purpose: 
//   Does the work for EnableToolAction.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:53:18 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
EnableToolAction::Execute()
{
    windowMgr->SetToolEnabled(args.GetToolId(), args.GetBoolFlag(),
                              window->GetWindowId());
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetToolUpdateModeAction::Setup
//
// Purpose: 
//   This method is called when the toolbar or popup menu are used. It stores
//   values into the args object so it is ready for the Execute method.
//
// Programmer: Jeremy Meredith
// Creation:   February  2, 2010
//
// Modifications:
//   
// ****************************************************************************

void
SetToolUpdateModeAction::Setup(int activeAction, bool toggled)
{
    args.SetToolUpdateMode(activeAction);
}

// ****************************************************************************
// Method: SetToolUpdateModeAction::Execute
//
// Purpose: 
//   Does the work for SetToolUpdateModeAction.
//
// Arguments:
//   val : The choice that the user made.
//
// Programmer: Jeremy Meredith
// Creation:   February  2, 2010
//
// Modifications:
//   
// ****************************************************************************

void
SetToolUpdateModeAction::Execute()
{
    TOOLUPDATE_MODE mode = (TOOLUPDATE_MODE)args.GetToolUpdateMode();
    windowMgr->SetToolUpdateMode(mode, window->GetWindowId());
}

// ****************************************************************************
// Method: DeIconifyAllWindowsAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::DeIconifyAllWindowsRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
DeIconifyAllWindowsAction::Execute()
{
    windowMgr->DeIconifyAllWindows();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: DisableRedrawAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::DisableRedrawRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
DisableRedrawAction::Execute()
{
    windowMgr->DisableRedraw();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: HideAllWindowsAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::HideAllWindowsRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
HideAllWindowsAction::Execute()
{
    windowMgr->HideAllWindows();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: IconifyAllWindowsAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::IconifyAllWindowsRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
IconifyAllWindowsAction::Execute()
{
    windowMgr->IconifyAllWindows();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: MoveAndResizeWindowAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::MoveAndResizeWindowRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
MoveAndResizeWindowAction::Execute()
{
    windowMgr->MoveAndResizeWindow(
        args.GetWindowId()-1,
        args.GetIntArg1(),
        args.GetIntArg2(),
        args.GetIntArg3(),
        args.GetWindowLayout());
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: MoveWindowAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::MoveWindowRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
MoveWindowAction::Execute()
{
    windowMgr->MoveWindow(args.GetWindowId()-1, args.GetIntArg1(), args.GetIntArg2());
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: RedrawAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::RedrawRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
RedrawAction::Execute()
{
    windowMgr->RedrawWindow();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ResizeWindowAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::ResizeWindowRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
ResizeWindowAction::Execute()
{
    windowMgr->ResizeWindow(args.GetWindowId()-1, args.GetIntArg1(), args.GetIntArg2());
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetWindowAreaAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SetWindowAreaRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
SetWindowAreaAction::Execute()
{
    const char *area = args.GetWindowArea().c_str();
    
    //
    // Recalculate the layouts and reposition the windows.
    //
    if(GetViewerProperties()->GetWindowSmall())
    {
        int x, y, w, h;
        if (sscanf(area, "%dx%d+%d+%d", &w, &h, &x, &y) == 4)
        {
            char tmp[30];
            w /= 2;
            h /= 2;
            SNPRINTF(tmp, 30, "%dx%d+%d+%d", w, h, x, y);
            windowMgr->SetGeometry(tmp);
        }
    }
    else
        windowMgr->SetGeometry(area);

    windowMgr->SetWindowLayout(windowMgr->GetWindowLayout());
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ShowAllWindowsAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::ShowAllWindowsRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
ShowAllWindowsAction::Execute()
{
    windowMgr->ShowAllWindows();
}
