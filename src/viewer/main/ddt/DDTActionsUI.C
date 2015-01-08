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
