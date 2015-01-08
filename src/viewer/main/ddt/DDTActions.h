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
