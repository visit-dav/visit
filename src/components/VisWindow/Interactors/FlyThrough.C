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
        RotateCamera(LastPos[0], LastPos[1]);

        rwi->CreateTimer(VTKI_TIMER_UPDATE);
        break;

      case VTKIS_PAN:
        PanCamera(LastPos[0], LastPos[1]);

        rwi->CreateTimer(VTKI_TIMER_UPDATE);
        break;

      case VTKIS_ZOOM:
        ZoomCamera(LastPos[0], LastPos[1]);

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
                RotateCamera(spinNewX, spinNewY);
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


// ****************************************************************************
//  Method: FlyThrough::RotateCamera
//
//  Purpose:
//    Handle rotating the camera.
//
//  Programmer: Eric Brugger
//  Creation:   October 28, 2004
//
// ****************************************************************************

void
FlyThrough::RotateCamera(const int x, const int y)
{
    vtkRenderWindowInteractor *rwi = Interactor;

    if ((OldX != x) || (OldY != y))
    {
        spinOldX = OldX;
        spinOldY = OldY;
        spinNewX = x;
        spinNewY = y;

        int       size[2];
        int       width, height;
        double    unitX, unitY;
        double    unitPrevX, unitPrevY;

        rwi->GetSize(size);
        width = size[0];
        height = size[1];

        unitX = 1.0 * ((2.0 * x) / width - 1.0);
        unitY = 1.0 * (1.0 - (2.0 * y) / height);

        unitPrevX = 1.0 * ((2.0 * OldX) / width - 1.0);
        unitPrevY = 1.0 * (1.0 - (2.0 * OldY) / height);

        //
        // Build the quaternion and convert it to a rotation matrix.
        // The sign is flipped on the 3 component of the quaternion
        // to get the rotations about the screen z axis in the correct
        // direction.  The first component of the quaternion contains
        // x rotation information, the second component contains y
        // rotation information, and the third component contains z
        // rotation information.  The fourth component controlls the
        // magnitude of the rotation, the closer to 1 the smaller.
        // The quaternion will be of unit length.
        //
        avtMatrix rot;

        rot.MakeTrackball(unitPrevX, unitPrevY, unitX, unitY, true);
        rot.Transpose();

        //
        // Get the current view information.
        //
        VisWindow *vw = proxy;

        const avtView3D &oldView3D = vw->GetView3D();

        avtVector VPN(oldView3D.normal);
        avtVector VUP(oldView3D.viewUp);
        avtVector Focus(oldView3D.focus);

        //
        // Calculate the coordinate transformation from the world coordinate
        // space to the screen coordinate space (and its inverse).
        //
        avtVector VPNCrossVUP;
        avtMatrix mata, mataInverse;

        VPNCrossVUP = VPN % VUP;

        mata[0][0] = VPNCrossVUP.x;
        mata[0][1] = VPNCrossVUP.y;
        mata[0][2] = VPNCrossVUP.z;
        mata[0][3] = 0.;
        mata[1][0] = VUP.x;
        mata[1][1] = VUP.y;
        mata[1][2] = VUP.z;
        mata[1][3] = 0.;
        mata[2][0] = VPN.x;
        mata[2][1] = VPN.y;
        mata[2][2] = VPN.z;
        mata[2][3] = 0.;
        mata[3][0] = 0.;
        mata[3][1] = 0.;
        mata[3][2] = 0.;
        mata[3][3] = 1.;

        mataInverse = mata;
        mataInverse.Transpose();

        //
        // Calculate the composite transformation.
        //
        avtMatrix mat;

        mat = mataInverse * rot * mata;

        //
        // Calculate the new view plane normal, view up vector and focus.
        //
        avtVector camera;

        double distance = oldView3D.parallelScale /
                          tan(oldView3D.viewAngle * 3.1415926535 / 360.);
        camera.x = oldView3D.focus[0] + oldView3D.normal[0] * distance;
        camera.y = oldView3D.focus[1] + oldView3D.normal[1] * distance;
        camera.z = oldView3D.focus[2] + oldView3D.normal[2] * distance;

        avtVector newVPN(mat ^ VPN);
        avtVector newVUP(mat ^ VUP);
        avtVector newFocus(camera - newVPN * distance);

        //
        // Orthogonalize the new view plane normal and view up vector.
        //
        newVPN.normalize();
        VPNCrossVUP = newVPN % newVUP;
        newVUP = VPNCrossVUP % newVPN;
        newVUP.normalize();

        //
        // Set the new view.
        //
        avtView3D newView3D = vw->GetView3D();

        newView3D.normal[0] = newVPN.x;
        newView3D.normal[1] = newVPN.y;
        newView3D.normal[2] = newVPN.z;
        newView3D.viewUp[0] = newVUP.x;
        newView3D.viewUp[1] = newVUP.y;
        newView3D.viewUp[2] = newVUP.z;
        newView3D.focus[0]  = newFocus.x;
        newView3D.focus[1]  = newFocus.y;
        newView3D.focus[2]  = newFocus.z;

        vw->SetView3D(newView3D);

        OldX = x;
        OldY = y;
        rwi->Render();
    }
}

// ****************************************************************************
//  Method: FlyThrough::PanCamera
//
//  Purpose:
//    Handle panning the camera.
//
//  Programmer: Eric Brugger
//  Creation:   October 28, 2004
//
// ****************************************************************************

void
FlyThrough::PanCamera(const int x, const int y)
{
    vtkRenderWindowInteractor *rwi = Interactor;

    if ((OldX != x) || (OldY != y))
    {
        //
        // Determine the size of the window.
        //
        int       size[2];
        int       width, height;

        rwi->GetSize(size);
        width = size[0];
        height = size[1];

        //
        // Get the current view information.
        //
        VisWindow *vw = proxy;

        const avtView3D &oldView3D = vw->GetView3D();

        avtVector VPN(oldView3D.normal);
        avtVector VUP(oldView3D.viewUp);
        avtVector Focus(oldView3D.focus);

        //
        // Calculate the coordinate transformation from the world coordinate
        // space to the screen coordinate space.
        //
        avtVector VUPCrossVPN;
        avtMatrix mat, matTranspose;

        VUPCrossVPN = VUP % VPN;

        mat[0][0] = VUPCrossVPN.x;
        mat[0][1] = VUPCrossVPN.y;
        mat[0][2] = VUPCrossVPN.z;
        mat[0][3] = 0.;
        mat[1][0] = VUP.x;
        mat[1][1] = VUP.y;
        mat[1][2] = VUP.z;
        mat[1][3] = 0.;
        mat[2][0] = VPN.x;
        mat[2][1] = VPN.y;
        mat[2][2] = VPN.z;
        mat[2][3] = 0.;
        mat[3][0] = 0.;
        mat[3][1] = 0.;
        mat[3][2] = 0.;
        mat[3][3] = 1.;

        matTranspose = mat;
        matTranspose.Transpose();

        //
        // Calculate the x and y pan distances in the view plane.
        //
        avtVector pan;

        pan.x = ((double)((OldX - x) / height)) *
                 2.0 * oldView3D.parallelScale;
        pan.y = ((double)((OldY - y) / height)) *
                 2.0 * oldView3D.parallelScale;
        pan.z = 0.;

        //
        // Transform the screen space pan factors to world space pan factors.
        //
        avtVector pan2;

        pan2 = matTranspose * pan;

        //
        // Set the new origin.
        //
        avtView3D newView3D = vw->GetView3D();

        newView3D.focus[0] += pan2.x;
        newView3D.focus[1] += pan2.y;
        newView3D.focus[2] += pan2.z;

        vw->SetView3D(newView3D);

        OldX = x;
        OldY = y;
        rwi->Render();
    }
}

// ****************************************************************************
//  Method: FlyThrough::ZoomCamera
//
//  Purpose:
//    Handle zooming the camera.
//
//  Programmer: Eric Brugger
//  Creation:   October 28, 2004
//
// ****************************************************************************

void
FlyThrough::ZoomCamera(const int x, const int y)
{
    vtkRenderWindowInteractor *rwi = Interactor;

    if (OldY != y)
    {
        //
        // Calculate the zoom factor.
        //
        double dyf = -1.0 * MotionFactor * (double)(y - OldY) /
                         (double)(Center[1]);

        //
        // Calculate the new parallel scale.
        //
        VisWindow *vw = proxy;

        avtView3D newView3D = vw->GetView3D();

        newView3D.focus[0] +=
            newView3D.normal[0] * newView3D.parallelScale * dyf;
        newView3D.focus[1] +=
            newView3D.normal[1] * newView3D.parallelScale * dyf;
        newView3D.focus[2] +=
            newView3D.normal[2] * newView3D.parallelScale * dyf;

        vw->SetView3D(newView3D);

        OldX = x;
        OldY = y;
        rwi->Render();
    }
}
