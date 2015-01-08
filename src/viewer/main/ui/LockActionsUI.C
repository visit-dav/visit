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

#include <LockActionsUI.h>
#include <ViewerActionLogic.h>
#include <ViewerPlotList.h>
#include <ViewerPopupMenu.h>
#include <ViewerProperties.h>
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
// Method: ToggleLockViewActionUI::ToggleAllowPopupActionUI
//
// Purpose:
//   Constructor for the ToggleAllowPopupActionUI class.
//
// Arguments:
//   win : The viewer window that owns this ActionUI.
//
// Programmer: Marc Durant
// Creation:   Tue Jan 10 09:18:00 MST 2012
//
// ****************************************************************************
ToggleAllowPopupActionUI::ToggleAllowPopupActionUI(ViewerActionLogic *L) :
  ViewerActionUIToggle(L)
{
    SetAllText(tr("Allow Popup"));
    if (!GetViewerProperties()->GetNowin())
        SetIcons(QPixmap(toollockon_xpm), QPixmap(toollockoff_xpm));
}

bool
ToggleAllowPopupActionUI::Checked() const
{
#if 1
    cerr << "NEED TO IMPLEMENT: ToggleAllowPopupActionUI::Checked()" << endl;
    return true;
#else
    return GetLogic()->GetWindow()->GetPopupMenu()->IsEnabled();
#endif
}

bool
ToggleAllowPopupActionUI::Enabled() const
{
    return false;
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ToggleLockViewActionUI::ToggleLockViewActionUI
//
// Purpose: 
//   Constructor for the ToggleLockViewActionUI class.
//
// Arguments:
//   win : The viewer window that owns this ActionUI.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 18 17:15:36 PST 2005
//
// Modifications:
//   
// ****************************************************************************

ToggleLockViewActionUI::ToggleLockViewActionUI(ViewerActionLogic *L) :
    ViewerActionUIToggle(L)
{
    SetAllText(tr("Lock view"));
    if (!GetViewerProperties()->GetNowin())
        SetIcons(QPixmap(viewlockon_xpm), QPixmap(viewlockoff_xpm));
}

bool
ToggleLockViewActionUI::Enabled() const
{
    // This ActionUI should only be enabled if the window to which the ActionUI belongs
    // has plots in it.
    return (GetLogic()->GetWindow()->GetPlotList()->GetNumPlots() > 0);
}

bool
ToggleLockViewActionUI::Checked() const
{
    return GetLogic()->GetWindow()->GetViewIsLocked();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ToggleLockTimeActionUI::ToggleLockTimeActionUI
//
// Purpose: 
//   Constructor for the ToggleLockTimeActionUI class.
//
// Arguments:
//   win : The viewer window that owns this ActionUI.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 18 17:15:36 PST 2005
//
// Modifications:
//   
// ****************************************************************************

ToggleLockTimeActionUI::ToggleLockTimeActionUI(ViewerActionLogic *L) :
    ViewerActionUIToggle(L)
{
    SetAllText(tr("Lock time"));
    if (!GetViewerProperties()->GetNowin())
        SetIcons(QPixmap(timelockon_xpm), QPixmap(timelockoff_xpm));
}

bool
ToggleLockTimeActionUI::Checked() const
{
    return GetLogic()->GetWindow()->GetTimeLock();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ToggleLockToolActionUI::ToggleLockToolActionUI
//
// Purpose: 
//   Constructor for the ToggleLockToolActionUI class.
//
// Arguments:
//   win : The viewer window that owns this ActionUI.
//
// Programmer: Jeremy Meredith
// Creation:   February 15, 2008
//
// Modifications:
//   
// ****************************************************************************

ToggleLockToolActionUI::ToggleLockToolActionUI(ViewerActionLogic *L) :
    ViewerActionUIToggle(L)
{
    SetAllText(tr("Lock tools"));
    if (!GetViewerProperties()->GetNowin())
        SetIcons(QPixmap(toollockon_xpm), QPixmap(toollockoff_xpm));
}

bool
ToggleLockToolActionUI::Checked() const
{
    return GetLogic()->GetWindow()->GetToolLock();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: TurnOffAllLocksActionUI::TurnOffAllLocksActionUI
//
// Purpose: 
//   Constructor for the TurnOffAllLocksActionUI class.
//
// Arguments:
//   win : The viewer window that owns this ActionUI.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 23 10:35:14 PST 2008
//
// Modifications:
//   
// ****************************************************************************

TurnOffAllLocksActionUI::TurnOffAllLocksActionUI(ViewerActionLogic *L) :
    ViewerActionUISingle(L)
{
    SetAllText(tr("Unlock everything"));
}
