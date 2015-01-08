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
// ****************************************************************************

void
AnimationSetNFramesAction::Execute()
{
    window->GetPlotList()->SetNKeyframes(args.GetNFrames());
    windowMgr->UpdateKeyframeAttributes();
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
