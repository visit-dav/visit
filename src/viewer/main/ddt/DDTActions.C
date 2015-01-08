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
#include <DDTActions.h>

#ifdef HAVE_DDT
#include <DDTManager.h>
#include <DDTSession.h>
#endif

#include <ViewerEngineManagerInterface.h>
#include <ViewerMessaging.h>
#include <ViewerText.h>
#include <ViewerWindow.h>
#include <ViewerPlotList.h>

#include <avtDatabaseMetaData.h>
#include <avtSimulationCommandSpecification.h>

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: DDTConnectAction::Execute
//
// Purpose:
//   Connects/disconnects the viewer with DDT
//
// Programmer: Jonathan Byrd (Allinea Software)
// Creation:   December 18, 2011
//
// ****************************************************************************

void
DDTConnectAction::Execute()
{
#ifdef HAVE_DDT
    DDTManager* manager = DDTManager::getInstance();
    DDTSession* ddt = manager->getSessionNC();
    if (ddt!=NULL && ddt->connected())
        manager->disconnect();
    else if (ddt==NULL)
        ddt = manager->makeConnection();
#endif
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: DDTFocusAction::Execute
//
// Purpose:
//   Instructs DDT to focus on a specific domain & element
//
// Programmer: Jonathan Byrd (Allinea Software)
// Creation:   December 18, 2011
//
// ****************************************************************************

void
DDTFocusAction::Execute()
{
#ifdef HAVE_DDT
    const int domain  = args.GetIntArg1();
    const int element = args.GetIntArg2();
    const std::string variable = args.GetStringArg1();
    const std::string value    = args.GetStringArg2();

    DDTSession* ddt = DDTManager::getInstance()->getSession();
    if (ddt!=NULL)
        ddt->setFocusOnElement(domain, variable, element, value);
    else
    {
        GetViewerMessaging()->Error(
           TR("Cannot focus DDT on domain %0, element %1 of %2:: failed "
              "to connect to DDT").
           arg(domain).
           arg(element).
           arg(variable));
    }
#endif
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ReleaseToDDTAction::Execute
//
// Purpose:
//   Performs the 'release to DDT' Action
//
// Programmer: Jonathan Byrd (Allinea Software)
// Creation:   December 18, 2011
//
// ****************************************************************************

void
ReleaseToDDTAction::Execute()
{
#ifdef HAVE_DDT
    if (DDTManager::isDatabaseDDTSim(GetWindow()->GetPlotList()->GetDatabaseName()))
    {
        const EngineKey &key = GetWindow()->GetPlotList()->GetEngineKey();
        if (key.IsSimulation())
            GetViewerEngineManager()->SendSimulationCommand(key, "DDT", "");
    }
#endif
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: PlotDDTVispointVariablesAction::Execute
//
// Purpose:
//   Performs the 'plot vispoint variables' Action
//
// Programmer: Jonathan Byrd (Allinea Software)
// Creation:   July 15, 2013
//
// ****************************************************************************

void
PlotDDTVispointVariablesAction::Execute()
{
#ifdef HAVE_DDT
    if (DDTManager::isDatabaseDDTSim(GetWindow()->GetPlotList()->GetDatabaseName()))
    {
        const EngineKey &key = GetWindow()->GetPlotList()->GetEngineKey();
        if (key.IsSimulation())
            GetViewerEngineManager()->SendSimulationCommand(key, "plot", "force");
    }
#endif
}

///////////////////////////////////////////////////////////////////////////////

void
AnimationStopActionDDT::Execute()
{
#ifdef HAVE_DDT
    if (DDTManager::isDDTSim(window))
    {
        const EngineKey &key = window->GetPlotList()->GetEngineKey();
        GetViewerEngineManager()->SendSimulationCommand(key, DDTSIM_CMD_STOP, "");
    }
    else
#endif
        AnimationStopAction::Execute();
}

///////////////////////////////////////////////////////////////////////////////

void
AnimationPlayActionDDT::Execute()
{
#ifdef HAVE_DDT
    if (DDTManager::isDDTSim(window))
    {
        const EngineKey &key = window->GetPlotList()->GetEngineKey();
        const avtDatabaseMetaData *md = GetViewerEngineManager()->GetSimulationMetaData(key);
        if (md->GetSimInfo().GetMode()==avtSimulationInformation::Running)
            GetViewerEngineManager()->SendSimulationCommand(key, DDTSIM_CMD_STOP, "");
        else
            GetViewerEngineManager()->SendSimulationCommand(key, DDTSIM_CMD_PLAY, "");
    }
    else
#endif
        AnimationPlayAction::Execute();
}

///////////////////////////////////////////////////////////////////////////////

void
TimeSliderForwardStepActionDDT::Execute()
{
#ifdef HAVE_DDT
     if (DDTManager::isDDTSim(window))
     {
         GetViewerEngineManager()->SendSimulationCommand(
             window->GetPlotList()->GetEngineKey(), DDTSIM_CMD_STEP, "");
     }
     else
#endif
         TimeSliderForwardStepAction::Execute();
}
