// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef ANIMATION_ACTION_UI_H
#define ANIMATION_ACTION_UI_H
#include <viewer_exports.h>
#include <ViewerActionUISingle.h>
#include <ViewerActionUIToggle.h>

// ****************************************************************************
// Class: TimeSliderReverseStepActionUI
//
// Purpose:
//   Handles the reverse step through an animation ActionUI.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 15:56:39 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API TimeSliderReverseStepActionUI : public ViewerActionUISingle
{
public:
    TimeSliderReverseStepActionUI(ViewerActionLogic *L);
    virtual ~TimeSliderReverseStepActionUI() { }

    virtual bool Enabled() const;
};

// ****************************************************************************
// Class: AnimationReversePlayActionUI
//
// Purpose:
//   Handles the reverse play through an animation ActionUI.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 15:56:42 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API AnimationReversePlayActionUI : public ViewerActionUIToggle
{
public:
    AnimationReversePlayActionUI(ViewerActionLogic *L);
    virtual ~AnimationReversePlayActionUI() { }

    virtual bool Enabled() const;
    virtual bool Checked() const;
};

// ****************************************************************************
// Class: AnimationStopActionUI
//
// Purpose:
//   Handles the stop ActionUI for an animation.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 15:57:23 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API AnimationStopActionUI : public ViewerActionUIToggle
{
public:
    AnimationStopActionUI(ViewerActionLogic *L);
    virtual ~AnimationStopActionUI() { }

    virtual bool Enabled() const;
    virtual bool Checked() const;
};

// ****************************************************************************
// Class: AnimationPlayActionUI
//
// Purpose:
//   Handles the play ActionUI for an animation.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 15:58:23 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API AnimationPlayActionUI : public ViewerActionUIToggle
{
public:
    AnimationPlayActionUI(ViewerActionLogic *L);
    virtual ~AnimationPlayActionUI() { }

    virtual bool Enabled() const;
    virtual bool Checked() const;
};

// ****************************************************************************
// Class: TimeSliderForwardStepActionUI
//
// Purpose:
//   Handles the forward step ActionUI for an animation.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 15:58:43 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API TimeSliderForwardStepActionUI : public ViewerActionUISingle
{
public:
    TimeSliderForwardStepActionUI(ViewerActionLogic *L);
    virtual ~TimeSliderForwardStepActionUI() { }

    virtual bool Enabled() const;
};

#endif
