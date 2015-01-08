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

