// ************************************************************************* //
//                                Navigate3D.C                               //
// ************************************************************************* //

#include <Navigate3D.h>

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
        if (vw->GetSpinMode() == true)
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
// ****************************************************************************

void
Navigate3D::EndMiddleButtonAction()
{
    EndZoom();

    EndBoundingBox();
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
        double    q[4];
        double    rot[3][3];

        BuildQuaternion(q, unitPrevX, unitPrevY, unitX, unitY);
        q[2] = -q[2];
        MatrixFromQuaternion(rot, q);

        //
        // Get the current view information.
        //
        VisWindow *vw = proxy;

        const avtView3D &oldView3D = vw->GetView3D();

        double    VPN[3];
        double    VUP[3];

        VPN[0] = oldView3D.normal[0];
        VPN[1] = oldView3D.normal[1];
        VPN[2] = oldView3D.normal[2];
        VUP[0] = oldView3D.viewUp[0];
        VUP[1] = oldView3D.viewUp[1];
        VUP[2] = oldView3D.viewUp[2];

        //
        // Calculate the coordinate transformation from the world coordinate
        // space to the screen coordinate space (and its inverse).
        //
        double    VPNCrossVUP[3];
        double    mat[3][3], matTranspose[3][3];

        VectorCross(VPN, VUP, VPNCrossVUP);

        MatrixSet(mat, VPNCrossVUP, VUP, VPN);
        MatrixTranspose(mat, matTranspose);

        //
        // Calculate the new view plane normal.
        //
        double    vecTemp[3], vecTemp2[3];
        double    newVPN[3];

        VectorMatrixMult(VPN, mat, vecTemp);
        VectorMatrixMult(vecTemp, rot, vecTemp2);
        VectorMatrixMult(vecTemp2, matTranspose, newVPN);

        //
        // Calculate the new view up vector.
        //
        double    newVUP[3];

        VectorMatrixMult(VUP, mat, vecTemp);
        VectorMatrixMult(vecTemp, rot, vecTemp2);
        VectorMatrixMult(vecTemp2, matTranspose, newVUP);

        //
        // Orthogonalize the new view plane normal and view up vector.
        //
        VectorScale(newVPN, 1.0 / VectorLength(newVPN));
        VectorCross(newVPN, newVUP, VPNCrossVUP);
        VectorCross(VPNCrossVUP, newVPN, newVUP);
        VectorScale(newVUP, 1.0 / VectorLength(newVUP));

        //
        // Set the new view.
        //
        avtView3D newView3D = vw->GetView3D();

        newView3D.normal[0] = newVPN[0];
        newView3D.normal[1] = newVPN[1];
        newView3D.normal[2] = newVPN[2];
        newView3D.viewUp[0] = newVUP[0];
        newView3D.viewUp[1] = newVUP[1];
        newView3D.viewUp[2] = newVUP[2];

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


//
// Use a Witch of Agnesi for a trackball.  The default is a sphere
// unioned with a plane.
//
#define AGNESI

//
// This size should really be based on the distance from the center
// of rotation to the point on the object underneath the mouse.  That
// point would then track the mouse as closely as possible.
//
#define AGNESI_RADIUS   0.8     // Z value at x = y = 0.0
#define COMPRESSION     3.5     // Multipliers for x and y
#define SPHERE_RADIUS   1.0     // Radius of sphere embedded in plane

#define AR3 (AGNESI_RADIUS*AGNESI_RADIUS*AGNESI_RADIUS)

// ****************************************************************************
//  Method: Navigate3D::BuildQuaternion
//
//  Purpose:
//    Build a quaterion from mouse motion using a virtual trackball.
//
//  Arguments:
//    q        The rotation quaternion.
//    p1x      The x coordinate of the first mouse position.
//    p1y      The y coordinate of the first mouse position.
//    p2x      The x coordinate of the second mouse position.
//    p2y      The y coordinate of the second mouse position.
//
//  Notes:
//    Project the mouse points onto the virtual trackball, then
//    figure out the axis of rotation which is the cross product of
//    the two mouse positions projected onto the trackball.
//
//  Programmer: Robb Matzke
//  Creation:   May 1992
//
// ****************************************************************************

void
Navigate3D::BuildQuaternion(double q[4], const double p1x, const double p1y,
   const double p2x, const double p2y) const
{
    double    p1[3], p2[3];
    double    d[3], t;
    double    phi;

    //
    // Check for zero mouse movement.
    //
    if (p1x == p2x && p1y == p2y)
    {
        q[0] = 0.0; q[1] = 0.0; q[2] = 0.0; q[3] = 0.0;
        return;
    }

    //
    // Compute z-coordinates for projection of P1 and P2 onto
    // the trackball.
    //
    VectorSet(p1, p1x, p1y, ProjectToSphere(p1x, p1y));
    VectorSet(p2, p2x, p2y, ProjectToSphere(p2x, p2y));

    //
    // Compute the axis of rotation and temporarily store it
    // in the quaternion.
    //
    VectorCross(p2, p1, (double*)q);
    VectorScale(q, 1.0 / VectorLength(q));

    //
    // Figure how much to rotate around that axis.
    //
    VectorSubtract(p2, p1, d);
    t = VectorLength(d);
    if (t > 1.0)
        t = 1.0;
    else if (t < -1.0)
        t = -1.0;
#ifdef AGNESI
    phi = 2.0 * asin (t / (2.0 * AGNESI_RADIUS));
#else
    phi = 2.0 * asin (t / (2.0 * SPHERE_RADIUS));
#endif
    VectorScale(q, sin((double) phi / 2.0));
    q[3] = cos((double) phi / 2.0);

    //
    // Normalize quaternion to unit magnitude.
    //
    t = 1.0 / sqrt(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
    q[0] *= t; q[1] *= t; q[2] *= t; q[3] *= t;
}

// ****************************************************************************
//  Method: Navigate3D::ProjectToSphere
//
//  Purpose:
//    Given an (x,y) coordinate in the viewport, calculate the z-coordinate.
//    If AGNESI is defined, then use a Witch of Agnesi as the trackball.
//    Otherwise, use the "cap" of a sphere and the x-y plane.
//
//  Arguments:
//    x        The x coordinate of the pointer position.
//    y        The y coordinate of the pointer position.
//
//  Returns:    The z-coordinate.
//
//  Programmer: Robb Matzke
//  Creation:   May 1992
//
// ****************************************************************************

double
Navigate3D::ProjectToSphere(const double x, const double y) const
{
    double z;

#ifdef AGNESI
    z = (double) AR3 / ((x * x + y * y) * COMPRESSION + AR3);
#else
    double t;

    t = SPHERE_RADIUS*SPHERE_RADIUS - x * x - y * y;
    if ( t >= 0.0)
    {
        z = sqrt ((double)t) - SPHERE_RADIUS / 1.4;
        if (z < 0.0) z = 0.0;
    }
    else
    {
        z = 0.0;
    }
#endif

   return z;
}

// ****************************************************************************
//  Method: Navigate3D::MatrixFromQuaternion
//
//  Purpose:
//    Convert a quaternion to a 3x3 transformation matrix.
//
//  Arguments:
//    m        The transformation matrix.
//    q        The quaternion.
//
//  Programmer: Robb Matzke
//  Creation:   May 1992
//
// ****************************************************************************

void
Navigate3D::MatrixFromQuaternion(double m[3][3], const double q[4]) const
{
    m[0][0] = 1.0 - 2.0 * (q[1] * q[1] + q[2] * q[2]);
    m[0][1] = 2.0 * (q[0] * q[1] - q[2] * q[3]);
    m[0][2] = 2.0 * (q[2] * q[0] + q[1] * q[3]);

    m[1][0] = 2.0 * (q[0] * q[1] + q[2] * q[3]);
    m[1][1] = 1.0 - 2.0 * (q[2] * q[2] + q[0] * q[0]);
    m[1][2] = 2.0 * (q[1] * q[2] - q[0] * q[3]);

    m[2][0] = 2.0 * (q[2] * q[0] - q[1] * q[3]);
    m[2][1] = 2.0 * (q[1] * q[2] + q[0] * q[3]);
    m[2][2] = 1.0 - 2.0 * (q[1] * q[1] + q[0] * q[0]);
}
