// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <AnimationActionsUI.h>
#include <ViewerActionLogic.h>
#include <ViewerFileServer.h>
#include <ViewerPlotList.h>
#include <ViewerProperties.h>
#include <ViewerWindow.h>
#include <ViewerWindowManager.h>
#include <ViewerEngineManagerInterface.h>
#include <avtSimulationInformation.h>
#include <avtDatabaseMetaData.h>

#ifdef HAVE_DDT
#include <DDTManager.h>
#endif

#include <DatabaseCorrelationList.h>
#include <DebugStream.h>

#include <QIcon>
#include <QPixmap>

// Include icons
#include <animationreversestep.xpm>
#include <animationreverseplayon.xpm>
#include <animationreverseplayoff.xpm>
#include <animationstopon.xpm>
#include <animationstopoff.xpm>
#include <animationplayon.xpm>
#include <animationplayoff.xpm>
#include <animationforwardstep.xpm>

///////////////////////////////////////////////////////////////////////////////

TimeSliderReverseStepActionUI::TimeSliderReverseStepActionUI(ViewerActionLogic *L) :
    ViewerActionUISingle(L)
{
    SetAllText(tr("Reverse step"));
    SetToolTip(tr("Step back one frame"));
    if (!GetViewerProperties()->GetNowin())
        SetIcon(QIcon(QPixmap(animationreversestep_xpm)));
}

bool
TimeSliderReverseStepActionUI::Enabled() const
{
    return GetLogic()->GetWindow()->GetPlotList()->HasActiveTimeSlider() &&
           GetLogic()->GetWindow()->GetPlotList()->GetNumPlots() > 0;
}

///////////////////////////////////////////////////////////////////////////////

AnimationReversePlayActionUI::AnimationReversePlayActionUI(ViewerActionLogic *L) :
    ViewerActionUIToggle(L)
{
    SetAllText(tr("Reverse play"));
    SetToolTip(tr("Play animation in reverse"));
    if (!GetViewerProperties()->GetNowin())
        SetIcons(QPixmap(animationreverseplayon_xpm), QPixmap(animationreverseplayoff_xpm));
}

bool
AnimationReversePlayActionUI::Enabled() const
{
    return GetLogic()->GetWindow()->GetPlotList()->HasActiveTimeSlider() &&
           GetLogic()->GetWindow()->GetPlotList()->GetNumPlots() > 0;
}

bool
AnimationReversePlayActionUI::Checked() const
{
    return (GetLogic()->GetWindow()->GetPlotList()->GetAnimationAttributes().GetAnimationMode() ==
            AnimationAttributes::ReversePlayMode);
}

///////////////////////////////////////////////////////////////////////////////

AnimationStopActionUI::AnimationStopActionUI(ViewerActionLogic *L) :
    ViewerActionUIToggle(L)
{
    SetAllText(tr("Stop"));
    SetToolTip(tr("Stop animation"));
    if (!GetViewerProperties()->GetNowin())
        SetIcons(QPixmap(animationstopon_xpm), QPixmap(animationstopoff_xpm));
}

bool 
AnimationStopActionUI::Enabled() const
{
#ifdef HAVE_DDT
    if (DDTManager::isDDTSim(GetLogic()->GetWindow()))
        return true;
    else
#endif
        return GetLogic()->GetWindow()->GetPlotList()->HasActiveTimeSlider() &&
            GetLogic()->GetWindow()->GetPlotList()->GetNumPlots() > 0;
}

bool
AnimationStopActionUI::Checked() const
{
    bool retval = false;
#ifdef HAVE_DDT
    if (DDTManager::isDDTSim(GetLogic()->GetWindow()))
    {
        const EngineKey &key = GetLogic()->GetWindow()->GetPlotList()->GetEngineKey();
        const avtDatabaseMetaData *md = GetViewerEngineManager()->GetSimulationMetaData(key);
        if (md && key.IsSimulation())
        {
            retval = md->GetSimInfo().GetMode()==avtSimulationInformation::Stopped;
        }
    }
    else
#endif
    retval = (GetLogic()->GetWindow()->GetPlotList()->GetAnimationAttributes().GetAnimationMode() ==
              AnimationAttributes::StopMode);

    return retval;
}

///////////////////////////////////////////////////////////////////////////////

AnimationPlayActionUI::AnimationPlayActionUI(ViewerActionLogic *L) :
    ViewerActionUIToggle(L)
{
    SetAllText(tr("Play"));
    SetToolTip(tr("Play animation"));
    if (!GetViewerProperties()->GetNowin())
        SetIcons(QPixmap(animationplayon_xpm), QPixmap(animationplayoff_xpm));
}

bool
AnimationPlayActionUI::Enabled() const
{
#ifdef HAVE_DDT
    if (DDTManager::isDDTSim(GetLogic()->GetWindow()))
        return true;
    else
#endif
        return GetLogic()->GetWindow()->GetPlotList()->HasActiveTimeSlider() &&
               GetLogic()->GetWindow()->GetPlotList()->GetNumPlots() > 0;
}

bool
AnimationPlayActionUI::Checked() const
{
    bool retval = false;
#ifdef HAVE_DDT
    if (DDTManager::isDDTSim(GetLogic()->GetWindow()))
    {
        const EngineKey &key = GetLogic()->GetWindow()->GetPlotList()->GetEngineKey();
        const avtDatabaseMetaData *md = GetViewerEngineManager()->GetSimulationMetaData(key);
        if (md && key.IsSimulation())
            retval = md->GetSimInfo().GetMode()==avtSimulationInformation::Running;
        else
            retval = true;
    }
    else
#endif
        retval = GetLogic()->GetWindow()->GetPlotList()->GetAnimationAttributes().GetAnimationMode() ==
                 AnimationAttributes::PlayMode;
    return retval;
}

///////////////////////////////////////////////////////////////////////////////

TimeSliderForwardStepActionUI::TimeSliderForwardStepActionUI(ViewerActionLogic *L) :
    ViewerActionUISingle(L)
{
    SetAllText(tr("Forward step"));
    SetToolTip(tr("Step forward one frame"));
    if (!GetViewerProperties()->GetNowin())
        SetIcon(QIcon(QPixmap(animationforwardstep_xpm)));
}

bool
TimeSliderForwardStepActionUI::Enabled() const
{
#ifdef HAVE_DDT
    if (DDTManager::isDDTSim(GetLogic()->GetWindow()))
        return true;
    else
#endif
        return GetLogic()->GetWindow()->GetPlotList()->HasActiveTimeSlider() &&
               GetLogic()->GetWindow()->GetPlotList()->GetNumPlots() > 0;
}

