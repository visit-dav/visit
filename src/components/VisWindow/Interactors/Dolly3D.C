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

// ************************************************************************* //
//                                 Dolly3D.C                                 //
// ************************************************************************* //

#include <Dolly3D.h>

#include <avtVector.h>
#include <avtMatrix.h>

#include <VisWindow.h>
#include <VisWindowInteractorProxy.h>

#include <vtkRenderWindowInteractor.h>


// ****************************************************************************
//  Method: Dolly3D constructor
//
//  Programmer: Eric Brugger
//  Creation:   December 27, 2004
//
// ****************************************************************************

Dolly3D::Dolly3D(VisWindowInteractorProxy &v) : VisitInteractor(v)
{
    ctrlOrShiftPushed = false;
    shouldSpin = false;
}

// ****************************************************************************
//  Method: Dolly3D::OnTimer
//
//  Purpose:
//    Handles the timer event.  For Dolly, this means the user has
//    pressed a mouse key and that it is time to sample the mouse position
//    to see if the view should be panned, zoomed or rotated.
//
//  Programmer: Eric Brugger
//  Creation:   December 27, 2004
//
// ****************************************************************************

void
Dolly3D::OnTimer(void)
{
    vtkRenderWindowInteractor *rwi = Interactor;

    int LastPos[2];
    rwi->GetLastEventPosition(LastPos);

    bool matchedUpState = true;
    switch (State)
    {
      case VTKIS_ROTATE:
        RotateAboutFocus3D(LastPos[0], LastPos[1], true);

        rwi->CreateTimer(VTKI_TIMER_UPDATE);
        break;

      case VTKIS_PAN:
        PanCamera3D(LastPos[0], LastPos[1]);

        rwi->CreateTimer(VTKI_TIMER_UPDATE);
        break;

      case VTKIS_ZOOM:
        DollyCameraTowardFocus3D(LastPos[0], LastPos[1]);

        rwi->CreateTimer(VTKI_TIMER_UPDATE);
        break;

      default:
        matchedUpState = false;
        break;
    }

    if (!matchedUpState && shouldSpin)
    {
        VisWindow *vw = proxy;
        if(!vw->GetSpinModeSuspended())
        {
            if (vw->GetSpinMode())
            {
                OldX = spinOldX;
                OldY = spinOldY;
                RotateAboutFocus3D(spinNewX, spinNewY, true);
                rwi->CreateTimer(VTKI_TIMER_UPDATE);
            }
            else
            {
                DisableSpinMode();
            }
        }
        else if(vw->GetSpinMode())
        {
            // Don't mess with the camera, just create another timer so
            // we keep getting into this method until spin mode is no
            // longer suspended.
            rwi->CreateTimer(VTKI_TIMER_UPDATE);
        }
    }
}

// ****************************************************************************
//  Method: Dolly3D::StartLeftButtonAction
//
//  Purpose:
//    Handles the left button being pushed down.  For Dolly, this means
//    panning if the ctrl or shift is pushed, rotating otherwise.  Also,
//    this should start bounding box mode.
//
//  Programmer: Eric Brugger
//  Creation:   December 27, 2004
//
// ****************************************************************************

void
Dolly3D::StartLeftButtonAction()
{
    DisableSpinMode();

    StartBoundingBox();

    //
    // If ctrl or shift is pushed, pan, otherwise rotate.  Save which one we
    // did so we can issue the proper "End.." statement when the button is
    // released.
    //
    if (Interactor->GetControlKey()|| Interactor->GetShiftKey())
    {
        StartPan();
        ctrlOrShiftPushed = true;
    }
    else
    {
        StartRotate();
        ctrlOrShiftPushed = false;
    }
}

// ****************************************************************************
//  Method: Dolly3D::EndLeftButtonAction
//
//  Purpose:
//    Handles the left button being released.  For Dolly, this means
//    panning if the ctrl or shift button was held down while the left
//    button was pushed, a rotation otherwise.
//
//  Programmer: Eric Brugger
//  Creation:   December 27, 2004
//
// ****************************************************************************

void
Dolly3D::EndLeftButtonAction()
{
    //
    // We must issue the proper end state for either pan or rotate depending
    // on whether the shift or ctrl button was pushed.
    //
    if (ctrlOrShiftPushed)
    {
        EndPan();
    }
    else
    {
        EndRotate();

        EnableSpinMode();
    }

    EndBoundingBox();

    IssueViewCallback();
}

// ****************************************************************************
//  Method: Dolly3D::StartMiddleButtonAction
//
//  Purpose:
//    Handles the middle button being pushed down.  For Dolly, this 
//    means zooming.
//
//  Programmer: Eric Brugger
//  Creation:   December 27, 2004
//
// ****************************************************************************

void
Dolly3D::StartMiddleButtonAction()
{
    DisableSpinMode();

    StartBoundingBox();

    StartZoom();
}

// ****************************************************************************
//  Method: Dolly3D::EndMiddleButtonAction
//
//  Purpose:
//    Handles the middle button being released.  For Dolly, this means
//    ending a zoom.
//
//  Programmer: Eric Brugger
//  Creation:   December 27, 2004
//
// ****************************************************************************

void
Dolly3D::EndMiddleButtonAction()
{
    EndZoom();

    EndBoundingBox();

    IssueViewCallback();
}

// ****************************************************************************
//  Method: Dolly3D::EnableSpinMode
//
//  Purpose:
//      Enables spin mode.  This will determine if spin mode is appropriate,
//      and make the correct calls to start it, if so.
//
//  Programmer: Eric Brugger
//  Creation:   December 27, 2004
//
// ****************************************************************************

void
Dolly3D::EnableSpinMode(void)
{
    VisWindow *vw = proxy;
    if (vw->GetSpinMode())
    {
        shouldSpin = true;

        //
        // VTK will not be happy unless we enter one of its pre-defined modes.
        // Timer seems as appropriate as any (there idea of spin is much
        // different than ours).  Also, set up the first timer so our spinning
        // can get started.
        //
        StartTimer();
        vtkRenderWindowInteractor *rwi = Interactor;
        rwi->CreateTimer(VTKI_TIMER_UPDATE);
    }
}

// ****************************************************************************
//  Method: Dolly3D::DisableSpinMode
//
//  Purpose:
//      Disables spin mode if it is currently in action.  This may be called
//      at any time, even if spin mode is not currently on or even enabled.
//
//  Programmer: Eric Brugger
//  Creation:   December 27, 2004
//
// ****************************************************************************

void
Dolly3D::DisableSpinMode(void)
{
    if (shouldSpin)
    {
        EndTimer();
        shouldSpin = false;
    }
}
