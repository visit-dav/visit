// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//  Modifications:
//    Kathleen Bonnell, Wed Jun  8 10:03:41 PDT 2011
//    Use current EventPosition instead of last.
//
// ****************************************************************************

void
Dolly3D::OnTimer(void)
{
    vtkRenderWindowInteractor *rwi = Interactor;

    int Pos[2];
    rwi->GetEventPosition(Pos);

    bool matchedUpState = true;
    switch (State)
    {
      case VTKIS_ROTATE:
        RotateAboutFocus3D(Pos[0], Pos[1], true);

        rwi->CreateTimer(VTKI_TIMER_UPDATE);
        break;

      case VTKIS_PAN:
        PanCamera3D(Pos[0], Pos[1]);

        rwi->CreateTimer(VTKI_TIMER_UPDATE);
        break;

      case VTKIS_ZOOM:
        DollyCameraTowardFocus3D(Pos[0], Pos[1]);

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


// ****************************************************************************
//  Method: Dolly3D::OnMouseWheelForward
//
//  Purpose:
//    Handles the mouse wheel turned forward.
//
//  Programmer: Kathleen Biagas
//  Creation:   March 16, 2022
//
// ****************************************************************************

void
Dolly3D::OnMouseWheelForward()
{
    StartZoom();
    DollyCameraTowardFocus3D(MotionFactor * 0.2 * this->MouseWheelMotionFactor);
    EndZoom();
    IssueViewCallback(true);
}


// ****************************************************************************
//  Method: Dolly3D::OnMouseWheelBackward
//
//  Purpose:
//    Handles the mouse wheel turned backward.
//
//  Programmer: Kathleen Biagas
//  Creation:   March 16, 2022
//
// ****************************************************************************

void
Dolly3D::OnMouseWheelBackward()
{
    StartZoom();
    DollyCameraTowardFocus3D(MotionFactor * -0.2 * this->MouseWheelMotionFactor);
    EndZoom();
    IssueViewCallback(true);
}
