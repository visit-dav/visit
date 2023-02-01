// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <IOActions.h>

#include <EngineKey.h>
#include <ViewerEngineManagerInterface.h>
#include <ViewerMessaging.h>
#include <ViewerPlot.h>
#include <ViewerPlotList.h>
#include <ViewerState.h>
#include <ViewerStateManager.h>
#include <ViewerText.h>
#include <ViewerWindow.h>
#include <ViewerWindowManager.h>

#include <ExportDBAttributes.h>
#include <avtColorTables.h>
#include <HostProfileList.h>
#include <MachineProfile.h>
#include <InstallationFunctions.h>
#include <SingleAttributeConfigManager.h>

//
// These methods were adapted from ViewerSubject handlers.
//

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ExportColorTableAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::ExportColorTableRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
ExportColorTableAction::Execute()
{
    //
    // Perform the rpc.
    //
    const std::string &ctName = args.GetColorTableName();
    std::string msg;
    if(avtColorTables::Instance()->ExportColorTable(ctName, msg))
    {
        // If we successfully exported the color table, msg is set to the
        // name of the color table file that was created. We want to send
        // a status message and a message.
        GetViewerMessaging()->Status(
            TR("Color table %1 exported to %2").
            arg(ctName).
            arg(msg));

        // Tell the user what happened.
        GetViewerMessaging()->Message(
            TR("VisIt exported color table \"%1\" to the file: %2. "
               "You can share that file with colleagues who want to use your "
               "color table. Simply put the file in their .visit directory, run "
               "VisIt and the color table will appear in their list of color "
               "tables when VisIt starts up.").
            arg(ctName).
            arg(msg));
    }
    else
    {
        GetViewerMessaging()->Error(msg);
    }
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ExportDBAction::GetActivePlotNetworkIds
//
// Purpose: 
//   Gets the network ids for the active plots.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 25 00:03:23 EDT 2014
//
// Modifications:
//
// ****************************************************************************

int
ExportDBAction::GetActivePlotNetworkIds(ViewerPlotList *plist, intVector &networkIds, EngineKey &key)
{
    intVector plotIDs;
    networkIds.clear();
    plist->GetActivePlotIDs(plotIDs);
    if (plotIDs.empty())
        return 1;
    for(size_t i = 0; i < plotIDs.size(); ++i)
    {
        const ViewerPlot *plot = plist->GetPlot(plotIDs[i]);
        const EngineKey &engineKey = plot->GetEngineKey();
        if(i == 0)
            key = engineKey;
        else
        {
            if(key != engineKey)
                return 2;
        }

        networkIds.push_back(plot->GetNetworkID());
    }
    return 0;
}

// ****************************************************************************
// Method: ExportDBAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::ExportDBRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   Kathleen Biagas, Fri Apr 23 2021
//   ExportDatabases has new signature with return atts.
//   Add location of export to message returned to user.
//
//   Cyrus Harrison, Mon Jun 13 12:08:19 PDT 2022
//   Fix how path is display when dirname is empty.
//
// ****************************************************************************

void
ExportDBAction::Execute()
{
    //
    // Get the network ids and check that they are all on the same engine.
    //
    ViewerPlotList *plist = GetWindow()->GetPlotList();
    intVector networkIds;
    EngineKey key;
    int err = GetActivePlotNetworkIds(plist, networkIds, key);
    if(err == 1)
    {
        GetViewerMessaging()->Error(
            TR("To export a database, you must have an active plot.  "
               "No database was exported."));
        return;
    }
    else if(err == 2)
    {
        GetViewerMessaging()->Error(
            TR("To export the database for multiple plots, all plots "
               "must be processed by the same compute engine."));
        return;
    }

    TRY
    {
        ExportDBAttributes exportAtts(*GetViewerState()->GetExportDBAttributes());

        // Do export of all time states.
        if(!key.IsSimulation() && exportAtts.GetAllTimes() && plist->HasActiveTimeSlider())
        {
            // Get the time slider information.
            int state = 0, nStates = 0;
            plist->GetTimeSliderStates(plist->GetActiveTimeSlider(), state, nStates);

            // Iterate through time and export the current time state.
            char digits[128];
            std::string filename(exportAtts.GetFilename());
            int status = 0;
            for(int i = 0; i < nStates && status == 0; ++i)
            {
                // Make a new filename for the exported file.
                // if(nStates >= 10000)
                //     snprintf(digits, 10, "%08d", i);
                // else
                //     snprintf(digits, 10, "%04d", i);
                snprintf(digits, 10, exportAtts.GetTimeStateFormat().c_str(), i);
              
                std::string timeSuffix(digits);

                TRY
                {
                    plist->SetTimeSliderState(i);
                    int err = GetActivePlotNetworkIds(plist, networkIds, key);
                    if(err == 0)
                    {
                        // If we're trying to export to a simulation but the data is not from
                        // a simulation then issue an error message.
                        if((GetViewerState()->GetExportDBAttributes()->GetDb_type() == "SimV1" || 
                            GetViewerState()->GetExportDBAttributes()->GetDb_type() == "SimV2") &&
                            !key.IsSimulation())
                        {
                            GetViewerMessaging()->Error(
                                TR("VisIt can only export data to a simulation if the "
                                   "data being exported originated in a simulation."));
                            return;
                        }
                        else if(GetViewerEngineManager()->ExportDatabases(
                                key, networkIds, &exportAtts,
                                timeSuffix, exportAtts))
                        {
                            std::string host = key.OriginalHostName();
                            if (host == "localhost")
                                host = "";
                            else
                                host += ":";
                            std::string path = exportAtts.GetDirname();
                            // don't add '/' if dir name is empty,
                            // b/c it will make it look like we wrote
                            // to the root file system
                            if(!path.empty())
                              path += "/";
                            GetViewerMessaging()->Message(
                                TR("Exported database time state %1 to %2%3%4").
                                arg(i).
                                arg(host).
                                arg(path).
                                arg(exportAtts.GetFilename()));
                        }
                        else
                            status = 1;
                    }
                    else
                        status = 1;

                    if(status == 1)
                    {
                        GetViewerMessaging()->Error(
                            TR("Unable to export database time "
                               "state %1. Stopping export.").arg(i));
                    }
                }
                CATCH2(VisItException,e)
                {
                    GetViewerMessaging()->Error(
                        TR("An unexpected error prevented export of "
                           "database time state %1. Stopping export. %2").
                        arg(i).
                        arg(e.Message()));
                    status = 2;
                }
                ENDTRY
            }

            // Go back to the original state.
            plist->SetTimeSliderState(state);
        }
        // Do export of current time state.
        else if (GetViewerEngineManager()->ExportDatabases(key,
                 networkIds, &exportAtts, "", exportAtts))
        {
            std::string host = key.OriginalHostName();
            if (host == "localhost")
                host = "";
            else
                host += ":";
            std::string path = exportAtts.GetDirname();
            // don't add '/' if dir name is empty,
            // b/c it will make it look like we wrote
            // to the root file system
            if(!path.empty())
              path += "/";
            GetViewerMessaging()->Message(
                TR("Exported database to %1%2%3").
                arg(host).
                arg(path).
                arg(exportAtts.GetFilename()));
        }
        else
        {
            GetViewerMessaging()->Error(TR("Unable to export database"));
        }
    }
    CATCH2(VisItException, e)
    {
        GetViewerMessaging()->Error(
            TR("Unable to export database: %1").
            arg(e.Message()));
    }
    ENDTRY
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ExportEntireStateAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::ExportEntireStateRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//    David Camp, Thu Aug 27 09:40:00 PDT 2015
//    Added hostname to passed arguments.
//   
// ****************************************************************************

void
ExportEntireStateAction::Execute()
{
    GetViewerStateManager()->SaveSession(args.GetStringArg1(),
                                         args.GetVariable());
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ExportHostProfileAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::ExportHostProfileRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun  3 16:27:38 PDT 2016
//
// Modifications:
//   
// ****************************************************************************

void
ExportHostProfileAction::Execute()
{
    std::string profileName(args.GetStringArg1());
    std::string fileName(args.GetStringArg2());
    bool saveInUserDir(args.GetBoolFlag());

    std::string userdir = GetAndMakeUserVisItHostsDirectory();
    HostProfileList *hpl = GetViewerState()->GetHostProfileList();

    for (int i = 0; i < hpl->GetNumMachines(); ++i)
    {
        MachineProfile &pl = hpl->GetMachines(i);
        std::string host = pl.GetHostNickname();

        if(host != profileName) continue;

        std::string name = "";

        if(!saveInUserDir)
            name = fileName;
        else
            name = userdir + VISIT_SLASH_STRING + fileName;

        GetViewerMessaging()->Status(
            TR("Host profile %1 exported to %2").
               arg(host).
               arg(name));

        // Tell the user what happened.
        GetViewerMessaging()->Message(
            TR("VisIt exported host profile \"%1\" to the file: %2. ").
               arg(host).
               arg(name));

        SingleAttributeConfigManager mgr(&pl);
        mgr.Export(name);
        break;
    }
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ImportEntireStateAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::ImportEntireStateRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//    David Camp, Thu Aug 27 09:40:00 PDT 2015
//    Added hostname to argument list.
//   
// ****************************************************************************

void
ImportEntireStateAction::Execute()
{
    stringVector empty;
    GetViewerStateManager()->RestoreSession(args.GetStringArg1(),
                                            args.GetVariable(),
                                            args.GetBoolFlag(),
                                            empty);
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ImportEntireStateWithDifferentSourcesAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::ImportEntireStateWithDifferentSourcesRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//    David Camp, Thu Aug 27 09:40:00 PDT 2015
//    Added hostname to argument list.
//
//    Brad Whitlock, Tue Jul 17 17:06:38 PDT 2018
//    Add support for passing in session file contents.
//
// ****************************************************************************

void
ImportEntireStateWithDifferentSourcesAction::Execute()
{
    if(args.GetIntArg1() == 1)
    {
        // We have passed in the contents of the session file.
        GetViewerStateManager()->RestoreSession(std::string("memory"),
                                                args.GetVariable(),
                                                args.GetProgramOptions());
    }
    else
    {
        // We need to actually read the session file.
        GetViewerStateManager()->RestoreSession(args.GetStringArg1(),
                                                args.GetVariable(),
                                                args.GetBoolFlag(),
                                                args.GetProgramOptions());
    }
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SaveWindowAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SaveWindowRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
SaveWindowAction::Execute()
{
    windowMgr->SaveWindow();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: WriteConfigFileAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::WriteConfigFileRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
WriteConfigFileAction::Execute()
{
    GetViewerStateManager()->WriteConfigFile();
    GetViewerStateManager()->WriteHostProfiles();
}

