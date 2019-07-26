// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <DDTActionsUI.h>

#include <ViewerActionLogic.h>
#include <ViewerPlotList.h>
#include <ViewerProperties.h>
#include <ViewerQueryManager.h>
#include <ViewerWindow.h>
#include <ViewerWindowManager.h>
#include <ViewerEngineManagerInterface.h>
#include <avtDatabaseMetaData.h>
#include <avtSimulationCommandSpecification.h>

#include <DDTManager.h>

#include <QIcon>
#include <QPixmap>

#include <releasetoddt.xpm>

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ReleaseToDDTActionUI::ReleaseToDDTActionUI
//
// Purpose:
//   Constructor for the 'release to DDT' Action
//
// Arguments:
//   win : The ViewerWindow this action applies to
//
// Programmer: Jonathan Byrd (Allinea Software)
// Creation:   December 18, 2011
//
// ****************************************************************************

ReleaseToDDTActionUI::ReleaseToDDTActionUI(ViewerActionLogic *L) :
    ViewerActionUISingle(L)
{
    SetAllText(tr("Release control of this simulation to DDT"));
    if (!GetViewerProperties()->GetNowin())
        SetIcon(QIcon(QPixmap(releasetoddt_xpm)));
}

// ****************************************************************************
// Method: ReleaseToDDTActionUI::Enabled
//
// Purpose:
//   Determines the enabled status for the 'release to DDT' Action
//
// Returns:
//   true if this Action should appear enabled
//
// Programmer: Jonathan Byrd (Allinea Software)
// Creation:   December 18, 2011
//
// ****************************************************************************

bool
ReleaseToDDTActionUI::Enabled() const
{
    // This action should only be enabled if the window to which the action belongs
    // is displaying a ddtsim-sourced simulation
    return (GetLogic()->GetWindow()->GetPlotList()->GetEngineKey().IsSimulation() &&
            DDTManager::isDatabaseDDTSim(GetLogic()->GetWindow()->GetPlotList()->GetDatabaseName()));
}

// ****************************************************************************
// Method: PlotDDTVispointVariablesActionUI::PlotDDTVispointVariablesActionUI
//
// Purpose:
//   Constructor for the 'plot vispoint variables' Action
//
// Arguments:
//   win : The ViewerWindow this action applies to
//
// Programmer: Jonathan Byrd (Allinea Software)
// Creation:   July 15, 2013
//
// ****************************************************************************

PlotDDTVispointVariablesActionUI::PlotDDTVispointVariablesActionUI(ViewerActionLogic *L) :
    ViewerActionUISingle(L)
{
    SetAllText(tr("Plot DDT vispoint variables"));
}

// ****************************************************************************
// Method: PlotDDTVispointVariablesActionUI::Enabled
//
// Purpose:
//   Determines the enabled status for the 'plot vispoint variables' Action
//
// Returns:
//   true if this Action should appear enabled
//
// Programmer: Jonathan Byrd (Allinea Software)
// Creation:   July 15, 2013
//
// ****************************************************************************

bool
PlotDDTVispointVariablesActionUI::Enabled() const
{
    const EngineKey &key = GetLogic()->GetWindow()->GetPlotList()->GetEngineKey();
    if (key.IsSimulation())
    {
        const avtDatabaseMetaData *md = 
            GetViewerEngineManager()->GetSimulationMetaData(key);

        // This action should only be enabled if the window to which the action belongs
        // is displaying a ddtsim-sourced simulation
       if (md && DDTManager::isDatabaseDDTSim(GetLogic()->GetWindow()->GetPlotList()->GetDatabaseName()))
       {
            // The ddtsim library in use must also support the "plot" command
            const int numCommands = md->GetSimInfo().GetNumGenericCommands();
            const std::string plotCmd = "plot";
            for (int i=0; i<numCommands; ++i)
                if (md->GetSimInfo().GetGenericCommands(i).GetName()==plotCmd)
                    return true;
       }
    }
    return false;
}
