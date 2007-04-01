#ifndef ANIMATION_ACTIONS_H
#define ANIMATION_ACTIONS_H
#include <viewer_exports.h>
#include <ViewerAction.h>
#include <ViewerToggleAction.h>

class QIconSet;

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

class VIEWER_API TimeSliderReverseStepAction : public ViewerAction
{
public:
    TimeSliderReverseStepAction(ViewerWindow *win);
    virtual ~TimeSliderReverseStepAction() { }

    virtual void Execute();
    virtual bool Enabled() const;
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

class VIEWER_API AnimationReversePlayAction : public ViewerToggleAction
{
public:
    AnimationReversePlayAction(ViewerWindow *win);
    virtual ~AnimationReversePlayAction() { }

    virtual void Execute();
    virtual bool Enabled() const;
    virtual bool Toggled() const;
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

class VIEWER_API AnimationStopAction : public ViewerToggleAction
{
public:
    AnimationStopAction(ViewerWindow *win);
    virtual ~AnimationStopAction() { }

    virtual void Execute();
    virtual bool Enabled() const;
    virtual bool Toggled() const;
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

class AnimationPlayAction : public ViewerToggleAction
{
public:
    AnimationPlayAction(ViewerWindow *win);
    virtual ~AnimationPlayAction() { }

    virtual void Execute();
    virtual bool Enabled() const;
    virtual bool Toggled() const;
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

class TimeSliderForwardStepAction : public ViewerAction
{
public:
    TimeSliderForwardStepAction(ViewerWindow *win);
    virtual ~TimeSliderForwardStepAction() { }

    virtual void Execute();
    virtual bool Enabled() const;
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

class AnimationSetNFramesAction : public ViewerAction
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

class SetTimeSliderStateAction : public ViewerAction
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

class SetActiveTimeSliderAction : public ViewerAction
{
public:
    SetActiveTimeSliderAction(ViewerWindow *win);
    virtual ~SetActiveTimeSliderAction() { };

    virtual void Execute();
};

#endif
