// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef DDT_ACTIONS_H
#define DDT_ACTIONS_H
#include <ViewerActionLogic.h>

// ****************************************************************************
// Class: DDTConnectAction
//
// Purpose:
//   Connects/disconnects the viewer with DDT
//
// Notes:
//
// Programmer: Jonathan Byrd (Allinea Software)
// Creation:   December 18, 2011
//
// Modifications:
//
// ****************************************************************************

class DDTConnectAction : public ViewerActionLogic
{
public:
    DDTConnectAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~DDTConnectAction(){}

    virtual void Execute();
};

// ****************************************************************************
// Class: DDTFocusAction
//
// Purpose:
//   Instructs DDT to focus on a specific domain & element
//
// Notes:
//
// Programmer: Jonathan Byrd (Allinea Software)
// Creation:   December 18, 2011
//
// Modifications:
//
// ****************************************************************************

class DDTFocusAction : public ViewerActionLogic
{
public:
    DDTFocusAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~DDTFocusAction(){}

    virtual void Execute();
};

// ****************************************************************************
// Class: ReleaseToDDTAction
//
// Purpose:
//   Handles the 'release to DDT' action.
//
// Notes:
//
// Programmer: Jonathan Byrd (Allinea Software)
// Creation:   December 18, 2011
//
// Modifications:
//
// ****************************************************************************

class ReleaseToDDTAction : public ViewerActionLogic
{
public:
    ReleaseToDDTAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~ReleaseToDDTAction(){}

    virtual void Execute();
};

// ****************************************************************************
// Class: PlotDDTVispointVariablesAction
//
// Purpose:
//   Action to automatically plot variables at the current DDT vispoint.
//
// Notes:
//
// Programmer: Jonathan Byrd (Allinea Software)
// Creation:   July 15, 2013
//
// Modifications:
//
// ****************************************************************************

class PlotDDTVispointVariablesAction : public ViewerActionLogic
{
public:
    PlotDDTVispointVariablesAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~PlotDDTVispointVariablesAction(){}

    virtual void Execute();
};


///////////////////////////////////////////////////////////////////////////////
/// ACTIONS THAT HAVE WE WANT TO AUGMENT WITH SOME DDT BEHAVIOR. WE DERIVE
/// FROM THE REGULAR ACTION LOGIC AND THEN WE ADD SOME DDT TO IT.
///////////////////////////////////////////////////////////////////////////////

#include <AnimationActions.h>

// ****************************************************************************
// Class: AnimationStopActionDDT
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

class AnimationStopActionDDT : public AnimationStopAction
{
public:
    AnimationStopActionDDT(ViewerWindow *win) : AnimationStopAction(win) { }
    virtual ~AnimationStopActionDDT() { }

    virtual void Execute();
};

// ****************************************************************************
// Class: AnimationPlayActionDDT
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

class AnimationPlayActionDDT : public AnimationPlayAction
{
public:
    AnimationPlayActionDDT(ViewerWindow *win) : AnimationPlayAction(win) { }
    virtual ~AnimationPlayActionDDT() { }

    virtual void Execute();
};

// ****************************************************************************
// Class: TimeSliderForwardStepActionDDT
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

class TimeSliderForwardStepActionDDT : public TimeSliderForwardStepAction
{
public:
    TimeSliderForwardStepActionDDT(ViewerWindow *win) : TimeSliderForwardStepAction(win) { }
    virtual ~TimeSliderForwardStepActionDDT() { }

    virtual void Execute();
};


#endif
