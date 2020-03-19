// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtViewAxisArray.C                                //
// ************************************************************************* //

#include <avtViewAxisArray.h>

#include <avtViewInfo.h>
#include <DebugStream.h>
#include <ViewAxisArrayAttributes.h>


// ****************************************************************************
//  Method: avtViewAxisArray constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
// ****************************************************************************

avtViewAxisArray::avtViewAxisArray()
{
    SetToDefault();
}

// ****************************************************************************
//  Method: avtViewAxisArray operator =
//
//  Arguments:
//    vi        The view info to copy.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Modifications:
//
// ****************************************************************************

avtViewAxisArray &
avtViewAxisArray::operator=(const avtViewAxisArray &vi)
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
//  Method: avtViewAxisArray operator ==
//
//  Arguments:
//    vi        The view info to compare to.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Modifications:
//
// ****************************************************************************

bool
avtViewAxisArray::operator==(const avtViewAxisArray &vi)
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
//  Method: avtViewAxisArray::SetToDefault
//
//  Purpose:
//    Sets up with default values.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Modifications:
//    Jeremy Meredith, Mon Feb  4 13:29:03 EST 2008
//    Set a more reasonable default for the viewport (and one that
//    matches the defaults in the attributes).
//
// ****************************************************************************

void
avtViewAxisArray::SetToDefault()
{
    viewport[0] = 0.15;
    viewport[1] = 0.9;
    viewport[2] = 0.10;
    viewport[3] = 0.85;
    domain[0]   = 0.;
    domain[1]   = 1.;
    range[0]    = 0.;
    range[1]    = 1.;
}

// ****************************************************************************
//  Method: avtViewAxisArray::SetViewInfoFromView
//
//  Purpose:
//    Set the avtViewInfo, which is used to set the view within avt and
//    ultimately vtk, based on the 3d view.
//
//  Arguments:
//    viewInfo   The avtViewInfo in which to store the AxisArray view. 
//    size       The size of the window.
//
//  Notes:
//    Taken from avtView2D.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Modifications:
//    Eric Brugger, Mon Nov  5 15:14:46 PST 2012
//    I added the ability to display the parallel axes either horizontally
//    or vertically.
//
// ****************************************************************************

void
avtViewAxisArray::SetViewInfoFromView(avtViewInfo &viewInfo,
    bool swapDomainRange, int *size)
{
    CheckAndCorrectDomainRange();

    //
    // Calculate a new range so that we get a 1 to 1 aspect ration.
    //
    double    realRange[2];

    if (swapDomainRange)
        viewScale = ((domain[1] - domain[0]) / (range[1] -  range[0])) *
            ((viewport[1] - viewport[0]) / (viewport[3] - viewport[2])) *
            ((double) size[0] / (double) size[1]) ;
    else
        viewScale = ((domain[1] - domain[0]) / (range[1] -  range[0])) *
            ((viewport[3] - viewport[2]) / (viewport[1] - viewport[0])) *
            ((double) size[1] / (double) size[0]) ;

    realRange[0] = range[0] * viewScale;
    realRange[1] = range[1] * viewScale;

    //
    // Reset the view up vector, the focal point and the camera position.
    // The width is set based on the y window dimension.
    //
    double    width;

    if (swapDomainRange)
        width = domain[1] - domain[0];
    else
        width = realRange[1] - realRange[0];

    if (swapDomainRange)
    {
        viewInfo.viewUp[0] = 1.;
        viewInfo.viewUp[1] = 0.;
        viewInfo.viewUp[2] = 0.;
    }
    else
    {
        viewInfo.viewUp[0] = 0.;
        viewInfo.viewUp[1] = 1.;
        viewInfo.viewUp[2] = 0.;
    }

    viewInfo.focus[0] = (domain[1] + domain[0]) / 2.;
    viewInfo.focus[1] = (realRange[1]  + realRange[0]) / 2.;
    viewInfo.focus[2] = 0.;

    if (swapDomainRange)
    {
        viewInfo.camera[0] = viewInfo.focus[0];
        viewInfo.camera[1] = viewInfo.focus[1];
        viewInfo.camera[2] = -1.;
    }
    else
    {
        viewInfo.camera[0] = viewInfo.focus[0];
        viewInfo.camera[1] = viewInfo.focus[1];
        viewInfo.camera[2] = 1.;
    }

    //
    // Set the projection mode, parallel scale and view angle.  The
    // projection mode is always parallel for AxisArray.  The parallel scale is
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
//  Method: avtViewAxisArray::SetViewport
//
//  Purpose: Sets the window's viewport 
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
// ****************************************************************************

void
avtViewAxisArray::SetViewport(double *newViewport)
{
    viewport[0] = newViewport[0];
    viewport[1] = newViewport[1];
    viewport[2] = newViewport[2];
    viewport[3] = newViewport[3];
}

// ****************************************************************************
//  Method: avtViewAxisArray::GetViewport
//
//  Purpose:
//    Gets the window's viewport 
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
// ****************************************************************************

void
avtViewAxisArray::GetViewport(double *winViewport) const
{
    winViewport[0] = viewport[0];
    winViewport[1] = viewport[1];
    winViewport[2] = viewport[2];
    winViewport[3] = viewport[3];
}

// ****************************************************************************
//  Method: avtViewAxisArray::GetScaleFactor
//
//  Purpose:
//    Gets the window's scale factor.
//
//  Arguments:
//    size      The size of the renderable area.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Modifications:
//    Eric Brugger, Mon Nov  5 15:14:46 PST 2012
//    I added the ability to display the parallel axes either horizontally
//    or vertically.
//
// ****************************************************************************

double
avtViewAxisArray::GetScaleFactor(int *size)
{
   return viewScale;
}

// ****************************************************************************
//  Method: avtViewAxisArray::SetFromViewAxisArrayAttributes
//
//  Purpose: 
//    Sets the avtAxisArrayView from the ViewAxisArrayAttributes object.
//
//  Arguments:
//    viewAtts : A pointer to the ViewAxisArrayAttributes object to use.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Modifications:
//   
// ****************************************************************************

void
avtViewAxisArray::SetFromViewAxisArrayAttributes(const ViewAxisArrayAttributes *viewAtts)
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
//  Method: avtViewAxisArray::SetToViewAxisArrayAttributes
//
//  Purpose: 
//    Sets the ViewAxisArrayAttributes from the avtViewAxisArray object.
//
//  Arguments:
//    viewAtts : A pointer to the ViewAxisArrayAttributes object to use.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Modifications:
//
// ****************************************************************************

void
avtViewAxisArray::SetToViewAxisArrayAttributes(ViewAxisArrayAttributes *viewAtts) const
{
    viewAtts->SetViewportCoords(viewport);
    viewAtts->SetDomainCoords(domain);
    viewAtts->SetRangeCoords(range);
}

// ****************************************************************************
//  Method: avtViewAxisArray::CheckAndCorrectDomainRange
//
//  Purpose:
//    Checks the window parameters and corrects them if they are invalid.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
// ****************************************************************************

void
avtViewAxisArray::CheckAndCorrectDomainRange()
{
    //
    // Account for degenerate views.
    //
    double width  = domain[1] - domain[0];
    double height = range[1]  - range[0];
    if (width <= 0. && height <= 0.)
    {
        if (domain[0] == 0. && range[0] == 0.)
        {
            domain[0] = -1.;
            domain[1] =  1.;
            range[0]  = -1.;
            range[1]  =  1.;
        }
        else if (domain[0] == 0.)
        {
            domain[0] -= range[0];
            domain[1] += range[1];
            range[0]  -= range[0];
            range[1]  += range[1];
        }
        else if (range[0] == 0.)
        {
            range[0]  -= domain[0];
            range[1]  += domain[1];
            domain[0] -= domain[0];
            domain[1] += domain[1];
        }
        else
        {
            domain[0] -= domain[0];
            domain[1] += domain[1];
            range[0]  -= range[0];
            range[1]  += range[1];
        }
    }
    else if (width <= 0.)
    {
        domain[0] -= height / 2.;
        domain[1] += height / 2.;
    }
    else if (height <= 0.)
    {
        range[0] -= width / 2.;
        range[1] += width / 2.;
    }
}

