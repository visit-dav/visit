// ************************************************************************* //
//                                FlyThrough.C                               //
// ************************************************************************* //

#include <FlyThrough.h>

#include <avtVector.h>
#include <avtMatrix.h>

#include <VisWindow.h>
#include <VisWindowInteractorProxy.h>

#include <vtkRenderWindowInteractor.h>


// ****************************************************************************
//  Method: FlyThrough constructor
//
//  Programmer: Eric Brugger
//  Creation:   October 28, 2004
//
// ****************************************************************************

FlyThrough::FlyThrough(VisWindowInteractorProxy &v) : VisitInteractor(v)
{
    ctrlOrShiftPushed = false;
    shouldSpin = false;
}

// ****************************************************************************
//  Method: FlyThrough::OnTimer
//
//  Purpose:
//    Handles the timer event.  For FlyThrough, this means the user has
//    pressed a mouse key and that it is time to sample the mouse position
//    to see if the view should be panned, zoomed or rotated.
//
//  Programmer: Eric Brugger
//  Creation:   October 28, 2004
//
//  Modifications:
//    Eric Brugger, Tue Dec 28 16:48:52 PST 2004
//    I moved RotateCamera, PanCamera and ZoomCamera to the VisitInteractor
//    class as RotateAboutCamera3D, PanCamera3D and DollyCameraAndFocus3D.
//
// ****************************************************************************

void
FlyThrough::OnTimer(void)
{
    vtkRenderWindowInteractor *rwi = Interactor;

    int LastPos[2];
    rwi->GetLastEventPosition(LastPos);

    bool matchedUpState = true;
    switch (State)
    {
      case VTKIS_ROTATE:
        RotateAboutCamera3D(LastPos[0], LastPos[1]);

        rwi->CreateTimer(VTKI_TIMER_UPDATE);
        break;

      case VTKIS_PAN:
        PanCamera3D(LastPos[0], LastPos[1]);

        rwi->CreateTimer(VTKI_TIMER_UPDATE);
        break;

      case VTKIS_ZOOM:
        DollyCameraAndFocus3D(LastPos[0], LastPos[1]);

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
                RotateAboutCamera3D(spinNewX, spinNewY);
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
//  Method: FlyThrough::StartLeftButtonAction
//
//  Purpose:
//    Handles the left button being pushed down.  For FlyThrough, this means
//    panning if the ctrl or shift is pushed, rotating otherwise.  Also,
//    this should start bounding box mode.
//
//  Programmer: Eric Brugger
//  Creation:   October 28, 2004
//
// ****************************************************************************

void
FlyThrough::StartLeftButtonAction()
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
//  Method: FlyThrough::EndLeftButtonAction
//
//  Purpose:
//    Handles the left button being released.  For FlyThrough, this means
//    panning if the ctrl or shift button was held down while the left
//    button was pushed, a rotation otherwise.
//
//  Programmer: Eric Brugger
//  Creation:   October 28, 2004
//
// ****************************************************************************

void
FlyThrough::EndLeftButtonAction()
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
//  Method: FlyThrough::StartMiddleButtonAction
//
//  Purpose:
//    Handles the middle button being pushed down.  For FlyThrough, this 
//    means zooming.
//
//  Programmer: Eric Brugger
//  Creation:   October 28, 2004
//
// ****************************************************************************

void
FlyThrough::StartMiddleButtonAction()
{
    DisableSpinMode();

    StartBoundingBox();

    StartZoom();
}

// ****************************************************************************
//  Method: FlyThrough::EndMiddleButtonAction
//
//  Purpose:
//    Handles the middle button being released.  For FlyThrough, this means
//    ending a zoom.
//
//  Programmer: Eric Brugger
//  Creation:   October 28, 2004
//
// ****************************************************************************

void
FlyThrough::EndMiddleButtonAction()
{
    EndZoom();

    EndBoundingBox();

    IssueViewCallback();
}

// ****************************************************************************
//  Method: FlyThrough::EnableSpinMode
//
//  Purpose:
//      Enables spin mode.  This will determine if spin mode is appropriate,
//      and make the correct calls to start it, if so.
//
//  Programmer: Eric Brugger
//  Creation:   October 28, 2004
//
// ****************************************************************************

void
FlyThrough::EnableSpinMode(void)
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
//  Method: FlyThrough::DisableSpinMode
//
//  Purpose:
//      Disables spin mode if it is currently in action.  This may be called
//      at any time, even if spin mode is not currently on or even enabled.
//
//  Programmer: Eric Brugger
//  Creation:   October 28, 2004
//
// ****************************************************************************

void
FlyThrough::DisableSpinMode(void)
{
    if (shouldSpin)
    {
        EndTimer();
        shouldSpin = false;
    }
}
