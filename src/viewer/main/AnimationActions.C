#include <AnimationActions.h>
#include <ViewerAnimation.h>
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

AnimationReverseStepAction::AnimationReverseStepAction(ViewerWindow *win) :
    ViewerAction(win, "ReverseStep")
{
    SetAllText("Reverse step");
    SetToolTip("Step back one frame");
    if (!win->GetNoWinMode())
        SetIconSet(QIconSet(QPixmap(animationreversestep_xpm)));
}

void
AnimationReverseStepAction::Execute()
{
     windowMgr->PrevFrame(windowId);
}

bool
AnimationReverseStepAction::Enabled() const
{
    return (window->GetAnimation()->GetNFrames() > 1) &&
           (window->GetAnimation()->GetPlotList()->GetNumPlots() > 0);
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
    return (window->GetAnimation()->GetNFrames() > 1) &&
           (window->GetAnimation()->GetPlotList()->GetNumPlots() > 0);
}

bool
AnimationReversePlayAction::Toggled() const
{
    return (window->GetAnimation()->GetMode() == ViewerAnimation::ReversePlayMode);
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
    return (window->GetAnimation()->GetNFrames() > 1) &&
           (window->GetAnimation()->GetPlotList()->GetNumPlots() > 0);
}

bool
AnimationStopAction::Toggled() const
{
    return (window->GetAnimation()->GetMode() == ViewerAnimation::StopMode);
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
    return (window->GetAnimation()->GetNFrames() > 1) &&
           (window->GetAnimation()->GetPlotList()->GetNumPlots() > 0);
}

bool
AnimationPlayAction::Toggled() const
{
    return (window->GetAnimation()->GetMode() == ViewerAnimation::PlayMode);
}

///////////////////////////////////////////////////////////////////////////////

AnimationForwardStepAction::AnimationForwardStepAction(ViewerWindow *win) :
    ViewerAction(win, "ForwardStep")
{
    SetAllText("Forward step");
    SetToolTip("Step forward one frame");
    if (!win->GetNoWinMode())
        SetIconSet(QIconSet(QPixmap(animationforwardstep_xpm)));
}

void
AnimationForwardStepAction::Execute()
{
     windowMgr->NextFrame(windowId);
}

bool
AnimationForwardStepAction::Enabled() const
{
    return (window->GetAnimation()->GetNFrames() > 1) &&
           (window->GetAnimation()->GetPlotList()->GetNumPlots() > 0);
}

///////////////////////////////////////////////////////////////////////////////

AnimationSetNFramesAction::AnimationSetNFramesAction(ViewerWindow *win) :
    ViewerAction(win, "AnimationSetNFramesAction")
{
    DisableVisual();
}

void
AnimationSetNFramesAction::Execute()
{
    window->GetAnimation()->SetNFrames(args.GetNFrames());
    windowMgr->UpdateGlobalAtts();
}

///////////////////////////////////////////////////////////////////////////////

AnimationSetFrameAction::AnimationSetFrameAction(ViewerWindow *win) :
    ViewerAction(win, "AnimationSetFrameAction")
{
    DisableVisual();
}

void
AnimationSetFrameAction::Execute()
{
    windowMgr->SetFrameIndex(args.GetFrameNumber(), windowId);
}
