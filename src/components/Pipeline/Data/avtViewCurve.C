// ************************************************************************* //
//                             avtViewCurve.C                                //
// ************************************************************************* //

#include <avtViewCurve.h>

#include <avtViewInfo.h>
#include <DebugStream.h>
#include <ViewAttributes.h>

// ****************************************************************************
//  Method: avtViewCurve constructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 30, 2002
//
// ****************************************************************************

avtViewCurve::avtViewCurve()
{
    SetToDefault();
}

// ****************************************************************************
//  Method: avtViewCurve operator =
//
//  Arguments:
//    vi        The view info to copy.
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 30, 2002
//
// ****************************************************************************

avtViewCurve &
avtViewCurve::operator=(const avtViewCurve &vi)
{
    viewport[0]  = vi.viewport[0];
    viewport[1]  = vi.viewport[1];
    viewport[2]  = vi.viewport[2];
    viewport[3]  = vi.viewport[3];
    window[0]    = vi.window[0];
    window[1]    = vi.window[1];
    window[2]    = vi.window[2];
    window[3]    = vi.window[3];
    yScale       = vi.yScale;

    return *this;
}


// ****************************************************************************
//  Method: avtViewCurve operator ==
//
//  Arguments:
//    vi        The view info to compare to.
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 30, 2002
//
// ****************************************************************************

bool
avtViewCurve::operator==(const avtViewCurve &vi)
{
    if (viewport[0] != vi.viewport[0] || viewport[1] != vi.viewport[1] ||
        viewport[2] != vi.viewport[2] || viewport[3] != vi.viewport[3])
    {
        return false;
    }

    if (window[0] != vi.window[0] || window[1] != vi.window[1] ||
        window[2] != vi.window[2] || window[3] != vi.window[3])
    {
        return false;
    }
    if (yScale != vi.yScale)
    {
        return false;
    }


    return true;
}


// ****************************************************************************
//  Method: avtViewCurve::SetToDefault
//
//  Purpose:
//    Sets up with default values.
//
//  Programmer:  Kathleen Bonnell
//  Creation:    April 30, 2002
//
// ****************************************************************************

void
avtViewCurve::SetToDefault()
{
    viewport[0] = 0.;
    viewport[1] = 1.;
    viewport[2] = 0.;
    viewport[3] = 1.;
    window[0]   = 0.;
    window[1]   = 1.;
    window[2]   = 0.;
    window[3]   = 1.;
    yScale      = 1.;
}

// ****************************************************************************
//  Method: avtViewCurve::SetViewFromViewInfo
//
//  Purpose:
//    Set the view based on the the avtViewInfo, which is used to set the view
//    within avt and ultimately vtk.
//
//  Arguments:
//    viewInfo   The viewInfo from which to set the Curve view.
//
//  Notes:
//    Taken from avtView2D.
//
//  Programmer:  Kathleen Bonnell
//  Creation:    April 30, 2002
//
// ****************************************************************************

void
avtViewCurve::SetViewFromViewInfo(const avtViewInfo &viewInfo)
{
    //
    // Determine the new window.  We assume that the viewport stays the
    // same, since panning and zooming can not change the viewport.  We
    // assume that the window has the same aspect ratio as the previous
    // view since panning and zooming can not change the aspect ratio.
    // The parallel scale is set from the y window dimension.
    //
    double    oldHalfWidth, curHalfWidth;
    double    xcenter, ycenter;
    double    aspectRatio;
    double    scale;

    xcenter = viewInfo.focus[0];
    ycenter = viewInfo.focus[1];

    oldHalfWidth = (window[3] - window[2]) / 2.;
    curHalfWidth = viewInfo.parallelScale;
    aspectRatio = (window[1] - window[0]) / (window[3] - window[2]);
    scale = curHalfWidth / oldHalfWidth;
    window[0] = xcenter - oldHalfWidth * scale * aspectRatio;
    window[1] = xcenter + oldHalfWidth * scale * aspectRatio;
    window[2] = ycenter - oldHalfWidth * scale;
    window[3] = ycenter + oldHalfWidth * scale;
}

// ****************************************************************************
//  Method: avtViewCurve::SetViewInfoFromView
//
//  Purpose:
//    Set the avtViewInfo, which is used to set the view within avt and
//    ultimately vtk, based on the 3d view.
//
//  Arguments:
//    viewInfo   The avtViewInfo in which to store the Curve view. 
//
//  Notes:
//    Taken from avtView2D.
//
//  Programmer:  Kathleen Bonnell
//  Creation:    April 30, 2002
//
// ****************************************************************************

void
avtViewCurve::SetViewInfoFromView(avtViewInfo &viewInfo) const
{
    //
    // Reset the view up vector, the focal point and the camera position.
    // The width is set based on the y window dimension.
    //
    double    width;

    width = window[3] - window[2];

    viewInfo.viewUp[0] = 0.;
    viewInfo.viewUp[1] = 1.;
    viewInfo.viewUp[2] = 0.;

    viewInfo.focus[0] = (window[1] + window[0]) / 2.;
    viewInfo.focus[1] = (window[3] + window[2]) / 2.;
    viewInfo.focus[2] = 0.;

    viewInfo.camera[0] = viewInfo.focus[0];
    viewInfo.camera[1] = viewInfo.focus[1];
    viewInfo.camera[2] = width / 2.;

    //
    // Set the projection mode, parallel scale and view angle.  The
    // projection mode is always parallel for Curve.  The parallel scale is
    // off.
    //
    viewInfo.orthographic = true;
    viewInfo.setScale = true;
    viewInfo.parallelScale = width / 2.;
    viewInfo.viewAngle = 30.;

    //
    // Calculate the near and far clipping planes.  These clipping planes
    // should match what vtk would generate.
    //
    viewInfo.nearPlane = width / 4.;
    viewInfo.farPlane  = 3. * width / 4.;
}

// ****************************************************************************
//  Method: avtViewCurve::SetViewportFromView
//
//  Purpose:
//    Calculate the viewport to use based on the size of the window so as
//    to maintain a 1 to 1 aspect ratio yet maximize the size of the viewport
//    within the specified viewport.
//
//  Arguments:
//    winViewport  The viewport modified to take into account the window.
//    <unused>     The width in pixels of the window.
//    <unused>     The height in pixels of the window.
//
//  Programmer:  Kathleen Bonnell
//  Creation:    April 30, 2002
//
// ****************************************************************************

void
avtViewCurve::SetViewportFromView(double *winViewport, const int, const int) const
{
    //
    //  Always use max viewport
    //
    winViewport[0] = viewport[0];
    winViewport[1] = viewport[1];
    winViewport[2] = viewport[2];
    winViewport[3] = viewport[3];
}

// ****************************************************************************
// Method: avtViewCurve::SetFromViewAttributes
//
// Purpose: 
//   Sets the avtCurveView from the ViewAttributes object.
//
// Arguments:
//   viewAtts : A pointer to the ViewAttributes object to use.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 1 13:00:39 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
avtViewCurve::SetFromViewAttributes(const ViewAttributes *viewAtts)
{
    for(int i = 0; i < 4; ++i)
    {
        window[i] = viewAtts->GetWindowCoords()[i];
        viewport[i] = viewAtts->GetViewportCoords()[i];
    }
}

// ****************************************************************************
// Method: avtViewCurve::SetToViewAttributes
//
// Purpose: 
//   Sets the ViewAttributes from the avtView2D object.
//
// Arguments:
//   viewAtts : A pointer to the ViewAttributes object to use.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 1 13:00:39 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
avtViewCurve::SetToViewAttributes(ViewAttributes *viewAtts) const
{
    viewAtts->SetWindowCoords(window);
    viewAtts->SetViewportCoords(viewport);
}
