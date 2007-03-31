// ************************************************************************* //
//                             avtViewCurve.C                                //
// ************************************************************************* //

#include <avtViewCurve.h>

#include <avtViewInfo.h>
#include <DebugStream.h>
#include <ViewCurveAttributes.h>

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
//  Modifications:
//    Eric Brugger, Wed Aug 20 09:42:38 PDT 2003
//    I removed yScale and replaced window with domain and range.
//
// ****************************************************************************

avtViewCurve &
avtViewCurve::operator=(const avtViewCurve &vi)
{
    viewport[0]  = vi.viewport[0];
    viewport[1]  = vi.viewport[1];
    viewport[2]  = vi.viewport[2];
    viewport[3]  = vi.viewport[3];
    domain[0]    = vi.domain[0];
    domain[1]    = vi.domain[1];
    range[0]     = vi.range[0];
    range[1]     = vi.range[1];

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
//  Modifications:
//    Eric Brugger, Wed Aug 20 09:42:38 PDT 2003
//    I removed yScale and replaced window with domain and range.
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

    if (domain[0] != vi.domain[0] || domain[1] != vi.domain[1])
    {
        return false;
    }

    if (range[0] != vi.range[0] || range[1] != vi.range[1])
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
//  Modifications:
//    Eric Brugger, Wed Aug 20 09:42:38 PDT 2003
//    I removed yScale and replaced window with domain and range.
//
// ****************************************************************************

void
avtViewCurve::SetToDefault()
{
    viewport[0] = 0.;
    viewport[1] = 1.;
    viewport[2] = 0.;
    viewport[3] = 1.;
    domain[0]   = 0.;
    domain[1]   = 1.;
    range[0]    = 0.;
    range[1]    = 1.;
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
//    size       The size of the window.
//
//  Notes:
//    Taken from avtView2D.
//
//  Programmer:  Kathleen Bonnell
//  Creation:    April 30, 2002
//
//  Modifications:
//    Eric Brugger, Wed Aug 20 09:42:38 PDT 2003
//    I replaced window with domain and range.  I added a window size argument
//    so that the routine could handle non-square windows and viewports.
//
// ****************************************************************************

void
avtViewCurve::SetViewFromViewInfo(const avtViewInfo &viewInfo, int *size)
{
    //
    // Determine the new window.  We assume that the viewport stays the
    // same, since panning and zooming can not change the viewport.  We
    // assume that the window has the same aspect ratio as the previous
    // view since panning and zooming can not change the aspect ratio.
    // The parallel scale is set from the y window dimension.  The viewScale
    // handles non-square viewports and windows.  It is the opposite of
    // SetViewInfoFromView.
    //
    double    viewScale;
    double    xcenter, ycenter;
    double    oldHalfWidth, curHalfWidth;
    double    aspectRatio;
    double    scale;

    viewScale = ((range[1] - range[0]) / (domain[1] - domain[0])) *
                ((viewport[1] - viewport[0]) / (viewport[3] - viewport[2])) *
                ((double) size[0] / (double) size[1]) ;

    xcenter = viewInfo.focus[0];
    ycenter = viewInfo.focus[1] * viewScale;

    oldHalfWidth = (range[1] - range[0]) / 2.;
    curHalfWidth = viewInfo.parallelScale * viewScale;
    aspectRatio = (domain[1] - domain[0]) / (range[1] - range[0]);
    scale = curHalfWidth / oldHalfWidth;
    domain[0] = xcenter - oldHalfWidth * scale * aspectRatio;
    domain[1] = xcenter + oldHalfWidth * scale * aspectRatio;
    range[0]  = ycenter - oldHalfWidth * scale;
    range[1]  = ycenter + oldHalfWidth * scale;
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
//    size       The size of the window.
//
//  Notes:
//    Taken from avtView2D.
//
//  Programmer:  Kathleen Bonnell
//  Creation:    April 30, 2002
//
//  Modifications:
//    Eric Brugger, Wed Aug 20 09:42:38 PDT 2003
//    I replaced window with domain and range.  I added a window size argument
//    so that the routine could handle non-square windows and viewports.
//
// ****************************************************************************

void
avtViewCurve::SetViewInfoFromView(avtViewInfo &viewInfo, int *size) const
{
    //
    // Calculate a new range so that we get a 1 to 1 aspect ration.
    //
    double    viewScale;
    double    range2[2];

    viewScale = ((domain[1] - domain[0]) / (range[1] - range[0])) *
                ((viewport[3] - viewport[2]) / (viewport[1] - viewport[0])) *
                ((double) size[1] / (double) size[0]) ;

    range2[0] = range[0] * viewScale;
    range2[1] = range[1] * viewScale;

    //
    // Reset the view up vector, the focal point and the camera position.
    // The width is set based on the y window dimension.
    //
    double    width;

    width = range2[1] - range2[0];

    viewInfo.viewUp[0] = 0.;
    viewInfo.viewUp[1] = 1.;
    viewInfo.viewUp[2] = 0.;

    viewInfo.focus[0] = (domain[1] + domain[0]) / 2.;
    viewInfo.focus[1] = (range2[1] + range2[0]) / 2.;
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
//  Method: avtViewCurve::SetFromViewCurveAttributes
//
//  Purpose: 
//    Sets the avtCurveView from the ViewCurveAttributes object.
//
//  Arguments:
//    viewAtts : A pointer to the ViewCurveAttributes object to use.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Jul 1 13:00:39 PST 2003
//
//  Modifications:
//    Eric Brugger, Wed Aug 20 09:42:38 PDT 2003
//    I renamed this method.  I replaced window with domain and range.
//   
// ****************************************************************************

void
avtViewCurve::SetFromViewCurveAttributes(const ViewCurveAttributes *viewAtts)
{
    for(int i = 0; i < 4; ++i)
    {
        viewport[i] = viewAtts->GetViewportCoords()[i];
    }
    domain[0] = viewAtts->GetDomainCoords()[0];
    domain[1] = viewAtts->GetDomainCoords()[1];
    range[0]  = viewAtts->GetRangeCoords()[0];
    range[1]  = viewAtts->GetRangeCoords()[1];
}

// ****************************************************************************
//  Method: avtViewCurve::SetToViewCurveAttributes
//
//  Purpose: 
//    Sets the ViewCurveAttributes from the avtViewCurve object.
//
//  Arguments:
//    viewAtts : A pointer to the ViewCurveAttributes object to use.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Jul 1 13:00:39 PST 2003
//
//  Modifications:
//    Eric Brugger, Wed Aug 20 09:42:38 PDT 2003
//    I renamed this method.  I replaced window with domain and range.
//   
// ****************************************************************************

void
avtViewCurve::SetToViewCurveAttributes(ViewCurveAttributes *viewAtts) const
{
    viewAtts->SetViewportCoords(viewport);
    viewAtts->SetDomainCoords(domain);
    viewAtts->SetRangeCoords(range);
}
