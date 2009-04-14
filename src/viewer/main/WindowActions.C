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

#include <WindowActions.h>
#include <ViewerPlotList.h>
#include <ViewerProperties.h>
#include <ViewerQueryManager.h>
#include <ViewerWindow.h>
#include <ViewerWindowManager.h>
#include <PlotPluginManager.h>
#include <OperatorPluginManager.h>

#include <QIcon>
#include <QPixmap>

// Include the icons
#include <newwindow.xpm>
#include <clearwindow.xpm>
#include <copymenu.xpm>
#include <deletewindow.xpm>
#include <checkwindow.xpm>
#include <blankwindow.xpm>
#include <spinon.xpm>
#include <spinoff.xpm>
#include <navigatebboxon.xpm>
#include <navigatebboxoff.xpm>
#include <layout1x1.xpm>
#include <layout1x2.xpm>
#include <layout2x2.xpm>
#include <layout2x3.xpm>
#include <layout2x4.xpm>
#include <layout3x3.xpm>
#include <navigatemode.xpm>
#include <nodepickmode.xpm>
#include <zonepickmode.xpm>
#include <spreadsheetpickmode.xpm>
#include <zoommode.xpm>
#include <lineoutmode.xpm>
#include <VisWindowTypes.h>
#include <boxtool.xpm>
#include <linetool.xpm>
#include <planetool.xpm>
#include <pointtool.xpm>
#include <spheretool.xpm>
#include <axisrestrictiontool.xpm>
#include <invertbackground.xpm>

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: AddWindowAction::AddWindowAction
//
// Purpose: 
//   Constructor for the AddWindowAction class.
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

AddWindowAction::AddWindowAction(ViewerWindow *win) :
    ViewerAction(win)
{
    SetAllText(tr("Add new window"));
    SetMenuText(tr("Add"));
    if (!GetViewerProperties()->GetNowin())
        SetIcon(QIcon(QPixmap(newwindow_xpm)));
}

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
// Method: CloneWindowAction::CloneWindowAction
//
// Purpose: 
//   Constructor for the CloneWindowAction class.
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

CloneWindowAction::CloneWindowAction(ViewerWindow *win) :
    ViewerAction(win)
{
    SetAllText(tr("Clone window"));
    SetMenuText(tr("Clone"));
    if (!GetViewerProperties()->GetNowin())
        SetIcon(QIcon(QPixmap(copymenu_xpm)));
}

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
// Method: DeleteWindowAction::DeleteWindowAction
//
// Purpose: 
//   Constructor for the DeleteWindowAction class.
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

DeleteWindowAction::DeleteWindowAction(ViewerWindow *win) :
    ViewerAction(win)
{
    SetAllText(tr("Delete window"));
    SetMenuText(tr("Delete"));
    if (!GetViewerProperties()->GetNowin())
        SetIcon(QIcon(QPixmap(deletewindow_xpm)));
}

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

// ****************************************************************************
// Method: DeleteWindowAction::Enabled
//
// Purpose: 
//   Returns when the action is enabled.
//
// Returns:    True if the action is enabled; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:34:02 PST 2003
//
// Modifications:
//   
// ****************************************************************************

bool
DeleteWindowAction::Enabled() const
{
    return (windowMgr->GetNumWindows() > 1);
}

void
DeleteWindowAction::Update()
{
    // Override the Update method because we get memory errors because the
    // window is only partially deleted at this point.
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ClearWindowAction::ClearWindowAction
//
// Purpose: 
//   Constructor for the ClearWindowAction class.
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

ClearWindowAction::ClearWindowAction(ViewerWindow *win) :
    ViewerAction(win)
{
    SetAllText(tr("Clear plots"));
    if (!GetViewerProperties()->GetNowin())
        SetIcon(QIcon(QPixmap(clearwindow_xpm)));
}

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

// ****************************************************************************
// Method: ClearWindowAction::Enabled
//
// Purpose: 
//   Returns when the action is enabled.
//
// Returns:    True if the action is enabled; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:35:11 PST 2003
//
// Modifications:
//   
// ****************************************************************************

bool
ClearWindowAction::Enabled() const
{
    // This action should only be enabled if the window to which the action belongs
    // has plots in it.
    return (window->GetPlotList()->GetNumPlots() > 0);
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ClearAllWindowsAction::ClearAllWindowsAction
//
// Purpose: 
//   Constructor for the ClearAllWindowsAction class.
//
// Arguments:
//   win : The window to which the action belongs.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:35:39 PST 2003
//
// Modifications:
//   
// ****************************************************************************

ClearAllWindowsAction::ClearAllWindowsAction(ViewerWindow *win) :
    ViewerAction(win)
{
    SetAllText(tr("Clear all windows"));
}

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
// Method: ClearPickPointsAction::ClearPickPointsAction
//
// Purpose: 
//   Constructor for the ClearPickPointsAction class.
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

ClearPickPointsAction::ClearPickPointsAction(ViewerWindow *win) :
    ViewerAction(win)
{
    SetAllText(tr("Clear pick points"));
}

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

// ****************************************************************************
// Method: ClearPickPointsAction::Enabled
//
// Purpose: 
//   Returns when the action is enabled.
//
// Returns:    True if the action is enabled; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:35:11 PST 2003
//
// Modifications:
//   
// ****************************************************************************

bool
ClearPickPointsAction::Enabled() const
{
    // This action should only be enabled if the window to which the action belongs
    // has plots in it.
    return (window->GetPlotList()->GetNumPlots() > 0);
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ClearReferenceLinesAction::ClearReferenceLinesAction
//
// Purpose: 
//   Constructor for the ClearReferenceLinesAction class.
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

ClearReferenceLinesAction::ClearReferenceLinesAction(ViewerWindow *win) :
    ViewerAction(win)
{
    SetAllText(tr("Clear reference lines"));
}

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

// ****************************************************************************
// Method: ClearReferenceLinesAction::Enabled
//
// Purpose: 
//   Returns when the action is enabled.
//
// Returns:    True if the action is enabled; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:35:11 PST 2003
//
// Modifications:
//   
// ****************************************************************************

bool
ClearReferenceLinesAction::Enabled() const
{
    // This action should only be enabled if the window to which the action belongs
    // has plots in it.
    return (window->GetPlotList()->GetNumPlots() > 0);
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetActiveWindowAction::SetActiveWindowAction
//
// Purpose: 
//   Constructor for the SetActiveWindowAction class.
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

SetActiveWindowAction::SetActiveWindowAction(ViewerWindow *win) :
    ViewerToggleAction(win)
{
    SetAllText(tr("Make active"));
    SetToolTip(tr("Make window active"));
    if (!GetViewerProperties()->GetNowin())
        SetIcons(QPixmap(checkwindow_xpm), QPixmap(blankwindow_xpm));
}

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
SetActiveWindowAction::Setup()
{
    args.SetWindowId(window->GetWindowId() + 1);
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
    windowMgr->SetActiveWindow(id);
}

// ****************************************************************************
// Method: SetActiveWindowAction::Checked
//
// Purpose: 
//   Returns whether or not the action is toggled.
//
// Returns:    True if the action's window is the active window; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:38:02 PST 2003
//
// Modifications:
//   
// ****************************************************************************

bool
SetActiveWindowAction::Checked() const
{
    // This action should be toggled if it belongs to the active window.
    return (window == windowMgr->GetActiveWindow()); 
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ToggleSpinModeAction::ToggleSpinModeAction
//
// Purpose: 
//   Constructor for the ToggleSpinModeAction class.
//
// Arguments:
//   win : The window to which the action belongs.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:38:46 PST 2003
//
// Modifications:
//   
// ****************************************************************************

ToggleSpinModeAction::ToggleSpinModeAction(ViewerWindow *win) :
    ViewerToggleAction(win)
{
    SetAllText(tr("Spin"));
    SetToolTip(tr("Toggle spin mode"));
    if (!GetViewerProperties()->GetNowin())
        SetIcons(QPixmap(spinon_xpm), QPixmap(spinoff_xpm));
}

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

// ****************************************************************************
// Method: ToggleSpinModeAction::Enabled
//
// Purpose: 
//   Returns when the action is enabled.
//
// Returns:    True if the action is enabled; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:39:36 PST 2003
//
// Modifications:
//   
// ****************************************************************************

bool 
ToggleSpinModeAction::Enabled() const
{
    // This action should only be enabled if the window to which the action belongs
    // has plots in it.
    return (window->GetPlotList()->GetNumPlots() > 0);
}

// ****************************************************************************
// Method: ToggleSpinModeAction::Checked
//
// Purpose: 
//   Returns if the window is in spin mode and the action should be toggled.
//
// Returns:    Whether the window is in spin mode and the action should be toggled.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:40:03 PST 2003
//
// Modifications:
//   
// ****************************************************************************

bool
ToggleSpinModeAction::Checked() const
{
    return window->GetSpinMode();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ToggleBoundingBoxModeAction::ToggleBoundingBoxModeAction
//
// Purpose: 
//   Constructor for the ToggleBoundingBoxModeAction class.
//
// Arguments:
//   win : The window to which the action belongs.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:40:56 PST 2003
//
// Modifications:
//   
// ****************************************************************************

ToggleBoundingBoxModeAction::ToggleBoundingBoxModeAction(ViewerWindow *win) :
    ViewerToggleAction(win)
{
    SetAllText(tr("Navigate bbox"));
    SetToolTip(tr("Toggle bounding box navigation"));
    if (!GetViewerProperties()->GetNowin())
        SetIcons(QPixmap(navigatebboxon_xpm), QPixmap(navigatebboxoff_xpm));
}

// ****************************************************************************
// Method: ToggleBoundingBoxModeAction::Execute
//
// Purpose: 
//   Does the work for ToggleBoundingBoxModeAction.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:41:30 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ToggleBoundingBoxModeAction::Execute()
{
    windowMgr->ToggleBoundingBoxMode(window->GetWindowId());
}

// ****************************************************************************
// Method: ToggleBoundingBoxModeAction::Enabled
//
// Purpose: 
//   Returns when the action is enabled.
//
// Returns:    True if the action is enabled; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:41:45 PST 2003
//
// Modifications:
//   
// ****************************************************************************

bool
ToggleBoundingBoxModeAction::Enabled() const
{
    // This action should only be enabled if the window to which the action belongs
    // has plots in it.
    return (window->GetPlotList()->GetNumPlots() > 0);
}

// ****************************************************************************
// Method: ToggleBoundingBoxModeAction::Checked
//
// Purpose: 
//   Returns whether the action should be toggled.
//
// Returns:    Whether the action should be toggled.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:42:12 PST 2003
//
// Modifications:
//   
// ****************************************************************************

bool
ToggleBoundingBoxModeAction::Checked() const
{
    return window->GetBoundingBoxMode();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetWindowLayoutAction::SetWindowLayoutAction
//
// Purpose: 
//   Constructor for the SetWindowLayoutAction class.
//
// Arguments:
//   win : The window to which the action belongs.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:42:42 PST 2003
//
// Modifications:
//   Eric Brugger, Thu Jun 30 09:45:55 PDT 2005
//   Added a 2x3 window layout and removed the 4x4 window layout.
//   
// ****************************************************************************

SetWindowLayoutAction::SetWindowLayoutAction(ViewerWindow *win) :
    ViewerMultipleAction(win)
{
    SetAllText(tr("Layout"));
    SetToolTip(tr("Set window layout"));
    if (!GetViewerProperties()->GetNowin())
        SetIcon(QIcon(QPixmap(layout2x2_xpm)));
    SetExclusive(true);

    if (!GetViewerProperties()->GetNowin())
    {
        AddChoice(tr("1x1"), tr("1 x 1 window layout"), QPixmap(layout1x1_xpm), QPixmap(layout1x1_xpm));
        AddChoice(tr("1x2"), tr("1 x 2 window layout"), QPixmap(layout1x2_xpm), QPixmap(layout1x2_xpm));
        AddChoice(tr("2x2"), tr("2 x 2 window layout"), QPixmap(layout2x2_xpm), QPixmap(layout2x2_xpm));
        AddChoice(tr("2x3"), tr("2 x 3 window layout"), QPixmap(layout2x3_xpm), QPixmap(layout2x3_xpm));
        AddChoice(tr("2x4"), tr("2 x 4 window layout"), QPixmap(layout2x4_xpm), QPixmap(layout2x4_xpm));
        AddChoice(tr("3x3"), tr("3 x 3 window layout"), QPixmap(layout3x3_xpm), QPixmap(layout3x3_xpm));
    }
    else
    {
        AddChoice(tr("1x1"));
        AddChoice(tr("1x2"));
        AddChoice(tr("2x2"));
        AddChoice(tr("2x3"));
        AddChoice(tr("2x4"));
        AddChoice(tr("3x3"));
    }
}

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
SetWindowLayoutAction::Setup()
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
SetWindowLayoutAction::Execute(int)
{
    // Set the active window, if necessary, before changing the layout.
    if(window != windowMgr->GetActiveWindow())
        windowMgr->SetActiveWindow(window->GetWindowId() + 1);

    windowMgr->SetWindowLayout(args.GetWindowLayout());
}

// ****************************************************************************
// Method: SetWindowLayoutAction::ChoiceChecked
//
// Purpose: 
//   Returns if the i'th choice should be toggled.
//
// Arguments:
//   i : The choice being considered.
//
// Returns:    True if the i'th choice should be toggled.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:44:31 PST 2003
//
// Modifications:
//   Eric Brugger, Thu Jun 30 09:45:55 PDT 2005
//   Added a 2x3 window layout and removed the 4x4 window layout.
//   
// ****************************************************************************

bool
SetWindowLayoutAction::ChoiceChecked(int i) const
{
    bool retval;

    switch(i)
    {
    case 1:
        retval = windowMgr->GetWindowLayout() == 2;
        break;
    case 2:
        retval = windowMgr->GetWindowLayout() == 4;
        break;
    case 3:
        retval = windowMgr->GetWindowLayout() == 6;
        break;
    case 4:
        retval = windowMgr->GetWindowLayout() == 8;
        break;
    case 5:
        retval = windowMgr->GetWindowLayout() == 9;
        break;
    default:
        retval = windowMgr->GetWindowLayout() == 1;
        break;
    }

    return retval;
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: InvertBackgroundAction::InvertBackgroundAction
//
// Purpose: 
//   Constructor for the InvertBackgroundAction class.
//
// Arguments:
//   win : The window to which the action belongs.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:45:34 PST 2003
//
// Modifications:
//   
// ****************************************************************************

InvertBackgroundAction::InvertBackgroundAction(ViewerWindow *win) :
    ViewerAction(win)
{
    SetAllText(tr("Invert background"));
    SetToolTip(tr("Swap background and foreground colors"));
    if (!GetViewerProperties()->GetNowin())
        SetIcon(QIcon(QPixmap(invertbackground_xpm)));
}

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
// Method: SetWindowModeAction::SetWindowModeAction
//
// Purpose: 
//   Constructor for the SetWindowModeAction class.
//
// Arguments:
//   win : The window to which the action belongs.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:46:18 PST 2003
//
// Modifications:
//   Kathleen Bonnell, Wed Jun 25 13:41:23 PDT 2003
//   Change Pick to ZonePick, added NodePick.
//   
//   Kathleen Bonnell, Thu Jul  8 12:52:46 PDT 2004 
//   Change order of ZonePick and NodePick for NoWin mode.
//   
//   Gunther H. Weber, Wed Mar 19 16:10:11 PDT 2008
//   Added Spreadsheet Pick choice 
// ****************************************************************************

SetWindowModeAction::SetWindowModeAction(ViewerWindow *win) :
    ViewerMultipleAction(win)
{
    SetAllText(tr("Mode"));
    SetToolTip(tr("Set window mode"));
    SetExclusive(true);

    if (!GetViewerProperties()->GetNowin())
    {
        AddChoice(tr("Navigate"), tr("Navigate mode"), QPixmap(navigatemode_xpm));
        AddChoice(tr("Zone Pick"), tr("Zone Pick mode"), QPixmap(zonepickmode_xpm));
        AddChoice(tr("Node Pick"), tr("Node Pick mode"), QPixmap(nodepickmode_xpm));
        AddChoice(tr("Zoom"), tr("Zoom mode"), QPixmap(zoommode_xpm));
        AddChoice(tr("Lineout"), tr("Lineout mode"), QPixmap(lineoutmode_xpm));
        AddChoice(tr("Spreadsheet Pick"), tr("Spreadsheet Pick mode"), QPixmap(spreadsheetpickmode_xpm));
    }
    else 
    {
        AddChoice(tr("Navigate"));
        AddChoice(tr("Zone Pick"));
        AddChoice(tr("Node Pick"));
        AddChoice(tr("Zoom"));
        AddChoice(tr("Lineout"));
        AddChoice(tr("Spreadsheet Pick"));
    }
}


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
SetWindowModeAction::Setup()
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
SetWindowModeAction::Execute(int)
{
    INTERACTION_MODE mode = (INTERACTION_MODE)args.GetWindowMode();
    windowMgr->SetInteractionMode(mode, window->GetWindowId());
}

// ****************************************************************************
// Method: SetWindowModeAction::Enabled
//
// Purpose: 
//   Returns when the action is enabled.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:49:04 PST 2003
//
// Modifications:
//   Brad Whitlock, Fri Apr 4 15:49:24 PST 2003
//   Made it also use ViewerMultipleAction::Enabled.
//
// ****************************************************************************

bool
SetWindowModeAction::Enabled() const
{
    return ViewerMultipleAction::Enabled() &&
           (window->GetPlotList()->GetNumPlots() > 0);
}

// ****************************************************************************
// Method: SetWindowModeAction::ChoiceEnabled
//
// Purpose: 
//   Returns when the i'th choice is enabled.
//
// Arguments:
//   i : The choice to consider.
//
// Returns:    true if the choice is enabled; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:49:49 PST 2003
//
// Modifications:
//   Kathleen Bonnell, Tue Feb 25 17:11:18 PST 2003
//   Made enabling of lineout mode dependent upon loaded Curve and Lineout 
//   plugins.
//
//   Jeremy Meredith, Fri Feb 28 12:36:21 PST 2003
//   Made it use PluginAvailable instead of PluginLoaded so that it could
//   attempt to load the plugins on demand.
//
//   Brad Whitlock, Fri Apr 4 15:50:50 PST 2003
//   I swapped the cases for pick and zoom so they work in curve windows.
//
//   Kathleen Bonnell, Wed Jun 25 13:41:23 PDT 2003 
//   Added case for Node pick. 
//
//   Eric Brugger, Wed Aug 20 13:31:31 PDT 2003
//   I modified the logic to use GetWindowMode instead of GetTypeIsCurve.
//
//   Kathleen Bonnell, Tue Dec  2 17:36:44 PST 2003 
//   Allow pick to work in curve windows.
//
//   Jeremy Meredith, Thu Jan 31 14:56:06 EST 2008
//   Added new axis array window mode.  Most interaction modes are disabled
//   for this type of window.
//
//   Jeremy Meredith, Mon Feb  4 13:24:08 EST 2008
//   Added zoom interactor for AxisArray mode.
//
//   Gunther H. Weber, Wed Mar 19 18:47:34 PDT 2008
//   Added logic for spreadsheet pick 
//
//   Brad Whitlock, Tue Jun 24 14:53:49 PDT 2008
//   Changed how the plugin managers are accessed.
//
// ****************************************************************************

bool
SetWindowModeAction::ChoiceEnabled(int i) const
{
    bool retval = false;

    if(i == 0)
        retval = true;
    else if(i == 1) // zone pick
    {
        retval = (window->GetWindowMode() != WINMODE_AXISARRAY);
    }
    else if(i == 2) // node pick
    {
        retval = (window->GetWindowMode() != WINMODE_AXISARRAY);
    }
    else if(i == 3) // zoom
    {
        retval = true;
    }
    else if(i == 4)
    {
        retval = (window->GetWindowMode() == WINMODE_2D) &&
                 GetPlotPluginManager()->PluginAvailable("Curve_1.0") &&
                 GetOperatorPluginManager()->PluginAvailable("Lineout_1.0");
    }
    else if(i == 5) // spreadsheet pick
    {
        retval = (window->GetWindowMode() != WINMODE_AXISARRAY &&
                  GetPlotPluginManager()->PluginAvailable("Spreadsheet_1.0"));
    }

    return retval;
}

// ****************************************************************************
// Method: SetWindowModeAction::ChoiceChecked
//
// Purpose: 
//   Returns when the i'th choice is toggled.
//
// Arguments:
//   i : The choice to consider.
//
// Returns:    true if the choice is toggled; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:51:13 PST 2003
//
// Modifications:
//   
// ****************************************************************************

bool
SetWindowModeAction::ChoiceChecked(int i) const
{
    bool val = (window->GetInteractionMode() == (INTERACTION_MODE)i);
    return val;
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: EnableToolAction::EnableToolAction
//
// Purpose: 
//   Constructor for the EnableToolAction class.
//
// Arguments:
//   win : The window to which the action belongs.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:51:56 PST 2003
//
// Modifications:
//
//    Mark Blair, Wed Aug 30 14:09:00 PDT 2006
//    Added support for Extents tool.
//   
//    Jeremy Meredith, Fri Feb  1 17:59:23 EST 2008
//    Added Axis restriction tool.
//
//    Jeremy Meredith, Tue Apr 22 14:31:20 EDT 2008
//    Removed Extents tool.  (Subsumed by axis restriction tool.)
//
// ****************************************************************************

EnableToolAction::EnableToolAction(ViewerWindow *win) :
    ViewerMultipleAction(win)
{
    SetAllText(tr("Tools"));
    SetExclusive(false);

    for(int i = 0; i < window->GetNumTools(); ++i)
    {
        std::string tool(window->GetToolName(i));
        if (!GetViewerProperties()->GetNowin())
        {
            if(tool == "Box")
                AddChoice(tr("Box"), tr("Box tool"), QPixmap(boxtool_xpm));
            else if(tool == "Line")
                AddChoice(tr("Line"), tr("Line tool"), QPixmap(linetool_xpm));
            else if(tool == "Plane")
                AddChoice(tr("Plane"), tr("Plane tool"), QPixmap(planetool_xpm));
            else if(tool == "Sphere")
                AddChoice(tr("Sphere"), tr("Sphere tool"), QPixmap(spheretool_xpm));
            else if(tool == "Point")
                AddChoice(tr("Point"), tr("Point tool"), QPixmap(pointtool_xpm));
            else if(tool == "AxisRestriction")
                AddChoice(tr("Axis Restriction"), tr("Axis Restriction tool"), QPixmap(axisrestrictiontool_xpm));
            else
                AddChoice(tool.c_str());
        }
        else
        {
            AddChoice(tool.c_str());
        }
    }
}

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
EnableToolAction::Setup()
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
EnableToolAction::Execute(int)
{
    windowMgr->SetToolEnabled(args.GetToolId(), args.GetBoolFlag(),
                              window->GetWindowId());
}

// ****************************************************************************
// Method: EnableToolAction::Enabled
//
// Purpose: 
//   Returns when the action is enabled.
//
// Returns:    true if the action is enabled; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:53:57 PST 2003
//
// Modifications:
//   Brad Whitlock, Fri Apr 4 15:54:34 PST 2003
//   I made it use ViewerMultipleAction::Enabled too.
//
// ****************************************************************************

bool
EnableToolAction::Enabled() const
{
    return ViewerMultipleAction::Enabled() &&
           (window->GetPlotList()->GetNumPlots() > 0);
}

// ****************************************************************************
// Method: EnableToolAction::ChoiceEnabled
//
// Purpose: 
//   Returns when the i'th choice is enabled.
//
// Arguments:
//   i : The choice to consider.
//
// Returns:    true if the choice is enabled; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:54:47 PST 2003
//
// Modifications:
//   
// ****************************************************************************

bool
EnableToolAction::ChoiceEnabled(int i) const
{
    return window->GetToolAvailable(i);
}

// ****************************************************************************
// Method: EnableToolAction::ChoiceChecked
//
// Purpose: 
//   Returns when the i'th choice should be toggled.
//
// Arguments:
//   i : The choice to consider.
//
// Returns:    true if the choice should be toggled; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:55:33 PST 2003
//
// Modifications:
//   
// ****************************************************************************

bool
EnableToolAction::ChoiceChecked(int i) const
{
    return window->GetToolEnabled(i);
}
