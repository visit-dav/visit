#include <AnimationActions.h>
#include <ViewerPlotList.h>
#include <ViewerWindow.h>
#include <ViewerWindowManager.h>

#include <qiconset.h>
#include <qpixmap.h>

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

TimeSliderReverseStepAction::TimeSliderReverseStepAction(ViewerWindow *win) :
    ViewerAction(win, "ReverseStep")
{
    SetAllText("Reverse step");
    SetToolTip("Step back one frame");
    if (!win->GetNoWinMode())
        SetIconSet(QIconSet(QPixmap(animationreversestep_xpm)));
}

void
TimeSliderReverseStepAction::Execute()
{
     windowMgr->PrevFrame(windowId);
}

bool
TimeSliderReverseStepAction::Enabled() const
{
    return window->GetPlotList()->HasActiveTimeSlider() &&
           window->GetPlotList()->GetNumPlots() > 0;
}

///////////////////////////////////////////////////////////////////////////////

AnimationReversePlayAction::AnimationReversePlayAction(ViewerWindow *win) :
    ViewerToggleAction(win, "ReversePlay")
{
    SetAllText("Reverse play");
    SetToolTip("Play animation in reverse");
    if (!win->GetNoWinMode())
        SetIcons(QPixmap(animationreverseplayon_xpm), QPixmap(animationreverseplayoff_xpm));
}

void
AnimationReversePlayAction::Execute()
{
     windowMgr->ReversePlay(windowId);
}

bool
AnimationReversePlayAction::Enabled() const
{
    return window->GetPlotList()->HasActiveTimeSlider() &&
           window->GetPlotList()->GetNumPlots() > 0;
}

bool
AnimationReversePlayAction::Toggled() const
{
    return (window->GetPlotList()->GetAnimationMode() ==
            ViewerPlotList::ReversePlayMode);
}

///////////////////////////////////////////////////////////////////////////////

AnimationStopAction::AnimationStopAction(ViewerWindow *win) :
    ViewerToggleAction(win, "Stop")
{
    SetAllText("Stop");
    SetToolTip("Stop animation");
    if (!win->GetNoWinMode())
        SetIcons(QPixmap(animationstopon_xpm), QPixmap(animationstopoff_xpm));
}

void
AnimationStopAction::Execute()
{
     windowMgr->Stop(windowId);
}

bool 
AnimationStopAction::Enabled() const
{
    return window->GetPlotList()->HasActiveTimeSlider() &&
           window->GetPlotList()->GetNumPlots() > 0;
}

bool
AnimationStopAction::Toggled() const
{
    return (window->GetPlotList()->GetAnimationMode() ==
            ViewerPlotList::StopMode);
}

///////////////////////////////////////////////////////////////////////////////

AnimationPlayAction::AnimationPlayAction(ViewerWindow *win) :
    ViewerToggleAction(win, "Play")
{
    SetAllText("Play");
    SetToolTip("Play animation");
    if (!win->GetNoWinMode())
        SetIcons(QPixmap(animationplayon_xpm), QPixmap(animationplayoff_xpm));
}

void
AnimationPlayAction::Execute()
{
     windowMgr->Play(windowId);
}

bool
AnimationPlayAction::Enabled() const
{
    return window->GetPlotList()->HasActiveTimeSlider() &&
           window->GetPlotList()->GetNumPlots() > 0;
}

bool
AnimationPlayAction::Toggled() const
{
    return (window->GetPlotList()->GetAnimationMode() ==
            ViewerPlotList::PlayMode);
}

///////////////////////////////////////////////////////////////////////////////

TimeSliderForwardStepAction::TimeSliderForwardStepAction(ViewerWindow *win) :
    ViewerAction(win, "ForwardStep")
{
    SetAllText("Forward step");
    SetToolTip("Step forward one frame");
    if (!win->GetNoWinMode())
        SetIconSet(QIconSet(QPixmap(animationforwardstep_xpm)));
}

void
TimeSliderForwardStepAction::Execute()
{
     windowMgr->NextFrame(windowId);
}

bool
TimeSliderForwardStepAction::Enabled() const
{
    return window->GetPlotList()->HasActiveTimeSlider() &&
           window->GetPlotList()->GetNumPlots() > 0;
}

///////////////////////////////////////////////////////////////////////////////

AnimationSetNFramesAction::AnimationSetNFramesAction(ViewerWindow *win) :
    ViewerAction(win, "AnimationSetNFramesAction")
{
    DisableVisual();
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
// ****************************************************************************

void
AnimationSetNFramesAction::Execute()
{
    window->GetPlotList()->SetNKeyframes(args.GetNFrames());
    windowMgr->UpdateKeyframeAttributes();
}

///////////////////////////////////////////////////////////////////////////////

SetTimeSliderStateAction::SetTimeSliderStateAction(ViewerWindow *win) :
    ViewerAction(win, "SetTimeSliderStateAction")
{
    DisableVisual();
}

void
SetTimeSliderStateAction::Execute()
{
    windowMgr->SetFrameIndex(args.GetStateNumber(), windowId);
}

///////////////////////////////////////////////////////////////////////////////

SetActiveTimeSliderAction::SetActiveTimeSliderAction(ViewerWindow *win) :
    ViewerAction(win, "SetActiveTimeSliderAction")
{
    DisableVisual();
}

void
SetActiveTimeSliderAction::Execute()
{
    windowMgr->SetActiveTimeSlider(args.GetDatabase(), windowId);
}
