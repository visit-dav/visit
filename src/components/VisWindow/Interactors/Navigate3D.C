// ************************************************************************* //
//                                Navigate3D.C                               //
// ************************************************************************* //

#include <Navigate3D.h>

#include <avtVector.h>
#include <avtMatrix.h>

#include <VisWindow.h>
#include <VisWindowInteractorProxy.h>

#include <vtkRenderWindowInteractor.h>


// ****************************************************************************
//  Method: Navigate3D constructor
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//
//    Hank Childs, Wed May 29 10:35:19 PDT 2002
//    Initialized shouldSpin.
//
// ****************************************************************************

Navigate3D::Navigate3D(VisWindowInteractorProxy &v) : VisitInteractor(v)
{
    ctrlOrShiftPushed = false;
    shouldSpin = false;
}


// ****************************************************************************
//  Method: Navigate3D::OnTimer
//
//  Purpose:
//    Handles the timer event.  For Navigate3D, this means the user has
//    pressed a mouse key and that it is time to sample the mouse position
//    to see if the view should be panned, zoomed or rotated.
//
//  Programmer: Eric Brugger
//  Creation:   August 10, 2001
//
//  Modifications:
//
//    Hank Childs, Wed May 29 10:35:19 PDT 2002
//    Added support for 'spin' mode.
//
//    Hank Childs, Wed May 29 14:15:51 PDT 2002
//    If spin mode was set to be false while we are mid-spin, then honor that
//    and stop spinning.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002 
//    Retrieve the LastPosition from the renderWindowInteractor.  It is no
//    longer a member of the parent class. 
//
//    Brad Whitlock, Wed Sep 10 16:05:08 PST 2003
//    I added support for temporarily suspending spin mode.
//
// ****************************************************************************

void
Navigate3D::OnTimer(void)
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
//  Method: Navigate3D::StartLeftButtonAction
//
//  Purpose:
//    Handles the left button being pushed down.  For Navigate3D, this means
//    panning if the ctrl or shift is pushed, rotating otherwise.  Also,
//    this should start bounding box mode.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Mar 18 13:48:55 PST 2002
//    Renamed from OnLeftButtonDown.
//
//    Hank Childs, Tue Mar 19 14:34:26 PST 2002
//    Pushed code for starting bounding box into base class.
//
//    Hank Childs, Wed May 29 10:35:19 PDT 2002
//    Added support for 'spin' mode.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002
//    Removed arguments to match vtk's new interactor api, they are accessed
//    directly through the Interactor now.  
//
// ****************************************************************************

void
Navigate3D::StartLeftButtonAction()
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
//  Method: Navigate3D::EndLeftButtonAction
//
//  Purpose:
//    Handles the left button being released.  For Navigate3D, this means
//    panning if the ctrl or shift button was held down while the left
//    button was pushed, a rotation otherwise.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Mar 18 13:48:55 PST 2002
//    Renamed from OnLeftButtonUp.
//
//    Hank Childs, Tue Mar 19 14:34:26 PST 2002
//    Pushed code for end bounding box into base class.
//
//    Hank Childs, Wed May 29 10:35:19 PDT 2002
//    Added support for 'spin' mode.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002
//    Removed arguments to match vtk's new interactor api. 
//
//    Eric Brugger, Thu Nov 20 15:24:48 PST 2003
//    Added code to call the view callback.
//
// ****************************************************************************

void
Navigate3D::EndLeftButtonAction()
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
//  Method: Navigate3D::StartMiddleButtonAction
//
//  Purpose:
//    Handles the middle button being pushed down.  For Navigate3D, this 
//    means zooming.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Mar 18 13:48:55 PST 2002
//    Renamed from OnMiddleButtonDown.
//
//    Hank Childs, Tue Mar 19 14:34:26 PST 2002
//    Pushed code for starting bounding box into base class.
//
//    Hank Childs, Wed May 29 10:35:19 PDT 2002
//    Added support for 'spin' mode.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002
//    Removed arguments to match vtk's new interactor api.
//
// ****************************************************************************

void
Navigate3D::StartMiddleButtonAction()
{
    DisableSpinMode();

    StartBoundingBox();

    StartZoom();
}


// ****************************************************************************
//  Method: Navigate3D::EndMiddleButtonAction
//
//  Purpose:
//    Handles the middle button being released.  For Navigate3D, this means
//    ending a zoom.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Mar 18 13:48:55 PST 2002
//    Renamed from OnMiddleButtonUp.
//
//    Hank Childs, Tue Mar 19 14:34:26 PST 2002
//    Pushed code for ending bounding box into base class.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002
//    Removed arguments to match vtk's new interactor api.
//
//    Eric Brugger, Thu Nov 20 15:24:48 PST 2003
//    Added code to call the view callback.
//
// ****************************************************************************

void
Navigate3D::EndMiddleButtonAction()
{
    EndZoom();

    EndBoundingBox();

    IssueViewCallback();
}


// ****************************************************************************
//  Method: Navigate3D::EnableSpinMode
//
//  Purpose:
//      Enables spin mode.  This will determine if spin mode is appropriate,
//      and make the correct calls to start it, if so.
//
//  Programmer: Hank Childs
//  Creation:   May 29, 2002
//
//  Modifications:
//
// ****************************************************************************

void
Navigate3D::EnableSpinMode(void)
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
//  Method: Navigate3D::DisableSpinMode
//
//  Purpose:
//      Disables spin mode if it is currently in action.  This may be called
//      at any time, even if spin mode is not currently on or even enabled.
//
//  Programmer: Hank Childs
//  Creation:   May 29, 2002
//
// ****************************************************************************

void
Navigate3D::DisableSpinMode(void)
{
    if (shouldSpin)
    {
        EndTimer();
        shouldSpin = false;
    }
}


// ****************************************************************************
//  Method: Navigate3D::RotateCamera
//
//  Purpose:
//    Handle rotating the camera.
//
//  Programmer: Eric Brugger
//  Creation:   August 10, 2001
//
//  Modifications:
//
//    Eric Brugger, Tue Apr  2 11:28:11 PST 2002
//    I modified the routine to make the viewing changes using an avtView3D.
//
//    Hank Childs, Wed May 29 10:50:12 PDT 2002
//    Initialized data members needed for determining which direction to spin.
//
//    Eric Brugger, Tue Feb 10 08:41:08 PST 2004
//    I modified the routine to rotate about the center of rotation if one
//    is specified.
//
// ****************************************************************************

void
Navigate3D::RotateCamera(const int x, const int y)
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

        unitX = (2.0 * x) / width - 1.0;
        unitY = 1.0 - (2.0 * y) / height;

        unitPrevX = (2.0 * OldX) / width - 1.0;
        unitPrevY = 1.0 - (2.0 * OldY) / height;

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
        // Calculate the translation to the center of rotation (and its
        // inverse).
        //
        avtMatrix matb, matbInverse;

        matb.MakeTranslate(-oldView3D.centerOfRotation[0],
                           -oldView3D.centerOfRotation[1],
                           -oldView3D.centerOfRotation[2]);
        matbInverse.MakeTranslate(oldView3D.centerOfRotation[0],
                                  oldView3D.centerOfRotation[1],
                                  oldView3D.centerOfRotation[2]);

        //
        // Calculate the composite transformation.
        //
        avtMatrix mat;

        mat = matbInverse * mataInverse * rot * mata * matb;

        //
        // Calculate the new view plane normal, view up vector and focus.
        //
        avtVector newVPN(mat ^ VPN);
        avtVector newVUP(mat ^ VUP);
        avtVector newFocus(mat * Focus);

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
        if (oldView3D.centerOfRotationSet)
        {
            newView3D.focus[0]  = newFocus.x;
            newView3D.focus[1]  = newFocus.y;
            newView3D.focus[2]  = newFocus.z;
        }

        vw->SetView3D(newView3D);

        OldX = x;
        OldY = y;
        rwi->Render();
    }
}

// ****************************************************************************
//  Method: Navigate3D::PanCamera
//
//  Purpose:
//    Handle panning the camera.
//
//  Programmer: Eric Brugger
//  Creation:   August 10, 2001
//
//  Modifications:
//    Eric Brugger, Tue Apr  2 11:28:11 PST 2002
//    I modified the routine to make the viewing changes using an avtView3D.
//
//    Eric Brugger, Wed Jun 11 08:55:07 PDT 2003
//    I changed the pan behavior so that it pans the image and doesn't
//    change the camera or focal point.
//
// ****************************************************************************

void
Navigate3D::PanCamera(const int x, const int y)
{
    vtkRenderWindowInteractor *rwi = Interactor;

    if ((OldX != x) || (OldY != y))
    {
        //
        // Determine the size of the window.
        //
        int       size[2];

        rwi->GetSize(size);

        //
        // Get the current view information.
        //
        VisWindow *vw = proxy;

        double    pan[2];

        avtView3D newView3D = vw->GetView3D();

        pan[0] = ((double)((x - OldX) / size[0])) / newView3D.imageZoom;
        pan[1] = ((double)((y - OldY) / size[1])) / newView3D.imageZoom;

        newView3D.imagePan[0] += pan[0];
        newView3D.imagePan[1] += pan[1];

        vw->SetView3D(newView3D);

        OldX = x;
        OldY = y;
        rwi->Render();
    }
}

// ****************************************************************************
//  Method: Navigate3D::ZoomCamera
//
//  Purpose:
//    Handle zooming the camera.
//
//  Programmer: Eric Brugger
//  Creation:   August 10, 2001
//
//  Modifications:
//    Hank Childs, Tue Feb 26 10:22:43 PST 2002
//    Do not let the near plane be set to something very close to the camera,
//    because the viewing transformation gets out of whack and we lose too
//    much in depth buffering.
//
//    Eric Brugger, Tue Apr  2 11:28:11 PST 2002
//    I modified the routine to make the viewing changes using an avtView3D.
//
//    Eric Brugger, Wed Jun 11 08:55:07 PDT 2003
//    I changed the zoom behavior so that it zooms the image and doesn't
//    change the camera or focal point.
//
// ****************************************************************************

void
Navigate3D::ZoomCamera(const int x, const int y)
{
    vtkRenderWindowInteractor *rwi = Interactor;

    if (OldY != y)
    {
        //
        // Calculate the zoom factor.
        //
        double dyf = MotionFactor * (double)(y - OldY) /
                         (double)(Center[1]);
        double zoomFactor = pow((double)1.1, dyf);

        //
        // Calculate the new parallel scale.
        //
        VisWindow *vw = proxy;

        avtView3D newView3D = vw->GetView3D();

        newView3D.imageZoom = newView3D.imageZoom * zoomFactor;

        vw->SetView3D(newView3D);

        OldX = x;
        OldY = y;
        rwi->Render();
    }
}
