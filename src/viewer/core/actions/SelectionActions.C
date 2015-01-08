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
#include <SelectionActions.h>

#include <EngineKey.h>
#include <ViewerEngineManagerInterface.h>
#include <ViewerFileServerInterface.h>
#include <ViewerMessaging.h>
#include <ViewerPlot.h>
#include <ViewerPlotList.h>
#include <ViewerState.h>
#include <ViewerStateManager.h>
#include <ViewerText.h>
#include <ViewerVariableMethods.h>
#include <ViewerWindow.h>
#include <ViewerWindowManager.h>

#include <DebugStream.h>
#include <avtDatabaseMetaData.h>
#include <AxisRestrictionAttributes.h>
#include <GlobalAttributes.h>
#include <SelectionList.h>
#include <SelectionProperties.h>
#include <SelectionSummary.h>

#include <snprintf.h>

//
// These methods were adapted from ViewerSubject handlers.
//

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SelectionActionBase::GetNamedSelectionEngineKey
//
// Purpose: 
//   Get the engine key of the plot associated with the named selection.
//
// Arguments:
//   selName : The name of the selection.
//   ek      : The return engine key.
//
// Returns:    True if the engine key was found. false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 11 11:45:08 PDT 2010
//
// Modifications:
//   Brad Whitlock, Thu Jun  9 11:20:44 PDT 2011
//   Allow for selections that are not associated with plots.
//
// ****************************************************************************

bool
SelectionActionBase::GetNamedSelectionEngineKey(const std::string &selName,
    EngineKey &ek)
{
    bool retval = false;

    int index = GetViewerState()->GetSelectionList()->GetSelection(selName);
    if(index != -1)
    {
        std::string source(GetViewerState()->GetSelectionList()->
              GetSelections(index).GetOriginatingPlot());

        // Look for the plot whose name is the same as the originating plot.
        // If we find a match, use the plot's engine key.
        std::vector<ViewerWindow *> windows = windowMgr->GetWindows();
        for(size_t i = 0; i < windows.size(); ++i)
        {
            ViewerPlotList *plist = windows[i]->GetPlotList();
            for(int j = 0; j < plist->GetNumPlots(); ++j)
            {
                ViewerPlot *plot = plist->GetPlot(j);
                if(plot->GetPlotName() == source)
                {
                    ek = plot->GetEngineKey();
                    return true;
                }
            }
        }

        // There was no plot with the selection's source name. Assume that
        // it is a database.
        std::string host, db, sim;
        GetViewerFileServer()->ExpandDatabaseName(source, host, db);
        const avtDatabaseMetaData *md = GetViewerFileServer()->GetMetaData(host, db);
        if (md != NULL)
        {
            if(md->GetIsSimulation())
                sim = db;
            ek = EngineKey(host, sim);
            retval = true;
        }
    }

    return retval;
}

// ****************************************************************************
// Method: SelectionActionBase::ReplaceNamedSelection
//
// Purpose: 
//   This code replaces the selection on all plots that use it with another
//   selection.
//
// Arguments:
//   engineKey  : The engine that defines the selection.
//   selName    : The selection to replace.
//   newSelName : The new selection.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 12 15:33:05 PDT 2010
//
// Modifications:
//   Brad Whitlock, Mon Aug 22 11:04:50 PDT 2011
//   I removed some code to associate selections with plots in the engine since
//   it is no longer necessary.
//
// ****************************************************************************

void
SelectionActionBase::ReplaceNamedSelection(const EngineKey &engineKey,
    const std::string &selName, const std::string &newSelName)
{
    // Replace the selection in all plots that use it.
    std::vector<ViewerWindow *> windows = windowMgr->GetWindows();
    bool *plotlistsChanged = new bool[windows.size()+1];
    for(size_t i = 0; i < windows.size(); ++i)
    {
        plotlistsChanged[i] = false;

        ViewerPlotList *plist = windows[i]->GetPlotList();
        for(int j = 0; j < plist->GetNumPlots(); ++j)
        {
            ViewerPlot *plot = plist->GetPlot(j);
            if(plot->GetNamedSelection() == selName)
            {
                plot->SetNamedSelection(newSelName);
                plot->ClearActors();

                plotlistsChanged[i] = true;
            }
        }
    }

    // Update the plot list in the client.
    GetWindow()->GetPlotList()->UpdatePlotList();

    TRY
    {   
        // Reexecute all of the affected plots.
        for(size_t i = 0; i < windows.size(); ++i)
        {
            if(plotlistsChanged[i])
                windows[i]->GetPlotList()->RealizePlots(false);
        }
    }
    CATCH(VisItException)
    {
        ;
    }
    ENDTRY

    delete [] plotlistsChanged;
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ApplyNamedSelectionAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::ApplyNamedSelectionRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
ApplyNamedSelectionAction::Execute()
{
    std::string selName = args.GetStringArg1();

    //
    // Get some information about the selection.
    //
    std::string originatingPlot;
    if(selName != "")
    {
        int selIndex = GetViewerState()->GetSelectionList()->GetSelection(selName);
        if(selIndex < 0)
        {
            GetViewerMessaging()->Error(
                TR("An invalid selection name was provided. No selection was applied."));
            return;
        }
        originatingPlot = GetViewerState()->GetSelectionList()->GetSelections(selIndex).GetOriginatingPlot();
    }

    // 
    // Get the indices of the plots to which the selection may be applied.
    //
    ViewerPlotList *plist = GetWindow()->GetPlotList();
    intVector plotIDs;
    if(GetViewerState()->GetGlobalAttributes()->GetApplySelection())
    {
        // If we're applying selection to all plots, get all plot ids.
        for(int i = 0; i < plist->GetNumPlots(); ++i)
            plotIDs.push_back(i);
    }
    else
        plist->GetActivePlotIDs(plotIDs, false);
    if (plotIDs.size() <= 0)
    {
        GetViewerMessaging()->Error(
            TR("To apply a named selection, you must have an active "
               "plot.  No named selection was applied."));
        return;
    }

    //
    // Make sure that all of the named selections being applied are for
    // the same engine as the first plot. Also exclude the plot if it
    // is the originating plot for a selection since we can't apply a
    // selection to the plot that generates it.
    //
    intVector ePlotIDs;
    ViewerPlot *plot0 = plist->GetPlot(plotIDs[0]);
    const EngineKey &engineKey = plot0->GetEngineKey();
    for (size_t i = 0 ; i < plotIDs.size() ; i++)
    {
        ViewerPlot *plot = plist->GetPlot(plotIDs[i]);
        if (plot->GetEngineKey() != engineKey)
        {
            GetViewerMessaging()->Error(
                TR("All plots involving a named selection must come from "
                   "the same engine.  No named selection was applied."));
            return;
        }
        else if(plot->GetPlotName() != originatingPlot)
        {
            ePlotIDs.push_back(plotIDs[i]);
        }
    }

    //
    // Apply the named selection.
    //
    TRY
    {
        for(size_t i = 0; i < ePlotIDs.size(); ++i)
        {
            ViewerPlot *plot = plist->GetPlot(ePlotIDs[i]);
            plot->SetNamedSelection(selName);
            plot->ClearActors();
        }
        plist->RealizePlots(false);
        plist->UpdatePlotList();

        if(!selName.empty())
            GetViewerMessaging()->Message(TR("Applied named selection"));
    }
    CATCH2(VisItException, e)
    {
        GetViewerMessaging()->Error(
            ViewerText("(%1): %2\n").
            arg(e.GetExceptionType()).
            arg(e.Message()));
    }
    ENDTRY
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: CreateNamedSelectionAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::CreateNamedSelectionRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
CreateNamedSelectionAction::Execute()
{
    const char *mName = "ViewerSubject::CreateNamedSelection: ";
    std::string selName = args.GetStringArg1();
    bool useCurrentPlot = args.GetBoolFlag();

    debug1 << mName << "0: selName=" << selName << endl;
    SelectionList *selList = GetViewerState()->GetSelectionList();
    SelectionProperties &currentProps = *GetViewerState()->GetSelectionProperties();

    // We'll fill in these properties to get the selection properties we send 
    // down to the engine to create the selection.
    int         networkId = -1;
    EngineKey   engineKey;
    std::string selSource;

    //
    // Look up some information from the originating plot
    //
    if(useCurrentPlot)
    {
        ViewerPlotList *plist = GetWindow()->GetPlotList();
        intVector plotIDs;
        plist->GetActivePlotIDs(plotIDs);
        if (plotIDs.size() <= 0)
        {
            GetViewerMessaging()->Error(
                TR("To create a named selection, you must have an active "
                   "plot that has been drawn.  No named selection was created."));
            return;
        }
        if (plotIDs.size() > 1)
        {
            GetViewerMessaging()->Error(
                TR("You can only have one active plot when creating a named "
                   "selection.  No named selection was created."));
            return;
        }
    
        ViewerPlot *plot = plist->GetPlot(plotIDs[0]);
        networkId = plot->GetNetworkID();
        engineKey = plot->GetEngineKey();
        selSource = plot->GetPlotName();
    }
    else
    {
        //
        // Turn the current selection source into a db and engine key.
        //
        std::string host, db, sim;
        GetViewerFileServer()->ExpandDatabaseName(currentProps.GetSource(), host, db);

        const avtDatabaseMetaData *md = GetViewerFileServer()->GetMetaData(host, db);
        if (md != NULL && md->GetIsSimulation())
            sim = db;

        engineKey = EngineKey(host, sim);
        selSource = db;

        // We're doing a selection based directly on the database. We need to
        // send the expression definitions to the engine since we haven't yet
        // created any plots.
        ExpressionList exprList;
        GetViewerStateManager()->GetVariableMethods()->GetAllExpressions(
            exprList, host, db, ViewerFileServerInterface::ANY_STATE);
        GetViewerEngineManager()->UpdateExpressions(engineKey, exprList);
    }

    TRY
    {
        SelectionProperties props;
        int index = -1;
        if(currentProps.GetName() == selName)
        {
            props = currentProps;
        }
        else
        {
            index = selList->GetSelection(selName);
            if(index >= 0)
            {
                // We found an existing definition in the list so use it.
                props = selList->GetSelections(index);
            }
            else
            {
                props.SetName(selName);
            }
        }

        // Set the source for the selection.
        props.SetSource(selSource);

        // Remove the summary if it is there.
        int sindex = GetViewerState()->GetSelectionList()->
                     GetSelectionSummary(props.GetName());
        if(sindex >= 0)
            GetViewerState()->GetSelectionList()->RemoveSelectionSummary(sindex);

        debug1 << mName << "1" << endl;

        SelectionSummary summary;
        if (GetViewerEngineManager()->CreateNamedSelection(engineKey, 
            networkId, props, summary))
        {
            GetViewerState()->GetSelectionList()->AddSelectionSummary(summary);
 
            GetViewerMessaging()->Message(TR("Created named selection"));
            debug1 << mName << "2" << endl;

            // Add a new selection to the selection list.
            if(index < 0)
                selList->AddSelections(props);

            debug1 << mName << "3" << endl;
            currentProps = props;
        }
        else
        {
            GetViewerMessaging()->Error(TR("Unable to create named selection."));
        }
    }
    CATCH2(VisItException, e)
    {
        GetViewerMessaging()->Error(
            ViewerText("(%1): %2\n").
            arg(e.GetExceptionType()).
            arg(e.Message()));
    }
    ENDTRY

    debug1 << mName << "4" << endl;

    // Send list of selections to the clients.
    selList->Notify();

    debug1 << mName << "5" << endl;
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: DeleteNamedSelectionAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::DeleteNamedSelectionRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
DeleteNamedSelectionAction::Execute()
{
    //
    // Get the rpc arguments.
    //
    std::string selName(args.GetStringArg1());

    //
    // Perform the RPC.
    //
    bool okay = false;
    EngineKey engineKey;

    TRY
    {
        // Actually delete the selection.
        okay = GetNamedSelectionEngineKey(selName, engineKey);
        if(okay)
        {
            // Remove the summary if it is there.
            int sindex = GetViewerState()->GetSelectionList()->GetSelectionSummary(selName);
            if(sindex >= 0)
                 GetViewerState()->GetSelectionList()->RemoveSelectionSummary(sindex);

            okay = GetViewerEngineManager()->DeleteNamedSelection(engineKey, selName);
        }
    }
    CATCH2(VisItException, e)
    {
        GetViewerMessaging()->Error(
            ViewerText("(%1): %2\n").
            arg(e.GetExceptionType()).
            arg(e.Message()));
    }
    ENDTRY

    // Make all plots that used the selection have no selection and make
    // them redraw.
    ReplaceNamedSelection(engineKey, selName, "");

    if(okay)
        GetViewerMessaging()->Message(TR("Deleted named selection"));
    else
        GetViewerMessaging()->Error(TR("Unable to delete named selection"));

    // Remove the selection from the selection list.
    int index = GetViewerState()->GetSelectionList()->GetSelection(selName);
    if(index != -1)
    {
        GetViewerState()->GetSelectionList()->RemoveSelections(index);
        GetViewerState()->GetSelectionList()->Notify();
    }
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: InitializeNamedSelectionVariablesAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::InitializeNamedSelectionVariablesRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
InitializeNamedSelectionVariablesAction::Execute()
{
    std::string selName(args.GetStringArg1());

    EngineKey engineKey;
    bool okay = GetNamedSelectionEngineKey(selName, engineKey);
    if(!okay)
        return;

    int selIndex = GetViewerState()->GetSelectionList()->GetSelection(selName);
    if(selIndex < 0)
        return;
    SelectionProperties &props = GetViewerState()->GetSelectionList()->
        GetSelections(selIndex);
    std::string originatingPlot = props.GetOriginatingPlot();

    bool notHandled = true;
    std::vector<ViewerWindow *> windows = windowMgr->GetWindows();
    for(size_t w = 0; w < windows.size() && notHandled; ++w)
    {
        ViewerPlotList *pL = windows[w]->GetPlotList();
        for(int i = 0; i < pL->GetNumPlots() && notHandled; ++i)
        {
            // We found the originating plot
            if(pL->GetPlot(i)->GetPlotName() == originatingPlot)
            {
                AttributeSubject *vr = pL->GetPlot(i)->GetPlotAtts()->
                    CreateCompatible("AxisRestrictionAttributes");
                AxisRestrictionAttributes *varRanges = 
                    dynamic_cast<AxisRestrictionAttributes *>(vr);
                if(varRanges != NULL)
                {
                    // Override the variables and ranges with the ones from the plot.
                    props.SetVariables(varRanges->GetNames());
                    props.SetVariableMins(varRanges->GetMinima());
                    props.SetVariableMaxs(varRanges->GetMaxima());

                    delete vr;

                    // Send out the new selection list
                    GetViewerState()->GetSelectionList()->SelectAll();
                    GetViewerState()->GetSelectionList()->Notify();
                }
                
                notHandled = false;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: LoadNamedSelectionAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::LoadNamedSelectionRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
LoadNamedSelectionAction::Execute()
{
    //
    // Get the rpc arguments.
    //
    std::string selName = args.GetStringArg1();
    const std::string &hostName = args.GetProgramHost();
    const std::string &simName  = args.GetProgramSim();

    EngineKey engineKey(hostName, simName);

    //
    // Perform the RPC.
    //
    TRY
    {
        if (GetViewerEngineManager()->LoadNamedSelection(engineKey, selName))
        {
            GetViewerMessaging()->Message(TR("Loaded named selection"));

            // Remove any selection that may already exist by this name.
            int index = GetViewerState()->GetSelectionList()->GetSelection(selName);
            if(index >= 0)
                GetViewerState()->GetSelectionList()->RemoveSelections(index);

            // Add a new selection to the selection list. Just set the name so
            // it will not have an originating plot.
            SelectionProperties props;
            props.SetName(selName);
            GetViewerState()->GetSelectionList()->AddSelections(props);
            GetViewerState()->GetSelectionList()->Notify();
        }
        else
        {
            GetViewerMessaging()->Error(TR("Unable to load named selection"));
        }
    }
    CATCH2(VisItException, e)
    {
        GetViewerMessaging()->Error(
            ViewerText("(%1): %2\n").
            arg(e.GetExceptionType()).
            arg(e.Message()));
    }
    ENDTRY
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SaveNamedSelectionAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SaveNamedSelectionRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
SaveNamedSelectionAction::Execute()
{
    //
    // Get the rpc arguments.
    //
    std::string selName = args.GetStringArg1();

    //
    // Perform the RPC.
    //
    TRY
    {
        EngineKey engineKey;
        bool okay = GetNamedSelectionEngineKey(selName, engineKey);

        if(okay)
            okay = GetViewerEngineManager()->SaveNamedSelection(engineKey, selName);

        if (okay)
        {
            GetViewerMessaging()->Message(TR("Saved named selection"));
        }
        else
        {
            GetViewerMessaging()->Error(TR("Unable to save named selection"));
        }
    }
    CATCH2(VisItException, e)
    {
        GetViewerMessaging()->Error(
            ViewerText("(%1): %2\n").
            arg(e.GetExceptionType()).
            arg(e.Message()));
    }
    ENDTRY
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetNamedSelectionAutoApplyAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SetNamedSelectionAutoApplyRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
SetNamedSelectionAutoApplyAction::Execute()
{
    GetViewerState()->GetSelectionList()->SetAutoApplyUpdates(args.GetBoolFlag());
    GetViewerState()->GetSelectionList()->Notify();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: UpdateNamedSelectionAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::UpdateNamedSelectionRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
UpdateNamedSelectionAction::Execute()
{
    std::string selName(args.GetStringArg1());

    // Poke the new selection properties into the selection list.
    if(args.GetBoolFlag())
    {
        int selIndex = GetViewerState()->GetSelectionList()->GetSelection(selName);
        if(selIndex < 0)
            return;
        SelectionProperties &props = GetViewerState()->GetSelectionList()->
            GetSelections(selIndex);
        props = *GetViewerState()->GetSelectionProperties();
    }

    bool updatePlots = (args.GetIntArg1() != 0);
    bool allowCache = (args.GetIntArg2() != 0);

    UpdateNamedSelection(selName, updatePlots, allowCache);
}

// ****************************************************************************
// Method: UpdateNamedSelectionAction::UpdateNamedSelection
//
// Purpose: 
//   Update the specified named selection.
//
// Arguments:
//   selName     : The name of the selection to update.
//   updatePlots : Whether to update plots.
//   allowCache  : Whether the NSM's intermediate data cache can be used.
//
// Returns:    
//
// Note:       This routine deletes the named selection, recreates it and
//             updates any plots that use it.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 13 13:59:06 PDT 2010
//
// Modifications:
//   Brad Whitlock, Tue Dec 14 11:47:12 PST 2010
//   Pass selection properties to the engine manager.
//
//   Brad Whitlock, Thu Jun  9 11:26:20 PDT 2011
//   Adjust to allow for selections that come from files.
//
//   Brad Whitlock, Fri Aug 19 12:33:03 PDT 2011
//   Send expressions to the engine to make sure that it has them.
//
//   Brad Whitlock, Wed Sep  7 14:42:21 PDT 2011
//   I added the allowCache argument.
//
// ****************************************************************************

void
UpdateNamedSelectionAction::UpdateNamedSelection(const std::string &selName,
    bool updatePlots, bool allowCache)
{
    EngineKey engineKey;
    bool okay = GetNamedSelectionEngineKey(selName, engineKey);
    if(!okay)
    {
        GetViewerMessaging()->Error(
            TR("VisIt could not determine the source or plot that creates "
               "the %1 selection.").
            arg(selName));
        return;
    }

    int selIndex = GetViewerState()->GetSelectionList()->GetSelection(selName);
    if(selIndex < 0)
    {
        GetViewerMessaging()->Error(
            TR("VisIt cannot update the %1 selection because it does "
               "not exist").
            arg(selName));
        return;
    }
    const SelectionProperties &props = GetViewerState()->GetSelectionList()->
        GetSelections(selIndex);
    std::string originatingPlot = props.GetOriginatingPlot();

    //
    // Get the network id of the originating plot.
    //
    int networkId = -1;
    std::vector<ViewerWindow *> windows = windowMgr->GetWindows();
    stringVector plotNames;
    for(size_t i = 0; i < windows.size() && networkId == -1; ++i)
    {
        ViewerPlotList *plist = windows[i]->GetPlotList();
        for(int j = 0; j < plist->GetNumPlots() && networkId == -1; ++j)
        {
            ViewerPlot *plot = plist->GetPlot(j);
            if(plot->GetPlotName() == originatingPlot)
                networkId = plot->GetNetworkID();
        }
    }

    // If we have a selection based on a database, send the expression list 
    // to the engine.
    if(networkId == -1)
    {
        ExpressionList exprList;
        std::string host, db, sim, src(props.GetSource());
        GetViewerFileServer()->ExpandDatabaseName(src, host, db);
        GetViewerStateManager()->GetVariableMethods()->GetAllExpressions(
            exprList, host, db, ViewerFileServerInterface::ANY_STATE);
        GetViewerEngineManager()->UpdateExpressions(engineKey, exprList);
    }

    // Remove the named selection summary.
    int sindex = GetViewerState()->GetSelectionList()->
                     GetSelectionSummary(props.GetName());
    if(sindex >= 0)
        GetViewerState()->GetSelectionList()->RemoveSelectionSummary(sindex);

    // Create the named selection again and reapply it to plots that use it.
    SelectionSummary summary;
    if(GetViewerEngineManager()->UpdateNamedSelection(
         engineKey, networkId, props, allowCache, summary))
    {
        // Add the new summary to the list.
        GetViewerState()->GetSelectionList()->AddSelectionSummary(summary);

        if(updatePlots)
            ReplaceNamedSelection(engineKey, selName, selName);
    }

    // Send list of selections to the clients so the selection summary is 
    // sent back.
    GetViewerState()->GetSelectionList()->Notify();
}
