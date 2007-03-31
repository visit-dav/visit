// ************************************************************************* //
//                                avtView2D.C                                //
// ************************************************************************* //

#include <avtView2D.h>

#include <avtViewInfo.h>
#include <ViewAttributes.h>

// ****************************************************************************
//  Method: avtView2D constructor
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2001
//
// ****************************************************************************

avtView2D::avtView2D()
{
    SetToDefault();
}

// ****************************************************************************
//  Method: avtView2D operator =
//
//  Arguments:
//    vi        The view info to copy.
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2001
//
//  Modifications:
//    Kathleen Bonnell, Thu May 15 09:46:46 PDT 2003 
//    Copy axisScaleFactor and axisScaleType.
//
// ****************************************************************************

avtView2D &
avtView2D::operator=(const avtView2D &vi)
{
    viewport[0]  = vi.viewport[0];
    viewport[1]  = vi.viewport[1];
    viewport[2]  = vi.viewport[2];
    viewport[3]  = vi.viewport[3];
    window[0]    = vi.window[0];
    window[1]    = vi.window[1];
    window[2]    = vi.window[2];
    window[3]    = vi.window[3];

    axisScaleFactor = vi.axisScaleFactor;
    axisScaleType   = vi.axisScaleType;
    return *this;
}


// ****************************************************************************
//  Method: avtView2D operator ==
//
//  Arguments:
//    vi        The view info to compare to.
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2001
//
//  Modifications:
//    Kathleen Bonnell, Thu May 15 09:46:46 PDT 2003 
//    Compare axisScaleFactor and axisScaleType.
//
// ****************************************************************************

bool
avtView2D::operator==(const avtView2D &vi)
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
    if (axisScaleFactor != vi.axisScaleFactor)
    {
        return false;
    }
    if (axisScaleType != vi.axisScaleType)
    {
        return false;
    }

    return true;
}


// ****************************************************************************
//  Method: avtView2D::SetToDefault
//
//  Purpose:
//    Sets up with default values.
//
//  Programmer:  Eric Brugger
//  Creation:    August 17, 2001
//
//  Modifications:
//    Kathleen Bonnell, Thu May 15 09:46:46 PDT 2003 
//    Initialize axisScaleFactor and axisScaleType.
//
// ****************************************************************************

void
avtView2D::SetToDefault()
{
    viewport[0] = 0.;
    viewport[1] = 1.;
    viewport[2] = 0.;
    viewport[3] = 1.;
    window[0]   = 0.;
    window[1]   = 1.;
    window[2]   = 0.;
    window[3]   = 1.;
    axisScaleFactor = 0.; // no scaling will take place
    axisScaleType = 1; // y_axis
}

// ****************************************************************************
//  Method: avtView2D::SetViewFromViewInfo
//
//  Purpose:
//    Set the view based on the the avtViewInfo, which is used to set the view
//    within avt and ultimately vtk.
//
//  Arguments:
//    viewInfo   The viewInfo from which to set the 2D view.
//
//  Programmer:  Eric Brugger
//  Creation:    August 17, 2001
//
//  Modifications:
//
//    Hank Childs, Mon May  5 13:29:51 PDT 2003
//    Account for degenerate situation that is hard to prevent.
//
// ****************************************************************************

void
avtView2D::SetViewFromViewInfo(const avtViewInfo &viewInfo)
{
    double valid_window[4];
    GetValidWindow(valid_window);

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

    oldHalfWidth = (valid_window[3] - valid_window[2]) / 2.;
    curHalfWidth = viewInfo.parallelScale;
    aspectRatio = (valid_window[1] - valid_window[0]) /
                  (valid_window[3] - valid_window[2]);
    scale = curHalfWidth / oldHalfWidth;
    if (scale <= 0.)
        scale = oldHalfWidth;  // We fixed any problems with this value above.
    window[0] = xcenter - oldHalfWidth * scale * aspectRatio;
    window[1] = xcenter + oldHalfWidth * scale * aspectRatio;
    window[2] = ycenter - oldHalfWidth * scale;
    window[3] = ycenter + oldHalfWidth * scale;
}

// ****************************************************************************
//  Method: avtView2D::SetViewInfoFromView
//
//  Purpose:
//    Set the avtViewInfo, which is used to set the view within avt and
//    ultimately vtk, based on the 3d view.
//
//  Arguments:
//    viewInfo   The avtViewInfo in which to store the 2D view. 
//
//  Programmer:  Eric Brugger
//  Creation:    August 17, 2001
//
//  Modifications:
//
//    Hank Childs, Wed May  7 08:14:33 PDT 2003
//    Account for degenerate situation that is hard to prevent.
//
// ****************************************************************************

void
avtView2D::SetViewInfoFromView(avtViewInfo &viewInfo) const
{
    double valid_window[4];
    GetValidWindow(valid_window);

    //
    //
    // Reset the view up vector, the focal point and the camera position.
    // The width is set based on the y window dimension.
    //
    double    width;

    width = valid_window[3] - valid_window[2];

    viewInfo.viewUp[0] = 0.;
    viewInfo.viewUp[1] = 1.;
    viewInfo.viewUp[2] = 0.;

    viewInfo.focus[0] = (valid_window[1] + valid_window[0]) / 2.;
    viewInfo.focus[1] = (valid_window[3] + valid_window[2]) / 2.;
    viewInfo.focus[2] = 0.;

    viewInfo.camera[0] = viewInfo.focus[0];
    viewInfo.camera[1] = viewInfo.focus[1];
    viewInfo.camera[2] = width / 2.;

    //
    // Set the projection mode, parallel scale and view angle.  The
    // projection mode is always parallel for 2D.  The parallel scale is
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
//  Method: avtView2D::SetViewportFromView
//
//  Purpose:
//    Calculate the viewport to use based on the size of the window so as
//    to maintain a 1 to 1 aspect ratio yet maximize the size of the viewport
//    within the specified viewport.
//
//  Arguments:
//    winViewport  The viewport modified to take into account the window.
//    width      The width in pixels of the window.
//    height     The height in pixels of the window.
//
//  Programmer:  Eric Brugger
//  Creation:    August 17, 2001
//
//  Modifications:
//
//    Hank Childs, Wed May  7 08:25:48 PDT 2003
//    Always make sure we are dealing with a valid window.
//
// ****************************************************************************

void
avtView2D::SetViewportFromView(double *winViewport, const int width,
    const int height) const
{
    double valid_window[4];
    GetValidWindow(valid_window);

    double    viewportDX, viewportDY, viewportDXDY;
    double    windowDX, windowDY, windowDXDY;

    viewportDX = viewport[1] - viewport[0];
    viewportDY = viewport[3] - viewport[2];
    viewportDXDY = (viewportDX / viewportDY) *
        ((double) width / (double) height);

    windowDX = valid_window[1] - valid_window[0];
    windowDY = valid_window[3] - valid_window[2];
    windowDXDY = windowDX / windowDY;

    if ((viewportDXDY >= 1. && viewportDXDY <= windowDXDY) ||
        (viewportDXDY <  1. && viewportDXDY <  windowDXDY))
    {
        //
        // Max out the width.
        //
        winViewport[0] = viewport[0];
        winViewport[1] = viewport[1];
        winViewport[2] = viewport[2];
        winViewport[3] = viewport[2] + (viewport[3] - viewport[2]) *
                                       (viewportDXDY / windowDXDY);
    }
    else
    {
        //
        // Max out the height.
        //
        winViewport[0] = viewport[0];
        winViewport[1] = viewport[0] + (viewport[1] - viewport[0]) *
                                       (windowDXDY / viewportDXDY);
        winViewport[2] = viewport[2];
        winViewport[3] = viewport[3];
    }
}

// ****************************************************************************
//  Method: avtView2D::GetValidWindow
//
//  Purpose:
//      Gets the window parameters and makes sure that they are valid
//      (meaning width and height are both positive).
//
//  Programmer: Hank Childs
//  Creation:   May 7, 2003
//
// ****************************************************************************

void
avtView2D::GetValidWindow(double *valid_window) const
{
    //
    // Copy over the original window.
    //
    valid_window[0] = window[0];
    valid_window[1] = window[1];
    valid_window[2] = window[2];
    valid_window[3] = window[3];

    //
    // Account for degenerate views.
    //
    double width  = valid_window[1] - valid_window[0];
    double height = valid_window[3] - valid_window[2];
    if (width <= 0. && height <= 0.)
    {
        valid_window[1] = valid_window[0] + 1.;
        valid_window[3] = valid_window[2] + 1.;
    }
    else if (width <= 0)
    {
        valid_window[1] = valid_window[0] + height;
    }
    else if (height <= 0)
    {
        valid_window[3] = valid_window[2] + width;
    }
}

// ****************************************************************************
// Method: avtView2D::SetFromViewAttributes
//
// Purpose: 
//   Sets the view from the ViewAttributes.
//
// Arguments:
//   viewAtts : The ViewAttributes to use.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 1 14:05:11 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
avtView2D::SetFromViewAttributes(const ViewAttributes *viewAtts)
{
    for(int i = 0; i < 4; ++i)
    {
        viewport[i] = viewAtts->GetViewportCoords()[i];
        window[i] = viewAtts->GetWindowCoords()[i];
    }
}

// ****************************************************************************
// Method: avtView2D::SetToViewAttributes
//
// Purpose: 
//   Sets a ViewAttributes from the avtView2D.
//
// Arguments:
//   viewAtts : The ViewAttributes object to set.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 1 14:05:48 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
avtView2D::SetToViewAttributes(ViewAttributes *viewAtts) const
{
    viewAtts->SetWindowCoords(window);
    viewAtts->SetViewportCoords(viewport);
}

