// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef ANIMATION_ACTIONS_H
#define ANIMATION_ACTIONS_H
#include <ViewerActionLogic.h>

// ****************************************************************************
// Class: TimeSliderReverseStepAction
//
// Purpose:
//   Handles the reverse step through an animation action.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 15:56:39 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API TimeSliderReverseStepAction : public ViewerActionLogic
{
public:
    TimeSliderReverseStepAction(ViewerWindow *win);
    virtual ~TimeSliderReverseStepAction() { }

    virtual void Execute();
};

// ****************************************************************************
// Class: AnimationReversePlayAction
//
// Purpose:
//   Handles the reverse play through an animation action.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 15:56:42 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API AnimationReversePlayAction : public ViewerActionLogic
{
public:
    AnimationReversePlayAction(ViewerWindow *win);
    virtual ~AnimationReversePlayAction() { }

    virtual void Execute();
};

// ****************************************************************************
// Class: AnimationStopAction
//
// Purpose:
//   Handles the stop action for an animation.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 15:57:23 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API AnimationStopAction : public ViewerActionLogic
{
public:
    AnimationStopAction(ViewerWindow *win);
    virtual ~AnimationStopAction() { }

    virtual void Execute();
};

// ****************************************************************************
// Class: AnimationPlayAction
//
// Purpose:
//   Handles the play action for an animation.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 15:58:23 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API AnimationPlayAction : public ViewerActionLogic
{
public:
    AnimationPlayAction(ViewerWindow *win);
    virtual ~AnimationPlayAction() { }

    virtual void Execute();
};

// ****************************************************************************
// Class: TimeSliderForwardStepAction
//
// Purpose:
//   Handles the forward step action for an animation.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 15:58:43 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API TimeSliderForwardStepAction : public ViewerActionLogic
{
public:
    TimeSliderForwardStepAction(ViewerWindow *win);
    virtual ~TimeSliderForwardStepAction() { }

    virtual void Execute();
};

// ****************************************************************************
// Class: AnimationSetNFramesAction
//
// Purpose:
//   Handles the setNFrames action for an animtation.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 15:59:00 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API AnimationSetNFramesAction : public ViewerActionLogic
{
public:
    AnimationSetNFramesAction(ViewerWindow *win);
    virtual ~AnimationSetNFramesAction() { }

    virtual void Execute();
};

// ****************************************************************************
// Class: SetTimeSliderStateAction
//
// Purpose:
//   Handles the set state action for the active time slider.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 15:59:14 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetTimeSliderStateAction : public ViewerActionLogic
{
public:
    SetTimeSliderStateAction(ViewerWindow *win);
    virtual ~SetTimeSliderStateAction() { }

    virtual void Execute();
};

// ****************************************************************************
// Class: SetActiveTimeSliderAction
//
// Purpose:
//   Handles setting the active time slider.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Sun Jan 25 02:15:16 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetActiveTimeSliderAction : public ViewerActionLogic
{
public:
    SetActiveTimeSliderAction(ViewerWindow *win);
    virtual ~SetActiveTimeSliderAction() { };

    virtual void Execute();
};

// ****************************************************************************
// Class: StartPlotAnimationAction
//
// Purpose:
//   This action turns on animation for some of the window's plots.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 12 15:29:22 PDT 2013
//
// Modifications:
//   
// ****************************************************************************

class StartPlotAnimationAction : public ViewerActionLogic
{
public:
    StartPlotAnimationAction(ViewerWindow *);
    virtual ~StartPlotAnimationAction() { }

    virtual void Execute();
};

// ****************************************************************************
// Class: StopPlotAnimationAction
//
// Purpose:
//   This action turns off animation for some of the window's plots.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 12 15:29:22 PDT 2013
//
// Modifications:
//   
// ****************************************************************************

class StopPlotAnimationAction : public ViewerActionLogic
{
public:
    StopPlotAnimationAction(ViewerWindow *);
    virtual ~StopPlotAnimationAction() { }

    virtual void Execute();
};

#endif
