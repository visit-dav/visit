/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

#include <LockActions.h>
#include <ViewerPlotList.h>
#include <ViewerWindow.h>
#include <ViewerWindowManager.h>

// Include icons
#include <timelockon.xpm>
#include <timelockoff.xpm>
#include <viewlockon.xpm>
#include <viewlockoff.xpm>
#include <toollockon.xpm>
#include <toollockoff.xpm>

// ****************************************************************************
// Method: ToggleLockViewAction::ToggleLockViewAction
//
// Purpose: 
//   Constructor for the ToggleLockViewAction class.
//
// Arguments:
//   win : The viewer window that owns this action.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 18 17:15:36 PST 2005
//
// Modifications:
//   
// ****************************************************************************

ToggleLockViewAction::ToggleLockViewAction(ViewerWindow *win) :
    ViewerToggleAction(win, "ToggleLockViewAction")
{
    SetAllText(tr("Lock view"));
    if (!win->GetNoWinMode())
        SetIcons(QPixmap(viewlockon_xpm), QPixmap(viewlockoff_xpm));
}

void
ToggleLockViewAction::Execute()
{
    windowMgr->ToggleLockViewMode(windowId);
}

bool
ToggleLockViewAction::Enabled() const
{
    // This action should only be enabled if the window to which the action belongs
    // has plots in it.
    return (window->GetPlotList()->GetNumPlots() > 0);
}

bool
ToggleLockViewAction::Toggled() const
{
    return window->GetViewIsLocked();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ToggleLockTimeAction::ToggleLockTimeAction
//
// Purpose: 
//   Constructor for the ToggleLockTimeAction class.
//
// Arguments:
//   win : The viewer window that owns this action.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 18 17:15:36 PST 2005
//
// Modifications:
//   
// ****************************************************************************

ToggleLockTimeAction::ToggleLockTimeAction(ViewerWindow *win) :
    ViewerToggleAction(win, "ToggleLockTimeAction")
{
    SetAllText(tr("Lock time"));
    if (!win->GetNoWinMode())
        SetIcons(QPixmap(timelockon_xpm), QPixmap(timelockoff_xpm));
}

void
ToggleLockTimeAction::Execute()
{
    windowMgr->ToggleLockTime(windowId);
}

bool
ToggleLockTimeAction::Toggled() const
{
    return window->GetTimeLock();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ToggleLockToolAction::ToggleLockToolAction
//
// Purpose: 
//   Constructor for the ToggleLockToolAction class.
//
// Arguments:
//   win : The viewer window that owns this action.
//
// Programmer: Jeremy Meredith
// Creation:   February 15, 2008
//
// Modifications:
//   
// ****************************************************************************

ToggleLockToolAction::ToggleLockToolAction(ViewerWindow *win) :
    ViewerToggleAction(win, "ToggleLockToolAction")
{
    SetAllText(tr("Lock tools"));
    if (!win->GetNoWinMode())
        SetIcons(QPixmap(toollockon_xpm), QPixmap(toollockoff_xpm));
}

void
ToggleLockToolAction::Execute()
{
    windowMgr->ToggleLockTools(windowId);
}

bool
ToggleLockToolAction::Toggled() const
{
    return window->GetToolLock();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: TurnOffAllLocksAction::TurnOffAllLocksAction
//
// Purpose: 
//   Constructor for the TurnOffAllLocksAction class.
//
// Arguments:
//   win : The viewer window that owns this action.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 23 10:35:14 PST 2008
//
// Modifications:
//   
// ****************************************************************************

TurnOffAllLocksAction::TurnOffAllLocksAction(ViewerWindow *win) :
    ViewerAction(win, "TurnOffAllLocksAction")
{
    SetAllText(tr("Unlock everything"));
}

// ****************************************************************************
// Method: TurnOffAllLocksAction::Execute
//
// Purpose: 
//   Turns off all of the locks for all windows.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 23 10:36:28 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
TurnOffAllLocksAction::Execute()
{
    windowMgr->TurnOffAllLocks();
}
