// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
