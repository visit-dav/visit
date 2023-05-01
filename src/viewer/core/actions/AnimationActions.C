// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <AnimationActions.h>
#include <ViewerFileServerInterface.h>
#include <ViewerPlotList.h>
#include <ViewerProperties.h>
#include <ViewerState.h>
#include <ViewerWindow.h>
#include <ViewerWindowManager.h>
#include <ViewerActionManager.h>
#include <avtSimulationInformation.h>
#include <avtDatabaseMetaData.h>

#include <DatabaseCorrelationList.h>
#include <DebugStream.h>

///////////////////////////////////////////////////////////////////////////////

TimeSliderReverseStepAction::TimeSliderReverseStepAction(ViewerWindow *win) :
    ViewerActionLogic(win)
{
}

void
TimeSliderReverseStepAction::Execute()
{
     windowMgr->PrevFrame(GetWindow()->GetWindowId());
}

///////////////////////////////////////////////////////////////////////////////

AnimationReversePlayAction::AnimationReversePlayAction(ViewerWindow *win) :
    ViewerActionLogic(win)
{
}

void
AnimationReversePlayAction::Execute()
{
     windowMgr->ReversePlay(GetWindow()->GetWindowId());
}

///////////////////////////////////////////////////////////////////////////////

AnimationStopAction::AnimationStopAction(ViewerWindow *win) :
    ViewerActionLogic(win)
{
}

void
AnimationStopAction::Execute()
{
    windowMgr->Stop(GetWindow()->GetWindowId());
}

///////////////////////////////////////////////////////////////////////////////

AnimationPlayAction::AnimationPlayAction(ViewerWindow *win) :
    ViewerActionLogic(win)
{
}

void
AnimationPlayAction::Execute()
{
    windowMgr->Play(GetWindow()->GetWindowId());
}

///////////////////////////////////////////////////////////////////////////////

TimeSliderForwardStepAction::TimeSliderForwardStepAction(ViewerWindow *win) :
    ViewerActionLogic(win)
{
}

void
TimeSliderForwardStepAction::Execute()
{
     windowMgr->NextFrame(GetWindow()->GetWindowId());
}

///////////////////////////////////////////////////////////////////////////////

AnimationSetNFramesAction::AnimationSetNFramesAction(ViewerWindow *win) :
    ViewerActionLogic(win)
{
}

// ****************************************************************************
// Method: AnimationSetNFramesAction::Execute
//
// Purpose: 
//   Executes the set n frames action, which is around for keyframing.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 3 16:19:53 PST 2004
//
// Modifications:
//   Brad Whitlock, Tue Feb 3 16:20:22 PST 2004
//   I made it update the keyframe attributes instead of the window info.
//
//   Eric Brugger, Eric Brugger, Wed Mar 22 16:23:12 PDT 2023
//   Modified where the viewer window manager keyframe and timeslider
//   attributes are updated in response to changes in the keyframe mode
//   and number of keyframes.
//
// ****************************************************************************

void
AnimationSetNFramesAction::Execute()
{
    window->GetPlotList()->SetNKeyframes(args.GetNFrames());
    windowMgr->UpdateKeyframeAttributes();
    windowMgr->UpdateWindowInformation(WINDOWINFO_TIMESLIDERS);
}

///////////////////////////////////////////////////////////////////////////////

SetTimeSliderStateAction::SetTimeSliderStateAction(ViewerWindow *win) :
    ViewerActionLogic(win)
{
}

void
SetTimeSliderStateAction::Execute()
{
    windowMgr->SetFrameIndex(args.GetStateNumber(), GetWindow()->GetWindowId());
}

///////////////////////////////////////////////////////////////////////////////

SetActiveTimeSliderAction::SetActiveTimeSliderAction(ViewerWindow *win) :
    ViewerActionLogic(win)
{
}

// ****************************************************************************
// Method: SetActiveTimeSliderAction::Execute
//
// Purpose: 
//   This method executes the SetActiveTimeSlider action.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 31 09:50:32 PDT 2004
//
// Modifications:
//   Brad Whitlock, Wed Mar 31 09:50:37 PDT 2004
//   Added code to expand the time slider name in case it contains relative
//   paths, etc.
//
//   Brad Whitlock, Mon Aug 30 14:13:55 PST 2004
//   I added code to prevent the keyframing time slider from incorrectly being
//   expanded as a database name.
//
// ****************************************************************************

void
SetActiveTimeSliderAction::Execute()
{
    //
    // If we don't find a correlation for the desired time slider, expand the
    // time slider name and try and use the expanded time slider name.
    //
    DatabaseCorrelationList *cL = GetViewerState()->GetDatabaseCorrelationList();
    std::string tsName(args.GetDatabase());

    if(cL->FindCorrelation(tsName) == 0)
    {
        // If we're in keyframe mode, only expand the database name if the
        // name of the time slider is not the same as the keyframing time
        // slider.
        if(window->GetPlotList()->GetKeyframeMode() ? (tsName != KF_TIME_SLIDER) : true)
        {
            std::string host, db;
            GetViewerFileServer()->ExpandDatabaseName(tsName, host, db);
            debug3 << "The new time slider had to be expanded before we could "
                      "use it. It was called: "
                   << args.GetDatabase().c_str() << ". Now it is called: "
                   << tsName.c_str() << endl;
        }
    } 

    windowMgr->SetActiveTimeSlider(tsName, GetWindow()->GetWindowId());
}

///////////////////////////////////////////////////////////////////////////////

StartPlotAnimationAction::StartPlotAnimationAction(ViewerWindow *win) :
    ViewerActionLogic(win)
{
}

void
StartPlotAnimationAction::Execute()
{
    const intVector &activePlots = args.GetActivePlotIds();
    GetWindow()->GetPlotList()->StartPlotAnimation(activePlots);
}

///////////////////////////////////////////////////////////////////////////////

StopPlotAnimationAction::StopPlotAnimationAction(ViewerWindow *win) :
    ViewerActionLogic(win)
{
}

void
StopPlotAnimationAction::Execute()
{
    const intVector &activePlots = args.GetActivePlotIds();
    GetWindow()->GetPlotList()->StopPlotAnimation(activePlots);
}
