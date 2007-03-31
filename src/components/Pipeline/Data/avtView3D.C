// ************************************************************************* //
//                                avtView3D.C                                //
// ************************************************************************* //

#include <avtView3D.h>

#include <math.h>

#include <avtViewInfo.h>
#include <View3DAttributes.h>

//
// Local macros.
//
#define max(x,y) ((x) > (y) ? (x) : (y))


// ****************************************************************************
//  Method: avtView3D constructor
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2001
//
// ****************************************************************************

avtView3D::avtView3D()
{
    SetToDefault();
}

// ****************************************************************************
//  Method: avtView3D operator =
//
//  Arguments:
//    vi        The view info to copy.
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2001
//
//  Modifications:
//    Eric Brugger, Fri Jun  6 15:24:17 PDT 2003
//    I added image pan and image zoom.
//
//    Hank Childs, Wed Oct 15 13:05:33 PDT 2003
//    Added eye angle.
//
// ****************************************************************************

avtView3D &
avtView3D::operator=(const avtView3D &vi)
{
    normal[0]     = vi.normal[0];
    normal[1]     = vi.normal[1];
    normal[2]     = vi.normal[2];
    focus[0]      = vi.focus[0];
    focus[1]      = vi.focus[1];
    focus[2]      = vi.focus[2];
    viewUp[0]     = vi.viewUp[0];
    viewUp[1]     = vi.viewUp[1];
    viewUp[2]     = vi.viewUp[2];
    viewAngle     = vi.viewAngle;
    parallelScale = vi.parallelScale;
    nearPlane     = vi.nearPlane;
    farPlane      = vi.farPlane;
    imagePan[0]   = vi.imagePan[0];
    imagePan[1]   = vi.imagePan[1];
    imageZoom     = vi.imageZoom;
    perspective   = vi.perspective;
    eyeAngle      = vi.eyeAngle;

    return *this;
}

// ****************************************************************************
//  Method: avtView3D operator ==
//
//  Arguments:
//    vi        The view info to compare to.
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2001
//
//  Modifications:
//    Eric Brugger, Fri Jun  6 15:24:17 PDT 2003
//    I added image pan and image zoom.
//
//    Hank Childs, Wed Oct 15 13:05:33 PDT 2003
//    Added eye angle.
//
// ****************************************************************************

bool
avtView3D::operator==(const avtView3D &vi)
{
    if (normal[0] != vi.normal[0] || normal[1] != vi.normal[1] ||
        normal[2] != vi.normal[2])
    {
        return false;
    }

    if (focus[0] != vi.focus[0] || focus[1] != vi.focus[1] ||
        focus[2] != vi.focus[2])
    {
        return false;
    }

    if (viewUp[0] != vi.viewUp[0] || viewUp[1] != vi.viewUp[1] ||
        viewUp[2] != vi.viewUp[2])
    {
        return false;
    }

    if (viewAngle != vi.viewAngle || parallelScale != vi.parallelScale ||
        nearPlane != vi.nearPlane || farPlane != vi.farPlane ||
        imagePan[0] != vi.imagePan[0] || imagePan[1] != vi.imagePan[1] ||
        imageZoom != vi.imageZoom || perspective != vi.perspective ||
        eyeAngle != vi.eyeAngle)
    {
        return false;
    }

    return true;
}

// ****************************************************************************
//  Method: avtView3D::SetToDefault
//
//  Purpose:
//    Sets up with default values.
//
//  Programmer:  Eric Brugger
//  Creation:    August 17, 2001
//
//  Modifications:
//    Eric Brugger, Fri Mar 29 15:12:41 PST 2002
//    I changed the default values for the near and far clipping planes
//    since I changed their definition to be the distance from the focus
//    instead of the camera.
//
//    Eric Brugger, Fri Jun  6 15:24:17 PDT 2003
//    I added image pan and image zoom.
//
//    Hank Childs, Wed Oct 15 13:05:33 PDT 2003
//    Added eye angle.
//
// ****************************************************************************

void
avtView3D::SetToDefault()
{
    normal[0]     = 0.;
    normal[1]     = 0.;
    normal[2]     = 1.;
    focus[0]      = 0.;
    focus[1]      = 0.;
    focus[2]      = 0.;
    viewUp[0]     = 0.;
    viewUp[1]     = 1.;
    viewUp[2]     = 0.;
    viewAngle     = 30.;
    parallelScale = 0.5;
    nearPlane     = -0.5;
    farPlane      =  0.5;
    imagePan[0]   = 0.;
    imagePan[1]   = 0.;
    imageZoom     = 1.;
    eyeAngle      = 2.0;
    perspective   = false;
}

// ****************************************************************************
//  Method: avtView3D::SetViewInfoFromView
//
//  Purpose:
//    Set the avtViewInfo, which is used to set the view within avt and
//    ultimately vtk, based on the 3D view.
//
//  Arguments:
//    viewInfo   The avtViewInfo in which to store the 3D view. 
//
//  Programmer:  Eric Brugger
//  Creation:    August 17, 2001
//
//  Modifications:
//    Eric Brugger, Fri Mar 29 15:12:41 PST 2002
//    I changed the definition of the near and far clipping planes to be
//    the distance from the focus instead of the camera.
//
//    Eric Brugger, Tue Apr  2 16:25:51 PST 2002
//    I added code to set a minimum value for the vtk near clipping distance.
//
//    Eric Brugger, Mon Apr 15 12:30:01 PDT 2002
//    I modified the setting of the minimum value for the vtk near clipping
//    distance to be adaptive based on the distance between the avt near and
//    far clipping distances.
//
//    Brad Whitlock, Tue May 7 14:50:54 PST 2002
//    Fixed a bug I introduced when porting to MS Windows.
//
//    Eric Brugger, Wed Jan  8 13:34:01 PST 2003
//    I modified the routine to normalize the normal before using it.
//
//    Eric Brugger, Fri Jun  6 15:24:17 PDT 2003
//    I added image pan and image zoom.
//
//    Hank Childs, Wed Oct 15 13:05:33 PDT 2003
//    Added eye angle.
//
// ****************************************************************************

void
avtView3D::SetViewInfoFromView(avtViewInfo &viewInfo) const
{
    double    distance;
    double    normal2[3];

    //
    // Calculate a unit length normal.
    //
    distance = sqrt(normal[0] * normal[0] + normal[1] * normal[1] +
                    normal[2] * normal[2]);
    distance = (distance != 0) ? distance : 1.;
    normal2[0] = normal[0] / distance;
    normal2[1] = normal[1] / distance;
    normal2[2] = normal[2] / distance;

    //
    // The view up vector and focal point are the same.  The distance from the
    // camera to the focal point can be calculated from the parallel scale and
    // view angle.  The camera position is then found by moving along the view
    // plane normal from the focal point by the distance.
    //
    viewInfo.viewUp[0] = viewUp[0];
    viewInfo.viewUp[1] = viewUp[1];
    viewInfo.viewUp[2] = viewUp[2];

    viewInfo.focus[0] = focus[0];
    viewInfo.focus[1] = focus[1];
    viewInfo.focus[2] = focus[2];

    viewInfo.eyeAngle = eyeAngle;

    distance = parallelScale / tan(viewAngle * 3.1415926535 / 360.);
    viewInfo.camera[0] = focus[0] + normal2[0] * distance;
    viewInfo.camera[1] = focus[1] + normal2[1] * distance;
    viewInfo.camera[2] = focus[2] + normal2[2] * distance;

    //
    // Orthographic is the opposite of perspective, setScale is always true.
    // It forces vtk to use the parallel scale.
    //
    viewInfo.orthographic  = !perspective;
    viewInfo.setScale      = true;
    viewInfo.parallelScale = parallelScale;
    viewInfo.viewAngle     = viewAngle;

    //
    // The minimum near clipping distance must be adaptive to make good use
    // of the zbuffer.  The distance between the near and far planes seemed
    // like a good choice, another possibility could have been the distance
    // between the camera and focus.  The 5000. is a magic number.  The
    // number should be as large as possible.  10000 would probably also
    // work, but 100000 would start showing z buffering artifacts.
    //
    viewInfo.nearPlane = max (nearPlane + distance, (farPlane - nearPlane) / 5000.);
    viewInfo.farPlane = farPlane + distance;

    //
    // Set the image pan and image zoom.
    //
    viewInfo.imagePan[0] = imagePan[0];
    viewInfo.imagePan[1] = imagePan[1];
    viewInfo.imageZoom   = imageZoom;
}

// ****************************************************************************
//  Method: avtView3D::SetFromView3DAttributes
//
//  Purpose: 
//    Sets the avtView3D from a View3DAttributes object.
//
//  Arguments:
//    view3DAtts : The View3DAttributes to use.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Jul 1 14:00:50 PST 2003
//
//  Modifications:
//    Eric Brugger, Wed Aug 20 09:39:11 PDT 2003
//    I renamed this routine.
//   
//    Hank Childs, Wed Oct 15 13:05:33 PDT 2003
//    Added eye angle.
//
// ****************************************************************************

void
avtView3D::SetFromView3DAttributes(const View3DAttributes *view3DAtts)
{
    for(int i = 0; i < 3; ++i)
    {
        normal[i] = view3DAtts->GetViewNormal()[i];
        focus[i]  = view3DAtts->GetFocus()[i];
        viewUp[i] = view3DAtts->GetViewUp()[i];
    }

    viewAngle = view3DAtts->GetViewAngle();
    parallelScale = view3DAtts->GetParallelScale();
    nearPlane = view3DAtts->GetNearPlane();
    farPlane = view3DAtts->GetFarPlane();
    imagePan[0] = view3DAtts->GetImagePan()[0];
    imagePan[1] = view3DAtts->GetImagePan()[1];
    imageZoom = view3DAtts->GetImageZoom();
    perspective = view3DAtts->GetPerspective();
    eyeAngle = view3DAtts->GetEyeAngle();
}

// ****************************************************************************
//  Method: avtView3D::SetToView3DAttributes
//
//  Purpose: 
//    Sets a View3DAttributes from the avtView3D.
//
//  Arguments:
//    view3DAtts : The View3DAttributes object to set.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Jul 1 14:01:23 PST 2003
//
//  Modifications:
//    Eric Brugger, Wed Aug 20 09:39:11 PDT 2003
//    I renamed this routine.
//   
//    Hank Childs, Wed Oct 15 13:05:33 PDT 2003
//    Added eye angle.
//
// ****************************************************************************

void
avtView3D::SetToView3DAttributes(View3DAttributes *view3DAtts) const
{
    view3DAtts->SetViewNormal(normal);
    view3DAtts->SetFocus(focus);
    view3DAtts->SetViewUp(viewUp);
    view3DAtts->SetViewAngle(viewAngle);
    view3DAtts->SetParallelScale(parallelScale);
    view3DAtts->SetNearPlane(nearPlane);
    view3DAtts->SetFarPlane(farPlane);
    view3DAtts->SetImagePan(imagePan);
    view3DAtts->SetImageZoom(imageZoom);
    view3DAtts->SetPerspective(perspective);
    view3DAtts->SetEyeAngle(eyeAngle);
}
