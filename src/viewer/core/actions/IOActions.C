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
            char digits[10];
            std::string filename(exportAtts.GetFilename());
            int status = 0;
            for(int i = 0; i < nStates && status == 0; ++i)
            {
                // Make a new filename for the exported file.
                if(nStates >= 10000)
                    SNPRINTF(digits, 10, "%08d", i);
                else
                    SNPRINTF(digits, 10, "%04d", i);
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
                                key, networkIds, exportAtts, timeSuffix))
                        {
                            GetViewerMessaging()->Message(
                                TR("Exported database time state %1").
                                arg(i));
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
        else if (GetViewerEngineManager()->ExportDatabases(key, networkIds, exportAtts, ""))
        {
            GetViewerMessaging()->Message(TR("Exported database"));
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
//   
// ****************************************************************************

void
ExportEntireStateAction::Execute()
{
    GetViewerStateManager()->SaveSession(args.GetVariable());
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
//   
// ****************************************************************************

void
ImportEntireStateAction::Execute()
{
    stringVector empty;
    GetViewerStateManager()->RestoreSession(args.GetVariable(),
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
//   
// ****************************************************************************

void
ImportEntireStateWithDifferentSourcesAction::Execute()
{
    GetViewerStateManager()->RestoreSession(args.GetVariable(),
                                            args.GetBoolFlag(),
                                            args.GetProgramOptions());
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
