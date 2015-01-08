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

#include <PlotAndOperatorActions.h>
#include <ViewerMessaging.h>
#include <ViewerOperatorFactory.h>
#include <ViewerPlotFactory.h>
#include <ViewerPlotList.h>
#include <ViewerProperties.h>
#include <ViewerQueryManager.h>
#include <ViewerState.h>
#include <ViewerText.h>
#include <ViewerWindow.h>
#include <ViewerWindowManager.h>

#include <GlobalAttributes.h>
#include <InvalidExpressionException.h>

#include <OperatorPluginManager.h>
#include <OperatorPluginInfo.h>
#include <PlotPluginManager.h>
#include <PlotPluginInfo.h>

// ****************************************************************************
// Method: AddOperatorAction::AddOperatorAction
//
// Purpose: 
//   Constructor for the AddOperator action.
//
// Arguments:
//   win : The window that owns the action.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 17 09:12:08 PDT 2003
//
// Modifications:
//
// ****************************************************************************

AddOperatorAction::AddOperatorAction(ViewerWindow *win) :
    ViewerActionLogic(win), graphicalPlugins()
{
    //
    // Iterate through all of the loaded operator plugins and add a
    // choice for all of the ones that have icons.
    //
    ViewerOperatorPluginInfo *info = 0;
    OperatorPluginManager *pluginMgr = GetOperatorPluginManager();
    int nTypes = pluginMgr->GetNEnabledPlugins();
    for (int i = 0; i < nTypes; ++i)
    {
        info = pluginMgr->GetViewerPluginInfo(pluginMgr->GetEnabledID(i));
        if(info)
        {
            if(!GetViewerProperties()->GetNowin() && 
               info->XPMIconData() != 0 &&
               info->GetUserSelectable())
            {
                // Record that this plugin has an icon.
                graphicalPlugins.push_back(i);
            }
        }
    }
}

// ****************************************************************************
// Method: AddOperatorAction::Setup
//
// Purpose: 
//   This method is called when the toolbar button is clicked. It stores the
//   information about the button into the args so that the Execute method
//   can work for both toolbar clicks and commands from the client.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 17 09:20:22 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
AddOperatorAction::Setup(int activeAction, bool toggled)
{
    args.SetOperatorType(graphicalPlugins[activeAction]);
}

// ****************************************************************************
// Method: AddOperatorAction::Execute
//
// Purpose: 
//   Adds an operator to the plots in the window's plot list.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 17 09:21:25 PDT 2003
//
// Modifications:
//    Jeremy Meredith, Tue Jun 17 18:17:55 PDT 2003
//    Fixed a bug where disabling an operator would cause crashes when
//    using unrelated operators; it was using AllID instead of EnabledID.
//   
//    Kathleen Bonnell, Thu Sep 11 11:35:08 PDT 2003
//    Added 'from Default' to 'AddOperator'. 
//    
//    Kathleen Bonnell, Fri Jul  9 13:40:42 PDT 2004
//    Make ViewerWindow handle Lineouts (instead of ViewerQueryManager).
//    This is to ensure that new lineout windows that get created are 
//    properly initialized in SR mode. 
//    
// ****************************************************************************

void
AddOperatorAction::Execute()
{
    //
    // Get the rpc arguments.
    //
    int type = args.GetOperatorType();
    bool fromDefault = args.GetBoolFlag();

    OperatorPluginManager *opMgr = GetOperatorPluginManager();
    std::string name(opMgr->GetPluginName(opMgr->GetEnabledID(type)));
    if (name == "Lineout") // PLUGIN SIN!!!
    {
        window->Lineout(fromDefault);
    }
    else
    {
        //
        // Add the operator to the window's plot list.
        //
        bool applyToAllWindows = GetViewerState()->GetGlobalAttributes()->GetApplyWindow();
        bool applyOperatorToAllPlots = GetViewerState()->GetGlobalAttributes()->GetApplyOperator();

        if( applyToAllWindows )
        {
          for( int i=0; i< windowMgr->GetNumWindows(); ++i )
          {
            windowMgr->GetWindow(i)->GetPlotList()->
              AddOperator(type, applyOperatorToAllPlots, fromDefault);
          }
        }
        else // Just the active window
        {
          window->GetPlotList()->
            AddOperator(type, applyOperatorToAllPlots, fromDefault);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: PromoteOperatorAction::Execute
//
// Purpose: 
//   This method executes the action.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 10 09:48:59 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
PromoteOperatorAction::Execute()
{
    bool applyToAllWindows = GetViewerState()->GetGlobalAttributes()->GetApplyWindow();
    bool applyOperatorToAllPlots = GetViewerState()->GetGlobalAttributes()->GetApplyOperator();

    if( applyToAllWindows )
    {
      for( int i=0; i< windowMgr->GetNumWindows(); ++i )
      {
        windowMgr->GetWindow(i)->GetPlotList()->
          PromoteOperator(args.GetOperatorType(), applyOperatorToAllPlots);
      }
    }
    else // Just the active window
    {
      window->GetPlotList()->
        PromoteOperator(args.GetOperatorType(), applyOperatorToAllPlots);
    }
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: DemoteOperatorAction::Execute
//
// Purpose: 
//   This method executes the action.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 10 09:48:59 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
DemoteOperatorAction::Execute()
{
    bool applyToAllWindows = GetViewerState()->GetGlobalAttributes()->GetApplyWindow();
    bool applyOperatorToAllPlots = GetViewerState()->GetGlobalAttributes()->GetApplyOperator();

    if( applyToAllWindows )
    {
      for( int i=0; i< windowMgr->GetNumWindows(); ++i )
      {
        windowMgr->GetWindow(i)->GetPlotList()->
          DemoteOperator(args.GetOperatorType(), applyOperatorToAllPlots);
      }
    }
    else // Just the active window
    {
      window->GetPlotList()->
        DemoteOperator(args.GetOperatorType(), applyOperatorToAllPlots);
    }
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: RemoveOperatorAction::Execute
//
// Purpose: 
//   This method executes the action.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 10 09:48:59 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
RemoveOperatorAction::Execute()
{
    bool applyToAllWindows = GetViewerState()->GetGlobalAttributes()->GetApplyWindow();
    bool applyOperatorToAllPlots = GetViewerState()->GetGlobalAttributes()->GetApplyOperator();

    if( applyToAllWindows )
    {
      for( int i=0; i< windowMgr->GetNumWindows(); ++i )
      {
        windowMgr->GetWindow(i)->GetPlotList()->
          RemoveOperator(args.GetOperatorType(), applyOperatorToAllPlots);
      }
    }
    else // Just the active window
    {
      window->GetPlotList()->
        RemoveOperator(args.GetOperatorType(), applyOperatorToAllPlots);
    }
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: RemoveLastOperatorAction::Execute
//
// Purpose: 
//   Executes the work for RemoveLastOperatorAction.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 20 12:41:50 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
RemoveLastOperatorAction::Execute()
{
    bool applyToAllWindows = GetViewerState()->GetGlobalAttributes()->GetApplyWindow();
    bool applyOperatorToAllPlots = GetViewerState()->GetGlobalAttributes()->GetApplyOperator();

    if( applyToAllWindows )
    {
      for( int i=0; i<windowMgr->GetNumWindows(); ++i )
      {
        windowMgr->GetWindow(i)->GetPlotList()->
          RemoveLastOperator(applyOperatorToAllPlots);
      }
    }
    else // Just the active window
    {
      window->GetPlotList()->
        RemoveLastOperator(applyOperatorToAllPlots);
    }
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: RemoveAllOperatorsAction::Execute
//
// Purpose: 
//   Executes the RemoveAllOperatorsAction.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 20 12:41:50 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
RemoveAllOperatorsAction::Execute()
{
    bool applyToAllWindows = GetViewerState()->GetGlobalAttributes()->GetApplyWindow();
    bool applyOperatorToAllPlots = GetViewerState()->GetGlobalAttributes()->GetApplyOperator();

    if( applyToAllWindows )
    {
      for( int i=0; i<windowMgr->GetNumWindows(); ++i )
      {
        windowMgr->GetWindow(i)->GetPlotList()->
          RemoveAllOperators(applyOperatorToAllPlots);
      }
    }
    else // Just the active window
    {
      window->GetPlotList()->
        RemoveAllOperators(applyOperatorToAllPlots);
    }
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetOperatorOptionsAction::Execute
//
// Purpose: 
//   Sets the operator attributes.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 08:02:54 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
SetOperatorOptionsAction::Execute()
{
    //
    // Set the operator attributes.
    //
    int  oper = args.GetOperatorType();
    bool applyToAllWindows = GetViewerState()->GetGlobalAttributes()->GetApplyWindow();
    bool applyOperatorToAllPlots = GetViewerState()->GetGlobalAttributes()->GetApplyOperator();
    
    if( applyToAllWindows )
    {
      for( int i=0; i<windowMgr->GetNumWindows(); ++i )
      {
        windowMgr->GetWindow(i)->GetPlotList()->
          SetPlotOperatorAtts(oper, windowMgr->GetWindow(i)==window,
                              applyToAllWindows, applyOperatorToAllPlots);
      }
    }
    else // Just the active window
    {
      window->GetPlotList()->
        SetPlotOperatorAtts(oper, true,
                            applyToAllWindows, applyOperatorToAllPlots);
    }
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: AddEmbeddedPlotAction::Execute
//
// Purpose: 
//   This method is called when the AddEmbeddedPlotAction must be executed.
//
// Programmer: Marc Durant
// Creation:   June 19, 2011
//
// Modifications:
//
// ****************************************************************************

void
AddEmbeddedPlotAction::Execute()
{
  bool replacePlots = GetViewerState()->GetGlobalAttributes()->GetReplacePlots();
  bool applyOperator = GetViewerState()->GetGlobalAttributes()->GetApplyOperator();
  bool applySelection = GetViewerState()->GetGlobalAttributes()->GetApplySelection();
  bool inheritSILRestriction = GetViewerState()->GetGlobalAttributes()->
  GetNewPlotsInheritSILRestriction();
  
  //
  // Try and create the plot.
  //
  window->GetPlotList()->AddPlot(args.GetPlotType(),
                                 args.GetVariable().c_str(), replacePlots, applyOperator,
                                 inheritSILRestriction, applySelection, NULL, args.GetEmbeddedPlotId());
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: AddPlotAction::AddPlotAction
//
// Purpose: 
//   Constructor for the AddPlotAction class.
//
// Arguments:
//   win : The window that owns the action.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 20 12:41:50 PDT 2003
//
// Modifications:
//   Brad Whitlock, Mon Sep 29 17:39:17 PST 2003
//   Initialized host.
//
//   Brad Whitlock, Tue Jul 27 18:09:16 PST 2004
//   Changed so pixmaps are not created when we're in -nowin mode.
//
//   Brad Whitlock, Tue Apr 29 11:41:42 PDT 2008
//   Added tr(), remove code to disable Curve plot. Made the code use the
//   plot's menu name.
//
//   Brad Whitlock, Thu May 22 13:53:02 PDT 2008
//   Qt 4.
//
// ****************************************************************************

AddPlotAction::AddPlotAction(ViewerWindow *win) : ViewerActionLogic(win),
    graphicalPlugins()
{
    //
    // Iterate through all of the loaded plot plugins and add a
    // choice for all of the ones that have icons.
    //
    ViewerPlotPluginInfo *info = 0;
    PlotPluginManager *pluginMgr = GetPlotPluginManager();
    for(int i = 0; i < pluginMgr->GetNEnabledPlugins(); ++i)
    {
        info = pluginMgr->GetViewerPluginInfo(pluginMgr->GetEnabledID(i));
        if(info)
        {
            if(!GetViewerProperties()->GetNowin() && info->XPMIconData() != 0)
            {
                // Record the plugin entry.
                graphicalPlugins.push_back(i);
            }
        }
    }
}

void
AddPlotAction::SetArguments(int plotType, const std::string &var)
{
    args.SetPlotType(plotType);
    args.SetVariable(var);
}

// ****************************************************************************
// Method: AddPlotAction::Execute
//
// Purpose: 
//   This method is called when the AddPlotAction must be executed.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 20 12:44:42 PDT 2003
//
// Modifications:
//   Brad Whitlock, Thu Jan 24 11:39:15 PDT 2008
//   Made inheriting the SIL restriction optional.
//
//   Brad Whitlock, Thu Aug 12 14:36:18 PDT 2010
//   Pass applySelection so it affects whether a plot is created with a 
//   named selection.
//
// ****************************************************************************

void
AddPlotAction::Execute()
{
    bool replacePlots = GetViewerState()->GetGlobalAttributes()->GetReplacePlots();
    bool applyOperator = GetViewerState()->GetGlobalAttributes()->GetApplyOperator();
    bool applySelection = GetViewerState()->GetGlobalAttributes()->GetApplySelection();
    bool inheritSILRestriction = GetViewerState()->GetGlobalAttributes()->
        GetNewPlotsInheritSILRestriction();

    //
    // Try and create the plot.
    //
    window->GetPlotList()->AddPlot(args.GetPlotType(),
        args.GetVariable().c_str(), replacePlots, applyOperator,
        inheritSILRestriction, applySelection);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: DrawPlotsAction::Execute
//
// Purpose: 
//   Draws the plots
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 21 15:51:48 PST 2003
//
// Modifications:
//  Ellen Tarwater October 12, 2007
//  added flag for active vs all plots
//   
// ****************************************************************************

void
DrawPlotsAction::Execute()
{
    bool applyToAllWindows = GetViewerState()->GetGlobalAttributes()->GetApplyWindow();
    
    if( applyToAllWindows )
    {
      for( int i=0; i<windowMgr->GetNumWindows(); ++i )
      {
        windowMgr->GetWindow(i)->GetPlotList()->
          RealizePlots(args.GetBoolFlag());
      }
    }
    else // Just the active window
    {
      window->GetPlotList()->RealizePlots(args.GetBoolFlag());
    }
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: HideActivePlotsAction::Execute
//
// Purpose: 
//   Hides the active plots.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 21 15:51:48 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
HideActivePlotsAction::Execute()
{
    window->GetPlotList()->HideActivePlots();
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: DeleteActivePlotsAction::Execute
//
// Purpose: 
//   Executes the DeleteActivePlots action.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 21 15:51:48 PST 2003
//
// Modifications:
//   Brad Whitlock, Mon Jun 23 16:30:51 PST 2003
//   I made the ClearPickPoints and ClearRefLines methods use the window
//   instead of the window manager.
//
//   Kathlen Bonnell, Thu Apr  1 19:13:59 PST 2004 
//   Added call to ResetTimeQueryDesignation. 
//
// ****************************************************************************

void
DeleteActivePlotsAction::Execute()
{
    window->GetPlotList()->DeleteActivePlots();
    if(window->GetPlotList()->GetNumPlots() == 0)
    {
        window->ClearPickPoints();
        window->ClearRefLines();
        windowMgr->ResetLineoutDesignation();
        windowMgr->ResetTimeQueryDesignation();
        windowMgr->DisableAllTools();
    }
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetActivePlotsAction::Execute
//
// Purpose: 
//   Sets the active plots for the window's plot list.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 07:50:49 PDT 2003
//
// Modifications:
//   Eric Brugger, Mon Jul 28 16:48:01 PDT 2003
//   Added code to make the database for the first active plot the database
//   for the plotlist when the plotlist has at least one active plot.
//
//   Brad Whitlock, Mon Dec 8 15:25:54 PST 2003
//   I removed Eric's code.
//
//   Kathleen Bonnell, Wed Aug 10 16:46:17 PDT 2005 
//   Notify ViewerQueryManger that active plot has changed if in Pick mode.
//   (So that plot can be reexecuted if necesssary).
//
//   Jonathan Byrd (Allinea Software) Sun Dec 18, 2011
//   Added DDT_PICK mode.
//
// ****************************************************************************

void
SetActivePlotsAction::Execute()
{
    //
    // Set the active plots through the window manager.
    //
    const intVector &activePlots     = args.GetActivePlotIds();
    const intVector &activeOperators = args.GetActiveOperatorIds();
    const intVector &expandedPlots   = args.GetExpandedPlotIds();
    bool moreThanPlotsValid          = args.GetBoolFlag();

    bool applyToAllWindows = GetViewerState()->GetGlobalAttributes()->GetApplyWindow();
    
    if( applyToAllWindows )
    {
      for( int i=0; i<windowMgr->GetNumWindows(); ++i )
      {
        windowMgr->GetWindow(i)->GetPlotList()->
          SetActivePlots(activePlots,
                         activeOperators, expandedPlots, moreThanPlotsValid);
      }
    }
    else // Just the active window
    {
      window->GetPlotList()->
        SetActivePlots(activePlots,
                       activeOperators, expandedPlots, moreThanPlotsValid);
    }

    if (window->GetInteractionMode() == ZONE_PICK || 
        window->GetInteractionMode() == NODE_PICK ||
        window->GetInteractionMode() == DDT_PICK)
       ViewerQueryManager::Instance()->ActivePlotsChanged(); 
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ChangeActivePlotsVarAction::Execute
//
// Purpose: 
//   Changes the active plots' variables.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 07:52:13 PDT 2003
//
// Modifications:
//   
//   Hank Childs, Tue Jul 22 11:50:12 PDT 2008
//   Added explicit error handling.
//
// ****************************************************************************

void
ChangeActivePlotsVarAction::Execute()
{
    //
    // Set the plot variable for the selected plots.
    //
    TRY
    {
        const char *var = args.GetVariable().c_str();
        window->GetPlotList()->SetPlotVar(var);
    }
    CATCH2(InvalidExpressionException, ve)
    {
        GetViewerMessaging()->Error(
            TR("VisIt was unable to change the active variable "
               "because: %1\n").
            arg(ve.Message()));
    }
    ENDTRY
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetPlotSILRestrictionAction::Execute
//
// Purpose: 
//   Sets the SIL restriction for the selected plots.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 07:55:20 PDT 2003
//
// Modifications:
//  Gunther H. Weber, Fri Jan 18 17:28:52 PST 2008
//  Toggle setting SIL restriction to all plots independently from applying
//  operators to all plots. 
// ****************************************************************************

void
SetPlotSILRestrictionAction::Execute()
{
    //
    // Tell the plot list set the current SIL restriction for the
    // selected plots.
    //
    bool applyToAllWindows = GetViewerState()->GetGlobalAttributes()->GetApplyWindow();
    bool applySelectionToAllPlots = GetViewerState()->GetGlobalAttributes()->GetApplySelection();
    
    if( applyToAllWindows )
    {
      for( int i=0; i<windowMgr->GetNumWindows(); ++i )
      {
        windowMgr->GetWindow(i)->GetPlotList()->
          SetPlotSILRestriction(applySelectionToAllPlots);
      }
    }
    else // Just the active window
    {
      window->GetPlotList()->SetPlotSILRestriction(applySelectionToAllPlots);
    }
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetPlotOptionsAction::Execute
//
// Purpose: 
//   Sets the plot options for the selected plots.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 08:24:25 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
SetPlotOptionsAction::Execute()
{
    window->GetPlotList()->SetPlotAtts(args.GetPlotType());
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetPlotFrameRangeAction::Execute
//
// Purpose: 
//   Sets the frame range for the selected plots.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 08:27:45 PDT 2003
//
// Modifications:
//   Brad Whitlock, Mon Apr 5 14:43:45 PST 2004
//   Renamed some method calls.
//
// ****************************************************************************

void
SetPlotFrameRangeAction::Execute()
{
    //
    // Set the plot frame range.
    //
    int plotId = args.GetIntArg1();
    int index0 = args.GetIntArg2();
    int index1 = args.GetIntArg3();
    ViewerPlotList *plotList = window->GetPlotList();
    plotList->SetPlotRange(plotId, index0, index1);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: DeletePlotKeyframeAction::Execute
//
// Purpose: 
//   Deletes a plot keyframe.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 08:34:10 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
DeletePlotKeyframeAction::Execute()
{
    //
    // Delete the plot keyframe.
    //
    int plotId = args.GetIntArg1();
    int index = args.GetIntArg2();
    window->GetPlotList()->DeletePlotKeyframe(plotId, index);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: MovePlotKeyframeAction::Execute
//
// Purpose: 
//   Moves a plot keyframe.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 08:36:14 PDT 2003
//
// Modifications:
//   Brad Whitlock, Mon Apr 5 14:45:00 PST 2004
//   Renamed some variables.
//
// ****************************************************************************

void
MovePlotKeyframeAction::Execute()
{
    //
    // Move the plot keyframe.
    //
    int plotId = args.GetIntArg1();
    int oldIndex = args.GetIntArg2();
    int newIndex = args.GetIntArg3();
    ViewerPlotList *plotList = window->GetPlotList();
    plotList->MovePlotKeyframe(plotId, oldIndex, newIndex);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetPlotDatabaseStateAction::Execute
//
// Purpose: 
//   Sets a database keyframe for the plot.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 08:39:14 PDT 2003
//
// Modifications:
//   Brad Whitlock, Mon Apr 5 14:45:22 PST 2004
//   Renamed some variables.
//
// ****************************************************************************

void
SetPlotDatabaseStateAction::Execute()
{
    //
    // Perform the rpc.
    //
    int plotId = args.GetIntArg1();
    int index = args.GetIntArg2();
    int state = args.GetIntArg3();
    ViewerPlotList *plotList = window->GetPlotList();
    plotList->SetPlotDatabaseState(plotId, index, state);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: DeletePlotDatabaseKeyframeAction::Execute
//
// Purpose: 
//   Deletes a database keyframe.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 08:42:04 PDT 2003
//
// Modifications:
//   Brad Whitlock, Mon Apr 5 14:45:40 PST 2004
//   I renamed some variables.
//
// ****************************************************************************

void
DeletePlotDatabaseKeyframeAction::Execute()
{
    //
    // Delete the database keyframe
    //
    int plotId = args.GetIntArg1();
    int index = args.GetIntArg2();
    ViewerPlotList *plotList = window->GetPlotList();
    plotList->DeletePlotDatabaseKeyframe(plotId, index);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: MovePlotDatabaseKeyframeAction::Execute
//
// Purpose: 
//   Moves a database keyframe.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 08:45:24 PDT 2003
//
// Modifications:
//   Brad Whitlock, Mon Apr 5 14:46:15 PST 2004
//   I renamed some variables.
//
// ****************************************************************************

void
MovePlotDatabaseKeyframeAction::Execute()
{
    //
    // Move the database keyframe.
    //
    int plotId = args.GetIntArg1();
    int oldIndex = args.GetIntArg2();
    int newIndex = args.GetIntArg3();
    ViewerPlotList *plotList = window->GetPlotList();
    plotList->MovePlotDatabaseKeyframe(plotId, oldIndex, newIndex);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: CopyPlotAction::Execute
//
// Purpose: 
//   Copies the active plots.
//
// Programmer: Ellen Tarwater
// Creation:   Fri Sept 28 15:54:27 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
CopyPlotAction::Execute()
{
    window->GetPlotList()->CopyActivePlots();
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetPlotFollowsTimeAction::Execute
//
// Purpose: 
//   Disconnects the active plot from the time slider.
//
// Programmer: Ellen Tarwater
// Creation:   Thurs, Dec 6, 2007
//
// Modifications:
//   Brad Whitlock, Tue Mar 29 11:16:42 PDT 2011
//   Pass flag to SetPlotFollowsTime.
//
// ****************************************************************************

void
SetPlotFollowsTimeAction::Execute()
{
     window->GetPlotList()->SetPlotFollowsTime(args.GetBoolFlag());
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void
SetPlotDescriptionAction::Execute()
{
    window->GetPlotList()->SetPlotDescription(args.GetIntArg1(), 
        args.GetStringArg1());
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void
MovePlotOrderTowardFirstAction::Execute()
{
    window->GetPlotList()->MovePlotOrderTowardFirst(args.GetIntArg1());
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void
MovePlotOrderTowardLastAction::Execute()
{
    window->GetPlotList()->MovePlotOrderTowardLast(args.GetIntArg1());
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void
SetPlotOrderToFirstAction::Execute()
{
    window->GetPlotList()->SetPlotOrderToFirst(args.GetIntArg1());
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

void
SetPlotOrderToLastAction::Execute()
{
    window->GetPlotList()->SetPlotOrderToLast(args.GetIntArg1());
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: AddInitializedOperatorAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::AddInitializedOperatorRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
AddInitializedOperatorAction::Execute()
{
    //
    // Get the rpc arguments.
    //
    int type = args.GetOperatorType();

    OperatorPluginManager *opMgr = GetOperatorPluginManager();
    bool lineout = (opMgr->GetPluginName(opMgr->GetEnabledID(type))
                    == "Lineout");

    //
    // Perform the rpc.
    //
    if (!lineout)
    {
        bool applyToAll = GetViewerState()->GetGlobalAttributes()->GetApplyOperator();
        GetWindow()->GetPlotList()->AddOperator(type, applyToAll, false);
    }
    else
    {
        GetWindow()->Lineout(false);
    }
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ResetOperatorOptionsAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::ResetOperatorOptionsRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
ResetOperatorOptionsAction::Execute()
{
    //
    // Get the rpc arguments.
    //
    int oper = args.GetOperatorType();

    //
    // Update the client so it has the default attributes.
    //
    GetOperatorFactory()->SetClientAttsFromDefault(oper);

    //
    // Perform the rpc.
    //
    bool applyToAll = GetViewerState()->GetGlobalAttributes()->GetApplyOperator();
    GetWindow()->GetPlotList()->SetPlotOperatorAtts(oper, applyToAll);

}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ResetPlotOptionsAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::ResetPlotOptionsRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
ResetPlotOptionsAction::Execute()
{
    //
    // Get the rpc arguments.
    //
    int plot = args.GetPlotType();

    //
    // Update the client so it has the default attributes.
    //
    GetPlotFactory()->SetClientAttsFromDefault(plot);

    //
    // Perform the rpc.
    //
    GetWindow()->GetPlotList()->SetPlotAtts(plot);
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetDefaultOperatorOptionsAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SetDefaultOperatorOptionsRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
SetDefaultOperatorOptionsAction::Execute()
{
    //
    // Get the rpc arguments.
    //
    int type = args.GetOperatorType();

    //
    // Perform the rpc.
    //
    GetOperatorFactory()->SetDefaultAttsFromClient(type);
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetDefaultPlotOptionsAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SetDefaultPlotOptionsRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
SetDefaultPlotOptionsAction::Execute()
{
    //
    // Get the rpc arguments.
    //
    int type = args.GetPlotType();

    //
    // Perform the rpc.
    //
    GetPlotFactory()->SetDefaultAttsFromClient(type);
}
