/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
