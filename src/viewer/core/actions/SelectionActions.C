// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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


//
// These methods were adapted from ViewerSubject handlers.
//

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
                                                EngineKey &engineKey)
{
    int selIndex = GetViewerState()->GetSelectionList()->GetSelection(selName);
    if(selIndex < 0)
    {
        GetViewerMessaging()->Error(
            TR("VisIt cannot get an engine key for %1 named selection because "
               "it does not exist").arg(selName));
        return false;
    }

    // Look for the plot whose name is the same as the originating plot.
    // If we find a match, use the plot's engine key.
    std::vector<ViewerWindow *> windows = windowMgr->GetWindows();
    
    std::string selSource(GetViewerState()->GetSelectionList()->
                          GetSelections(selIndex).GetOriginatingPlot());

    for(size_t i = 0; i < windows.size(); ++i)
    {
        ViewerPlotList *plist = windows[i]->GetPlotList();
        for(int j = 0; j < plist->GetNumPlots(); ++j)
        {
            ViewerPlot *plot = plist->GetPlot(j);
            if(plot->GetPlotName() == selSource)
            {
                engineKey = plot->GetEngineKey();
                return true;
            }
        }
    }

    // There was no plot with the selection's source name. Assume
    // that it is a database.
    std::string host(GetViewerState()->GetSelectionList()->
                     GetSelections(selIndex).GetHost());
    
    std::string db(GetViewerState()->GetSelectionList()->
                   GetSelections(selIndex).GetSource());
    
    const avtDatabaseMetaData *md =
      GetViewerFileServer()->GetMetaData(host, db);
    
    std::string sim;
    if (md != NULL && md->GetIsSimulation())
        sim = db;
    
    engineKey = EngineKey(host, sim);

    return true;
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

    if( selName.empty() )
    {
      ClearNamedSelection();
      return;
    }
    
    EngineKey engineKey;
    bool okay = GetNamedSelectionEngineKey(selName, engineKey);
    if(!okay)
    {
        GetViewerMessaging()->Error(
            TR("VisIt could not determine the source or plot that creates "
               "the %1 named selection.").arg(selName));
        return;
    }

    int selIndex = GetViewerState()->GetSelectionList()->GetSelection(selName);
        
    if(selIndex < 0)
    {
        GetViewerMessaging()->Error(
            TR("An invalid named selection %1 was provided to VisIt "
               "No named selection was applied. ").arg(selName));
        return;
    }

    // Get some information about the selection.
    std::string originatingPlot = GetViewerState()->GetSelectionList()->
      GetSelections(selIndex).GetOriginatingPlot();

    // Get the indices of the plots to which the selection may be applied.
    ViewerPlotList *plist = GetWindow()->GetPlotList();
    intVector plotIDs;

    if(GetViewerState()->GetGlobalAttributes()->GetApplySelection())
    {
        // If applying the selection to all plots, get all plot ids.
        for(int i = 0; i < plist->GetNumPlots(); ++i)
            plotIDs.push_back(i);
    }
    else
        plist->GetActivePlotIDs(plotIDs, false);

    if (plotIDs.size() <= 0)
    {
        GetViewerMessaging()->Error(
            TR("To apply a named selection %1, you must have an active "
               "plot.  No named selection was applied.").arg(selName));
        return;
    }

    // Make sure that the named selection being applied being applied
    // is one the same engine as the plots. Also exclude the plot if
    // it is the originating plot for a selection as it can not be
    // applied to itself.
    intVector ePlotIDs;
    
    for (size_t i = 0 ; i < plotIDs.size() ; ++i)
    {
        ViewerPlot *plot = plist->GetPlot(plotIDs[i]);
        
        if (plot->GetEngineKey() != engineKey)
        {
            GetViewerMessaging()->Error(
                TR("Named selections are engine specific. The plot %1 on "
                   "the engine %2 involving named selection %3 must be "
                   "on the engine %4.  No named selection was applied.")
                .arg(plot->GetPlotDescription())
                .arg(plot->GetHostName())
                .arg(selName)
                .arg(engineKey.HostName()));
            return;
        }
        else if(plot->GetPlotName() == originatingPlot)
        {
            GetViewerMessaging()->Error(
                TR("Can not apply named selection %1 to the plot %2 because "
                   "the named selection originates from it. No named"
                   "selection was applied.").arg(selName).arg(originatingPlot));
            return;
        }
        else
            ePlotIDs.push_back(plotIDs[i]);
    }

    // Apply the named selection.
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
            GetViewerMessaging()->Message(TR("Applied named selection %1.")
                                          .arg(selName));
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


// ****************************************************************************
// Method: ApplyNamedSelectionAction::ClearNamedSelection
//
// Purpose: 
//   Clears a named selection
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
ApplyNamedSelectionAction::ClearNamedSelection()
{
    // Get the indices of the plots to which the selection may be applied.
    ViewerPlotList *plist = GetWindow()->GetPlotList();
    intVector plotIDs;

    if(GetViewerState()->GetGlobalAttributes()->GetApplySelection())
    {
        // If applying the clear selection to all plots, get all plot ids.
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

    // Clear the named selection.
    TRY
    {
        for(size_t i = 0; i < plotIDs.size(); ++i)
        {
            ViewerPlot *plot = plist->GetPlot(plotIDs[i]);
            plot->SetNamedSelection("");
            plot->ClearActors();
        }
        plist->RealizePlots(false);
        plist->UpdatePlotList();

        GetViewerMessaging()->Message(TR("Cleared named selection."));
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
    SelectionProperties &currentProps =
      *GetViewerState()->GetSelectionProperties();

    // We'll fill in these properties to get the selection properties we send 
    // down to the engine to create the selection.
    int         networkId = -1;
    EngineKey   engineKey;
    std::string selHost;
    std::string selSource;

    // Look up some information from the originating plot
    if(useCurrentPlot)
    {
        ViewerPlotList *plist = GetWindow()->GetPlotList();
        intVector plotIDs;
        plist->GetActivePlotIDs(plotIDs);
        if (plotIDs.size() <= 0)
        {
            GetViewerMessaging()->Error(
                TR("To create a named selection, you must have an active plot "
                   "that has been drawn.  No named selection was created."));
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
        selHost   = plot->GetHostName();
        selSource = plot->GetPlotName();
    }
    else
    {
        // Turn the current selection source into a host and a db.
        std::string host, db;
        GetViewerFileServer()->ExpandDatabaseName(currentProps.GetSource(),
                                                  host, db);

        const avtDatabaseMetaData *md =
          GetViewerFileServer()->GetMetaData(host, db);
        
        std::string sim;
        if (md != NULL && md->GetIsSimulation())
            sim = db;
        
        // Create an engine key for the expressions.
        engineKey = EngineKey(host, sim);

        // Doing a selection based directly on the database so send
        // the expression definitions to the engine because there is
        // no associated plot.
        ExpressionList exprList;
        GetViewerStateManager()->GetVariableMethods()->GetAllExpressions(
            exprList, host, db, ViewerFileServerInterface::ANY_STATE);
        GetViewerEngineManager()->UpdateExpressions(engineKey, exprList);

        // Note: The source must be just the database name sans the
        // host name.
        selHost = host;
        selSource = db;
    }

    TRY
    {
        SelectionProperties props;
        int selIndex = -1;
        if(currentProps.GetName() == selName)
        {
            props = currentProps;
        }
        else
        {
            selIndex = selList->GetSelection(selName);

            if(selIndex >= 0)
            {
                // Found an existing definition in the list so use it.
                props = selList->GetSelections(selIndex);
            }
            else
            {
                props.SetName(selName);
            }
        }

        // Set the host and source for the selection.
        props.SetHost  (selHost);
        props.SetSource(selSource);

        // Remove the summary if it is there.
        int sumIndex = GetViewerState()->GetSelectionList()->
            GetSelectionSummary(props.GetName());
        
        if(sumIndex >= 0)
            GetViewerState()->GetSelectionList()->
              RemoveSelectionSummary(sumIndex);

        SelectionSummary summary;
        if (GetViewerEngineManager()->CreateNamedSelection(engineKey, 
            networkId, props, summary))
        {
            GetViewerState()->GetSelectionList()->AddSelectionSummary(summary);
 
            // Add a new selection to the selection list.
            if(selIndex < 0)
                selList->AddSelections(props);

            GetViewerMessaging()->Message(TR("Created named selection")
                                        .arg(selName));
        }
        else
        {
            GetViewerMessaging()->Error(TR("Unable to create named selection %1.")
                                        .arg(selName));
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

    // Send list of selections to the clients.
    selList->Notify();
}


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
    // Get the RPC arguments.
    std::string selName = args.GetStringArg1();
    const std::string &hostName = args.GetProgramHost();
    const std::string &simName  = args.GetProgramSim();

    EngineKey engineKey(hostName, simName);

    // Perform the RPC.
    TRY
    {
        if (GetViewerEngineManager()->LoadNamedSelection(engineKey, selName))
        {
            // Remove any selection that may already exist by this name.
            int selIndex =
              GetViewerState()->GetSelectionList()->GetSelection(selName);
            
            if(selIndex >= 0)
                GetViewerState()->GetSelectionList()->
                  RemoveSelections(selIndex);

            // Add a new selection to the selection list. Just set the
            // name so it will not have an originating plot.
            SelectionProperties props;
            props.SetName(selName);
            GetViewerState()->GetSelectionList()->AddSelections(props);
            GetViewerState()->GetSelectionList()->Notify();

            GetViewerMessaging()->Message(TR("Loaded named selection %")
                                          .arg(selName));
        }
        else
        {
            GetViewerMessaging()->Error(TR("Unable to load named selection %1.")
                                        .arg(selName));
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
    // Get the RPC arguments.
    std::string selName = args.GetStringArg1();

    EngineKey engineKey;
    bool okay = GetNamedSelectionEngineKey(selName, engineKey);
    if(!okay)
    {
        GetViewerMessaging()->Error(
            TR("VisIt could not determine the source or plot that creates "
               "the named selection %1.").arg(selName));
        return;
    }
    
    // Perform the RPC.
    TRY
    {
        okay = GetViewerEngineManager()->SaveNamedSelection(engineKey,
                                                            selName);

        if (okay)
            GetViewerMessaging()->Message(TR("Saved named selection %1.")
                                          .arg(selName));
        else
          GetViewerMessaging()->Error(TR("Unable to save named selection %1.")
                                      .arg(selName));
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
    // Get the RPC arguments.
    std::string selName(args.GetStringArg1());

    // Perform the RPC.
    EngineKey engineKey;
    bool okay = GetNamedSelectionEngineKey(selName, engineKey);
    if(!okay)
    {
        GetViewerMessaging()->Error(
            TR("VisIt could not determine the source or plot that creates "
               "the %1 selection.").arg(selName));
        return;
    }

    TRY
    {
        // Remove the summary if it is there.
        int sumIndex = GetViewerState()->GetSelectionList()->
            GetSelectionSummary(selName);
            
        if(sumIndex >= 0)
            GetViewerState()->GetSelectionList()->
              RemoveSelectionSummary(sumIndex);

        // Delete the selection.
        okay = GetViewerEngineManager()->DeleteNamedSelection(engineKey,
                                                              selName);

        if (okay)
            GetViewerMessaging()->Message(TR("Deleted named selection %1.")
                                          .arg(selName));

        else
            GetViewerMessaging()->Error(TR("Unable to delete named selection %1.")
                                        .arg(selName));
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

    // Remove the selection from the selection list.
    int selIndex = GetViewerState()->GetSelectionList()->GetSelection(selName);

    if(selIndex != -1)
    {
        GetViewerState()->GetSelectionList()->RemoveSelections(selIndex);
        GetViewerState()->GetSelectionList()->Notify();
    }
}


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
    {
        GetViewerMessaging()->Error(
            TR("VisIt could not determine the source or plot that creates "
               "the %1 selection.").arg(selName));
        return;
    }
    
    int selIndex = GetViewerState()->GetSelectionList()->GetSelection(selName);
    if(selIndex < 0)
    {
        GetViewerMessaging()->Error(
            TR("VisIt cannot update the named selection %1 because it does "
               "not exist").arg(selName));
        return;
    }

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
            // Found the originating plot
            if(pL->GetPlot(i)->GetPlotName() == originatingPlot)
            {
                AttributeSubject *vr = pL->GetPlot(i)->GetPlotAtts()->
                    CreateCompatible("AxisRestrictionAttributes");
                AxisRestrictionAttributes *varRanges = 
                    dynamic_cast<AxisRestrictionAttributes *>(vr);
                if(varRanges != NULL)
                {
                    // Override the variables and ranges with the ones
                    // from the plot.
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
        int selIndex =
            GetViewerState()->GetSelectionList()->GetSelection(selName);
        
        if(selIndex < 0)
        {
          GetViewerMessaging()->Error(
              TR("VisIt cannot update the named selection %1 because it does "
                 "not exist").arg(selName));
          return;
        }

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
                                                 bool updatePlots,
                                                 bool allowCache)
{
    EngineKey engineKey;
    bool okay = GetNamedSelectionEngineKey(selName, engineKey);
    if(!okay)
    {
        GetViewerMessaging()->Error(
            TR("VisIt could not determine the source or plot that creates "
               "the %1 selection.").arg(selName));
        return;
    }

    int selIndex = GetViewerState()->GetSelectionList()->GetSelection(selName);
    if(selIndex < 0)
    {
        GetViewerMessaging()->Error(
            TR("VisIt cannot update the named selection %1 because it does "
               "not exist").arg(selName));
        return;
    }

    const SelectionProperties &props = GetViewerState()->GetSelectionList()->
        GetSelections(selIndex);
    std::string originatingPlot = props.GetOriginatingPlot();

    // Get the network id of the originating plot.
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
        std::string host(props.GetHost());
        std::string db(props.GetSource());

        // Doing a selection based directly on the database so send
        // the expression definitions to the engine because there is
        // no associated plot.
        ExpressionList exprList;
        GetViewerStateManager()->GetVariableMethods()->GetAllExpressions(
            exprList, host, db, ViewerFileServerInterface::ANY_STATE);
        GetViewerEngineManager()->UpdateExpressions(engineKey, exprList);
    }

    // Remove the named selection summary.
    int sumIndex = GetViewerState()->GetSelectionList()->
                     GetSelectionSummary(props.GetName());

    if(sumIndex >= 0)
        GetViewerState()->GetSelectionList()->RemoveSelectionSummary(sumIndex);

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
