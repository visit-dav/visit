// ************************************************************************* //
//                                avtView2D.C                                //
// ************************************************************************* //

#include <avtView2D.h>

#include <avtViewInfo.h>
#include <View2DAttributes.h>

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
//    Eric Brugger, Wed Oct  8 16:45:35 PDT 2003
//    Replaced axisScaleFactor and axisScaleType with fullFrame.
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

    fullFrame    = vi.fullFrame;
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
//    Eric Brugger, Wed Oct  8 16:45:35 PDT 2003
//    Replaced axisScaleFactor and axisScaleType with fullFrame.
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
    if (fullFrame != vi.fullFrame)
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
//    Eric Brugger, Wed Oct  8 16:45:35 PDT 2003
//    Replaced axisScaleFactor and axisScaleType with fullFrame.
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
    fullFrame   = false;
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
//    Hank Childs, Wed May  7 08:14:33 PDT 2003
//    Account for degenerate situation that is hard to prevent.
//
//    Kathleen Bonnell, Wed Jul 16 16:46:02 PDT 2003 
//    Call ScaleWindow (effective only in full-frame mode). 
//
//    Akira Haddox, Wed Jul 16 16:50:49 PDT 2003
//    Force the clipping planes to be at least a certain distance away.
//
//    Eric Brugger, Wed Oct  8 15:19:48 PDT 2003
//    I removed Akira's previous change since it caused some problems.
//    Instead I Modified the routine to set the z camera position and near
//    and far clipping plane positions independent of the coordinate extents.
//
//    Eric Brugger, Wed Oct  8 16:45:35 PDT 2003
//    I added a window size argument so that the routine could handle
//    non-square windows and viewports.
//
//    Eric Brugger, Tue Nov 18 08:38:38 PST 2003
//    I replaced GetValidWindow with CheckAndCorrectWindow.
//
// ****************************************************************************

void
avtView2D::SetViewInfoFromView(avtViewInfo &viewInfo, int *size)
{
    CheckAndCorrectWindow();

    //
    // Handle full-frame mode if on.
    //
    double realWindow[4];

    realWindow[0] = window[0];
    realWindow[1] = window[1];
    if (fullFrame)
    {
        double    viewScale;

        viewScale = ((window[1] - window[0]) / (window[3] - window[2])) *
                    ((viewport[3] - viewport[2]) /
                     (viewport[1] - viewport[0])) *
                    ((double) size[1] / (double) size[0]) ;

        realWindow[2] = window[2] * viewScale;
        realWindow[3] = window[3] * viewScale;
    }
    else
    {
        realWindow[2] = window[2];
        realWindow[3] = window[3];
    }

    //
    //
    // Reset the view up vector, the focal point and the camera position.
    // The width is set based on the y window dimension.
    //
    double    width;

    width = realWindow[3] - realWindow[2];

    viewInfo.viewUp[0] = 0.;
    viewInfo.viewUp[1] = 1.;
    viewInfo.viewUp[2] = 0.;

    viewInfo.focus[0] = (realWindow[1] + realWindow[0]) / 2.;
    viewInfo.focus[1] = (realWindow[3] + realWindow[2]) / 2.;
    viewInfo.focus[2] = 0.;

    viewInfo.camera[0] = viewInfo.focus[0];
    viewInfo.camera[1] = viewInfo.focus[1];
    viewInfo.camera[2] = 1.;

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
    // Set the near and far clipping planes.  They are set independent of
    // the coordinate extents, since it doesn't matter.  Setting the values
    // too tight around the focus causes problems.
    //
    viewInfo.nearPlane = 0.5;
    viewInfo.farPlane  = 1.5;
}

// ****************************************************************************
//  Method: avtView2D::GetActualViewport
//
//  Purpose:
//    Get the actual viewport to use.  If in full frame mode then it is the
//    same as one set.  If not in full frame mode then it is based on the
//    size of the window so as to maintain a 1 to 1 aspect ratio yet maximize
//    the size of the viewport within the specified viewport.
//
//  Arguments:
//    winViewport  The actual viewport.
//    width      The width in pixels of the window.
//    height     The height in pixels of the window.
//
//  Programmer:  Eric Brugger
//  Creation:    August 17, 2001
//
//  Modifications:
//    Hank Childs, Wed May  7 08:25:48 PDT 2003
//    Always make sure we are dealing with a valid window.
//
//    Kathleen Bonnell, Wed Jul 16 16:46:02 PDT 2003 
//    Call ScaleWindow (effective only in full-frame mode). 
//
//    Eric Brugger, Wed Oct  8 16:45:35 PDT 2003
//    Modified to handle full frame mode properly.
//
//    Eric Brugger, Tue Nov 18 08:38:38 PST 2003
//    I replaced GetValidWindow with CheckAndCorrectWindow.
//
// ****************************************************************************

void
avtView2D::GetActualViewport(double *winViewport, const int width,
    const int height)
{

    if (fullFrame)
    {
        //
        //  Always use max viewport
        //
        winViewport[0] = viewport[0];
        winViewport[1] = viewport[1];
        winViewport[2] = viewport[2];
        winViewport[3] = viewport[3];
    }
    else
    {
        CheckAndCorrectWindow();

        double    viewportDX, viewportDY, viewportDXDY;
        double    windowDX, windowDY, windowDXDY;

        viewportDX = viewport[1] - viewport[0];
        viewportDY = viewport[3] - viewport[2];
        viewportDXDY = (viewportDX / viewportDY) *
            ((double) width / (double) height);

        windowDX = window[1] - window[0];
        windowDY = window[3] - window[2];
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
}

// ****************************************************************************
//  Method: avtView2D::GetScaleFactor
//
//  Purpose:
//    Gets the window's scale factor.
//
//  Arguments:
//    size      The size of the renderable area.
//
//  Programmer: Eric Brugger
//  Creation:   October 10, 2003
//
//  Modifications:
//    Eric Brugger, Tue Nov 18 08:38:38 PST 2003
//    I replaced GetValidWindow with CheckAndCorrectWindow.
//
// ****************************************************************************

double
avtView2D::GetScaleFactor(int *size)
{
    double s;

    if (fullFrame)
    {
        CheckAndCorrectWindow();

        double actualViewport[4];
        GetActualViewport(actualViewport, size[0], size[1]);

        s = ((window[1] - window[0]) / (window[3] - window[2])) *
            ((actualViewport[3] - actualViewport[2]) /
             (actualViewport[1] - actualViewport[0])) *
            ((double) size[1] / (double) size[0]);
    }
    else
    {
        s = 1.;
    }

    return s;
}

// ****************************************************************************
//  Method: avtView2D::SetFromView2DAttributes
//
//  Purpose: 
//    Sets the view from the View2DAttributes.
//
//  Arguments:
//    view2DAtts : The View2DAttributes to use.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Jul 1 14:05:11 PST 2003
//
//  Modifications:
//    Eric Brugger, Wed Aug 20 09:37:13 PDT 2003
//    I renamed this routine.
//   
//    Eric Brugger, Thu Oct 16 14:49:23 PDT 2003
//    I added fullFrame.
//
// ****************************************************************************

void
avtView2D::SetFromView2DAttributes(const View2DAttributes *view2DAtts)
{
    for(int i = 0; i < 4; ++i)
    {
        viewport[i] = view2DAtts->GetViewportCoords()[i];
        window[i] = view2DAtts->GetWindowCoords()[i];
    }
    fullFrame = view2DAtts->GetUseFullFrame();
}

// ****************************************************************************
//  Method: avtView2D::SetToView2DAttributes
//
//  Purpose: 
//    Sets a View2DAttributes from the avtView2D.
//
//  Arguments:
//    view2DAtts : The View2DAttributes object to set.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Jul 1 14:05:48 PST 2003
//
//  Modifications:
//    Eric Brugger, Wed Aug 20 09:37:13 PDT 2003
//    I renamed this routine.
//   
//    Eric Brugger, Thu Oct 16 14:49:23 PDT 2003
//    I added fullFrame.
//
// ****************************************************************************

void
avtView2D::SetToView2DAttributes(View2DAttributes *view2DAtts) const
{
    view2DAtts->SetWindowCoords(window);
    view2DAtts->SetViewportCoords(viewport);
    view2DAtts->SetUseFullFrame(fullFrame);
}

// ****************************************************************************
//  Method: avtView2D::CheckAndCorrectWindow
//
//  Purpose:
//    Checks the window parameters and corrects them if they are invalid.
//
//  Programmer: Eric Brugger
//  Creation:   November 18, 2003
//
// ****************************************************************************

void
avtView2D::CheckAndCorrectWindow()
{
    //
    // Account for degenerate views.
    //
    double width  = window[1] - window[0];
    double height = window[3] - window[2];
    if (width <= 0. && height <= 0.)
    {
        if (window[0] == 0. && window[2] == 0.)
        {
            window[0] = -1.;
            window[1] =  1.;
            window[2] = -1.;
            window[3] =  1.;
        }
        else if (window[0] == 0.)
        {
            window[0] -= window[2];
            window[1] += window[3];
            window[2] -= window[2];
            window[3] += window[3];
        }
        else if (window[2] == 0.)
        {
            window[2] -= window[0];
            window[3] += window[1];
            window[0] -= window[0];
            window[1] += window[1];
        }
        else
        {
            window[0] -= window[0];
            window[1] += window[1];
            window[2] -= window[2];
            window[3] += window[3];
        }
    }
    else if (width <= 0)
    {
        window[0] -= height / 2.;
        window[1] += height / 2.;
    }
    else if (height <= 0)
    {
        window[2] -= width / 2.;
        window[3] += width / 2.;
    }
}

