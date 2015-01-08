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

#include <WindowActionsUI.h>

#include <ViewerActionLogic.h>
#include <ViewerPlotList.h>
#include <ViewerProperties.h>
#include <ViewerQueryManager.h>
#include <ViewerWindow.h>
#include <ViewerWindowManager.h>
#include <PlotPluginManager.h>
#include <OperatorPluginManager.h>
#include <avtDatabaseMetaData.h>
#include <avtSimulationCommandSpecification.h>

#ifdef HAVE_DDT
#include <DDTManager.h>
#endif

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
#include <ddtpickmode.xpm>
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
// Method: AddWindowActionUI::AddWindowActionUI
//
// Purpose: 
//   Constructor for the AddWindowActionUI class.
//
// Arguments:
//   win : The window to which the action belongs.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:31:37 PST 2003
//
// Modifications:
//   
//   Allen Sanderson, Mon Mar  8 19:57:29 PST 2010
//   Change description displayed to the user.
//
// ****************************************************************************

AddWindowActionUI::AddWindowActionUI(ViewerActionLogic *L) :
    ViewerActionUISingle(L)
{
    SetAllText(tr("Create a new viewer window"));
    SetMenuText(tr("New"));
    if (!GetViewerProperties()->GetNowin())
        SetIcon(QIcon(QPixmap(newwindow_xpm)));
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: CloneWindowActionUI::CloneWindowActionUI
//
// Purpose: 
//   Constructor for the CloneWindowActionUI class.
//
// Arguments:
//   win : The window to which the action belongs.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:31:37 PST 2003
//
// Modifications:
//   
//   Allen Sanderson, Mon Mar  8 19:57:29 PST 2010
//   Change description displayed to the user.
//
// ****************************************************************************

CloneWindowActionUI::CloneWindowActionUI(ViewerActionLogic *L) :
    ViewerActionUISingle(L)
{
    SetAllText(tr("Clone this window"));
    SetMenuText(tr("Clone"));
    if (!GetViewerProperties()->GetNowin())
        SetIcon(QIcon(QPixmap(copymenu_xpm)));
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: DeleteWindowActionUI::DeleteWindowActionUI
//
// Purpose: 
//   Constructor for the DeleteWindowActionUI class.
//
// Arguments:
//   win : The window to which the action belongs.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:31:37 PST 2003
//
// Modifications:
//   
//   Allen Sanderson, Mon Mar  8 19:57:29 PST 2010
//   Change description displayed to the user.
//
// ****************************************************************************

DeleteWindowActionUI::DeleteWindowActionUI(ViewerActionLogic *L) :
    ViewerActionUISingle(L)
{
    SetAllText(tr("Delete this window"));
    SetMenuText(tr("Delete"));
    if (!GetViewerProperties()->GetNowin())
        SetIcon(QIcon(QPixmap(deletewindow_xpm)));
}

// ****************************************************************************
// Method: DeleteWindowActionUI::Enabled
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
DeleteWindowActionUI::Enabled() const
{
    return (ViewerWindowManager::Instance()->GetNumWindows() > 1);
}

void
DeleteWindowActionUI::Update()
{
    // Override the Update method because we get memory errors because the
    // window is only partially deleted at this point.
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ClearWindowActionUI::ClearWindowActionUI
//
// Purpose: 
//   Constructor for the ClearWindowActionUI class.
//
// Arguments:
//   win : The window to which the action belongs.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:31:37 PST 2003
//
// Modifications:
//   
//   Allen Sanderson, Mon Mar  8 19:57:29 PST 2010
//   Change description displayed to the user.
//
// ****************************************************************************

ClearWindowActionUI::ClearWindowActionUI(ViewerActionLogic *L) :
    ViewerActionUISingle(L)
{
    SetAllText(tr("Clear all plots from this window"));
    if (!GetViewerProperties()->GetNowin())
        SetIcon(QIcon(QPixmap(clearwindow_xpm)));
}

// ****************************************************************************
// Method: ClearWindowActionUI::Enabled
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
ClearWindowActionUI::Enabled() const
{
    // This action should only be enabled if the window to which the action belongs
    // has plots in it.
    return (GetLogic()->GetWindow()->GetPlotList()->GetNumPlots() > 0);
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ClearAllWindowsActionUI::ClearAllWindowsActionUI
//
// Purpose: 
//   Constructor for the ClearAllWindowsActionUI class.
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

ClearAllWindowsActionUI::ClearAllWindowsActionUI(ViewerActionLogic *L) :
    ViewerActionUISingle(L)
{
    SetAllText(tr("Clear all windows"));
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ClearPickPointsActionUI::ClearPickPointsActionUI
//
// Purpose: 
//   Constructor for the ClearPickPointsActionUI class.
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

ClearPickPointsActionUI::ClearPickPointsActionUI(ViewerActionLogic *L) :
    ViewerActionUISingle(L)
{
    SetAllText(tr("Clear pick points"));
}

// ****************************************************************************
// Method: ClearPickPointsActionUI::Enabled
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
ClearPickPointsActionUI::Enabled() const
{
    // This action should only be enabled if the window to which the action belongs
    // has plots in it.
    return (GetLogic()->GetWindow()->GetPlotList()->GetNumPlots() > 0);
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ClearReferenceLinesActionUI::ClearReferenceLinesActionUI
//
// Purpose: 
//   Constructor for the ClearReferenceLinesActionUI class.
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

ClearReferenceLinesActionUI::ClearReferenceLinesActionUI(ViewerActionLogic *L) :
    ViewerActionUISingle(L)
{
    SetAllText(tr("Clear reference lines"));
}

// ****************************************************************************
// Method: ClearReferenceLinesActionUI::Enabled
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
ClearReferenceLinesActionUI::Enabled() const
{
    // This action should only be enabled if the window to which the action belongs
    // has plots in it.
    return (GetLogic()->GetWindow()->GetPlotList()->GetNumPlots() > 0);
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetActiveWindowActionUI::SetActiveWindowActionUI
//
// Purpose: 
//   Constructor for the SetActiveWindowActionUI class.
//
// Arguments:
//   win : The window to which the action belongs.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:31:37 PST 2003
//
// Modifications:
//   
//   Allen Sanderson, Mon Mar  8 19:57:29 PST 2010
//   Change description displayed to the user.
//
// ****************************************************************************

SetActiveWindowActionUI::SetActiveWindowActionUI(ViewerActionLogic *L) :
    ViewerActionUIToggle(L)
{
    SetAllText(tr("Make active"));
    SetToolTip(tr("Make this window active"));
    if (!GetViewerProperties()->GetNowin())
        SetIcons(QPixmap(checkwindow_xpm), QPixmap(blankwindow_xpm));
}

// ****************************************************************************
// Method: SetActiveWindowActionUI::Checked
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
SetActiveWindowActionUI::Checked() const
{
    // This action should be toggled if it belongs to the active window.
    return (GetLogic()->GetWindow() == ViewerWindowManager::Instance()->GetActiveWindow()); 
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ToggleSpinModeActionUI::ToggleSpinModeActionUI
//
// Purpose: 
//   Constructor for the ToggleSpinModeActionUI class.
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

ToggleSpinModeActionUI::ToggleSpinModeActionUI(ViewerActionLogic *L) :
    ViewerActionUIToggle(L)
{
    SetAllText(tr("Spin"));
    SetToolTip(tr("Toggle spin mode"));
    if (!GetViewerProperties()->GetNowin())
        SetIcons(QPixmap(spinon_xpm), QPixmap(spinoff_xpm));
}

// ****************************************************************************
// Method: ToggleSpinModeActionUI::Enabled
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
ToggleSpinModeActionUI::Enabled() const
{
    // This action should only be enabled if the window to which the action belongs
    // has plots in it.
    return (GetLogic()->GetWindow()->GetPlotList()->GetNumPlots() > 0);
}

// ****************************************************************************
// Method: ToggleSpinModeActionUI::Checked
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
ToggleSpinModeActionUI::Checked() const
{
    return GetLogic()->GetWindow()->GetSpinMode();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetWindowLayoutActionUI::SetWindowLayoutActionUI
//
// Purpose: 
//   Constructor for the SetWindowLayoutActionUI class.
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

SetWindowLayoutActionUI::SetWindowLayoutActionUI(ViewerActionLogic *L) :
    ViewerActionUIMultiple(L)
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
// Method: SetWindowLayoutActionUI::ChoiceChecked
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
SetWindowLayoutActionUI::ChoiceChecked(int i) const
{
    bool retval;

    switch(i)
    {
    case 1:
        retval = ViewerWindowManager::Instance()->GetWindowLayout() == 2;
        break;
    case 2:
        retval = ViewerWindowManager::Instance()->GetWindowLayout() == 4;
        break;
    case 3:
        retval = ViewerWindowManager::Instance()->GetWindowLayout() == 6;
        break;
    case 4:
        retval = ViewerWindowManager::Instance()->GetWindowLayout() == 8;
        break;
    case 5:
        retval = ViewerWindowManager::Instance()->GetWindowLayout() == 9;
        break;
    default:
        retval = ViewerWindowManager::Instance()->GetWindowLayout() == 1;
        break;
    }

    return retval;
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: InvertBackgroundActionUI::InvertBackgroundActionUI
//
// Purpose: 
//   Constructor for the InvertBackgroundActionUI class.
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

InvertBackgroundActionUI::InvertBackgroundActionUI(ViewerActionLogic *L) :
    ViewerActionUISingle(L)
{
    SetAllText(tr("Invert background"));
    SetToolTip(tr("Swap background and foreground colors"));
    if (!GetViewerProperties()->GetNowin())
        SetIcon(QIcon(QPixmap(invertbackground_xpm)));
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetWindowModeActionUI::SetWindowModeActionUI
//
// Purpose: 
//   Constructor for the SetWindowModeActionUI class.
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
//
//   Allen Sanderson, Mon Mar  8 19:57:29 PST 2010
//   Reorder icons (put picks together).
//
//   Jonathan Byrd (Allinea Software), Sun Dec 18, 2011
//   Added the DDT_PICK mode
//
//   Dirk Schubert (Allinea Software), Fri Oct 12, 2012
//   Show DDT_PICK mode only if HAVE_DDT is defined, else hide it.
//
// ****************************************************************************

SetWindowModeActionUI::SetWindowModeActionUI(ViewerActionLogic *L) :
    ViewerActionUIMultiple(L)
{
    SetAllText(tr("Mode"));
    SetToolTip(tr("Set window mode"));
    SetExclusive(true);

    // NOTE The ordering of these glyphs must match the ordering of the enum 
    // INTERACTION_MODE that is defined in:
    // avt/VisWindow/VisWindow/VisWindowTypes.h

    // DO NOT ADD A GLYPH WITHOUT UPDATING THE ENUMS.

    // Also when adding new action please use a logical ordering even
    // if means putting it beween two existing actions.
    if (!GetViewerProperties()->GetNowin())
    {
        AddChoice(tr("Navigate"), tr("Navigate mode"), QPixmap(navigatemode_xpm));
        AddChoice(tr("Zoom"), tr("Zoom mode"), QPixmap(zoommode_xpm));
        AddChoice(tr("Zone Pick"), tr("Zone Pick mode"), QPixmap(zonepickmode_xpm));
        AddChoice(tr("Node Pick"), tr("Node Pick mode"), QPixmap(nodepickmode_xpm));
        AddChoice(tr("Spreadsheet Pick"), tr("Spreadsheet Pick mode"), QPixmap(spreadsheetpickmode_xpm));
        AddChoice(tr("Add DDT Pick"), tr("DDT Pick mode"), QPixmap(ddtpickmode_xpm));
        AddChoice(tr("Lineout"), tr("Lineout mode"), QPixmap(lineoutmode_xpm));
    }
    else 
    {
        AddChoice(tr("Navigate"));
        AddChoice(tr("Zoom"));
        AddChoice(tr("Zone Pick"));
        AddChoice(tr("Node Pick"));
        AddChoice(tr("Spreadsheet Pick"));
        AddChoice(tr("Add DDT Pick"));
        AddChoice(tr("Lineout"));
    }

#ifndef HAVE_DDT
    HideChoice(DDT_PICK);
#endif
}

// ****************************************************************************
// Method: SetWindowModeActionUI::Enabled
//
// Purpose: 
//   Returns when the action is enabled.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 4 15:49:04 PST 2003
//
// Modifications:
//   Brad Whitlock, Fri Apr 4 15:49:24 PST 2003
//   Made it also use ViewerActionUIMultiple::Enabled.
//
// ****************************************************************************

bool
SetWindowModeActionUI::Enabled() const
{
    return ViewerActionUIMultiple::Enabled() &&
           (GetLogic()->GetWindow()->GetPlotList()->GetNumPlots() > 0);
}

// ****************************************************************************
// Method: SetWindowModeActionUI::ChoiceEnabled
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
//   Hank Childs, Mon Mar  8 20:55:13 PST 2010
//   Update for new ordering.
//
//   Jonathan Byrd (Allinea Software), Sun Dec 18, 2011
//   Added the DDT_PICK mode
//
// ****************************************************************************

bool
SetWindowModeActionUI::ChoiceEnabled(int i) const
{
    bool retval = false;

    if(i == NAVIGATE) // navigate
    {
        retval = true;
    }
    else if(i == ZOOM) // zoom
    {
        retval = true;
    }
    else if(i == ZONE_PICK) // zone pick
    {
        retval = (GetLogic()->GetWindow()->GetWindowMode() != WINMODE_AXISARRAY);
    }
    else if(i == NODE_PICK) // node pick
    {
        retval = (GetLogic()->GetWindow()->GetWindowMode() != WINMODE_AXISARRAY);
    }
    else if(i == SPREADSHEET_PICK) // spreadsheet pick
    {
        retval = (GetLogic()->GetWindow()->GetWindowMode() != WINMODE_AXISARRAY &&
                  GetPlotPluginManager()->PluginAvailable("Spreadsheet_1.0"));
    }
    else if(i == DDT_PICK) // ddt pick
    {
        retval = (GetLogic()->GetWindow()->GetWindowMode() != WINMODE_AXISARRAY);
    }
    else if(i == LINEOUT) // lineout
    {
        retval = (GetLogic()->GetWindow()->GetWindowMode() == WINMODE_2D) &&
                 GetPlotPluginManager()->PluginAvailable("Curve_1.0") &&
                 GetOperatorPluginManager()->PluginAvailable("Lineout_1.0");
    }

    return retval;
}

// ****************************************************************************
// Method: SetWindowModeActionUI::ChoiceChecked
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
SetWindowModeActionUI::ChoiceChecked(int i) const
{
    bool val = (GetLogic()->GetWindow()->GetInteractionMode() == (INTERACTION_MODE)i);
    return val;
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: EnableToolActionUI::EnableToolActionUI
//
// Purpose: 
//   Constructor for the EnableToolActionUI class.
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

EnableToolActionUI::EnableToolActionUI(ViewerActionLogic *L) :
    ViewerActionUIMultiple(L)
{
    SetAllText(tr("Tools"));
    SetExclusive(false);

    for(int i = 0; i < GetLogic()->GetWindow()->GetNumTools(); ++i)
    {
        std::string tool(GetLogic()->GetWindow()->GetToolName(i));
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
// Method: EnableToolActionUI::Enabled
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
//   I made it use ViewerActionUIMultiple::Enabled too.
//
// ****************************************************************************

bool
EnableToolActionUI::Enabled() const
{
    return ViewerActionUIMultiple::Enabled() &&
           (GetLogic()->GetWindow()->GetPlotList()->GetNumPlots() > 0);
}

// ****************************************************************************
// Method: EnableToolActionUI::ChoiceEnabled
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
EnableToolActionUI::ChoiceEnabled(int i) const
{
    return GetLogic()->GetWindow()->GetToolAvailable(i);
}

// ****************************************************************************
// Method: EnableToolActionUI::ChoiceChecked
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
EnableToolActionUI::ChoiceChecked(int i) const
{
    return GetLogic()->GetWindow()->GetToolEnabled(i);
}
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetToolUpdateModeActionUI::SetToolUpdateModeActionUI
//
// Purpose: 
//   Constructor for the SetToolUpdateModeActionUI class.
//
// Arguments:
//   win : The window to which the action belongs.
//
// Programmer: Jeremy Meredith
// Creation:   February  2, 2010
//
// Modifications:
//
// ****************************************************************************

SetToolUpdateModeActionUI::SetToolUpdateModeActionUI(ViewerActionLogic *L) :
    ViewerActionUIMultiple(L)
{
    SetAllText(tr("Tool Updates"));
    SetToolTip(tr("Set tool update frequency"));
    SetExclusive(true);

    // We're not adding this to the toolbar, just the
    // pop-up menu, so don't bother with pixmaps or long/short names.
    AddChoice(tr("Continuously"));
    AddChoice(tr("On Mouse Release"));
    AddChoice(tr("On Tool Close"));
}

// ****************************************************************************
// Method: SetToolUpdateModeActionUI::Enabled
//
// Purpose: 
//   Returns when the action is enabled.
//
// Programmer: Jeremy Meredith
// Creation:   February  2, 2010
//
// Modifications:
//
// ****************************************************************************

bool
SetToolUpdateModeActionUI::Enabled() const
{
    return (ViewerActionUIMultiple::Enabled() &&
            GetLogic()->GetWindow()->GetPlotList()->GetNumPlots() > 0);
}

// ****************************************************************************
// Method: SetToolUpdateModeActionUI::ChoiceChecked
//
// Purpose: 
//   Returns when the i'th choice is toggled.
//
// Arguments:
//   i : The choice to consider.
//
// Returns:    true if the choice is toggled; false otherwise.
//
// Programmer: Jeremy Meredith
// Creation:   February  2, 2010
//
// Modifications:
//   
// ****************************************************************************

bool
SetToolUpdateModeActionUI::ChoiceChecked(int i) const
{
    bool val = (GetLogic()->GetWindow()->GetToolUpdateMode() == (TOOLUPDATE_MODE)i);
    return val;
}

// ****************************************************************************
// Method: SetToolUpdateModeActionUI::ChoiceChecked
//
// Purpose: 
//   Returns when the i'th choice is toggled.
//
// Arguments:
//   i : The choice to consider.
//
// Returns:    true if the choice is toggled; false otherwise.
//
// Programmer: Jeremy Meredith
// Creation:   February  2, 2010
//
// ****************************************************************************

bool
SetToolUpdateModeActionUI::ChoiceEnabled(int i) const
{
    if (GetLogic()->GetWindow()->GetBoundingBoxMode() == true &&
        (TOOLUPDATE_MODE)i == UPDATE_CONTINUOUS)
        return false;

    return true;
}
